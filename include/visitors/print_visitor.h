#pragma once
#include <fstream>

#include "visitor.h"

class PrintVisitor : public Visitor {
public:
    explicit PrintVisitor(std::ostream& stream);
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
    std::ostream& stream_;
    size_t number_of_tabs_;
    void PrintTabs() const;
};