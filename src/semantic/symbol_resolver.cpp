#include "include/semantic/symbol_resolver.h"

#include "include/ast/declarations.h"
#include "include/ast/expressions.h"

namespace {

FunctionDeclarator* GetFunctionDeclarator(Declarator* declarator) {
    if (auto* func = dynamic_cast<FunctionDeclarator*>(declarator)) {
        return func;
    }
    if (auto* ptr = dynamic_cast<PointerDeclarator*>(declarator)) {
        return GetFunctionDeclarator(ptr->GetDeclarator());
    }
    return nullptr;
}

}  // namespace

SymbolResolver::SymbolResolver(SymbolTable& symbol_table) : symbol_table_(symbol_table) {
    symbol_table_.EnterScope();
}

SymbolResolver::~SymbolResolver() {}

void SymbolResolver::Visit(FunctionDefinition* function) {
    auto function_declarator = GetFunctionDeclarator(function->GetDeclarator());
    if (!function_declarator) {
        ReportError("function definition is not a function");
        return;
    }

    if (!symbol_table_.IsInFileScope()) {
        ReportError("function definition is not allowed in local scope");
        return;
    }

    std::string name = function_declarator->GetId();
    if (symbol_table_.IsInCurrentScope(name)) {
        auto existing = symbol_table_.Lookup(name);
        if (existing->linkage != SymbolInfo::LinkageKind::External) {
            ReportError("conflicting declaration of '" + name + "'");
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

void SymbolResolver::Visit(Declaration* declaration) {
    StorageClass saved_storage_class = current_storage_class_;
    current_storage_class_ = declaration->GetDeclarationSpecifiers()->GetStorageClass();

    declaration->GetDeclaration()->Accept(this);

    current_storage_class_ = saved_storage_class;
}

void SymbolResolver::Visit(IdExpression* expression) {
    std::string original_name = expression->GetId();
    auto info = symbol_table_.Lookup(original_name);
    if (!info) {
        ReportError("use of undeclared variable '" + original_name + "'");
        return;
    }
    expression->SetId(info->name);
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

void SymbolResolver::Visit(ForStatement* statement) {
    symbol_table_.EnterScope();
    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);
    symbol_table_.ExitScope();
}

void SymbolResolver::Visit(FunctionCallExpression* expression) {
    if (!dynamic_cast<IdExpression*>(expression->GetFunction())) {
        ReportError("function call target is not an identifier");
        return;
    }
    SemanticVisitor::Visit(expression);
}

void SymbolResolver::Visit(IdentifierDeclarator* declarator) {
    std::string original_name = declarator->GetId();
    bool has_linkage =
        symbol_table_.IsInFileScope() || current_storage_class_ == StorageClass::Extern;

    if (symbol_table_.IsInCurrentScope(original_name)) {
        auto existing = symbol_table_.Lookup(original_name);
        if (!has_linkage || !existing->HasLinkage()) {
            ReportError("conflicting declaration of '" + original_name + "'");
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
        ReportError("static function declaration is not allowed in local scope");
        return;
    }

    if (symbol_table_.IsInCurrentScope(original_name)) {
        auto existing = symbol_table_.Lookup(original_name);
        if (existing->linkage != SymbolInfo::LinkageKind::External) {
            ReportError("conflicting declaration of '" + original_name + "'");
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
