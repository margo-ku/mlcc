#include "include/ast/statements.h"

#include "include/visitors/visitor.h"

///////////////////////////////////////////////

CompoundStatement::CompoundStatement(ItemList* body) : body_(body) {}

void CompoundStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

ItemList* CompoundStatement::GetBody() { return body_; }

///////////////////////////////////////////////

ReturnStatement::ReturnStatement() : expression_(nullptr) {}

ReturnStatement::ReturnStatement(Expression* expression)
    : expression_(expression) {}

void ReturnStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

bool ReturnStatement::HasExpression() const { return expression_ != nullptr; }

Expression* ReturnStatement::GetExpression() { return expression_; }