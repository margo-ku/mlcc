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

///////////////////////////////////////////////

JumpStatement::JumpStatement(JumpType type) : type_(type) {}

void JumpStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

JumpStatement::JumpType JumpStatement::GetType() const { return type_; }

void JumpStatement::SetLabel(std::string label) { label_ = label; }

std::string JumpStatement::GetLabel() const { return label_; }

///////////////////////////////////////////////

WhileStatement::WhileStatement(LoopType type, Expression* cond, Statement* body)
    : cond_(cond), body_(body), type_(type) {}

void WhileStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

WhileStatement::LoopType WhileStatement::GetType() const { return type_; }

Expression* WhileStatement::GetCondition() const { return cond_; }

Statement* WhileStatement::GetBody() const { return body_; }

void WhileStatement::SetLabel(std::string label) { label_ = label; }

std::string WhileStatement::GetLabel() const { return label_; }

///////////////////////////////////////////////

ForStatement::ForStatement(BaseElement* init, Expression* cond, Expression* inc, Statement* body)
    : init_(init), cond_(cond), inc_(inc), body_(body) {}

void ForStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

BaseElement* ForStatement::GetInit() const { return init_; }

Expression* ForStatement::GetCondition() const { return cond_; }

Expression* ForStatement::GetIncrement() const { return inc_; }

Statement* ForStatement::GetBody() const { return body_; }

void ForStatement::SetLabel(std::string label) { label_ = label; }

std::string ForStatement::GetLabel() const { return label_; }