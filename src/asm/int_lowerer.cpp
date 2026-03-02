#include "include/asm/int_lowerer.h"

#include "include/asm/instructions.h"

void IntLowerer::LowerAssign(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());
    Emit(std::make_shared<MovInstruction>(dst, lhs));
}

void IntLowerer::LowerBinary(const TACInstruction& instr) {
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
    } else if (instr.GetOp() == TACInstruction::OpCode::Mod) {
        auto div_op = is_signed ? BinaryOp::SDiv : BinaryOp::UDiv;

        auto size = dst->GetSize();
        std::string reg_prefix = (size == ASMOperand::Size::Byte8) ? "x" : "w";
        auto temp = std::make_shared<Register>(reg_prefix + "1");

        Emit(std::make_shared<BinaryInstruction>(div_op, temp, lhs, rhs));
        Emit(std::make_shared<BinaryInstruction>(BinaryOp::Mul, temp, temp, rhs));
        Emit(std::make_shared<BinaryInstruction>(BinaryOp::Sub, dst, lhs, temp));
        return;
    } else {
        throw std::runtime_error("Unknown binary operation");
    }

    Emit(std::make_shared<BinaryInstruction>(op, dst, lhs, rhs));
}

void IntLowerer::LowerUnary(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto lhs = MakeOperand(instr.GetLhs());

    if (instr.GetOp() == TACInstruction::OpCode::Plus) {
        Emit(std::make_shared<MovInstruction>(dst, lhs));
    } else if (instr.GetOp() == TACInstruction::OpCode::Minus) {
        Emit(std::make_shared<UnaryInstruction>(UnaryOp::Neg, dst, lhs));
    } else if (instr.GetOp() == TACInstruction::OpCode::BinaryNot) {
        Emit(std::make_shared<UnaryInstruction>(UnaryOp::Mvn, dst, lhs));
    } else if (instr.GetOp() == TACInstruction::OpCode::Not) {
        LowerCompareZero(lhs);
        Emit(std::make_shared<CSetInstruction>(dst, Condition::Eq));
    } else {
        throw std::runtime_error("Unknown binary operation");
    }
}

void IntLowerer::LowerCompare(const TACInstruction& instr) {
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

void IntLowerer::LowerCast(const TACInstruction& instr) {
    auto dst = MakeOperand(instr.GetDst());
    auto src = MakeOperand(instr.GetLhs());

    switch (instr.GetOp()) {
        case TACInstruction::OpCode::SignExtend:
            Emit(std::make_shared<ExtendInstruction>(dst, src, true));
            break;
        case TACInstruction::OpCode::ZeroExtend:
            Emit(std::make_shared<ExtendInstruction>(dst, src, false));
            break;
        case TACInstruction::OpCode::Truncate:
            Emit(std::make_shared<TruncateInstruction>(dst, src));
            break;
        default:
            throw std::runtime_error("Unknown cast opcode");
    }
}

void IntLowerer::LowerCompareZero(std::shared_ptr<ASMOperand> operand) {
    auto zero = MakeOperand(TACOperand(NumericConstant(0)));
    Emit(std::make_shared<CompareInstruction>(operand, zero));
}
