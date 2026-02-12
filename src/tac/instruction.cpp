#include "include/tac/instruction.h"

#include <sstream>

TACInstruction::TACInstruction(OpCode op, std::string dst, std::string lhs,
                               std::string rhs, std::string label)
    : op_(op),
      dst_(std::move(dst)),
      lhs_(std::move(lhs)),
      rhs_(std::move(rhs)),
      label_(std::move(label)) {}

TACInstruction TACInstruction::Label(const std::string& label) {
    return TACInstruction(OpCode::Label, "", "", "", label);
}

TACInstruction TACInstruction::GoTo(const std::string& target) {
    return TACInstruction(OpCode::GoTo, "", "", "", target);
}

TACInstruction TACInstruction::If(const std::string& target,
                                  const std::string& condition) {
    return TACInstruction(OpCode::If, "", condition, "", target);
}

TACInstruction TACInstruction::IfFalse(const std::string& target,
                                       const std::string& condition) {
    return TACInstruction(OpCode::IfFalse, "", condition, "", target);
}

TACInstruction TACInstruction::Function(const std::string& name, int param_count,
                                        bool is_global) {
    return TACInstruction(OpCode::Function, name, std::to_string(param_count),
                          is_global ? "1" : "0", name);
}

TACInstruction TACInstruction::Return(const std::string& value) {
    return TACInstruction(OpCode::Return, "", "", "", value);
}

TACInstruction TACInstruction::Call(const std::string& result,
                                    const std::string& func_name, int num_args) {
    return TACInstruction(OpCode::Call, result, func_name, std::to_string(num_args), "");
}

TACInstruction TACInstruction::Param(const std::string& value) {
    return TACInstruction(OpCode::Param, "", "", "", value);
}

TACInstruction TACInstruction::StaticVariable(const std::string& name,
                                              const std::string& init, bool is_global) {
    return TACInstruction(OpCode::StaticVariable, name, init, is_global ? "1" : "0", "");
}

TACInstruction TACInstruction::Assign(const std::string& dst, const std::string& src) {
    return TACInstruction(OpCode::Assign, dst, src, "", "");
}

TACInstruction TACInstruction::Binary(OpCode op, const std::string& dst,
                                      const std::string& lhs, const std::string& rhs) {
    return TACInstruction(op, dst, lhs, rhs, "");
}

TACInstruction TACInstruction::Unary(OpCode op, const std::string& dst,
                                     const std::string& src) {
    return TACInstruction(op, dst, src, "", "");
}

TACInstruction TACInstruction::SignExtend(const std::string& dst,
                                          const std::string& src) {
    return TACInstruction(OpCode::SignExtend, dst, src, "", "");
}

TACInstruction TACInstruction::ZeroExtend(const std::string& dst,
                                          const std::string& src) {
    return TACInstruction(OpCode::ZeroExtend, dst, src, "", "");
}

TACInstruction TACInstruction::Truncate(const std::string& dst, const std::string& src) {
    return TACInstruction(OpCode::Truncate, dst, src, "", "");
}

TACInstruction::OpCode TACInstruction::GetOp() const { return op_; }

const std::string& TACInstruction::GetDst() const { return dst_; }

const std::string& TACInstruction::GetLhs() const { return lhs_; }

const std::string& TACInstruction::GetRhs() const { return rhs_; }

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
        }
        return "unknown";
    };

    switch (op_) {
        case OpCode::Label:
            out << label_ << ":";
            break;
        case OpCode::Function:
            out << "function " << label_ << " with " << lhs_ << " args";
            break;
        case OpCode::StaticVariable:
            out << "static variable " << dst_ << " = " << lhs_ << " global " << rhs_;
            break;
        case OpCode::Return:
            if (!label_.empty()) {
                out << "return " << label_;
            } else {
                out << "return";
            }
            break;
        case OpCode::Assign:
            out << dst_ << " = " << lhs_;
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
            out << dst_ << " = " << lhs_ << " " << OpToStr(op_) << " " << rhs_;
            break;
        case OpCode::Call:
            out << dst_ << " = call " << lhs_ << " " << rhs_;
            break;
        case OpCode::Param:
            out << "param " << label_;
            break;
        case OpCode::Not:
        case OpCode::BinaryNot:
        case OpCode::Plus:
        case OpCode::Minus:
        case OpCode::SignExtend:
        case OpCode::ZeroExtend:
        case OpCode::Truncate:
            out << dst_ << " = " << OpToStr(op_) << " " << lhs_;
            break;
        case OpCode::If:
            out << "if " << lhs_ << " goto " << label_;
            break;
        case OpCode::IfFalse:
            out << "iffalse " << lhs_ << " goto " << label_;
            break;
        case OpCode::GoTo:
            out << "goto " << label_;
            break;
    }

    return out.str();
}
