#pragma once
#include <memory>
#include <variant>

#include "include/ast/translation_unit.h"
#include "include/types/type.h"

class Visitor;
class TypeSpecification;

class Expression : public BaseElement {
public:
    virtual ~Expression() = default;
    virtual void Accept(Visitor* visitor);
    TypeRef GetTypeRef() const;
    void SetTypeRef(TypeRef type);

private:
    TypeRef type_ref_;
};

///////////////////////////////////////////////

class PrimaryExpression : public Expression {
public:
    using ValueType = std::variant<int, long, unsigned int, unsigned long>;
    explicit PrimaryExpression(ValueType value);
    template <typename T>
    explicit PrimaryExpression(T value) : value_(value) {
        static_assert(std::is_same_v<T, int> || std::is_same_v<T, long> ||
                      std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned long>,
                      "PrimaryExpression supports int, long, unsigned int, unsigned long");
    }

    virtual ~PrimaryExpression() = default;
    void Accept(Visitor* visitor) override;

    template <typename T>
    bool Holds() const {
        return std::holds_alternative<T>(value_);
    }

    template <typename T>
    T Get() const {
        return std::get<T>(value_);
    }

    PrimaryExpression::ValueType GetValue() const;

    std::string ToString() const;

private:
    ValueType value_;

    struct ValueToString {
        std::string operator()(int value) const { return std::to_string(value); }
        std::string operator()(long value) const { return std::to_string(value); }
        std::string operator()(unsigned int value) const { return std::to_string(value); }
        std::string operator()(unsigned long value) const { return std::to_string(value); }
    };
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
    void SetLeftExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Expression> ExtractLeftExpression();
    void SetRightExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Expression> ExtractRightExpression();

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
    void SetLeftExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Expression> ExtractLeftExpression();
    void SetRightExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Expression> ExtractRightExpression();

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
    void SetRightExpression(std::unique_ptr<Expression> expression);
    std::unique_ptr<Expression> ExtractRightExpression();

private:
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};

///////////////////////////////////////////////

class ArgumentExpressionList : public Expression {
public:
    ArgumentExpressionList();
    virtual ~ArgumentExpressionList() = default;
    void Accept(Visitor* visitor) override;
    const std::vector<std::unique_ptr<Expression>>& GetArguments() const;
    std::vector<std::unique_ptr<Expression>>& GetArguments();
    void AddArgument(std::unique_ptr<Expression> argument);

private:
    std::vector<std::unique_ptr<Expression>> arguments_;
};

///////////////////////////////////////////////

class FunctionCallExpression : public Expression {
public:
    explicit FunctionCallExpression(std::unique_ptr<Expression> function);
    FunctionCallExpression(std::unique_ptr<Expression> function,
                           std::unique_ptr<ArgumentExpressionList> arguments);
    virtual ~FunctionCallExpression() = default;
    void Accept(Visitor* visitor) override;
    Expression* GetFunction() const;
    ArgumentExpressionList* GetArguments() const;
    bool HasArguments() const;

private:
    std::unique_ptr<Expression> function_;
    std::optional<std::unique_ptr<ArgumentExpressionList>> arguments_;
};

///////////////////////////////////////////////

class CastExpression : public Expression {
public:
    CastExpression(std::unique_ptr<TypeSpecification> type,
                   std::unique_ptr<Expression> expression);
    virtual ~CastExpression() = default;
    void Accept(Visitor* visitor) override;
    TypeSpecification* GetType() const;
    Expression* GetExpression() const;

private:
    std::unique_ptr<TypeSpecification> type_;
    std::unique_ptr<Expression> expression_;
};