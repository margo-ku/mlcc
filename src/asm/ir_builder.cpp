#include "include/asm/ir_builder.h"

#include <iostream>
#include <memory>

#include "include/asm/instructions.h"
#include "include/asm/operands.h"
#include "include/types/function_type.h"

LinearIRBuilder::LinearIRBuilder(
    const std::vector<std::vector<TACInstruction>>& tac_instructions,
    SymbolTable& symbol_table)
    : tac_instructions_(tac_instructions), symbol_table_(symbol_table) {}

void LinearIRBuilder::Build() {
    for (auto& instructions : tac_instructions_) {
        asm_instructions_.emplace_back();
        using Op = TACInstruction::OpCode;
        const bool is_function =
            !instructions.empty() && instructions.front().GetOp() == Op::Function;

        if (is_function) {
            stack_allocator_.PushFrame();
        }

        for (const auto& instruction : instructions) {
            LowerInstruction(instruction);
        }
        if (is_function) {
            AddFunctionEpilogue();
            ResolveOperands();
            ChangeStackSize();
            optimizer_.Optimize(asm_instructions_.back());
            stack_allocator_.PopFrame();
        }
    }
}

void LinearIRBuilder::Print(std::ostream& out) const {
    for (const auto& instructions : asm_instructions_) {
        for (const auto& instruction : instructions) {
            out << instruction->ToString() << std::endl;
        }
        out << std::endl;
    }
}

void LinearIRBuilder::LowerInstruction(const TACInstruction& instr) {
    using Op = TACInstruction::OpCode;

    switch (instr.GetOp()) {
        case Op::Assign:
            return LowerAssign(instr);

        case Op::Add:
        case Op::Sub:
        case Op::Mul:
        case Op::Div:
        case Op::BitwiseAnd:
        case Op::BitwiseXor:
        case Op::BitwiseOr:
        case Op::LeftShift:
        case Op::RightShift:
            return LowerBinaryOp(instr);
        case Op::Plus:
        case Op::Minus:
        case Op::Not:
        case Op::BinaryNot:
            return LowerUnaryOp(instr);
        case Op::Mod:
            return LowerMod(instr);

        case Op::Less:
        case Op::LessEqual:
        case Op::Greater:
        case Op::GreaterEqual:
        case Op::Equal:
        case Op::NotEqual:
            return LowerComparison(instr);

        case Op::If:
        case Op::IfFalse:
        case Op::GoTo:
            return LowerBranch(instr);

        case Op::Return:
        case Op::Label:
            return LowerControl(instr);
        case Op::Function:
            return LowerFunction(instr);
        case Op::Param:
            return LowerParam(instr);
        case Op::Call:
            return LowerCall(instr);

        case Op::SignExtend:
            return LowerExtend(instr, true);
        case Op::ZeroExtend:
            return LowerExtend(instr, false);
        case Op::Truncate:
            return LowerTruncate(instr);

        case Op::StaticVariable:
            return LowerStaticVariable(instr);

        default:
            throw std::runtime_error("Unhandled TAC instruction: " + instr.ToString());
    }
}

void LinearIRBuilder::ResolveOperands() {
    std::vector<std::shared_ptr<ASMInstruction>> new_instructions;

    auto base = std::make_shared<Register>("x29");
    std::vector<std::shared_ptr<Register>> temps;
    for (auto& instr : asm_instructions_.back()) {
        auto operands = instr->GetOperands();
        std::vector<std::shared_ptr<ASMInstruction>> before;
        std::vector<std::shared_ptr<ASMInstruction>> after;

        bool isLoad = dynamic_cast<LoadInstruction*>(instr.get()) != nullptr;
        bool isStore = dynamic_cast<StoreInstruction*>(instr.get()) != nullptr;

        for (size_t index = 0; index < operands.size(); ++index) {
            if (auto pseudo = std::dynamic_pointer_cast<Pseudo>(operands[index])) {
                auto size = pseudo->GetSize();
                int offset = stack_allocator_.GetLocalOffset(pseudo->GetName(),
                                                             static_cast<int>(size));
                operands[index] = std::make_shared<MemoryOperand>(base, offset, size);
            } else if (auto data_op =
                           std::dynamic_pointer_cast<DataOperand>(operands[index])) {
                auto size = data_op->GetSize();
                std::string symbol = "_" + data_op->GetName();

                auto addr_reg = reg_allocator_.Allocate(ASMOperand::Size::Byte8);
                temps.push_back(addr_reg);

                before.push_back(std::make_shared<AdrpInstruction>(addr_reg, symbol));

                bool is_dst = (index == 0 && !IsPureInputInstruction(instr));

                auto value_reg = reg_allocator_.Allocate(size);
                temps.push_back(value_reg);

                if (is_dst) {
                    after.push_back(std::make_shared<StoreGlobalInstruction>(
                        value_reg, addr_reg, symbol));
                } else {
                    before.push_back(std::make_shared<LoadGlobalInstruction>(
                        value_reg, addr_reg, symbol));
                }
                operands[index] = value_reg;
            }
        }

        ASMOperand::Size target_size = ASMOperand::Size::Byte4;
        for (const auto& op : operands) {
            if (!std::dynamic_pointer_cast<Immediate>(op)) {
                target_size = op->GetSize();
                break;
            }
        }

        for (size_t index = 0; index < operands.size(); ++index) {
            auto memory = std::dynamic_pointer_cast<MemoryOperand>(operands[index]);
            auto immediate = std::dynamic_pointer_cast<Immediate>(operands[index]);
            if (memory) {
                if (isLoad && index == 1) {
                    continue;
                }

                if (isStore && index == 1) {
                    continue;
                }

                auto size = memory->GetSize();
                auto reg = reg_allocator_.Allocate(size);
                temps.push_back(reg);

                if (isStore) {
                    before.push_back(std::make_shared<LoadInstruction>(reg, memory));
                    operands[index] = reg;
                    continue;
                } else if (isLoad) {
                    after.push_back(std::make_shared<StoreInstruction>(reg, memory));
                    operands[index] = reg;
                    continue;
                }

                bool is_dst = (index == 0 &&
                               !IsPureInputInstruction(instr));  // convention: dst first
                if (is_dst) {
                    after.push_back(std::make_shared<StoreInstruction>(reg, memory));
                } else {
                    before.push_back(std::make_shared<LoadInstruction>(reg, memory));
                }
                operands[index] = reg;
            } else if (immediate) {
                auto value = immediate->GetValue();
                auto size = target_size;
                auto reg = reg_allocator_.Allocate(size);
                temps.push_back(reg);

                auto load_seq =
                    MakeLoadImmediateInstrs(reg, static_cast<uint64_t>(value));
                before.insert(before.end(), load_seq.begin(), load_seq.end());
                operands[index] = reg;
            }
        }
        instr->SetOperands(operands);

        new_instructions.insert(new_instructions.end(), before.begin(), before.end());
        new_instructions.push_back(instr);
        new_instructions.insert(new_instructions.end(), after.begin(), after.end());

        while (!temps.empty()) {
            reg_allocator_.Free(temps.back());
            temps.pop_back();
        }
    }

    asm_instructions_.back() = std::move(new_instructions);
}

void LinearIRBuilder::LowerAssign(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    Emit(std::make_shared<MovInstruction>(dst, lhs));
}

void LinearIRBuilder::LowerUnaryOp(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());

    UnaryOp op;
    if (instr.GetOp() == TACInstruction::OpCode::Plus) {
        Emit(std::make_shared<MovInstruction>(dst, lhs));
    } else if (instr.GetOp() == TACInstruction::OpCode::Minus) {
        Emit(std::make_shared<UnaryInstruction>(UnaryOp::Neg, dst, lhs));
    } else if (instr.GetOp() == TACInstruction::OpCode::BinaryNot) {
        Emit(std::make_shared<UnaryInstruction>(UnaryOp::Mvn, dst, lhs));
    } else if (instr.GetOp() == TACInstruction::OpCode::Not) {
        auto zero = MakeOperand("0");
        Emit(std::make_shared<CompareInstruction>(lhs, zero));
        Emit(std::make_shared<CSetInstruction>(dst, Condition::Eq));
    } else {
        throw std::runtime_error("Unknown binary operation");
    }
}

bool LinearIRBuilder::IsSignedOperand(const std::string& name) const {
    if (!name.empty() && (name[0] == '-' || (name[0] >= '0' && name[0] <= '9'))) {
        return true;
    }
    return symbol_table_.FindByUniqueName(name)->type->IsSigned();
}

void LinearIRBuilder::LowerBinaryOp(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    auto rhs = MakeOperand(instr.GetRhs());

    bool is_signed = IsSignedOperand(instr.GetDst());

    BinaryOp op;
    if (instr.GetOp() == TACInstruction::OpCode::Add) {
        op = BinaryOp::Add;
    } else if (instr.GetOp() == TACInstruction::OpCode::Sub) {
        op = BinaryOp::Sub;
    } else if (instr.GetOp() == TACInstruction::OpCode::Mul) {
        op = BinaryOp::Mul;
    } else if (instr.GetOp() == TACInstruction::OpCode::Div) {
        op = is_signed ? BinaryOp::SDiv : BinaryOp::UDiv;
    } else if (instr.GetOp() == TACInstruction::OpCode::BitwiseAnd) {
        op = BinaryOp::And;
    } else if (instr.GetOp() == TACInstruction::OpCode::BitwiseOr) {
        op = BinaryOp::Orr;
    } else if (instr.GetOp() == TACInstruction::OpCode::BitwiseXor) {
        op = BinaryOp::Eor;
    } else if (instr.GetOp() == TACInstruction::OpCode::LeftShift) {
        op = BinaryOp::Lsl;
    } else if (instr.GetOp() == TACInstruction::OpCode::RightShift) {
        op = is_signed ? BinaryOp::Asr : BinaryOp::Lsr;
    } else {
        throw std::runtime_error("Unknown binary operation");
    }

    Emit(std::make_shared<BinaryInstruction>(op, dst, lhs, rhs));
}

void LinearIRBuilder::LowerMod(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    auto rhs = MakeOperand(instr.GetRhs());

    bool is_signed = IsSignedOperand(instr.GetDst());
    auto div_op = is_signed ? BinaryOp::SDiv : BinaryOp::UDiv;

    auto size = dst->GetSize();
    std::string reg_prefix = (size == ASMOperand::Size::Byte8) ? "x" : "w";
    auto temp = std::make_shared<Register>(reg_prefix + "1");

    Emit(std::make_shared<BinaryInstruction>(div_op, temp, lhs, rhs));
    Emit(std::make_shared<BinaryInstruction>(BinaryOp::Mul, temp, temp, rhs));
    Emit(std::make_shared<BinaryInstruction>(BinaryOp::Sub, dst, lhs, temp));
}

void LinearIRBuilder::LowerComparison(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    auto rhs = MakeOperand(instr.GetRhs());

    Emit(std::make_shared<CompareInstruction>(lhs, rhs));

    bool is_signed = IsSignedOperand(instr.GetLhs());

    Condition cond;
    switch (instr.GetOp()) {
        case TACInstruction::OpCode::Less:
            cond = is_signed ? Condition::Lt : Condition::Lo;
            break;
        case TACInstruction::OpCode::LessEqual:
            cond = is_signed ? Condition::Le : Condition::Ls;
            break;
        case TACInstruction::OpCode::Greater:
            cond = is_signed ? Condition::Gt : Condition::Hi;
            break;
        case TACInstruction::OpCode::GreaterEqual:
            cond = is_signed ? Condition::Ge : Condition::Hs;
            break;
        case TACInstruction::OpCode::Equal:
            cond = Condition::Eq;
            break;
        case TACInstruction::OpCode::NotEqual:
            cond = Condition::Ne;
            break;
        default:
            throw std::runtime_error("Unknown comparison opcode");
    }

    Emit(std::make_shared<CSetInstruction>(dst, cond));
}

void LinearIRBuilder::LowerBranch(const TACInstruction& instr) {
    Condition cond;
    BranchType type;

    if (instr.GetOp() == TACInstruction::OpCode::If) {
        type = BranchType::Conditional;
        cond = Condition::Ne;
    } else if (instr.GetOp() == TACInstruction::OpCode::IfFalse) {
        type = BranchType::Conditional;
        cond = Condition::Eq;
    } else if (instr.GetOp() == TACInstruction::OpCode::GoTo) {
        type = BranchType::Unconditional;
    } else {
        throw std::runtime_error("Unknown branch opcode");
    }

    if (instr.GetOp() != TACInstruction::OpCode::GoTo) {
        auto cond_operand = MakeOperand(instr.GetLhs());
        auto zero = MakeOperand("0");
        Emit(std::make_shared<CompareInstruction>(cond_operand, zero));
    }

    Emit(std::make_shared<BranchInstruction>(type, instr.GetLabel(), cond));
}

void LinearIRBuilder::LowerControl(const TACInstruction& instr) {
    switch (instr.GetOp()) {
        case TACInstruction::OpCode::Label:
            Emit(std::make_shared<LabelInstruction>(instr.GetLabel()));
            break;
        case TACInstruction::OpCode::Return:
            if (instr.GetLabel() != "") {
                auto value = MakeOperand(instr.GetLabel());
                auto ret_reg = GetReturnRegister();
                Emit(std::make_shared<MovInstruction>(ret_reg, value));
            }
            Emit(std::make_shared<BranchInstruction>(BranchType::Unconditional,
                                                     GetCurrentExitLabel()));
            break;
        default:
            throw std::runtime_error("Unknown control opcode");
    }
}

void LinearIRBuilder::LowerFunction(const TACInstruction& instr) {
    current_function_name_ = instr.GetDst();
    std::string asm_name = "_" + current_function_name_;
    Emit(std::make_shared<TextSectionDirective>());
    bool is_global = instr.GetRhs() == "1";
    if (is_global) {
        Emit(std::make_shared<GlobalDirective>(asm_name));
    }
    Emit(std::make_shared<LabelInstruction>(asm_name));
    AddFunctionPrologue();

    current_param_count_ = std::stoi(instr.GetLhs());
    MaterializeFormalParameters();
}

void LinearIRBuilder::LowerParam(const TACInstruction& instr) {
    auto src = MakeOperand(instr.GetLabel());
    pending_args_.push(src);
}

void LinearIRBuilder::LowerCall(const TACInstruction& instr) {
    SaveCallerRegisters();
    int index = 0;
    while (!pending_args_.empty() && index < 8) {
        auto arg = pending_args_.front();
        pending_args_.pop();

        auto size = arg->GetSize();
        std::string reg_prefix = (size == ASMOperand::Size::Byte8) ? "x" : "w";
        auto dst = std::make_shared<Register>(reg_prefix + std::to_string(index));
        Emit(std::make_shared<MovInstruction>(dst, arg));
        ++index;
    }
    int stack_args_size = stack_allocator_.ReserveStackArguments(pending_args_.size());

    auto sp = std::make_shared<Register>("sp");
    Emit(std::make_shared<AllocateStackInstruction>(
        std::make_shared<Immediate>(stack_args_size), true));
    int pending_args_size = pending_args_.size();
    for (size_t idx = 0; idx < pending_args_size; ++idx) {
        auto arg = pending_args_.front();
        auto size = arg->GetSize();
        int offset =
            stack_allocator_.GetArgumentOffsetForCaller(idx, static_cast<int>(size));
        auto mem = std::make_shared<MemoryOperand>(sp, offset, size);
        Emit(std::make_shared<StoreInstruction>(arg, mem));
        pending_args_.pop();
    }
    std::string call_name = "_" + instr.GetLhs();
    Emit(std::make_shared<BranchInstruction>(BranchType::Call, call_name));
    Emit(std::make_shared<DeallocateStackInstruction>(
        std::make_shared<Immediate>(stack_args_size), true));
    if (!instr.GetDst().empty()) {
        auto dst = MakeOperand(instr.GetDst());
        auto size = dst->GetSize();
        std::string reg_prefix = (size == ASMOperand::Size::Byte8) ? "x" : "w";
        auto ret_reg = std::make_shared<Register>(reg_prefix + "0");
        Emit(std::make_shared<MovInstruction>(dst, ret_reg));
    }
    LoadCallerRegisters();
}

void LinearIRBuilder::MaterializeFormalParameters() {
    std::vector<TypeRef> param_types;
    std::string func_name = current_function_name_;
    if (auto* info = symbol_table_.FindByUniqueName(func_name)) {
        if (auto func_type = std::dynamic_pointer_cast<FunctionType>(info->type)) {
            param_types = func_type->GetParamTypes();
        }
    }

    auto x29 = std::make_shared<Register>("x29");
    for (int index = 0; index < current_param_count_; ++index) {
        TypeRef param_type = nullptr;
        if (index < static_cast<int>(param_types.size())) {
            param_type = param_types[index];
        }

        auto size = ASMOperand::Size::Byte4;
        if (param_type) {
            size = static_cast<ASMOperand::Size>(param_type->Size());
        }

        std::string arg_name = "arg.." + std::to_string(index);
        symbol_table_.Register(
            {.name = arg_name, .original_name = arg_name, .type = param_type});

        auto dstPseudo = std::make_shared<Pseudo>(arg_name, size);
        if (index < 8) {
            std::string reg_prefix = (size == ASMOperand::Size::Byte8) ? "x" : "w";
            auto reg = std::make_shared<Register>(reg_prefix + std::to_string(index));
            Emit(std::make_shared<MovInstruction>(dstPseudo, reg));
        } else {
            int incoming_offset = stack_allocator_.GetArgumentOffset(
                "arg.." + std::to_string(index), static_cast<int>(size));
            auto mem = std::make_shared<MemoryOperand>(x29, incoming_offset, size);
            Emit(std::make_shared<LoadInstruction>(dstPseudo, mem));
        }
    }
}

std::shared_ptr<ASMOperand> LinearIRBuilder::MakeOperand(const std::string& value) {
    try {
        // to do: long long vs unsigned
        long long imm = std::stoull(value);
        return std::make_shared<Immediate>(imm);
    } catch (...) {
    }

    if (auto* info = symbol_table_.FindByUniqueName(value)) {
        if (info->type) {
            auto size = static_cast<ASMOperand::Size>(info->type->Size());
            if (info->HasStaticDuration()) {
                return std::make_shared<DataOperand>(value, size);
            }
            return std::make_shared<Pseudo>(value, size);
        }
    }
    throw std::runtime_error("Unknown operand: " + value);
}

void LinearIRBuilder::Emit(std::shared_ptr<ASMInstruction> instr) {
    asm_instructions_.back().push_back(std::move(instr));
}

void LinearIRBuilder::AddFunctionPrologue() {
    int temp_stack_size = 0;
    auto sp = std::make_shared<Register>("sp");
    auto x29 = std::make_shared<Register>("x29");
    auto x30 = std::make_shared<Register>("x30");

    asm_instructions_.back().push_back(std::make_shared<StorePairInstruction>(
        x29, x30,
        std::make_shared<MemoryOperand>(sp, -16, ASMOperand::Size::Byte8,
                                        MemoryOperand::Mode::PreIndexed)));

    asm_instructions_.back().push_back(std::make_shared<MovInstruction>(x29, sp));

    asm_instructions_.back().push_back(std::make_shared<AllocateStackInstruction>(
        std::make_shared<Immediate>(temp_stack_size)));
}

void LinearIRBuilder::AddFunctionEpilogue() {
    auto ret_reg = GetReturnRegister();
    auto zero = std::make_shared<Immediate>(0);
    asm_instructions_.back().push_back(std::make_shared<MovInstruction>(ret_reg, zero));
    asm_instructions_.back().push_back(
        std::make_shared<LabelInstruction>(GetCurrentExitLabel()));

    auto sp = std::make_shared<Register>("sp");
    auto x29 = std::make_shared<Register>("x29");
    auto x30 = std::make_shared<Register>("x30");

    asm_instructions_.back().push_back(std::make_shared<MovInstruction>(sp, x29));

    asm_instructions_.back().push_back(std::make_shared<LoadPairInstruction>(
        x29, x30,
        std::make_shared<MemoryOperand>(sp, 16, ASMOperand::Size::Byte8,
                                        MemoryOperand::Mode::PostIndexed)));
    asm_instructions_.back().push_back(std::make_shared<RetInstruction>());
}

void LinearIRBuilder::ChangeStackSize() {
    int stack_size = stack_allocator_.GetAlignedFrameSize();
    for (auto& instr : asm_instructions_.back()) {
        auto allocate = std::dynamic_pointer_cast<AllocateStackInstruction>(instr);
        if (allocate) {
            allocate->ChangeSize(std::make_shared<Immediate>(stack_size));
        }
        auto deallocate = std::dynamic_pointer_cast<DeallocateStackInstruction>(instr);
        if (deallocate) {
            deallocate->ChangeSize(std::make_shared<Immediate>(stack_size));
        }
    }
}

bool LinearIRBuilder::IsPureInputInstruction(
    const std::shared_ptr<ASMInstruction>& instr) {
    return dynamic_cast<CompareInstruction*>(instr.get()) != nullptr ||
           dynamic_cast<BranchInstruction*>(instr.get()) != nullptr ||
           dynamic_cast<RetInstruction*>(instr.get()) != nullptr;
}

std::string LinearIRBuilder::GetCurrentExitLabel() const {
    return "exit_" + std::to_string(asm_instructions_.size());
}

std::shared_ptr<Register> LinearIRBuilder::GetReturnRegister() const {
    std::string func_name = current_function_name_;
    if (auto* info = symbol_table_.FindByUniqueName(func_name)) {
        if (auto func_type = std::dynamic_pointer_cast<FunctionType>(info->type)) {
            auto ret_type = func_type->GetReturnType();
            if (ret_type && ret_type->Size() == 8) {
                return std::make_shared<Register>("x0");
            }
        }
    }
    return std::make_shared<Register>("w0");
}

void LinearIRBuilder::SaveCallerRegisters() const {}

void LinearIRBuilder::LoadCallerRegisters() const {}

std::vector<std::shared_ptr<ASMInstruction>> LinearIRBuilder::MakeLoadImmediateInstrs(
    std::shared_ptr<ASMOperand> dst, uint64_t value) {
    std::vector<std::shared_ptr<ASMInstruction>> out;

    bool is_32bit = false;
    if (auto reg = std::dynamic_pointer_cast<Register>(dst)) {
        is_32bit = reg->ToString().starts_with("w");
    }

    uint16_t parts[4];
    for (int index = 0; index < 4; ++index) {
        parts[index] = static_cast<uint16_t>((value >> (index * 16)) & 0xFFFFu);
    }
    int first_nonzero = -1;
    for (int index = 0; index < 4; ++index) {
        if (parts[index] != 0) {
            first_nonzero = index;
            break;
        }
    }

    if (first_nonzero == -1) {
        out.push_back(std::make_shared<MovzInstruction>(dst, 0, 0));
        return out;
    }

    int max_shift = is_32bit ? 1 : 3;
    out.push_back(
        std::make_shared<MovzInstruction>(dst, parts[first_nonzero], first_nonzero * 16));
    for (int index = 0; index <= max_shift; ++index) {
        if (index == first_nonzero) {
            continue;
        }
        if (parts[index] != 0) {
            out.push_back(
                std::make_shared<MovkInstruction>(dst, parts[index], index * 16));
        }
    }

    return out;
}

void LinearIRBuilder::LowerExtend(const TACInstruction& instr, bool is_signed) {
    auto dst = MakeOperand(instr.GetDst());
    auto src = MakeOperand(instr.GetLhs());
    Emit(std::make_shared<ExtendInstruction>(dst, src, is_signed));
}

void LinearIRBuilder::LowerTruncate(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto src = MakeOperand(instr.GetLhs());
    Emit(std::make_shared<TruncateInstruction>(dst, src));
}

void LinearIRBuilder::LowerStaticVariable(const TACInstruction& instr) {
    std::string name = instr.GetDst();
    long long value = 0;
    try {
        value = std::stoll(instr.GetLhs());
    } catch (...) {
    }
    bool is_global = instr.GetRhs() == "1";

    int size = 4;
    if (auto* info = symbol_table_.FindByUniqueName(name)) {
        if (info->type) {
            size = info->type->Size();
        }
    }

    Emit(std::make_shared<DataSectionDirective>());
    Emit(std::make_shared<StaticVariableDirective>(name, value, size, is_global));
}