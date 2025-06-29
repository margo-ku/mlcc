#pragma once
#include "include/ast/translation_unit.h"

class Visitor;

class Expression : public BaseElement {
public:
    virtual void Accept(Visitor* visitor) = 0;
};

///////////////////////////////////////////////

// to do: expand
class PrimaryExpression : public Expression {
public:
    explicit PrimaryExpression(int value);
    virtual void Accept(Visitor* visitor) override;
    int GetValue() const;

private:
    int value_;
};

///////////////////////////////////////////////

class UnaryExpression : public Expression {
public:
    enum class UnaryOperator {
        kMinus = 0,
        kPlus,
        kBinaryNot,
        kNot,
    };

    explicit UnaryExpression(UnaryOperator op, Expression* expression);
    virtual void Accept(Visitor* visitor) override;
    UnaryOperator GetOp() const;
    Expression* GetExpression() const;

private:
    UnaryOperator op_;
    Expression* expression_;
};

///////////////////////////////////////////////

class BinaryExpression : public Expression {
public:
    enum class BinaryOperator {
        kPlus = 0,
        kMinus,
        kMul,
        kDiv,
        kMod,
        kLess,
        kGreater,
        kLessEqual,
        kGreaterEqual,
        kEqual,
        kNotEqual,
        kAnd,
        kOr,
    };

    explicit BinaryExpression(BinaryOperator op, Expression* left, Expression* right);
    virtual void Accept(Visitor* visitor) override;
    BinaryOperator GetOp() const;
    Expression* GetLeftExpression() const;
    Expression* GetRightExpression() const;

private:
    BinaryOperator op_;
    Expression* left_;
    Expression* right_;
};