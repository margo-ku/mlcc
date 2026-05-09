#pragma once

#include "include/semantic/semantic_visitor.h"
#include "include/semantic/symbol_table.h"
#include "include/semantic/type_resolver.h"

class DeclarationTypeChecker : public SemanticVisitor {
public:
    explicit DeclarationTypeChecker(SymbolTable& symbol_table);
    ~DeclarationTypeChecker() = default;

    void Visit(TranslationUnit* translation_unit) override;
    void Visit(FunctionDefinition* function) override;
    void Visit(Declaration* declaration) override;
    void Visit(IdentifierDeclarator* declarator) override;
    void Visit(FunctionDeclarator* declarator) override;
    void Visit(ParameterDeclaration* declaration) override;
    void Visit(CompoundStatement* statement) override;
    void Visit(ReturnStatement* statement) override;
    void Visit(ExpressionStatement* statement) override;
    void Visit(SelectionStatement* statement) override;
    void Visit(WhileStatement* statement) override;
    void Visit(ForStatement* statement) override;

private:
    bool BindFunction(FunctionDeclarator* function_declarator,
                      TypeSpecification* return_type_spec,
                      StorageClass storage_class, bool is_definition,
                      Declarator* full_declarator = nullptr);
    bool BindFileScopeObject(IdentifierDeclarator* id_declarator,
                             TypeRef declared_type, StorageClass storage_class);
    bool BindBlockScopeObject(IdentifierDeclarator* id_declarator,
                              TypeRef declared_type, StorageClass storage_class);
    SymbolInfo* RequireSymbol(const std::string& unique_name);

    SymbolTable& symbol_table_;
    TypeResolver type_resolver_;
    bool in_file_scope_ = true;
};
