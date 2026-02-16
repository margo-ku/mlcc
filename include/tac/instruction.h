#pragma once

#include <string>
#include <variant>

#include "include/types/numeric_constant.h"

class TACOperand {
public:
    using Storage = std::variant<std::string, NumericConstant>;

    TACOperand(const std::string& identifier);
    explicit TACOperand(const NumericConstant& constant);

    bool IsIdentifier() const;
    bool IsConstant() const;
    bool Empty() const;

    const std::string& AsIdentifier() const;
    const NumericConstant& AsConstant() const;
    std::string ToString() const;

    bool operator==(const TACOperand& other) const;

private:
    Storage storage_;
};

class TACInstruction {
public:
    enum class OpCode {
        Label,
        Function,
        StaticVariable,
        Return,
        Assign,
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Not,
        Plus,
        Minus,
        BinaryNot,
        Less,
        LessEqual,
        Greater,
        GreaterEqual,
        Equal,
        NotEqual,
        If,
        IfFalse,
        GoTo,
        BitwiseAnd,
        BitwiseXor,
        BitwiseOr,
        LeftShift,
        RightShift,
        Call,
        Param,
        SignExtend,
        ZeroExtend,
        Truncate,
        DoubleToInt,
        DoubleToUInt,
        IntToDouble,
        UIntToDouble,
    };

    static TACInstruction Label(const std::string& label);
    static TACInstruction GoTo(const std::string& target);
    static TACInstruction If(const std::string& target, const TACOperand& condition);
    static TACInstruction IfFalse(const std::string& target, const TACOperand& condition);

    static TACInstruction Function(const std::string& name, int param_count,
                                   bool is_global);
    static TACInstruction Return();
    static TACInstruction Return(const TACOperand& value);
    static TACInstruction Call(const std::string& result, const std::string& func_name,
                               int num_args);
    static TACInstruction Param(const TACOperand& value);

    static TACInstruction StaticVariable(const std::string& name,
                                         const NumericConstant& init, bool is_global);
    static TACInstruction Assign(const TACOperand& dst, const TACOperand& src);

    static TACInstruction Binary(OpCode op, const TACOperand& dst, const TACOperand& lhs,
                                 const TACOperand& rhs);

    static TACInstruction Unary(OpCode op, const TACOperand& dst, const TACOperand& src);

    static TACInstruction SignExtend(const TACOperand& dst, const TACOperand& src);
    static TACInstruction ZeroExtend(const TACOperand& dst, const TACOperand& src);
    static TACInstruction Truncate(const TACOperand& dst, const TACOperand& src);
    static TACInstruction DoubleToInt(const TACOperand& dst, const TACOperand& src);
    static TACInstruction DoubleToUInt(const TACOperand& dst, const TACOperand& src);
    static TACInstruction IntToDouble(const TACOperand& dst, const TACOperand& src);
    static TACInstruction UIntToDouble(const TACOperand& dst, const TACOperand& src);

    std::string ToString() const;
    OpCode GetOp() const;

    const TACOperand& GetDst() const;
    const TACOperand& GetLhs() const;
    const TACOperand& GetRhs() const;
    const std::string& GetLabel() const;

    bool operator==(const TACInstruction& other) const;

private:
    TACInstruction(OpCode op, TACOperand dst, TACOperand lhs, TACOperand rhs,
                   std::string label);

    OpCode op_;
    TACOperand dst_;
    TACOperand lhs_;
    TACOperand rhs_;
    std::string label_;
};
