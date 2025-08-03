#pragma once
#include <unordered_map>
#include <vector>

#include "include/asm/allocator.h"
#include "visitor.h"

class SemanticVisitor : public Visitor {
public:
    SemanticVisitor();
    virtual void Visit(TranslationUnit* translation_unit) override;
    virtual void Visit(ItemList* item_list) override;
    virtual void Visit(FunctionDefinition* function) override;
    virtual void Visit(TypeSpecification* type) override;
    virtual void Visit(Declarator* declarator) override;
    virtual void Visit(InitDeclarator* declarator) override;
    virtual void Visit(Declaration* declaration) override;
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

private:
    void EnterScope();
    void ExitScope();

    bool IsInCurrentScope(const std::string& id) const;
    bool IsDeclaredInAnyScope(const std::string& id) const;

    std::string GenerateUniqueName(const std::string& base);
    std::string GetUniqueName(const std::string& base) const;

    void AddToCurrentScope(const std::string& original_name,
                           const std::string& unique_name);

    std::vector<std::unordered_map<std::string, std::string>> scopes_;
    std::unordered_map<std::string, int> name_counters_;
};