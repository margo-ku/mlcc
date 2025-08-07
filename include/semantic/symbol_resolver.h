#pragma once

#include <unordered_map>
#include <vector>

#include "include/visitors/visitor.h"

class SymbolResolver : public Visitor {
public:
    SymbolResolver();
    virtual ~SymbolResolver();
    void Visit(TranslationUnit* translation_unit) override;
    void Visit(ItemList* item_list) override;
    void Visit(FunctionDefinition* function) override;
    void Visit(TypeSpecification* type) override;
    void Visit(Declarator* declarator) override;
    void Visit(InitDeclarator* declarator) override;
    void Visit(Declaration* declaration) override;
    void Visit(Expression* expression) override;
    void Visit(IdExpression* expression) override;
    void Visit(PrimaryExpression* expression) override;
    void Visit(UnaryExpression* expression) override;
    void Visit(BinaryExpression* expression) override;
    void Visit(ConditionalExpression* expression) override;
    void Visit(AssignmentExpression* expression) override;
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
    std::vector<std::string> errors_;
    std::vector<std::unordered_map<std::string, std::string>> scopes_;
    std::unordered_map<std::string, int> name_counters_;

    void EnterScope();
    void ExitScope();

    bool IsInCurrentScope(const std::string& id) const;
    bool IsDeclaredInAnyScope(const std::string& id) const;

    void AddToCurrentScope(const std::string& original_name,
                           const std::string& unique_name);
    std::string GenerateUniqueName(const std::string& base);
    std::string GetUniqueName(const std::string& original_name);
};
