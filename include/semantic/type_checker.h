#pragma once

#include <memory>
#include <vector>

#include "include/semantic/symbol_table.h"
#include "include/visitors/visitor.h"

class TypeChecker : public Visitor {
public:
    explicit TypeChecker(SymbolTable& symbol_table);
    ~TypeChecker();
    void Visit(TranslationUnit* translation_unit) override;
    void Visit(ItemList* item_list) override;
    void Visit(FunctionDefinition* function) override;
    void Visit(TypeSpecification* type) override;
    void Visit(Declaration* declaration) override;
    void Visit(Expression* expression) override;
    void Visit(IdExpression* expression) override;
    void Visit(PrimaryExpression* expression) override;
    void Visit(UnaryExpression* expression) override;
    void Visit(BinaryExpression* expression) override;
    void Visit(ConditionalExpression* expression) override;
    void Visit(AssignmentExpression* expression) override;
    void Visit(CastExpression* expression) override;
    void Visit(CompoundStatement* statement) override;
    void Visit(ReturnStatement* statement) override;
    void Visit(ExpressionStatement* statement) override;
    void Visit(SelectionStatement* statement) override;
    void Visit(JumpStatement* statement) override;
    void Visit(WhileStatement* statement) override;
    void Visit(ForStatement* statement) override;
    void Visit(ParameterDeclaration* declaration) override;
    void Visit(ParameterList* list) override;
    void Visit(FunctionCallExpression* expression) override;
    void Visit(ArgumentExpressionList* list) override;
    void Visit(IdentifierDeclarator* declarator) override;
    void Visit(FunctionDeclarator* declarator) override;

    const std::vector<std::string>& GetErrors() const;

private:
    TypeRef ResolvePrimitiveType(TypeSpecification* type);
    std::unique_ptr<TypeSpecification> GetTypeSpecification(TypeRef type);
    TypeRef GetCommonType(TypeRef type1, TypeRef type2);
    TypeRef ResolveFunctionType(Declarator* declarator, TypeSpecification* return_type);
    void ReportError(const std::string& message);
    bool CanCast(TypeRef from, TypeRef to);
    std::unique_ptr<Expression> WrapWithCast(std::unique_ptr<Expression> expression,
                                             TypeRef target_type);
    bool ProcessFunctionDeclaration(FunctionDeclarator* func_declarator,
                                    TypeSpecification* return_type_spec,
                                    bool is_definition);
    bool ProcessIdentifierDeclaration(IdentifierDeclarator* id_declarator,
                                      TypeRef declared_type);

    std::vector<std::string> errors_;
    SymbolTable& symbol_table_;
    TypeRef current_return_type_;
};