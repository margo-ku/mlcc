#pragma once
#include <fstream>

#include "frame_info.h"
#include "visitor.h"

class CompileVisitor : public Visitor {
public:
    explicit CompileVisitor(std::ostream& stream, FrameInfo frame_info);
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

private:
    void ProcessBinaryOr(BinaryExpression* expression);
    void ProcessBinaryAnd(BinaryExpression* expression);
    std::string GetUniqueLabelId();

    void PrintTabs() const;
    void PrintToStream(const std::string&) const;
    std::ostream& stream_;
    FrameInfo frame_info_;
    size_t number_of_tabs_;
    size_t unique_label_id_;
    std::string function_exit_label_;
};