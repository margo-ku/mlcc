#include "include/ast/statements.h"

#include "include/visitors/visitor.h"

///////////////////////////////////////////////

CompoundStatement::CompoundStatement(ItemList* body) : body_(body) {}

void CompoundStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

ItemList* CompoundStatement::GetBody() { return body_; }

///////////////////////////////////////////////

ReturnStatement::ReturnStatement() : expression_(nullptr) {}

ReturnStatement::ReturnStatement(Expression* expression) : expression_(expression) {}

void ReturnStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

bool ReturnStatement::HasExpression() const { return expression_ != nullptr; }

Expression* ReturnStatement::GetExpression() { return expression_; }

///////////////////////////////////////////////

ExpressionStatement::ExpressionStatement() {}

ExpressionStatement::ExpressionStatement(Expression* expression)
    : expression_(expression) {}

void ExpressionStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

bool ExpressionStatement::HasExpression() const { return expression_.has_value(); }

Expression* ExpressionStatement::GetExpression() { return expression_.value(); }

///////////////////////////////////////////////

SelectionStatement::SelectionStatement(Expression* cond, Statement* then_stmt)
    : cond_(cond), then_stmt_(then_stmt) {}

SelectionStatement::SelectionStatement(Expression* cond, Statement* then_stmt,
                                       Statement* else_stmt)
    : cond_(cond), then_stmt_(then_stmt), else_stmt_(else_stmt) {}

void SelectionStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

bool SelectionStatement::HasElseStatement() const { return else_stmt_.has_value(); }

Expression* SelectionStatement::GetCondition() const { return cond_; }

Statement* SelectionStatement::GetThenStatement() const { return then_stmt_; }

Statement* SelectionStatement::GetElseStatement() const { return else_stmt_.value(); }