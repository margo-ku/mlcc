#pragma once
#include <fstream>

#include "visitor.h"

class CompileVisitor : public Visitor {
public:
    explicit CompileVisitor(std::ostream& stream);
    virtual void Visit(TranslationUnit* translation_unit) override;
    virtual void Visit(ItemList* item_list) override;
    virtual void Visit(FunctionDefinition* function) override;
    virtual void Visit(TypeSpecification* type) override;
    virtual void Visit(Declarator* declarator) override;
    virtual void Visit(PrimaryExpression* expression) override;
    virtual void Visit(UnaryExpression* expression) override;
    virtual void Visit(CompoundStatement* statement) override;
    virtual void Visit(ReturnStatement* statement) override;

private:
    void PrintTabs() const;
    std::ostream& stream_;
    size_t number_of_tabs_;
};