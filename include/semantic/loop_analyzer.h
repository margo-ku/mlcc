#pragma once

#include <stack>

#include "include/visitors/visitor.h"

class LoopAnalyzer : public Visitor {
public:
    LoopAnalyzer();
    virtual ~LoopAnalyzer();
    void Visit(TranslationUnit* translation_unit) override;
    void Visit(ItemList* item_list) override;
    void Visit(FunctionDefinition* function) override;
    void Visit(TypeSpecification* type) override;
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
    std::stack<std::string> loop_ids_;
    size_t loop_id_counter_ = 0;

    std::string GenerateLoopId();
};