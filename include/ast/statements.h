#pragma once
#include "include/ast/expressions.h"
#include "include/ast/translation_unit.h"

class Visitor;

class Statement : public BaseElement {
public:
    virtual void Accept(Visitor* visitor) = 0;
};

///////////////////////////////////////////////

class CompoundStatement : public Statement {
public:
    explicit CompoundStatement(ItemList* body);
    virtual void Accept(Visitor* visitor) override;
    ItemList* GetBody();

private:
    ItemList* body_;
};

///////////////////////////////////////////////

// to do: convert ReturnStatement into JumpStatement
class ReturnStatement : public Statement {
public:
    ReturnStatement();
    explicit ReturnStatement(Expression* expression);
    virtual void Accept(Visitor* visitor) override;
    bool HasExpression() const;
    Expression* GetExpression();

private:
    Expression* expression_;
};