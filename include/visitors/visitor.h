#pragma once
#include "include/ast/declarations.h"
#include "include/ast/expressions.h"
#include "include/ast/statements.h"
#include "include/ast/translation_unit.h"

class Visitor {
public:
    virtual void Visit(TranslationUnit* translation_unit) = 0;
    virtual void Visit(ItemList* item_list) = 0;
    virtual void Visit(FunctionDefinition* function) = 0;
    virtual void Visit(TypeSpecification* type) = 0;
    virtual void Visit(Declarator* declarator) = 0;
    virtual void Visit(InitDeclarator* declarator) = 0;
    virtual void Visit(Declaration* declaration) = 0;
    virtual void Visit(Expression* expression) = 0;
    virtual void Visit(IdExpression* expression) = 0;
    virtual void Visit(PrimaryExpression* expression) = 0;
    virtual void Visit(UnaryExpression* expression) = 0;
    virtual void Visit(BinaryExpression* expression) = 0;
    virtual void Visit(ConditionalExpression* expression) = 0;
    virtual void Visit(AssignmentExpression* expression) = 0;
    virtual void Visit(CompoundStatement* statement) = 0;
    virtual void Visit(ReturnStatement* statement) = 0;
    virtual void Visit(ExpressionStatement* statement) = 0;
    virtual void Visit(SelectionStatement* statement) = 0;
    virtual void Visit(JumpStatement* statement) = 0;
    virtual void Visit(WhileStatement* statement) = 0;
    virtual void Visit(ForStatement* statement) = 0;
};