#pragma once
#include "include/ast/expressions.h"
#include "visitor.h"

class RecursiveVisitor : public Visitor {
public:
    void Visit(TranslationUnit* translation_unit) override;
    void Visit(ItemList* item_list) override;
    void Visit(FunctionDefinition* function) override;
    void Visit(DeclarationSpecifiers* decl_specs) override;
    void Visit(TypeSpecification* type) override;
    void Visit(TypeName* type) override;
    void Visit(PointerAbstractDeclarator* declarator) override;
    void Visit(ArrayAbstractDeclarator* declarator) override;
    void Visit(Declaration* declaration) override;
    void Visit(Expression* expression) override;
    void Visit(IdExpression* expression) override;
    void Visit(PrimaryExpression* expression) override;
    void Visit(UnaryExpression* expression) override;
    void Visit(BinaryExpression* expression) override;
    void Visit(ConditionalExpression* expression) override;
    void Visit(AssignmentExpression* expression) override;
    void Visit(CastExpression* expression) override;
    void Visit(AddressExpression* expression) override;
    void Visit(DereferenceExpression* expression) override;
    void Visit(SubscriptExpression* expression) override;
    void Visit(FunctionCallExpression* expression) override;
    void Visit(CompoundStatement* statement) override;
    void Visit(ReturnStatement* statement) override;
    void Visit(ExpressionStatement* statement) override;
    void Visit(SelectionStatement* statement) override;
    void Visit(JumpStatement* statement) override;
    void Visit(WhileStatement* statement) override;
    void Visit(ForStatement* statement) override;
    void Visit(ParameterDeclaration* declaration) override;
    void Visit(ParameterList* list) override;
    void Visit(ArgumentExpressionList* list) override;
    void Visit(IdentifierDeclarator* declarator) override;
    void Visit(FunctionDeclarator* declarator) override;
    void Visit(PointerDeclarator* declarator) override;
    void Visit(Initializer* initializer) override;
    void Visit(SingleInitializer* initializer) override;
    void Visit(CompoundInitializer* initializer) override;
};