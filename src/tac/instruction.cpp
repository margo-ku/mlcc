#include "include/tac/instruction.h"

#include <sstream>
#include <stdexcept>
TACOperand::TACOperand(const std::string& identifier) : storage_(identifier) {}

TACOperand::TACOperand(const NumericConstant& constant) : storage_(constant) {}

bool TACOperand::IsIdentifier() const {
    return std::holds_alternative<std::string>(storage_);
}

bool TACOperand::IsConstant() const {
    return std::holds_alternative<NumericConstant>(storage_);
}

bool TACOperand::Empty() const {
    if (!IsIdentifier()) {
        return false;
    }
    return std::get<std::string>(storage_).empty();
}

const std::string& TACOperand::AsIdentifier() const {
    if (!IsIdentifier()) {
        throw std::runtime_error("TACOperand does not hold an identifier");
    }
    return std::get<std::string>(storage_);
}

const NumericConstant& TACOperand::AsConstant() const {
    if (!IsConstant()) {
        throw std::runtime_error("TACOperand does not hold a numeric constant");
    }
    return std::get<NumericConstant>(storage_);
}

std::string TACOperand::ToString() const {
    if (IsIdentifier()) {
        return AsIdentifier();
    }
    return AsConstant().ToString();
}

bool TACOperand::operator==(const TACOperand& other) const {
    if (IsIdentifier() != other.IsIdentifier()) {
        return false;
    }
    if (IsIdentifier()) {
        return AsIdentifier() == other.AsIdentifier();
    }
    return AsConstant().GetKind() == other.AsConstant().GetKind() &&
           AsConstant().ToString() == other.AsConstant().ToString();
}

TACInstruction::TACInstruction(OpCode op, TACOperand dst, TACOperand lhs, TACOperand rhs,
                               std::string label)
    : op_(op),
      dst_(std::move(dst)),
      lhs_(std::move(lhs)),
      rhs_(std::move(rhs)),
      label_(std::move(label)) {}

TACInstruction TACInstruction::Label(const std::string& label) {
    return TACInstruction(OpCode::Label, TACOperand(""), TACOperand(""), TACOperand(""),
                          label);
}

TACInstruction TACInstruction::GoTo(const std::string& target) {
    return TACInstruction(OpCode::GoTo, TACOperand(""), TACOperand(""), TACOperand(""),
                          target);
}

TACInstruction TACInstruction::If(const std::string& target,
                                  const TACOperand& condition) {
    return TACInstruction(OpCode::If, TACOperand(""), condition, TACOperand(""), target);
}

TACInstruction TACInstruction::IfFalse(const std::string& target,
                                       const TACOperand& condition) {
    return TACInstruction(OpCode::IfFalse, TACOperand(""), condition, TACOperand(""),
                          target);
}

TACInstruction TACInstruction::Function(const std::string& name, int param_count,
                                        bool is_global) {
    return TACInstruction(OpCode::Function, TACOperand(name),
                          TACOperand(NumericConstant(param_count)),
                          TACOperand(NumericConstant(is_global ? 1 : 0)), name);
}

TACInstruction TACInstruction::Return() {
    return TACInstruction(OpCode::Return, TACOperand(""), TACOperand(""), TACOperand(""),
                          "");
}

TACInstruction TACInstruction::Return(const TACOperand& value) {
    return TACInstruction(OpCode::Return, TACOperand(""), value, TACOperand(""), "");
}

TACInstruction TACInstruction::Call(const std::string& result,
                                    const std::string& func_name, int num_args) {
    return TACInstruction(OpCode::Call, TACOperand(result), TACOperand(func_name),
                          TACOperand(NumericConstant(num_args)), "");
}

TACInstruction TACInstruction::Param(const TACOperand& value) {
    return TACInstruction(OpCode::Param, TACOperand(""), value, TACOperand(""), "");
}

TACInstruction TACInstruction::StaticVariable(const std::string& name,
                                              const NumericConstant& init,
                                              bool is_global) {
    return TACInstruction(OpCode::StaticVariable, TACOperand(name), TACOperand(init),
                          TACOperand(NumericConstant(is_global ? 1 : 0)), "");
}

TACInstruction TACInstruction::Assign(const TACOperand& dst, const TACOperand& src) {
    return TACInstruction(OpCode::Assign, dst, src, TACOperand(""), "");
}

TACInstruction TACInstruction::Binary(OpCode op, const TACOperand& dst,
                                      const TACOperand& lhs, const TACOperand& rhs) {
    return TACInstruction(op, dst, lhs, rhs, "");
}

TACInstruction TACInstruction::Unary(OpCode op, const TACOperand& dst,
                                     const TACOperand& src) {
    return TACInstruction(op, dst, src, TACOperand(""), "");
}

TACInstruction TACInstruction::SignExtend(const TACOperand& dst, const TACOperand& src) {
    return TACInstruction(OpCode::SignExtend, dst, src, TACOperand(""), "");
}

TACInstruction TACInstruction::ZeroExtend(const TACOperand& dst, const TACOperand& src) {
    return TACInstruction(OpCode::ZeroExtend, dst, src, TACOperand(""), "");
}

TACInstruction TACInstruction::Truncate(const TACOperand& dst, const TACOperand& src) {
    return TACInstruction(OpCode::Truncate, dst, src, TACOperand(""), "");
}

TACInstruction TACInstruction::DoubleToInt(const TACOperand& dst, const TACOperand& src) {
    return TACInstruction(OpCode::DoubleToInt, dst, src, TACOperand(""), "");
}

TACInstruction TACInstruction::DoubleToUInt(const TACOperand& dst,
                                            const TACOperand& src) {
    return TACInstruction(OpCode::DoubleToUInt, dst, src, TACOperand(""), "");
}

TACInstruction TACInstruction::IntToDouble(const TACOperand& dst, const TACOperand& src) {
    return TACInstruction(OpCode::IntToDouble, dst, src, TACOperand(""), "");
}

TACInstruction TACInstruction::UIntToDouble(const TACOperand& dst,
                                            const TACOperand& src) {
    return TACInstruction(OpCode::UIntToDouble, dst, src, TACOperand(""), "");
}

TACInstruction::OpCode TACInstruction::GetOp() const { return op_; }

const TACOperand& TACInstruction::GetDst() const { return dst_; }

const TACOperand& TACInstruction::GetLhs() const { return lhs_; }

const TACOperand& TACInstruction::GetRhs() const { return rhs_; }

const std::string& TACInstruction::GetLabel() const { return label_; }

bool TACInstruction::operator==(const TACInstruction& other) const {
    return op_ == other.op_ && dst_ == other.dst_ && lhs_ == other.lhs_ &&
           rhs_ == other.rhs_ && label_ == other.label_;
}

std::string TACInstruction::ToString() const {
    std::ostringstream out;

    auto OpToStr = [](OpCode op) -> std::string {
        switch (op) {
            case OpCode::Label:
                return "label";
            case OpCode::Function:
                return "function";
            case OpCode::StaticVariable:
                return "static variable";
            case OpCode::Return:
                return "return";
            case OpCode::Assign:
                return "=";
            case OpCode::Add:
                return "+";
            case OpCode::Sub:
                return "-";
            case OpCode::Mul:
                return "*";
            case OpCode::Div:
                return "/";
            case OpCode::Mod:
                return "%";
            case OpCode::Not:
                return "!";
            case OpCode::Plus:
                return "+";
            case OpCode::Minus:
                return "-";
            case OpCode::BinaryNot:
                return "~";
            case OpCode::Less:
                return "<";
            case OpCode::LessEqual:
                return "<=";
            case OpCode::Greater:
                return ">";
            case OpCode::GreaterEqual:
                return ">=";
            case OpCode::Equal:
                return "==";
            case OpCode::NotEqual:
                return "!=";
            case OpCode::If:
                return "if";
            case OpCode::IfFalse:
                return "iffalse";
            case OpCode::GoTo:
                return "goto";
            case OpCode::BitwiseAnd:
                return "&";
            case OpCode::BitwiseXor:
                return "^";
            case OpCode::BitwiseOr:
                return "|";
            case OpCode::LeftShift:
                return "<<";
            case OpCode::RightShift:
                return ">>";
            case OpCode::Call:
                return "call";
            case OpCode::Param:
                return "param";
            case OpCode::SignExtend:
                return "sign extend";
            case OpCode::ZeroExtend:
                return "zero extend";
            case OpCode::Truncate:
                return "truncate";
            case OpCode::DoubleToInt:
                return "double to int";
            case OpCode::DoubleToUInt:
                return "double to uint";
            case OpCode::IntToDouble:
                return "int to double";
            case OpCode::UIntToDouble:
                return "uint to double";
        }
        return "unknown";
    };

    switch (op_) {
        case OpCode::Label:
            out << label_ << ":";
            break;
        case OpCode::Function:
            out << "function " << label_ << " with " << lhs_.ToString() << " args";
            break;
        case OpCode::StaticVariable:
            out << "static variable " << dst_.ToString() << " = " << lhs_.ToString()
                << " global " << rhs_.ToString();
            break;
        case OpCode::Return:
            if (!lhs_.Empty()) {
                out << "return " << lhs_.ToString();
            } else {
                out << "return";
            }
            break;
        case OpCode::Assign:
            out << dst_.ToString() << " = " << lhs_.ToString();
            break;
        case OpCode::Add:
        case OpCode::Sub:
        case OpCode::Mul:
        case OpCode::Div:
        case OpCode::Mod:
        case OpCode::Less:
        case OpCode::LessEqual:
        case OpCode::Greater:
        case OpCode::GreaterEqual:
        case OpCode::Equal:
        case OpCode::NotEqual:
        case OpCode::BitwiseAnd:
        case OpCode::BitwiseXor:
        case OpCode::BitwiseOr:
        case OpCode::LeftShift:
        case OpCode::RightShift:
            out << dst_.ToString() << " = " << lhs_.ToString() << " " << OpToStr(op_)
                << " " << rhs_.ToString();
            break;
        case OpCode::Call:
            out << dst_.ToString() << " = call " << lhs_.ToString() << " "
                << rhs_.ToString();
            break;
        case OpCode::Param:
            out << "param " << lhs_.ToString();
            break;
        case OpCode::Not:
        case OpCode::BinaryNot:
        case OpCode::Plus:
        case OpCode::Minus:
        case OpCode::SignExtend:
        case OpCode::ZeroExtend:
        case OpCode::Truncate:
        case OpCode::DoubleToInt:
        case OpCode::DoubleToUInt:
        case OpCode::IntToDouble:
        case OpCode::UIntToDouble:
            out << dst_.ToString() << " = " << OpToStr(op_) << " " << lhs_.ToString();
            break;
        case OpCode::If:
            out << "if " << lhs_.ToString() << " goto " << label_;
            break;
        case OpCode::IfFalse:
            out << "iffalse " << lhs_.ToString() << " goto " << label_;
            break;
        case OpCode::GoTo:
            out << "goto " << label_;
            break;
    }

    return out.str();
}
