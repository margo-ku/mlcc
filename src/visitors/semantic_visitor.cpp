#include "include/visitors/semantic_visitor.h"

SemanticVisitor::SemanticVisitor() {}

#include <iostream>

void SemanticVisitor::Visit(TranslationUnit* translation_unit) {
    // to do: check function names
    for (auto* declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }
}

void SemanticVisitor::Visit(ItemList* item_list) {
    for (auto* item : item_list->GetItems()) {
        item->Accept(this);
    }
}

void SemanticVisitor::Visit(FunctionDefinition* function) {
    function->GetBody()->Accept(this);
}

void SemanticVisitor::Visit(TypeSpecification* type) {}

void SemanticVisitor::Visit(Declarator* declarator) {}

void SemanticVisitor::Visit(InitDeclarator* declarator) {
    std::string original_name = declarator->GetDeclarator()->GetId();
    if (IsInCurrentScope(original_name)) {
        throw std::runtime_error("semantic error: duplicate declaration of variable '" +
                                 original_name + "'");
    }
    std::string unique_name = GenerateUniqueName(original_name);
    declarator->GetDeclarator()->SetId(unique_name);
    AddToCurrentScope(original_name, unique_name);

    if (declarator->HasInitializer()) {
        declarator->GetInitializer()->Accept(this);
    }
}

void SemanticVisitor::Visit(Declaration* declaration) {
    declaration->GetDeclaration()->Accept(this);
}

void SemanticVisitor::Visit(Expression* expression) {}

void SemanticVisitor::Visit(IdExpression* expression) {
    std::string original_name = expression->GetId();
    if (!IsDeclaredInAnyScope(original_name)) {
        throw std::runtime_error("semantic error: use of undeclared variable '" +
                                 original_name + "'");
    }
    expression->SetId(GetUniqueName(original_name));
}

void SemanticVisitor::Visit(PrimaryExpression* expression) {}

void SemanticVisitor::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);
}

void SemanticVisitor::Visit(BinaryExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void SemanticVisitor::Visit(ConditionalExpression* expression) {
    expression->GetCondition()->Accept(this);
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void SemanticVisitor::Visit(AssignmentExpression* expression) {
    if (!dynamic_cast<IdExpression*>(expression->GetLeftExpression())) {
        throw std::runtime_error(
            "semantic error: left-hand side of assignment is not assignable");
    }
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void SemanticVisitor::Visit(CompoundStatement* statement) {
    EnterScope();
    statement->GetBody()->Accept(this);
    ExitScope();
}

void SemanticVisitor::Visit(ReturnStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void SemanticVisitor::Visit(ExpressionStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void SemanticVisitor::Visit(SelectionStatement* statement) {
    statement->GetCondition()->Accept(this);
    statement->GetThenStatement()->Accept(this);
    if (statement->HasElseStatement()) {
        statement->GetElseStatement()->Accept(this);
    }
}

void SemanticVisitor::Visit(JumpStatement* statement) {
    if (loop_ids_.empty()) {
        throw std::runtime_error("semantic error: jump statement outside of loop");
    }
    statement->SetLabel(loop_ids_.top());
}

void SemanticVisitor::Visit(WhileStatement* statement) {
    std::string loop_id = GenerateLoopId();
    statement->SetLabel(loop_id);
    loop_ids_.push(loop_id);

    statement->GetCondition()->Accept(this);
    statement->GetBody()->Accept(this);

    loop_ids_.pop();
}

void SemanticVisitor::Visit(ForStatement* statement) {
    std::string loop_id = GenerateLoopId();
    statement->SetLabel(loop_id);
    loop_ids_.push(loop_id);

    EnterScope();
    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);

    ExitScope();
    loop_ids_.pop();
}

void SemanticVisitor::EnterScope() { scopes_.emplace_back(); }

void SemanticVisitor::ExitScope() { scopes_.pop_back(); }

bool SemanticVisitor::IsInCurrentScope(const std::string& name) const {
    return scopes_.back().count(name) > 0;
}

bool SemanticVisitor::IsDeclaredInAnyScope(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->count(name) > 0) {
            return true;
        }
    }
    return false;
}

std::string SemanticVisitor::GenerateUniqueName(const std::string& base) {
    int count = name_counters_[base]++;
    return base + "." + std::to_string(count);
}

std::string SemanticVisitor::GetUniqueName(const std::string& original_name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->contains(original_name)) {
            return it->at(original_name);
        }
    }
    throw std::runtime_error("semantic error: variable not found: " + original_name);
}

void SemanticVisitor::AddToCurrentScope(const std::string& original_name,
                                        const std::string& unique_name) {
    scopes_.back()[original_name] = unique_name;
}

std::string SemanticVisitor::GenerateLoopId() {
    return "loop." + std::to_string(name_counters_["loop."]++);
}