#include "include/asm/float_lowerer.h"

#include "include/asm/instructions.h"
#include "include/tac/instruction.h"

void FloatLowerer::LowerAssign(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto src = MakeOperand(instr.GetLhs());
    Emit(std::make_shared<MovInstruction>(dst, src, true));
}

void FloatLowerer::LowerBinary(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    auto rhs = MakeOperand(instr.GetRhs());

    BinaryOp op;
    if (instr.GetOp() == TACInstruction::OpCode::Add) {
        op = BinaryOp::FAdd;
    } else if (instr.GetOp() == TACInstruction::OpCode::Sub) {
        op = BinaryOp::FSub;
    } else if (instr.GetOp() == TACInstruction::OpCode::Mul) {
        op = BinaryOp::FMul;
    } else if (instr.GetOp() == TACInstruction::OpCode::Div) {
        op = BinaryOp::FDiv;
    } else {
        throw std::runtime_error("Unknown float binary operation");
    }

    Emit(std::make_shared<BinaryInstruction>(op, dst, lhs, rhs));
}

void FloatLowerer::LowerUnary(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());

    if (instr.GetOp() == TACInstruction::OpCode::Plus) {
        Emit(std::make_shared<MovInstruction>(dst, lhs, true));
    } else if (instr.GetOp() == TACInstruction::OpCode::Minus) {
        Emit(std::make_shared<UnaryInstruction>(UnaryOp::FNeg, dst, lhs));
    } else if (instr.GetOp() == TACInstruction::OpCode::BinaryNot) {
        LowerCompareZero(lhs);
        Emit(std::make_shared<CSetInstruction>(dst, Condition::Eq));
    } else {
        throw std::runtime_error("Unknown float unary operation");
    }
}

void FloatLowerer::LowerCompare(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    auto rhs = MakeOperand(instr.GetRhs());

    Emit(std::make_shared<CompareInstruction>(lhs, rhs, true));
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
            throw std::runtime_error("Unknown float comparison opcode");
    }
    Emit(std::make_shared<CSetInstruction>(dst, cond));
}

void FloatLowerer::LowerCast(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto src = MakeOperand(instr.GetLhs());
    bool is_signed;

    switch (instr.GetOp()) {
        case TACInstruction::OpCode::DoubleToInt:
        case TACInstruction::OpCode::DoubleToUInt:
            is_signed = IsSignedOperand(instr.GetDst());
            Emit(std::make_shared<FloatToIntInstruction>(dst, src, is_signed));
            break;
        case TACInstruction::OpCode::IntToDouble:
        case TACInstruction::OpCode::UIntToDouble:
            is_signed = IsSignedOperand(instr.GetLhs());
            Emit(std::make_shared<IntToFloatInstruction>(dst, src, is_signed));
            break;
        default:
            throw std::runtime_error("Unknown float cast opcode");
    }
}

void FloatLowerer::LowerCompareZero(std::shared_ptr<ASMOperand> operand) {
    auto zero = MakeOperand(TACOperand(NumericConstant(0.0)));
    Emit(std::make_shared<CompareInstruction>(operand, zero, true));
}
