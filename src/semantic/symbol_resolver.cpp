#include "include/semantic/symbol_resolver.h"

#include "include/ast/expressions.h"

SymbolResolver::SymbolResolver() { EnterScope(); }

SymbolResolver::~SymbolResolver() {}

void SymbolResolver::Visit(TranslationUnit* translation_unit) {
    for (auto& declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }
}

void SymbolResolver::Visit(ItemList* item_list) {
    for (auto& item : item_list->GetItems()) {
        item->Accept(this);
    }
}

void SymbolResolver::Visit(FunctionDefinition* function) {
    auto function_declarator =
        dynamic_cast<FunctionDeclarator*>(function->GetDeclarator());
    if (!function_declarator) {
        errors_.push_back("function definition is not a function");
        return;
    }

    std::string original_name = function_declarator->GetId();
    if (IsInCurrentScope(original_name)) {
        auto& existing_symbol = scopes_.back()[original_name];
        if (existing_symbol.linkage != SymbolInfo::LinkageKind::External) {
            errors_.push_back("duplicate declaration of variable '" + original_name +
                              "'");
            return;
        }
    }
    AddToCurrentScope(original_name, original_name, SymbolInfo::LinkageKind::External);

    EnterScope();
    suppress_next_compound_scope_ = true;
    if (function_declarator->HasParameters()) {
        function_declarator->GetParameters()->Accept(this);
    }
    function->GetBody()->Accept(this);
    ExitScope();
}

void SymbolResolver::Visit(TypeSpecification* type) {}

void SymbolResolver::Visit(Declaration* declaration) {
    declaration->GetDeclaration()->Accept(this);
}

void SymbolResolver::Visit(Expression* expression) {}

void SymbolResolver::Visit(IdExpression* expression) {
    std::string original_name = expression->GetId();
    if (!IsDeclaredInAnyScope(original_name)) {
        errors_.push_back("use of undeclared variable '" + original_name + "'");
        return;
    }
    expression->SetId(GetUniqueName(original_name));
}

void SymbolResolver::Visit(PrimaryExpression* expression) {}

void SymbolResolver::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);
}

void SymbolResolver::Visit(BinaryExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void SymbolResolver::Visit(ConditionalExpression* expression) {
    expression->GetCondition()->Accept(this);
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void SymbolResolver::Visit(AssignmentExpression* expression) {
    if (!dynamic_cast<IdExpression*>(expression->GetLeftExpression())) {
        errors_.push_back("left-hand side of assignment is not assignable");
        return;
    }
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);
}

void SymbolResolver::Visit(CompoundStatement* statement) {
    bool suppress_next_compound_scope = suppress_next_compound_scope_;
    suppress_next_compound_scope_ = false;

    if (!suppress_next_compound_scope) {
        EnterScope();
    }
    statement->GetBody()->Accept(this);
    if (!suppress_next_compound_scope) {
        ExitScope();
    }
}

void SymbolResolver::Visit(ReturnStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void SymbolResolver::Visit(ExpressionStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);
    }
}

void SymbolResolver::Visit(SelectionStatement* statement) {
    statement->GetCondition()->Accept(this);
    statement->GetThenStatement()->Accept(this);
    if (statement->HasElseStatement()) {
        statement->GetElseStatement()->Accept(this);
    }
}

void SymbolResolver::Visit(JumpStatement* statement) {}

void SymbolResolver::Visit(WhileStatement* statement) {
    statement->GetCondition()->Accept(this);
    statement->GetBody()->Accept(this);
}

void SymbolResolver::Visit(ForStatement* statement) {
    EnterScope();
    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);
    ExitScope();
}

void SymbolResolver::Visit(ParameterDeclaration* declaration) {
    declaration->GetDeclarator()->Accept(this);
}

void SymbolResolver::Visit(ParameterList* list) {
    for (auto& parameter : list->GetParameters()) {
        parameter->Accept(this);
    }
}

void SymbolResolver::Visit(FunctionCallExpression* expression) {
    if (!dynamic_cast<IdExpression*>(expression->GetFunction())) {
        errors_.push_back("function call target is not an identifier");
        return;
    }
    expression->GetFunction()->Accept(this);
    if (expression->HasArguments()) {
        expression->GetArguments()->Accept(this);
    }
}

void SymbolResolver::Visit(ArgumentExpressionList* list) {
    for (auto& argument : list->GetArguments()) {
        argument->Accept(this);
    }
}

void SymbolResolver::Visit(IdentifierDeclarator* declarator) {
    std::string original_name = declarator->GetId();
    if (IsInCurrentScope(original_name)) {
        errors_.push_back("duplicate declaration of variable '" + original_name + "'");
        return;
    }
    std::string unique_name = GenerateUniqueName(original_name);
    declarator->SetId(unique_name);
    AddToCurrentScope(original_name, unique_name, SymbolInfo::LinkageKind::None);
    if (declarator->HasInitializer()) {
        declarator->GetInitializer()->Accept(this);
    }
}

void SymbolResolver::Visit(FunctionDeclarator* declarator) {
    std::string original_name = declarator->GetId();

    if (IsInCurrentScope(original_name)) {
        auto& existing_symbol = scopes_.back()[original_name];

        if (existing_symbol.linkage != SymbolInfo::LinkageKind::External) {
            errors_.push_back("duplicate declaration of variable '" + original_name +
                              "'");
            return;
        }
    }
    AddToCurrentScope(original_name, original_name, SymbolInfo::LinkageKind::External);
    if (declarator->HasParameters()) {
        EnterScope();
        declarator->GetParameters()->Accept(this);
        ExitScope();
    }
}

const std::vector<std::string>& SymbolResolver::GetErrors() const { return errors_; }

void SymbolResolver::EnterScope() { scopes_.emplace_back(); }

void SymbolResolver::ExitScope() { scopes_.pop_back(); }

bool SymbolResolver::IsInCurrentScope(const std::string& name) const {
    return scopes_.back().count(name) > 0;
}

bool SymbolResolver::IsDeclaredInAnyScope(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->count(name) > 0) {
            return true;
        }
    }
    return false;
}

std::string SymbolResolver::GenerateUniqueName(const std::string& base) {
    int count = name_counters_[base]++;
    return base + "." + std::to_string(count);
}

std::string SymbolResolver::GetUniqueName(const std::string& original_name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->contains(original_name)) {
            return it->at(original_name).name;
        }
    }
    errors_.push_back("variable not found: " + original_name);
    return "";
}

void SymbolResolver::AddToCurrentScope(const std::string& original_name,
                                       const std::string& unique_name,
                                       SymbolInfo::LinkageKind linkage) {
    if (linkage == SymbolInfo::LinkageKind::External && IsInCurrentScope(original_name) &&
        scopes_.back()[original_name].linkage == SymbolInfo::LinkageKind::External) {
        return;
    }

    scopes_.back()[original_name] = SymbolInfo{unique_name, linkage};
}
