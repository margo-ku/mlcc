#include "include/ast/expressions.h"

#include "include/visitors/visitor.h"

PrimaryExpression::PrimaryExpression(int value) : value_(value) {}

void PrimaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

int PrimaryExpression::GetValue() const { return value_; }

///////////////////////////////////////////////

UnaryExpression::UnaryExpression(char op, Expression* expression)
    : expression_(expression) {
    switch (op) {
        case '-':
            op_ = UnaryOperator::kMinus;
            break;
        case '+':
            op_ = UnaryOperator::kPlus;
            break;
        case '~':
            op_ = UnaryOperator::kBinaryNot;
            break;
        case '!':
            op_ = UnaryOperator::kNot;
            break;
        default:
            std::string msg = "unknown unary operator: ";
            msg += op;
            throw std::invalid_argument(msg);
    }
}

void UnaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

UnaryExpression::UnaryOperator UnaryExpression::GetOp() const { return op_; }

Expression* UnaryExpression::GetExpression() const { return expression_; }

///////////////////////////////////////////////

BinaryExpression::BinaryExpression(char op, Expression* left, Expression* right)
    : left_(left), right_(right) {
    switch (op) {
        case '+':
            op_ = BinaryOperator::kPlus;
            break;
        case '-':
            op_ = BinaryOperator::kMinus;
            break;
        case '*':
            op_ = BinaryOperator::kMul;
            break;
        case '/':
            op_ = BinaryOperator::kDiv;
            break;
        case '%':
            op_ = BinaryOperator::kMod;
            break;
        default:
            std::string msg = "unknown binary operator: ";
            msg += op;
            throw std::invalid_argument(msg);
    }
}

void BinaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

BinaryExpression::BinaryOperator BinaryExpression::GetOp() const { return op_; }

Expression* BinaryExpression::GetLeftExpression() const { return left_; }

Expression* BinaryExpression::GetRightExpression() const { return right_; }