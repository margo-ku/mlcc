#pragma once

#include <optional>

#include "include/semantic/conversion_rules.h"
#include "include/semantic/semantic_visitor.h"
#include "include/semantic/symbol_table.h"
#include "include/semantic/type_resolver.h"

class ExpressionTypeChecker : public SemanticVisitor {
public:
    explicit ExpressionTypeChecker(SymbolTable& symbol_table);
    ~ExpressionTypeChecker() = default;

    void Visit(TranslationUnit* translation_unit) override;
    void Visit(FunctionDefinition* function) override;
    void Visit(Declaration* declaration) override;
    void Visit(IdExpression* expression) override;
    void Visit(PrimaryExpression* expression) override;
    void Visit(UnaryExpression* expression) override;
    void Visit(BinaryExpression* expression) override;
    void Visit(ConditionalExpression* expression) override;
    void Visit(AssignmentExpression* expression) override;
    void Visit(CastExpression* expression) override;
    void Visit(AddressExpression* expression) override;
    void Visit(DereferenceExpression* expression) override;
    void Visit(ReturnStatement* statement) override;
    void Visit(ForStatement* statement) override;
    void Visit(FunctionCallExpression* expression) override;
    void Visit(SubscriptExpression* expression) override;
    void Visit(SingleInitializer* initializer) override;
    void Visit(CompoundInitializer* initializer) override;

private:
    std::optional<ConversionRules::BinaryOpResult> CheckBinaryOperands(
        BinaryExpression::BinaryOperator op, TypeRef left_type, TypeRef right_type,
        Expression* left_expr, Expression* right_expr);
    bool CheckInitializerForObject(IdentifierDeclarator* declarator, SymbolInfo* info,
                                   StorageClass storage_class);
    bool CheckTypeNameExpressions(TypeName* type_name);
    bool CheckAbstractDeclaratorExpressions(AbstractDeclarator* declarator);

    SymbolTable& symbol_table_;
    TypeResolver type_resolver_;
    ConversionRules conversions_;
    TypeRef current_return_type_;
    bool in_file_scope_ = true;
};
