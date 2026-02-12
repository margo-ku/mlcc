#pragma once

#include <string>

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
    };

    static TACInstruction Label(const std::string& label);
    static TACInstruction GoTo(const std::string& target);
    static TACInstruction If(const std::string& target, const std::string& condition);
    static TACInstruction IfFalse(const std::string& target,
                                  const std::string& condition);

    static TACInstruction Function(const std::string& name, int param_count,
                                   bool is_global);
    static TACInstruction Return(const std::string& value = "");
    static TACInstruction Call(const std::string& result, const std::string& func_name,
                               int num_args);
    static TACInstruction Param(const std::string& value);

    static TACInstruction StaticVariable(const std::string& name, const std::string& init,
                                         bool is_global);
    static TACInstruction Assign(const std::string& dst, const std::string& src);

    static TACInstruction Binary(OpCode op, const std::string& dst,
                                 const std::string& lhs, const std::string& rhs);

    static TACInstruction Unary(OpCode op, const std::string& dst,
                                const std::string& src);

    static TACInstruction SignExtend(const std::string& dst, const std::string& src);
    static TACInstruction ZeroExtend(const std::string& dst, const std::string& src);
    static TACInstruction Truncate(const std::string& dst, const std::string& src);

    std::string ToString() const;
    OpCode GetOp() const;

    const std::string& GetDst() const;
    const std::string& GetLhs() const;
    const std::string& GetRhs() const;
    const std::string& GetLabel() const;

    bool operator==(const TACInstruction& other) const;

private:
    TACInstruction(OpCode op, std::string dst, std::string lhs, std::string rhs,
                   std::string label);

    OpCode op_;
    std::string dst_;
    std::string lhs_;
    std::string rhs_;
    std::string label_;
};
