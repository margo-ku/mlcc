#include "include/asm/ir_builder.h"

#include <memory>

LinearIRBuilder::LinearIRBuilder(const std::vector<TACInstruction>& tac_instructions)
    : tac_instructions_(tac_instructions) {}

void LinearIRBuilder::Build() {
    for (const auto& instruction : tac_instructions_) {
        LowerInstruction(instruction);
    }
    ResolveOperands();
    AddPrologueAndEpilogue();
}

void LinearIRBuilder::Print(std::ostream& out) const {
    for (const auto& instruction : asm_instructions_) {
        out << instruction->ToString() << std::endl;
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
        case Op::Global:
            return LowerControl(instr);

        case Op::EnterScope:
        case Op::ExitScope:
            // to do: remove both
            return;

        default:
            throw std::runtime_error("Unhandled TAC instruction: " + instr.ToString());
    }
}

void LinearIRBuilder::ResolveOperands() {
    std::vector<std::shared_ptr<ASMInstruction>> new_instructions;

    auto base = std::make_shared<Register>("x29");
    std::vector<std::shared_ptr<Register>> temps;
    for (auto& instr : asm_instructions_) {
        auto operands = instr->GetOperands();
        std::vector<std::shared_ptr<ASMInstruction>> before;
        std::vector<std::shared_ptr<ASMInstruction>> after;

        for (size_t index = 0; index < operands.size(); ++index) {
            auto pseudo = std::dynamic_pointer_cast<Pseudo>(operands[index]);
            if (pseudo) {
                int offset = stack_allocator_.GetOffset(pseudo->GetName());
                operands[index] = std::make_shared<MemoryOperand>(base, offset);
            }

            auto memory = std::dynamic_pointer_cast<MemoryOperand>(operands[index]);
            auto immediate = std::dynamic_pointer_cast<Immediate>(operands[index]);
            if (memory) {
                auto reg = reg_allocator_.Allocate();
                temps.push_back(reg);
                bool is_dst = (index == 0 &&
                               !IsPureInputInstruction(instr));  // convention: dst first
                if (is_dst) {
                    after.push_back(std::make_shared<StoreInstruction>(reg, memory));
                } else {
                    before.push_back(std::make_shared<LoadInstruction>(reg, memory));
                }
                operands[index] = reg;
            } else if (immediate) {
                auto reg = reg_allocator_.Allocate();
                temps.push_back(reg);
                before.push_back(std::make_shared<MovInstruction>(reg, immediate));
                operands[index] = reg;
            }
        }
        instr->SetOperands(operands);

        while (!temps.empty()) {
            reg_allocator_.Free(temps.back());
            temps.pop_back();
        }

        new_instructions.insert(new_instructions.end(), before.begin(), before.end());
        new_instructions.push_back(instr);
        new_instructions.insert(new_instructions.end(), after.begin(), after.end());
    }

    asm_instructions_ = std::move(new_instructions);
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

void LinearIRBuilder::LowerBinaryOp(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    auto rhs = MakeOperand(instr.GetRhs());

    BinaryOp op;
    if (instr.GetOp() == TACInstruction::OpCode::Add) {
        op = BinaryOp::Add;
    } else if (instr.GetOp() == TACInstruction::OpCode::Sub) {
        op = BinaryOp::Sub;
    } else if (instr.GetOp() == TACInstruction::OpCode::Mul) {
        op = BinaryOp::Mul;
    } else if (instr.GetOp() == TACInstruction::OpCode::Div) {
        op = BinaryOp::SDiv;
    } else {
        throw std::runtime_error("Unknown binary operation");
    }

    Emit(std::make_shared<BinaryInstruction>(op, dst, lhs, rhs));
}

void LinearIRBuilder::LowerMod(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    auto rhs = MakeOperand(instr.GetRhs());

    auto w1 = std::make_shared<Register>("w1");

    // to do: x1 to allocator
    Emit(std::make_shared<BinaryInstruction>(BinaryOp::SDiv, w1, lhs, rhs));
    Emit(std::make_shared<BinaryInstruction>(BinaryOp::Mul, w1, w1, rhs));
    Emit(std::make_shared<BinaryInstruction>(BinaryOp::Sub, dst, lhs, w1));
}

void LinearIRBuilder::LowerComparison(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    auto rhs = MakeOperand(instr.GetRhs());

    Emit(std::make_shared<CompareInstruction>(lhs, rhs));

    Condition cond;
    switch (instr.GetOp()) {
        case TACInstruction::OpCode::Less:
            cond = Condition::Lt;
            break;
        case TACInstruction::OpCode::LessEqual:
            cond = Condition::Le;
            break;
        case TACInstruction::OpCode::Greater:
            cond = Condition::Gt;
            break;
        case TACInstruction::OpCode::GreaterEqual:
            cond = Condition::Ge;
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
    // to do: exit labels
    auto w0 = std::make_shared<Register>("w0");
    switch (instr.GetOp()) {
        case TACInstruction::OpCode::Label:
            Emit(std::make_shared<LabelInstruction>(instr.GetLabel()));
            break;
        case TACInstruction::OpCode::Global:
            Emit(std::make_shared<GlobalDirective>(instr.GetLabel()));
            break;
        case TACInstruction::OpCode::Return:
            if (instr.GetLabel() != "") {
                auto value = MakeOperand(instr.GetLabel());
                Emit(std::make_shared<MovInstruction>(w0, value));
            }
            Emit(std::make_shared<BranchInstruction>(BranchType::Unconditional,
                                                     exit_label_));
            break;
        default:
            throw std::runtime_error("Unknown control opcode");
    }
}

std::shared_ptr<ASMOperand> LinearIRBuilder::MakeOperand(const std::string& value) {
    try {
        int imm = std::stoi(value);
        return std::make_shared<Immediate>(imm);
    } catch (...) {
    }
    return std::make_shared<Pseudo>(value);
}

void LinearIRBuilder::Emit(std::shared_ptr<ASMInstruction> instr) {
    asm_instructions_.push_back(std::move(instr));
}

void LinearIRBuilder::AddFunctionPrologue(
    std::vector<std::shared_ptr<ASMInstruction>>& instructions, int stack_size) {
    auto sp = std::make_shared<Register>("sp");
    auto x29 = std::make_shared<Register>("x29");
    auto x30 = std::make_shared<Register>("x30");

    instructions.push_back(std::make_shared<StorePairInstruction>(
        x29, x30,
        std::make_shared<MemoryOperand>(sp, -16, MemoryOperand::Mode::PreIndexed)));

    instructions.push_back(std::make_shared<MovInstruction>(x29, sp));

    instructions.push_back(std::make_shared<BinaryInstruction>(
        BinaryOp::Sub, sp, sp, std::make_shared<Immediate>(stack_size)));
}

void LinearIRBuilder::AddFunctionEpilogue(
    std::vector<std::shared_ptr<ASMInstruction>>& instructions, int stack_size) {
    auto w0 = std::make_shared<Register>("w0");
    auto zero = MakeOperand("0");
    instructions.push_back(std::make_shared<MovInstruction>(w0, zero));
    instructions.push_back(std::make_shared<LabelInstruction>(exit_label_));

    auto sp = std::make_shared<Register>("sp");
    auto x29 = std::make_shared<Register>("x29");
    auto x30 = std::make_shared<Register>("x30");

    instructions.push_back(std::make_shared<BinaryInstruction>(
        BinaryOp::Add, sp, sp, std::make_shared<Immediate>(stack_size)));

    instructions.push_back(std::make_shared<MovInstruction>(sp, x29));

    instructions.push_back(std::make_shared<LoadPairInstruction>(
        x29, x30,
        std::make_shared<MemoryOperand>(sp, 16, MemoryOperand::Mode::PostIndexed)));
    instructions.push_back(std::make_shared<RetInstruction>());
}

void LinearIRBuilder::AddPrologueAndEpilogue() {
    std::vector<std::shared_ptr<ASMInstruction>> new_instructions;
    std::vector<std::shared_ptr<ASMInstruction>> after;
    for (auto& instr : asm_instructions_) {
        after.clear();

        if (auto label = std::dynamic_pointer_cast<LabelInstruction>(instr)) {
            if (label->IsFunction()) {
                AddFunctionPrologue(after, stack_allocator_.GetAlignedFrameSize());
            }
        }
        new_instructions.push_back(instr);
        new_instructions.insert(new_instructions.end(), after.begin(), after.end());
    }

    AddFunctionEpilogue(new_instructions, stack_allocator_.GetAlignedFrameSize());
    asm_instructions_ = std::move(new_instructions);
}

bool LinearIRBuilder::IsPureInputInstruction(
    const std::shared_ptr<ASMInstruction>& instr) {
    return dynamic_cast<CompareInstruction*>(instr.get()) != nullptr ||
           dynamic_cast<BranchInstruction*>(instr.get()) != nullptr ||
           dynamic_cast<RetInstruction*>(instr.get()) != nullptr;
}