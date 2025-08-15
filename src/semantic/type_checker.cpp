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
    function->GetDeclarator()->Accept(this);
    std::string func_name = function->GetDeclarator()->GetId();
    auto func_type = dynamic_cast<FunctionType*>(symbol_table_[func_name].get());
    if (!func_type) {
        errors_.push_back("'" + func_name + "' is not a function");
        return;
    }
    if (func_type->IsDefined()) {
        errors_.push_back("function '" + function->GetDeclarator()->GetId() +
                          "' is already defined");
        return;
    }
    func_type->SetDefined();
    function->GetBody()->Accept(this);
}

void TypeChecker::Visit(TypeSpecification* type) {}

void TypeChecker::Visit(Declaration* declaration) {
    declaration->GetDeclaration()->Accept(this);
}

void TypeChecker::Visit(Expression* expression) {}

void TypeChecker::Visit(IdExpression* expression) {
    if (symbol_table_.contains(expression->GetId())) {
        auto var_type =
            dynamic_cast<VariableType*>(symbol_table_[expression->GetId()].get());
        if (!var_type || var_type->type != VariableType::PrimitiveType::Int) {
            errors_.push_back("variable '" + expression->GetId() + "' is not int");
            return;
        }
    }
}

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
    BaseElement* init = statement->GetInit();
    if (!dynamic_cast<Declaration*>(init) && !dynamic_cast<Expression*>(init)) {
        errors_.push_back("for statement has invalid initialization");
        return;
    }

    if (auto decl = dynamic_cast<Declaration*>(init)) {
        if (dynamic_cast<FunctionDeclarator*>(decl->GetDeclaration())) {
            errors_.push_back("function declaration in for loop header");
            return;
        }
    }
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
    auto func_expr = dynamic_cast<IdExpression*>(expression->GetFunction());
    std::string id = func_expr->GetId();
    if (!symbol_table_.contains(id)) {
        errors_.push_back("function '" + id + "' is not declared");
        return;
    }
    auto func_type = dynamic_cast<FunctionType*>(symbol_table_[id].get());
    if (!func_type) {
        errors_.push_back("'" + id + "' is not a function");
        return;
    }

    int args_count = 0;
    if (expression->HasArguments()) {
        args_count = expression->GetArguments()->GetArguments().size();
    }
    if (args_count != func_type->parameter_count) {
        errors_.push_back("function '" + id + "' expects " +
                          std::to_string(func_type->parameter_count) +
                          " arguments, but " + std::to_string(args_count) +
                          " were provided");
        return;
    }
    if (expression->HasArguments()) {
        expression->GetArguments()->Accept(this);
    }
}

void TypeChecker::Visit(ArgumentExpressionList* list) {
    for (auto& argument : list->GetArguments()) {
        argument->Accept(this);
    }
}

void TypeChecker::Visit(IdentifierDeclarator* declarator) {
    std::string id = declarator->GetId();
    if (symbol_table_.contains(id)) {
        errors_.push_back("variable '" + id + "' is already declared");
        return;
    }
    symbol_table_[id] = std::make_unique<VariableType>(VariableType::PrimitiveType::Int);
    if (declarator->HasInitializer()) {
        declarator->GetInitializer()->Accept(this);
    }
}

void TypeChecker::Visit(FunctionDeclarator* declarator) {
    Declarator* inner = declarator->GetDeclarator();
    if (IsFunctionDeclarator(inner)) {
        errors_.push_back("function '" + declarator->GetId() +
                          "' cannot return function");
        return;
    }

    int param_count = 0;
    if (declarator->HasParameters()) {
        param_count = declarator->GetParameters()->GetParameters().size();
    }

    std::string id = declarator->GetId();
    if (symbol_table_.contains(id)) {
        auto existing = symbol_table_[id].get();
        if (existing->IsVariable()) {
            errors_.push_back("variable '" + id + "' is already declared");
            return;
        }

        auto prev_func = dynamic_cast<FunctionType*>(existing);
        if (!prev_func) {
            errors_.push_back("internal type error for '" + id + "'");
            return;
        }

        if (prev_func->parameter_count != param_count) {
            errors_.push_back("function '" + id + "' has different parameter count");
            return;
        }
    } else {
        symbol_table_[id] = std::make_unique<FunctionType>(param_count);
    }
}

const std::vector<std::string>& TypeChecker::GetErrors() const { return errors_; }

bool TypeChecker::IsFunctionDeclarator(Declarator* declarator) {
    return dynamic_cast<FunctionDeclarator*>(declarator) != nullptr;
}