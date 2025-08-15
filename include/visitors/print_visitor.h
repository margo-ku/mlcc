#pragma once

#include <iostream>

#include "visitor.h"

class PrintVisitor : public Visitor {
public:
    explicit PrintVisitor(std::ostream& stream);
    virtual void Visit(TranslationUnit* translation_unit) override;
    virtual void Visit(ItemList* item_list) override;
    virtual void Visit(FunctionDefinition* function) override;
    virtual void Visit(TypeSpecification* type) override;
    virtual void Visit(Declaration* declaration) override;
    virtual void Visit(Expression* expression) override;
    virtual void Visit(IdExpression* expression) override;
    virtual void Visit(PrimaryExpression* expression) override;
    virtual void Visit(UnaryExpression* expression) override;
    virtual void Visit(BinaryExpression* expression) override;
    virtual void Visit(ConditionalExpression* expression) override;
    virtual void Visit(AssignmentExpression* expression) override;
    virtual void Visit(CompoundStatement* statement) override;
    virtual void Visit(ReturnStatement* statement) override;
    virtual void Visit(ExpressionStatement* statement) override;
    virtual void Visit(SelectionStatement* statement) override;
    virtual void Visit(JumpStatement* statement) override;
    virtual void Visit(WhileStatement* statement) override;
    virtual void Visit(ForStatement* statement) override;
    virtual void Visit(ParameterDeclaration* declaration) override;
    virtual void Visit(ParameterList* list) override;
    virtual void Visit(FunctionCallExpression* expression) override;
    virtual void Visit(ArgumentExpressionList* list) override;
    virtual void Visit(IdentifierDeclarator* declarator) override;
    virtual void Visit(FunctionDeclarator* declarator) override;

private:
    std::ostream& stream_;
    size_t number_of_tabs_;
    void PrintTabs() const;
};