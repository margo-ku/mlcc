#include "include/semantic/loop_analyzer.h"

#include "include/ast/expressions.h"

LoopAnalyzer::LoopAnalyzer(SymbolTable& symbol_table) : symbol_table_(symbol_table) {}

LoopAnalyzer::~LoopAnalyzer() {}

void LoopAnalyzer::Visit(TranslationUnit* translation_unit) {
    for (auto& declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }
}

void LoopAnalyzer::Visit(ItemList* item_list) {
    for (auto& item : item_list->GetItems()) {
        item->Accept(this);
    }
}

void LoopAnalyzer::Visit(FunctionDefinition* function) {
    function->GetBody()->Accept(this);
}

void LoopAnalyzer::Visit(TypeSpecification* type) {}

void LoopAnalyzer::Visit(Declaration* declaration) {
    declaration->GetDeclaration()->Accept(this);
}

void LoopAnalyzer::Visit(Expression* expression) {}

void LoopAnalyzer::Visit(IdExpression* expression) {}

void LoopAnalyzer::Visit(PrimaryExpression* expression) {}

void LoopAnalyzer::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);
}

void LoopAnalyzer::Visit(BinaryExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void LoopAnalyzer::Visit(ConditionalExpression* expression) {
    expression->GetCondition()->Accept(this);
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void LoopAnalyzer::Visit(AssignmentExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void LoopAnalyzer::Visit(CastExpression* expression) {
    expression->GetType()->Accept(this);
    expression->GetExpression()->Accept(this);
}

void LoopAnalyzer::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
}

void LoopAnalyzer::Visit(ReturnStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void LoopAnalyzer::Visit(ExpressionStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void LoopAnalyzer::Visit(SelectionStatement* statement) {
    statement->GetCondition()->Accept(this);
    statement->GetThenStatement()->Accept(this);
    if (statement->HasElseStatement()) {
        statement->GetElseStatement()->Accept(this);
    }
}

void LoopAnalyzer::Visit(JumpStatement* statement) {
    if (loop_ids_.empty()) {
        errors_.push_back("jump statement outside of loop");
        return;
    }
    statement->SetLabel(loop_ids_.top());
}

void LoopAnalyzer::Visit(WhileStatement* statement) {
    std::string loop_id = GenerateLoopId();
    statement->SetLabel(loop_id);
    loop_ids_.push(loop_id);

    statement->GetCondition()->Accept(this);
    statement->GetBody()->Accept(this);

    loop_ids_.pop();
}

void LoopAnalyzer::Visit(ForStatement* statement) {
    std::string loop_id = GenerateLoopId();
    statement->SetLabel(loop_id);
    loop_ids_.push(loop_id);

    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);

    loop_ids_.pop();
}

void LoopAnalyzer::Visit(ParameterDeclaration* declaration) {
    declaration->GetDeclarator()->Accept(this);
}

void LoopAnalyzer::Visit(ParameterList* list) {
    for (auto& parameter : list->GetParameters()) {
        parameter->Accept(this);
    }
}

void LoopAnalyzer::Visit(FunctionCallExpression* expression) {
    expression->GetFunction()->Accept(this);
    if (expression->HasArguments()) {
        expression->GetArguments()->Accept(this);
    }
}

void LoopAnalyzer::Visit(ArgumentExpressionList* list) {
    for (auto& argument : list->GetArguments()) {
        argument->Accept(this);
    }
}

void LoopAnalyzer::Visit(IdentifierDeclarator* declarator) {
    if (declarator->HasInitializer()) {
        declarator->GetInitializer()->Accept(this);
    }
}

void LoopAnalyzer::Visit(FunctionDeclarator* declarator) {
    if (declarator->HasParameters()) {
        declarator->GetParameters()->Accept(this);
    }
}

const std::vector<std::string>& LoopAnalyzer::GetErrors() const { return errors_; }

std::string LoopAnalyzer::GenerateLoopId() {
    return "loop." + std::to_string(loop_id_counter_++);
}