#include "include/semantic/type_checker.h"

TypeChecker::TypeChecker() {}

TypeChecker::~TypeChecker() {}

void TypeChecker::Visit(TranslationUnit* translation_unit) {
    for (auto& declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }
}

void TypeChecker::Visit(ItemList* item_list) {
    for (auto& item : item_list->GetItems()) {
        item->Accept(this);
    }
}

void TypeChecker::Visit(FunctionDefinition* function) {
    function->GetBody()->Accept(this);
}

void TypeChecker::Visit(TypeSpecification* type) {}

void TypeChecker::Visit(Declarator* declarator) {}

void TypeChecker::Visit(InitDeclarator* declarator) {
    if (declarator->HasInitializer()) {
        declarator->GetInitializer()->Accept(this);
    }
}

void TypeChecker::Visit(Declaration* declaration) {
    declaration->GetDeclaration()->Accept(this);
}

void TypeChecker::Visit(Expression* expression) {}

void TypeChecker::Visit(IdExpression* expression) {}

void TypeChecker::Visit(PrimaryExpression* expression) {}

void TypeChecker::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);
}

void TypeChecker::Visit(BinaryExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void TypeChecker::Visit(ConditionalExpression* expression) {
    expression->GetCondition()->Accept(this);
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void TypeChecker::Visit(AssignmentExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void TypeChecker::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
}

void TypeChecker::Visit(ReturnStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void TypeChecker::Visit(ExpressionStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void TypeChecker::Visit(SelectionStatement* statement) {
    statement->GetCondition()->Accept(this);
    statement->GetThenStatement()->Accept(this);
    if (statement->HasElseStatement()) {
        statement->GetElseStatement()->Accept(this);
    }
}

void TypeChecker::Visit(JumpStatement* statement) {}

void TypeChecker::Visit(WhileStatement* statement) {
    statement->GetCondition()->Accept(this);
    statement->GetBody()->Accept(this);
}

void TypeChecker::Visit(ForStatement* statement) {
    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);
}

void TypeChecker::Visit(ParameterDeclaration* declaration) {
    declaration->GetDeclarator()->Accept(this);
}

void TypeChecker::Visit(ParameterList* list) {
    for (auto& parameter : list->GetParameters()) {
        parameter->Accept(this);
    }
}

void TypeChecker::Visit(FunctionCallExpression* expression) {
    expression->GetFunction()->Accept(this);
    if (expression->HasArguments()) {
        expression->GetArguments()->Accept(this);
    }
}

void TypeChecker::Visit(ArgumentExpressionList* list) {
    for (auto& argument : list->GetArguments()) {
        argument->Accept(this);
    }
}

void TypeChecker::Visit(IdentifierDeclarator* declarator) {}

void TypeChecker::Visit(FunctionDeclarator* declarator) {
    if (declarator->HasParameters()) {
        declarator->GetParameters()->Accept(this);
    }
}

const std::vector<std::string>& TypeChecker::GetErrors() const { return errors_; }