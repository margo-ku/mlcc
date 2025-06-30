#include "include/visitors/semantic_visitor.h"

SemanticVisitor::SemanticVisitor() { scopes_.push_back({}); }

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
    std::string id = declarator->GetDeclarator()->GetId();
    if (scopes_[scopes_.size() - 1].contains(id)) {
        throw std::runtime_error("semantic error: duplicate declaration of variable '" +
                                 id + "'");
    }
    scopes_[scopes_.size() - 1].insert(id);
}

void SemanticVisitor::Visit(Declaration* declaration) {
    declaration->GetDeclaration()->Accept(this);
}

void SemanticVisitor::Visit(IdExpression* expression) {
    std::string id = expression->GetId();
    if (!scopes_[scopes_.size() - 1].contains(id)) {  // to do: expand
        throw std::runtime_error("semantic error: use of undeclared variable '" + id +
                                 "'");
    }
}  // ! 1

void SemanticVisitor::Visit(PrimaryExpression* expression) {}

void SemanticVisitor::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);
}

void SemanticVisitor::Visit(BinaryExpression* expression) {
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
}  // ! 3

void SemanticVisitor::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
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