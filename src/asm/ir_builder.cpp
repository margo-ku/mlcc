#include "include/asm/ir_builder.h"

#include <cassert>
#include <iostream>

#include "include/asm/operand_resolver.h"
#include "include/tac/instruction.h"
#include "include/types/function_type.h"

IRBuilder::IRBuilder(const std::vector<std::vector<TACInstruction>>& tac_instructions,
                     SymbolTable& symbol_table)
    : tac_instructions_(tac_instructions),
      symbol_table_(symbol_table),
      int_lowerer_(emitter_, symbol_table_),
      float_lowerer_(emitter_, symbol_table_) {}

void IRBuilder::Build() {
    for (auto& instructions : tac_instructions_) {
        emitter_.Clear();

        using OpCode = TACInstruction::OpCode;
        OpCode op = instructions.front().GetOp();
        assert(op == OpCode::Function || op == OpCode::StaticVariable);

        if (op == OpCode::StaticVariable) {
            DeclareStaticVariable(instructions.front());
            asm_instructions_.push_back(emitter_.Flatten());
            continue;
        }
        stack_allocator_.PushFrame();
        PerformLowering(instructions);
        PerformResolving();
        PerformFinalizing();
        asm_instructions_.push_back(emitter_.Flatten());
        stack_allocator_.PopFrame();
    }
    EmitDoubleConstants();
}

void IRBuilder::PerformLowering(const std::vector<TACInstruction>& instructions) {
    for (const auto& instruction : instructions) {
        using Op = TACInstruction::OpCode;
        Lowerer& lowerer = SelectLowerer(instruction);
        switch (instruction.GetOp()) {
            case Op::Assign:
                lowerer.LowerAssign(instruction);
                break;

            case Op::Add:
            case Op::Sub:
            case Op::Mul:
            case Op::Div:
            case Op::BitwiseAnd:
            case Op::BitwiseXor:
            case Op::BitwiseOr:
            case Op::LeftShift:
            case Op::RightShift:
            case Op::Mod:
                lowerer.LowerBinary(instruction);
                break;
            case Op::Plus:
            case Op::Minus:
            case Op::Not:
            case Op::BinaryNot:
                lowerer.LowerUnary(instruction);
                break;

            case Op::Less:
            case Op::LessEqual:
            case Op::Greater:
            case Op::GreaterEqual:
            case Op::Equal:
            case Op::NotEqual:
                lowerer.LowerCompare(instruction);
                break;
            case Op::SignExtend:
            case Op::ZeroExtend:
            case Op::Truncate:
            case Op::DoubleToInt:
            case Op::DoubleToUInt:
            case Op::IntToDouble:
            case Op::UIntToDouble:
                lowerer.LowerCast(instruction);
                break;

            case Op::If:
            case Op::IfFalse:
            case Op::GoTo:
                LowerBranch(instruction);
                break;

            case Op::Return:
            case Op::Label:
                LowerControl(instruction);
                break;
            case Op::Function:
                LowerFunction(instruction);
                break;
            case Op::Param:
                LowerParam(instruction);
                break;
            case Op::Call:
                LowerCall(instruction);
                break;
            case Op::Load:
            case Op::Store:
            case Op::Address:
                LowerPointerOps(instruction);
                break;

            case Op::StaticVariable:
            default:
                throw std::runtime_error("Unhandled TAC instruction: " +
                                         instruction.ToString());
        }
    }
}

void IRBuilder::PerformResolving() {
    OperandResolver resolver(stack_allocator_, reg_allocator_, constant_pool_);

    std::list<std::shared_ptr<ASMInstruction>> result;
    for (auto& instr : emitter_.GetInstructions()) {
        OperandResolver::InstrList before;
        OperandResolver::InstrList after;

        resolver.ResolveInstruction(instr, before, after);

        for (auto& inst : before) {
            result.push_back(std::move(inst));
        }
        result.push_back(instr);
        for (auto& inst : after) {
            result.push_back(std::move(inst));
        }

        resolver.FreeTemps();
    }

    emitter_.SetInstructions(std::move(result));
}

void IRBuilder::PerformFinalizing() {
    AddFunctionDirectives();
    AddFunctionPrologue();
    AddFunctionEpilogue();
    UpdateStackSize();
}

void IRBuilder::AddFunctionDirectives() {
    std::string asm_name = "_" + current_function_.name;

    emitter_.EmitFront(std::make_shared<LabelInstruction>(asm_name));
    if (current_function_.is_global) {
        emitter_.EmitFront(std::make_shared<GlobalDirective>(asm_name));
    }
    emitter_.EmitFront(std::make_shared<SectionDirective>(".text"));
}

void IRBuilder::AddFunctionPrologue() {
    auto sp = std::make_shared<Register>("sp");
    auto x29 = std::make_shared<Register>("x29");
    auto x30 = std::make_shared<Register>("x30");

    current_function_.frame_size_operand = std::make_shared<Immediate>(0);

    auto& instructions = emitter_.GetInstructions();
    auto it = instructions.begin();

    std::advance(it, current_function_.is_global ? 3 : 2);

    instructions.insert(
        it, std::make_shared<StrInstruction>(
                x29, x30,
                std::make_shared<MemoryOperand>(sp, -16, ASMOperand::Size::Byte8,
                                                MemoryOperand::Mode::PreIndexed)));
    instructions.insert(it, std::make_shared<MovInstruction>(x29, sp));
    instructions.insert(
        it, std::make_shared<BinaryInstruction>(BinaryOp::Sub, sp, sp,
                                                current_function_.frame_size_operand));
}

void IRBuilder::AddFunctionEpilogue() {
    auto sp = std::make_shared<Register>("sp");
    auto x29 = std::make_shared<Register>("x29");
    auto x30 = std::make_shared<Register>("x30");
    auto ret_reg = std::make_shared<Register>("x0");
    auto zero = std::make_shared<Immediate>(0);

    emitter_.EmitBack(std::make_shared<MovInstruction>(ret_reg, zero));
    emitter_.EmitBack(std::make_shared<LabelInstruction>(GetCurrentExitLabel()));
    emitter_.EmitBack(std::make_shared<MovInstruction>(sp, x29));
    emitter_.EmitBack(std::make_shared<LdrInstruction>(
        x29, x30,
        std::make_shared<MemoryOperand>(sp, 16, ASMOperand::Size::Byte8,
                                        MemoryOperand::Mode::PostIndexed)));
    emitter_.EmitBack(std::make_shared<RetInstruction>());
}

void IRBuilder::UpdateStackSize() {
    int size = stack_allocator_.GetAlignedFrameSize();
    if (current_function_.frame_size_operand) {
        current_function_.frame_size_operand->SetValue(NumericConstant(size));
    }
}

void IRBuilder::EmitDoubleConstants() {
    const auto& constants = constant_pool_.GetDoubleConstants();
    if (constants.empty()) {
        return;
    }

    std::vector<std::shared_ptr<ASMInstruction>> literal_section;
    literal_section.push_back(
        std::make_shared<SectionDirective>(".section __TEXT,__literal8"));
    for (const auto& [name, value] : constants) {
        literal_section.push_back(std::make_shared<DoubleConstantDirective>(name, value));
    }
    asm_instructions_.push_back(std::move(literal_section));
}

void IRBuilder::DeclareStaticVariable(const TACInstruction& instr) {
    std::string name = instr.GetDst().AsIdentifier();
    NumericConstant value = instr.GetLhs().AsConstant();

    bool is_global = instr.GetRhs().AsConstant().AsInt64() == 1;
    size_t size = symbol_table_.FindByUniqueName(name)->type->Size();

    emitter_.EmitBack(std::make_shared<SectionDirective>(".data"));
    emitter_.EmitBack(
        std::make_shared<StaticVariableDirective>(name, value, size, is_global));
}

void IRBuilder::LowerFunction(const TACInstruction& instr) {
    abi_.Reset();

    current_function_.name = instr.GetDst().AsIdentifier();
    current_function_.is_global = instr.GetRhs().AsConstant().AsInt64() == 1;
    current_function_.frame_size_operand = nullptr;

    std::vector<TypeRef> param_types;
    if (auto* info = symbol_table_.FindByUniqueName(current_function_.name)) {
        if (auto func_type = std::dynamic_pointer_cast<FunctionType>(info->type)) {
            param_types = func_type->GetParamTypes();
        }
    }

    size_t param_count = static_cast<size_t>(instr.GetLhs().AsConstant().AsInt64());
    for (size_t idx = 0; idx < param_count; ++idx) {
        TypeRef param_type = (idx < param_types.size()) ? param_types[idx] : nullptr;

        std::string arg_name = "arg.." + std::to_string(idx);
        symbol_table_.Register(
            {.name = arg_name, .original_name = arg_name, .type = param_type});

        auto size = param_type ? static_cast<ASMOperand::Size>(param_type->Size())
                               : ASMOperand::Size::Byte4;
        auto pseudo = std::make_shared<Pseudo>(arg_name, size);
        auto loc = abi_.NextFormalParameter(param_type);
        bool is_float = param_type && param_type->IsFloatingPoint();

        emitter_.EmitBack(
            std::make_shared<MovInstruction>(pseudo, loc.operand, is_float));
    }
}

void IRBuilder::LowerParam(const TACInstruction& instr) {
    auto operand = instr.GetLhs();
    pending_args_.push({MakeOperand(operand), GetType(operand)});
}

void IRBuilder::LowerCall(const TACInstruction& instr) {
    abi_.Reset();

    std::vector<std::pair<std::shared_ptr<ASMOperand>, std::shared_ptr<ASMOperand>>>
        stack_args;

    while (!pending_args_.empty()) {
        auto arg = pending_args_.front();
        pending_args_.pop();

        auto loc = abi_.NextCallArgument(arg.type);
        bool is_float = arg.type && arg.type->IsFloatingPoint();
        if (loc.kind == ABIHandler::ArgLocation::Kind::Register) {
            emitter_.EmitBack(
                std::make_shared<MovInstruction>(loc.operand, arg.operand, is_float));
        } else {
            stack_args.push_back({loc.operand, arg.operand});
        }
    }

    size_t stack_args_size = abi_.GetStackArgsSize();
    auto sp = std::make_shared<Register>("sp");

    if (stack_args_size > 0) {
        emitter_.EmitBack(std::make_shared<BinaryInstruction>(
            BinaryOp::Sub, sp, sp, std::make_shared<Immediate>(stack_args_size)));
    }

    for (const auto& [dst, src] : stack_args) {
        emitter_.EmitBack(std::make_shared<StrInstruction>(src, dst));
    }

    std::string func_name = "_" + instr.GetLhs().AsIdentifier();
    emitter_.EmitBack(std::make_shared<BranchInstruction>(BranchType::Call, func_name));

    if (stack_args_size > 0) {
        emitter_.EmitBack(std::make_shared<BinaryInstruction>(
            BinaryOp::Add, sp, sp, std::make_shared<Immediate>(stack_args_size)));
    }

    if (!instr.GetDst().Empty()) {
        auto dst = MakeOperand(instr.GetDst());
        auto ret_type = GetType(instr.GetDst());
        auto ret_reg = abi_.GetReturnRegister(ret_type);
        bool is_float = ret_type && ret_type->IsFloatingPoint();
        emitter_.EmitBack(std::make_shared<MovInstruction>(dst, ret_reg, is_float));
    }
}

void IRBuilder::LowerControl(const TACInstruction& instr) {
    switch (instr.GetOp()) {
        case TACInstruction::OpCode::Label:
            emitter_.EmitBack(std::make_shared<LabelInstruction>(instr.GetLabel()));
            break;
        case TACInstruction::OpCode::Return:
            if (!instr.GetLhs().Empty()) {
                auto value = MakeOperand(instr.GetLhs());
                auto return_type = GetType(instr.GetLhs());
                auto return_reg = abi_.GetReturnRegister(return_type);
                bool is_float = return_type && return_type->IsFloatingPoint();
                emitter_.EmitBack(
                    std::make_shared<MovInstruction>(return_reg, value, is_float));
            }
            emitter_.EmitBack(std::make_shared<BranchInstruction>(
                BranchType::Unconditional, GetCurrentExitLabel()));
            break;
        default:
            throw std::runtime_error("Unknown control opcode");
    }
}

void IRBuilder::LowerBranch(const TACInstruction& instr) {
    if (instr.GetOp() == TACInstruction::OpCode::GoTo) {
        emitter_.EmitBack(std::make_shared<BranchInstruction>(BranchType::Unconditional,
                                                              instr.GetLabel()));
        return;
    }

    Condition cond =
        (instr.GetOp() == TACInstruction::OpCode::If) ? Condition::Ne : Condition::Eq;

    auto cond_operand = MakeOperand(instr.GetLhs());
    Lowerer& lowerer = SelectLowerer(instr);
    lowerer.LowerCompareZero(cond_operand);
    emitter_.EmitBack(std::make_shared<BranchInstruction>(BranchType::Conditional,
                                                          instr.GetLabel(), cond));
}

void IRBuilder::LowerPointerOps(const TACInstruction& instr) {
    using Op = TACInstruction::OpCode;
    switch (instr.GetOp()) {
        case Op::Address: {
            auto dst = MakeOperand(instr.GetDst());
            auto src = MakeOperand(instr.GetLhs());
            emitter_.EmitBack(std::make_shared<AddressInstruction>(dst, src));
            break;
        }
        case Op::Load: {
            auto dst = MakeOperand(instr.GetDst());
            auto base = MakeOperand(instr.GetLhs());
            auto size = static_cast<ASMOperand::Size>(GetType(instr.GetDst())->Size());
            auto mem = std::make_shared<IndirectMemory>(base, size);
            emitter_.EmitBack(std::make_shared<LdrInstruction>(dst, mem));
            break;
        }
        case Op::Store: {
            auto base = MakeOperand(instr.GetDst());
            auto src = MakeOperand(instr.GetLhs());
            auto size = src->GetSize();
            auto mem = std::make_shared<IndirectMemory>(base, size);
            emitter_.EmitBack(std::make_shared<StrInstruction>(src, mem));
            break;
        }
        default:
            throw std::runtime_error("Unknown address opcode");
    }
}

TypeRef IRBuilder::GetType(const TACOperand& operand) const {
    if (operand.IsConstant()) {
        auto constant = operand.AsConstant();
        return constant.GetType();
    }

    auto identifier = operand.AsIdentifier();
    return symbol_table_.FindByUniqueName(identifier)->type;
}

std::shared_ptr<ASMOperand> IRBuilder::MakeOperand(const TACOperand& value) {
    return MakeASMOperand(value, symbol_table_);
}

std::string IRBuilder::GetCurrentExitLabel() const {
    return "exit_" + std::to_string(asm_instructions_.size());
}

Lowerer& IRBuilder::SelectLowerer(const TACInstruction& instr) {
    TypeRef type;

    using Op = TACInstruction::OpCode;
    switch (instr.GetOp()) {
        case Op::DoubleToInt:
        case Op::DoubleToUInt:
        case Op::IntToDouble:
        case Op::UIntToDouble:
            return float_lowerer_;
        case Op::Less:
        case Op::LessEqual:
        case Op::Greater:
        case Op::GreaterEqual:
        case Op::Equal:
        case Op::NotEqual:
        case Op::If:
        case Op::IfFalse:
            if (!instr.GetLhs().Empty()) {
                type = GetType(instr.GetLhs());
            }
            break;
        default:
            if (!instr.GetDst().Empty()) {
                type = GetType(instr.GetDst());
            } else if (!instr.GetLhs().Empty()) {
                type = GetType(instr.GetLhs());
            }
            break;
    }

    if (type && type->IsFloatingPoint()) {
        return float_lowerer_;
    }
    return int_lowerer_;
}

void IRBuilder::GenerateASM(std::ostream& out) const {
    for (const auto& instructions : asm_instructions_) {
        for (const auto& instruction : instructions) {
            out << instruction->ToString() << std::endl;
        }
        out << std::endl;
    }
}