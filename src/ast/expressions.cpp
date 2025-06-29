#include "include/ast/expressions.h"

#include "include/visitors/visitor.h"

PrimaryExpression::PrimaryExpression(int value) : value_(value) {}

void PrimaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

int PrimaryExpression::GetValue() const { return value_; }

///////////////////////////////////////////////

UnaryExpression::UnaryExpression(UnaryOperator op, Expression* expression)
    : op_(op), expression_(expression) {}

void UnaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

UnaryExpression::UnaryOperator UnaryExpression::GetOp() const { return op_; }

Expression* UnaryExpression::GetExpression() const { return expression_; }

///////////////////////////////////////////////

BinaryExpression::BinaryExpression(BinaryOperator op, Expression* left, Expression* right)
    : op_(op), left_(left), right_(right) {}

void BinaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

BinaryExpression::BinaryOperator BinaryExpression::GetOp() const { return op_; }

Expression* BinaryExpression::GetLeftExpression() const { return left_; }

Expression* BinaryExpression::GetRightExpression() const { return right_; }