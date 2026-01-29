#include "include/ast/statements.h"

#include "include/visitors/visitor.h"

///////////////////////////////////////////////

CompoundStatement::CompoundStatement(std::unique_ptr<ItemList> body)
    : body_(std::move(body)) {}

void CompoundStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

ItemList* CompoundStatement::GetBody() const { return body_.get(); }

///////////////////////////////////////////////

ReturnStatement::ReturnStatement() = default;

ReturnStatement::ReturnStatement(std::unique_ptr<Expression> expression)
    : expression_(std::move(expression)) {}

void ReturnStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

bool ReturnStatement::HasExpression() const { return expression_.has_value(); }

Expression* ReturnStatement::GetExpression() const {
    return expression_ ? expression_->get() : nullptr;
}

std::unique_ptr<Expression> ReturnStatement::ExtractExpression() {
    if (!expression_) {
        return nullptr;
    }
    auto result = std::move(*expression_);
    expression_.reset();
    return result;
}

void ReturnStatement::SetExpression(std::unique_ptr<Expression> expression) {
    expression_ = std::move(expression);
}

///////////////////////////////////////////////

ExpressionStatement::ExpressionStatement() = default;

ExpressionStatement::ExpressionStatement(std::unique_ptr<Expression> expression)
    : expression_(std::move(expression)) {}

void ExpressionStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

bool ExpressionStatement::HasExpression() const { return expression_.has_value(); }

Expression* ExpressionStatement::GetExpression() const {
    return expression_ ? expression_->get() : nullptr;
}

///////////////////////////////////////////////

SelectionStatement::SelectionStatement(std::unique_ptr<Expression> cond,
                                       std::unique_ptr<Statement> then_stmt)
    : cond_(std::move(cond)), then_stmt_(std::move(then_stmt)) {}

SelectionStatement::SelectionStatement(std::unique_ptr<Expression> cond,
                                       std::unique_ptr<Statement> then_stmt,
                                       std::unique_ptr<Statement> else_stmt)
    : cond_(std::move(cond)),
      then_stmt_(std::move(then_stmt)),
      else_stmt_(std::move(else_stmt)) {}

void SelectionStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

bool SelectionStatement::HasElseStatement() const { return else_stmt_.has_value(); }

Expression* SelectionStatement::GetCondition() const { return cond_.get(); }

Statement* SelectionStatement::GetThenStatement() const { return then_stmt_.get(); }

Statement* SelectionStatement::GetElseStatement() const {
    return else_stmt_ ? else_stmt_->get() : nullptr;
}

///////////////////////////////////////////////

JumpStatement::JumpStatement(JumpType type) : type_(type) {}

void JumpStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

JumpStatement::JumpType JumpStatement::GetType() const { return type_; }

void JumpStatement::SetLabel(std::string label) { label_ = std::move(label); }

std::string JumpStatement::GetLabel() const { return label_; }

///////////////////////////////////////////////

WhileStatement::WhileStatement(LoopType type, std::unique_ptr<Expression> cond,
                               std::unique_ptr<Statement> body)
    : cond_(std::move(cond)), body_(std::move(body)), type_(type) {}

void WhileStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

WhileStatement::LoopType WhileStatement::GetType() const { return type_; }

Expression* WhileStatement::GetCondition() const { return cond_.get(); }

Statement* WhileStatement::GetBody() const { return body_.get(); }

void WhileStatement::SetLabel(std::string label) { label_ = std::move(label); }

std::string WhileStatement::GetLabel() const { return label_; }

///////////////////////////////////////////////

ForStatement::ForStatement(std::unique_ptr<BaseElement> init,
                           std::unique_ptr<Expression> cond,
                           std::unique_ptr<Expression> inc,
                           std::unique_ptr<Statement> body)
    : init_(std::move(init)),
      cond_(std::move(cond)),
      inc_(std::move(inc)),
      body_(std::move(body)) {}

void ForStatement::Accept(Visitor* visitor) { visitor->Visit(this); }

BaseElement* ForStatement::GetInit() const { return init_.get(); }

Expression* ForStatement::GetCondition() const { return cond_.get(); }

Expression* ForStatement::GetIncrement() const { return inc_.get(); }

Statement* ForStatement::GetBody() const { return body_.get(); }

void ForStatement::SetLabel(std::string label) { label_ = std::move(label); }

std::string ForStatement::GetLabel() const { return label_; }