#include "include/ast/expressions.h"

#include "include/visitors/visitor.h"

PrimaryExpression::PrimaryExpression(int value) : value_(value) {}

void PrimaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

int PrimaryExpression::GetValue() const { return value_; }

///////////////////////////////////////////////

IdExpression::IdExpression(std::string id) : id_(id) {}

void IdExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

std::string IdExpression::GetId() const { return id_; }

void IdExpression::SetId(const std::string& id) { id_ = id; }

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

///////////////////////////////////////////////

ConditionalExpression::ConditionalExpression(Expression* cond, Expression* left,
                                             Expression* right)
    : cond_(cond), left_(left), right_(right) {}

void ConditionalExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

Expression* ConditionalExpression::GetCondition() const { return cond_; }

Expression* ConditionalExpression::GetLeftExpression() const { return left_; }

Expression* ConditionalExpression::GetRightExpression() const { return right_; }

///////////////////////////////////////////////

AssignmentExpression::AssignmentExpression(Expression* left, Expression* right)
    : left_(left), right_(right) {}

void AssignmentExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

Expression* AssignmentExpression::GetLeftExpression() const { return left_; }

Expression* AssignmentExpression::GetRightExpression() const { return right_; }