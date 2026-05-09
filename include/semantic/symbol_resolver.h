#pragma once

#include "include/semantic/semantic_visitor.h"
#include "include/semantic/symbol_table.h"

class SymbolResolver : public SemanticVisitor {
public:
    explicit SymbolResolver(SymbolTable& symbol_table);
    virtual ~SymbolResolver();
    void Visit(FunctionDefinition* function) override;
    void Visit(Declaration* declaration) override;
    void Visit(IdExpression* expression) override;
    void Visit(CompoundStatement* statement) override;
    void Visit(ForStatement* statement) override;
    void Visit(FunctionCallExpression* expression) override;
    void Visit(IdentifierDeclarator* declarator) override;
    void Visit(FunctionDeclarator* declarator) override;

private:
    bool suppress_next_compound_scope_ = false;
    SymbolTable& symbol_table_;
    StorageClass current_storage_class_ = StorageClass::None;
};
