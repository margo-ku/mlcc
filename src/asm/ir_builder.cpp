#include "include/asm/ir_builder.h"

#include <iostream>
#include <memory>

#include "include/asm/operands.h"

LinearIRBuilder::LinearIRBuilder(
    const std::vector<std::vector<TACInstruction>>& tac_instructions)
    : tac_instructions_(tac_instructions) {}

void LinearIRBuilder::Build() {
    for (auto& instructions : tac_instructions_) {
        asm_instructions_.emplace_back();
        stack_allocator_.PushFrame();

        for (const auto& instruction : instructions) {
            LowerInstruction(instruction);
        }
        AddFunctionEpilogue();
        ResolveOperands();
        ChangeStackSize();
        RunPeepholeOptimization();
        stack_allocator_.PopFrame();
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
                int offset = stack_allocator_.GetOffset(pseudo->GetName(), 8);
                operands[index] = std::make_shared<MemoryOperand>(base, offset);
            }

            auto memory = std::dynamic_pointer_cast<MemoryOperand>(operands[index]);
            auto immediate = std::dynamic_pointer_cast<Immediate>(operands[index]);
            if (memory) {
                if (isLoad && index == 1) {
                    continue;
                }

                if (isStore && index == 1) {
                    continue;
                }

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
                int value = immediate->GetValue();
                auto reg = reg_allocator_.Allocate();
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

// mov x, x
void LinearIRBuilder::RunPeepholeOptimization() {
    std::vector<std::shared_ptr<ASMInstruction>> new_instructions;
    for (auto& instr : asm_instructions_.back()) {
        auto mov = std::dynamic_pointer_cast<MovInstruction>(instr);
        if (mov) {
            auto operands = mov->GetOperands();
            if (operands[0]->ToString() == operands[1]->ToString()) {
                continue;
            }
        }
        new_instructions.push_back(instr);
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
    } else if (instr.GetOp() == TACInstruction::OpCode::BitwiseAnd) {
        op = BinaryOp::And;
    } else if (instr.GetOp() == TACInstruction::OpCode::BitwiseOr) {
        op = BinaryOp::Orr;
    } else if (instr.GetOp() == TACInstruction::OpCode::BitwiseXor) {
        op = BinaryOp::Eor;
    } else if (instr.GetOp() == TACInstruction::OpCode::LeftShift) {
        op = BinaryOp::Lsl;
    } else if (instr.GetOp() == TACInstruction::OpCode::RightShift) {
        op = BinaryOp::Asr;
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
    auto w0 = std::make_shared<Register>("w0");
    switch (instr.GetOp()) {
        case TACInstruction::OpCode::Label:
            Emit(std::make_shared<LabelInstruction>(instr.GetLabel()));
            break;
        case TACInstruction::OpCode::Return:
            if (instr.GetLabel() != "") {
                auto value = MakeOperand(instr.GetLabel());
                Emit(std::make_shared<MovInstruction>(w0, value));
            }
            Emit(std::make_shared<BranchInstruction>(BranchType::Unconditional,
                                                     GetCurrentExitLabel()));
            break;
        default:
            throw std::runtime_error("Unknown control opcode");
    }
}

void LinearIRBuilder::LowerFunction(const TACInstruction& instr) {
    auto function_name = instr.GetLabel();
    Emit(std::make_shared<GlobalDirective>(function_name));
    Emit(std::make_shared<LabelInstruction>(function_name));
    AddFunctionPrologue();

    current_param_count_ = std::stoi(instr.GetLhs());
    MaterializeFormalParameters();
}

void LinearIRBuilder::LowerParam(const TACInstruction& instr) {
    auto src = MakeOperand(instr.GetLabel());
    if (param_index_ < 8) {
        auto dst = std::make_shared<Register>("w" + std::to_string(param_index_++));
        Emit(std::make_shared<MovInstruction>(dst, src));
    } else {
        auto x29 = std::make_shared<Register>("x29");
        int offset =
            stack_allocator_.GetOffset("arg.." + std::to_string(param_index_++), 8);
        Emit(std::make_shared<StoreInstruction>(
            src, std::make_shared<MemoryOperand>(x29, offset)));
    }
}

void LinearIRBuilder::LowerCall(const TACInstruction& instr) {
    param_index_ = 0;
    SaveCallerRegisters();
    Emit(std::make_shared<BranchInstruction>(BranchType::Call, instr.GetLhs()));
    if (!instr.GetDst().empty()) {
        auto dst = MakeOperand(instr.GetDst());
        auto w0 = std::make_shared<Register>("w0");
        Emit(std::make_shared<MovInstruction>(dst, w0));
    }
    LoadCallerRegisters();
}

void LinearIRBuilder::MaterializeFormalParameters() {
    auto x29 = std::make_shared<Register>("x29");
    for (int index = 0; index < current_param_count_; ++index) {
        auto dstPseudo = std::make_shared<Pseudo>("arg.." + std::to_string(index));
        if (index < 8) {
            auto wi = std::make_shared<Register>("w" + std::to_string(index));
            Emit(std::make_shared<MovInstruction>(dstPseudo, wi));
        } else {
            int stack_arg_index = index - 8;
            int incoming_offset = 16 + 8 * stack_arg_index;
            auto mem = std::make_shared<MemoryOperand>(x29, incoming_offset);
            auto tmp = reg_allocator_.Allocate();
            Emit(std::make_shared<LoadInstruction>(tmp, mem));
            Emit(std::make_shared<MovInstruction>(dstPseudo, tmp));
            reg_allocator_.Free(tmp);
        }
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
    asm_instructions_.back().push_back(std::move(instr));
}

void LinearIRBuilder::AddFunctionPrologue() {
    int temp_stack_size = 0;
    auto sp = std::make_shared<Register>("sp");
    auto x29 = std::make_shared<Register>("x29");
    auto x30 = std::make_shared<Register>("x30");

    asm_instructions_.back().push_back(std::make_shared<StorePairInstruction>(
        x29, x30,
        std::make_shared<MemoryOperand>(sp, -16, MemoryOperand::Mode::PreIndexed)));

    asm_instructions_.back().push_back(std::make_shared<MovInstruction>(x29, sp));

    asm_instructions_.back().push_back(std::make_shared<AllocateStackInstruction>(
        std::make_shared<Immediate>(temp_stack_size)));

    stack_allocator_.PushFrame();
}

void LinearIRBuilder::AddFunctionEpilogue() {
    int temp_stack_size = 0;
    auto w0 = std::make_shared<Register>("w0");
    auto zero = MakeOperand("0");
    asm_instructions_.back().push_back(std::make_shared<MovInstruction>(w0, zero));
    asm_instructions_.back().push_back(
        std::make_shared<LabelInstruction>(GetCurrentExitLabel()));

    auto sp = std::make_shared<Register>("sp");
    auto x29 = std::make_shared<Register>("x29");
    auto x30 = std::make_shared<Register>("x30");

    asm_instructions_.back().push_back(std::make_shared<DeallocateStackInstruction>(
        std::make_shared<Immediate>(temp_stack_size)));

    asm_instructions_.back().push_back(std::make_shared<MovInstruction>(sp, x29));

    asm_instructions_.back().push_back(std::make_shared<LoadPairInstruction>(
        x29, x30,
        std::make_shared<MemoryOperand>(sp, 16, MemoryOperand::Mode::PostIndexed)));
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

void LinearIRBuilder::SaveCallerRegisters() const {}

void LinearIRBuilder::LoadCallerRegisters() const {}

std::vector<std::shared_ptr<ASMInstruction>> LinearIRBuilder::MakeLoadImmediateInstrs(
    std::shared_ptr<ASMOperand> dst, uint64_t value) {
    std::vector<std::shared_ptr<ASMInstruction>> out;

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

    out.push_back(
        std::make_shared<MovzInstruction>(dst, parts[first_nonzero], first_nonzero * 16));
    for (int index = 0; index < 4; ++index) {
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