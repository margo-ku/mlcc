#include "include/ast/expressions.h"

#include "include/visitors/visitor.h"

void Expression::Accept(Visitor* visitor) { visitor->Visit(this); }

///////////////////////////////////////////////

PrimaryExpression::PrimaryExpression(int value) : value_(value) {}

void PrimaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

int PrimaryExpression::GetValue() const { return value_; }

///////////////////////////////////////////////

IdExpression::IdExpression(std::string id) : id_(std::move(id)) {}

void IdExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

std::string IdExpression::GetId() const { return id_; }

void IdExpression::SetId(const std::string& id) { id_ = id; }

///////////////////////////////////////////////

UnaryExpression::UnaryExpression(UnaryOperator op, std::unique_ptr<Expression> expression)
    : op_(op), expression_(std::move(expression)) {}

void UnaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

UnaryExpression::UnaryOperator UnaryExpression::GetOp() const { return op_; }

Expression* UnaryExpression::GetExpression() const { return expression_.get(); }

///////////////////////////////////////////////

BinaryExpression::BinaryExpression(BinaryOperator op, std::unique_ptr<Expression> left,
                                   std::unique_ptr<Expression> right)
    : op_(op), left_(std::move(left)), right_(std::move(right)) {}

void BinaryExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

BinaryExpression::BinaryOperator BinaryExpression::GetOp() const { return op_; }

Expression* BinaryExpression::GetLeftExpression() const { return left_.get(); }

Expression* BinaryExpression::GetRightExpression() const { return right_.get(); }

///////////////////////////////////////////////

ConditionalExpression::ConditionalExpression(std::unique_ptr<Expression> cond,
                                             std::unique_ptr<Expression> left,
                                             std::unique_ptr<Expression> right)
    : cond_(std::move(cond)), left_(std::move(left)), right_(std::move(right)) {}

void ConditionalExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

Expression* ConditionalExpression::GetCondition() const { return cond_.get(); }

Expression* ConditionalExpression::GetLeftExpression() const { return left_.get(); }

Expression* ConditionalExpression::GetRightExpression() const { return right_.get(); }

///////////////////////////////////////////////

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> left,
                                           std::unique_ptr<Expression> right)
    : left_(std::move(left)), right_(std::move(right)) {}

void AssignmentExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

Expression* AssignmentExpression::GetLeftExpression() const { return left_.get(); }

Expression* AssignmentExpression::GetRightExpression() const { return right_.get(); }

///////////////////////////////////////////////

ArgumentExpressionList::ArgumentExpressionList() {}

void ArgumentExpressionList::Accept(Visitor* visitor) { visitor->Visit(this); }

const std::vector<std::unique_ptr<Expression>>& ArgumentExpressionList::GetArguments()
    const {
    return arguments_;
}

std::vector<std::unique_ptr<Expression>>& ArgumentExpressionList::GetArguments() {
    return arguments_;
}

void ArgumentExpressionList::AddArgument(std::unique_ptr<Expression> argument) {
    arguments_.push_back(std::move(argument));
}

///////////////////////////////////////////////

FunctionCallExpression::FunctionCallExpression(std::unique_ptr<Expression> function)
    : function_(std::move(function)) {}

FunctionCallExpression::FunctionCallExpression(
    std::unique_ptr<Expression> function,
    std::unique_ptr<ArgumentExpressionList> arguments)
    : function_(std::move(function)), arguments_(std::move(arguments)) {}

void FunctionCallExpression::Accept(Visitor* visitor) { visitor->Visit(this); }

Expression* FunctionCallExpression::GetFunction() const { return function_.get(); }

ArgumentExpressionList* FunctionCallExpression::GetArguments() const {
    return arguments_.has_value() ? arguments_.value().get() : nullptr;
}

bool FunctionCallExpression::HasArguments() const { return arguments_.has_value(); }