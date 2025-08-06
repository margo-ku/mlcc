#pragma once
#include "include/ast/translation_unit.h"

class Visitor;

class Expression : public BaseElement {
public:
    virtual ~Expression() = default;
    virtual void Accept(Visitor* visitor);
};

///////////////////////////////////////////////

class PrimaryExpression : public Expression {
public:
    explicit PrimaryExpression(int value);
    virtual ~PrimaryExpression() = default;
    void Accept(Visitor* visitor) override;
    int GetValue() const;

private:
    int value_;
};

///////////////////////////////////////////////

class IdExpression : public Expression {
public:
    explicit IdExpression(std::string id);
    virtual ~IdExpression() = default;
    void Accept(Visitor* visitor) override;
    std::string GetId() const;
    void SetId(const std::string& id);

private:
    std::string id_;
};

///////////////////////////////////////////////

class UnaryExpression : public Expression {
public:
    enum class UnaryOperator {
        Minus = 0,
        Plus,
        BinaryNot,
        Not,
    };

    explicit UnaryExpression(UnaryOperator op, std::unique_ptr<Expression> expression);
    virtual ~UnaryExpression() = default;
    void Accept(Visitor* visitor) override;
    UnaryOperator GetOp() const;
    Expression* GetExpression() const;

private:
    UnaryOperator op_;
    std::unique_ptr<Expression> expression_;
};

///////////////////////////////////////////////

class BinaryExpression : public Expression {
public:
    enum class BinaryOperator {
        Plus = 0,
        Minus,
        Mul,
        Div,
        Mod,
        Less,
        Greater,
        LessEqual,
        GreaterEqual,
        Equal,
        NotEqual,
        And,
        Or,
        BitwiseAnd,
        BitwiseXor,
        BitwiseOr,
        LeftShift,
        RightShift,
    };

    BinaryExpression(BinaryOperator op, std::unique_ptr<Expression> left,
                     std::unique_ptr<Expression> right);
    virtual ~BinaryExpression() = default;
    void Accept(Visitor* visitor) override;
    BinaryOperator GetOp() const;
    Expression* GetLeftExpression() const;
    Expression* GetRightExpression() const;

private:
    BinaryOperator op_;
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};

///////////////////////////////////////////////

class ConditionalExpression : public Expression {
public:
    ConditionalExpression(std::unique_ptr<Expression> condition,
                          std::unique_ptr<Expression> left,
                          std::unique_ptr<Expression> right);
    virtual ~ConditionalExpression() = default;
    void Accept(Visitor* visitor) override;
    Expression* GetCondition() const;
    Expression* GetLeftExpression() const;
    Expression* GetRightExpression() const;

private:
    std::unique_ptr<Expression> cond_;
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};

///////////////////////////////////////////////

class AssignmentExpression : public Expression {
public:
    AssignmentExpression(std::unique_ptr<Expression> left,
                         std::unique_ptr<Expression> right);
    virtual ~AssignmentExpression() = default;
    void Accept(Visitor* visitor) override;
    Expression* GetLeftExpression() const;
    Expression* GetRightExpression() const;

private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};