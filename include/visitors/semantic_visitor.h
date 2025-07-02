#pragma once
#include <fstream>
#include <set>
#include <vector>

#include "frame_info.h"
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
    virtual void Visit(AssignmentExpression* expression) override;
    virtual void Visit(CompoundStatement* statement) override;
    virtual void Visit(ReturnStatement* statement) override;
    virtual void Visit(ExpressionStatement* statement) override;

    FrameInfo GetFrameInfo();

private:
    bool HasVariable(const std::string& id) const;
    void AddVariable(const std::string& id);
    FrameInfo frame_info_;
};