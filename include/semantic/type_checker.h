#pragma once

#include <memory>
#include <optional>
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
    void Visit(DeclarationSpecifiers* decl_specs) override;
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
    void Visit(AddressExpression* expression) override;
    void Visit(DereferenceExpression* expression) override;
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
    void Visit(PointerDeclarator* declarator) override;
    void Visit(TypeName* type_name) override;
    void Visit(PointerAbstractDeclarator* declarator) override;

    const std::vector<std::string>& GetErrors() const;

private:
    TypeRef ResolvePrimitiveType(TypeSpecification* type);
    TypeRef ResolveTypeName(TypeName* type_name);
    TypeRef ResolveDeclaratorType(TypeRef base_type, Declarator* declarator);
    TypeRef ResolveAbstractDeclaratorType(TypeRef base_type,
                                          AbstractDeclarator* declarator);
    TypeRef ResolveFunctionType(Declarator* full_declarator,
                                FunctionDeclarator* func_declarator,
                                TypeSpecification* return_type);
    void ReportError(const std::string& message);

    std::unique_ptr<TypeName> GetTypeName(TypeRef type);
    IdentifierDeclarator* GetIdentifierDeclarator(Declarator* declarator);
    TypeRef GetCommonType(TypeRef type1, TypeRef type2);
    TypeRef GetCommonPointerType(Expression* left, Expression* right);

    bool IsNullPointerConstant(const Expression* expression) const;
    bool IsLValue(Expression* expression) const;
    bool CanCast(TypeRef from, TypeRef to);

    std::unique_ptr<Expression> ConvertByAssignment(
        std::unique_ptr<Expression> expression, TypeRef target_type);
    std::unique_ptr<Expression> WrapWithCast(std::unique_ptr<Expression> expression,
                                             TypeRef target_type);
    std::unique_ptr<Expression> PerformCompileTimeCast(
        std::unique_ptr<Expression> expression, TypeRef target_type);

    struct BinaryOpResult {
        TypeRef operand_type;
        TypeRef result_type;
    };
    std::optional<BinaryOpResult> CheckBinaryOperands(BinaryExpression::BinaryOperator op,
                                                      TypeRef left_type,
                                                      TypeRef right_type,
                                                      Expression* left_expr,
                                                      Expression* right_expr);

    bool ProcessFunctionDeclaration(FunctionDeclarator* func_declarator,
                                    TypeSpecification* return_type_spec,
                                    StorageClass storage_class, bool is_definition,
                                    Declarator* full_declarator = nullptr);
    bool ProcessFileScopeVariable(IdentifierDeclarator* id_declarator,
                                  TypeRef declared_type, StorageClass storage_class);
    bool ProcessBlockScopeVariable(IdentifierDeclarator* id_declarator,
                                   TypeRef declared_type, StorageClass storage_class);

    std::vector<std::string> errors_;
    SymbolTable& symbol_table_;
    TypeRef current_return_type_;
    bool in_file_scope_ = true;
};