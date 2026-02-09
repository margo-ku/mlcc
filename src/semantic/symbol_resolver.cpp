#include "include/semantic/symbol_resolver.h"

#include "include/ast/declarations.h"
#include "include/ast/expressions.h"

SymbolResolver::SymbolResolver(SymbolTable& symbol_table) : symbol_table_(symbol_table) {
    symbol_table_.EnterScope();
}

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

    if (!symbol_table_.IsInFileScope()) {
        errors_.push_back("function definition is not allowed in local scope");
        return;
    }

    std::string name = function_declarator->GetId();
    if (symbol_table_.IsInCurrentScope(name)) {
        auto existing = symbol_table_.Lookup(name);
        if (existing->linkage != SymbolInfo::LinkageKind::External) {
            errors_.push_back("conflicting declaration of '" + name + "'");
            return;
        }
    } else {
        symbol_table_.Declare(name, {.name = name,
                                     .original_name = name,
                                     .linkage = SymbolInfo::LinkageKind::External});
    }

    symbol_table_.EnterScope();
    suppress_next_compound_scope_ = true;
    if (function_declarator->HasParameters()) {
        function_declarator->GetParameters()->Accept(this);
    }
    function->GetBody()->Accept(this);
    symbol_table_.ExitScope();
}

void SymbolResolver::Visit(DeclarationSpecifiers* decl_specs) {}

void SymbolResolver::Visit(TypeSpecification* type) {}

void SymbolResolver::Visit(Declaration* declaration) {
    StorageClass saved_storage_class = current_storage_class_;
    current_storage_class_ = declaration->GetDeclarationSpecifiers()->GetStorageClass();

    declaration->GetDeclaration()->Accept(this);

    current_storage_class_ = saved_storage_class;
}

void SymbolResolver::Visit(Expression* expression) {}

void SymbolResolver::Visit(IdExpression* expression) {
    std::string original_name = expression->GetId();
    auto info = symbol_table_.Lookup(original_name);
    if (!info) {
        errors_.push_back("use of undeclared variable '" + original_name + "'");
        return;
    }
    expression->SetId(info->name);
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

void SymbolResolver::Visit(CastExpression* expression) {
    expression->GetType()->Accept(this);
    expression->GetExpression()->Accept(this);
}

void SymbolResolver::Visit(CompoundStatement* statement) {
    bool suppress_next_compound_scope = suppress_next_compound_scope_;
    suppress_next_compound_scope_ = false;

    if (!suppress_next_compound_scope) {
        symbol_table_.EnterScope();
    }
    statement->GetBody()->Accept(this);
    if (!suppress_next_compound_scope) {
        symbol_table_.ExitScope();
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
    symbol_table_.EnterScope();
    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);
    symbol_table_.ExitScope();
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
    bool has_linkage =
        symbol_table_.IsInFileScope() || current_storage_class_ == StorageClass::Extern;

    if (symbol_table_.IsInCurrentScope(original_name)) {
        auto existing = symbol_table_.Lookup(original_name);
        if (!has_linkage || !existing->HasLinkage()) {
            errors_.push_back("conflicting declaration of '" + original_name + "'");
            return;
        }
    } else {
        std::string resolved_name =
            has_linkage ? original_name : symbol_table_.GenerateUniqueName(original_name);
        declarator->SetId(resolved_name);

        symbol_table_.Declare(original_name,
                              {.name = resolved_name,
                               .original_name = original_name,
                               .linkage = has_linkage ? SymbolInfo::LinkageKind::External
                                                      : SymbolInfo::LinkageKind::None});
    }

    if (declarator->HasInitializer()) {
        declarator->GetInitializer()->Accept(this);
    }
}

void SymbolResolver::Visit(FunctionDeclarator* declarator) {
    std::string original_name = declarator->GetId();

    if (current_storage_class_ == StorageClass::Static &&
        !symbol_table_.IsInFileScope()) {
        errors_.push_back("static function declaration is not allowed in local scope");
        return;
    }

    if (symbol_table_.IsInCurrentScope(original_name)) {
        auto existing = symbol_table_.Lookup(original_name);
        if (existing->linkage != SymbolInfo::LinkageKind::External) {
            errors_.push_back("conflicting declaration of '" + original_name + "'");
            return;
        }
    } else {
        symbol_table_.Declare(original_name,
                              {.name = original_name,
                               .original_name = original_name,
                               .linkage = SymbolInfo::LinkageKind::External});
    }

    if (declarator->HasParameters()) {
        StorageClass saved_storage_class = current_storage_class_;
        current_storage_class_ = StorageClass::None;
        symbol_table_.EnterScope();
        declarator->GetParameters()->Accept(this);
        symbol_table_.ExitScope();
        current_storage_class_ = saved_storage_class;
    }
}

const std::vector<std::string>& SymbolResolver::GetErrors() const { return errors_; }