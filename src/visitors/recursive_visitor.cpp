#include "include/visitors/recursive_visitor.h"

#include "include/ast/expressions.h"

void RecursiveVisitor::Visit(TranslationUnit* translation_unit) {
    for (auto& declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }
}

void RecursiveVisitor::Visit(ItemList* item_list) {
    for (auto& item : item_list->GetItems()) {
        item->Accept(this);
    }
}

void RecursiveVisitor::Visit(FunctionDefinition* function) {
    function->GetBody()->Accept(this);
}

void RecursiveVisitor::Visit(DeclarationSpecifiers* decl_specs) {}

void RecursiveVisitor::Visit(TypeSpecification* type) {}

void RecursiveVisitor::Visit(TypeName* type) {
    type->GetTypeSpecification()->Accept(this);
    if (type->HasAbstractDeclarator()) {
        type->GetAbstractDeclarator()->Accept(this);
    }
}

void RecursiveVisitor::Visit(PointerAbstractDeclarator* declarator) {
    if (declarator->HasBase()) {
        declarator->GetBase()->Accept(this);
    }
}

void RecursiveVisitor::Visit(ArrayAbstractDeclarator* declarator) {
    if (declarator->HasBase()) {
        declarator->GetBase()->Accept(this);
    }
    if (declarator->GetSize()) {
        declarator->GetSize()->Accept(this);
    }
}

void RecursiveVisitor::Visit(Declaration* declaration) {
    declaration->GetDeclaration()->Accept(this);
}

void RecursiveVisitor::Visit(Expression* expression) {}

void RecursiveVisitor::Visit(IdExpression* expression) {}

void RecursiveVisitor::Visit(PrimaryExpression* expression) {}

void RecursiveVisitor::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);
}

void RecursiveVisitor::Visit(BinaryExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void RecursiveVisitor::Visit(ConditionalExpression* expression) {
    expression->GetCondition()->Accept(this);
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void RecursiveVisitor::Visit(AssignmentExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void RecursiveVisitor::Visit(CastExpression* expression) {
    expression->GetTypeName()->Accept(this);
    expression->GetExpression()->Accept(this);
}

void RecursiveVisitor::Visit(AddressExpression* expression) {
    expression->GetExpression()->Accept(this);
}

void RecursiveVisitor::Visit(DereferenceExpression* expression) {
    expression->GetExpression()->Accept(this);
}

void RecursiveVisitor::Visit(SubscriptExpression* expression) {
    expression->GetArrayExpression()->Accept(this);
    expression->GetIndexExpression()->Accept(this);
}

void RecursiveVisitor::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
}

void RecursiveVisitor::Visit(ReturnStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void RecursiveVisitor::Visit(ExpressionStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}
void RecursiveVisitor::Visit(SelectionStatement* statement) {
    statement->GetCondition()->Accept(this);
    statement->GetThenStatement()->Accept(this);
    if (statement->HasElseStatement()) {
        statement->GetElseStatement()->Accept(this);
    }
}

void RecursiveVisitor::Visit(JumpStatement* statement) {}

void RecursiveVisitor::Visit(WhileStatement* statement) {
    statement->GetCondition()->Accept(this);
    statement->GetBody()->Accept(this);
}

void RecursiveVisitor::Visit(ForStatement* statement) {
    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);
}

void RecursiveVisitor::Visit(ParameterDeclaration* declaration) {
    declaration->GetDeclarator()->Accept(this);
}

void RecursiveVisitor::Visit(ParameterList* list) {
    for (auto& parameter : list->GetParameters()) {
        parameter->Accept(this);
    }
}

void RecursiveVisitor::Visit(FunctionCallExpression* expression) {
    expression->GetFunction()->Accept(this);
    if (expression->HasArguments()) {
        expression->GetArguments()->Accept(this);
    }
}

void RecursiveVisitor::Visit(ArgumentExpressionList* list) {
    for (auto& argument : list->GetArguments()) {
        argument->Accept(this);
    }
}

void RecursiveVisitor::Visit(IdentifierDeclarator* declarator) {
    if (declarator->HasInitializer()) {
        declarator->GetInitializer()->Accept(this);
    }
}

void RecursiveVisitor::Visit(FunctionDeclarator* declarator) {
    if (declarator->HasParameters()) {
        declarator->GetParameters()->Accept(this);
    }
}

void RecursiveVisitor::Visit(PointerDeclarator* declarator) {
    declarator->GetDeclarator()->Accept(this);
}

void RecursiveVisitor::Visit(Initializer* initializer) {}

void RecursiveVisitor::Visit(SingleInitializer* initializer) {
    initializer->GetExpression()->Accept(this);
}

void RecursiveVisitor::Visit(CompoundInitializer* initializer) {
    for (auto& item : initializer->GetInitializers()) {
        item->Accept(this);
    }
}