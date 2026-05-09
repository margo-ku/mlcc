#include "include/semantic/declaration_type_checker.h"

#include "include/ast/declarations.h"
#include "include/ast/expressions.h"
#include "include/semantic/declarator_utils.h"

DeclarationTypeChecker::DeclarationTypeChecker(SymbolTable& symbol_table)
    : symbol_table_(symbol_table),
      type_resolver_([this](const std::string& message) { ReportError(message); }) {}

void DeclarationTypeChecker::Visit(TranslationUnit* translation_unit) {
    in_file_scope_ = true;
    for (auto& declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }
}

void DeclarationTypeChecker::Visit(FunctionDefinition* function) {
    function->GetDeclarator()->Accept(this);

    auto* function_declarator =
        declarator_utils::FindFunctionDeclarator(function->GetDeclarator());
    if (!function_declarator) {
        ReportError("function definition has non-function declarator");
        return;
    }

    if (!BindFunction(function_declarator, function->GetReturnType(),
                      function->GetDeclarationSpecifiers()->GetStorageClass(), true,
                      function->GetDeclarator())) {
        return;
    }

    if (function_declarator->HasParameters()) {
        function_declarator->GetParameters()->Accept(this);
    }

    bool saved_file_scope = in_file_scope_;
    in_file_scope_ = false;
    function->GetBody()->Accept(this);
    in_file_scope_ = saved_file_scope;
}

void DeclarationTypeChecker::Visit(Declaration* declaration) {
    if (!declaration->GetDeclarationSpecifiers()->HasTypeSpecifier()) {
        ReportError("declaration must have at least one type specifier");
        return;
    }

    Declarator* declarator = declaration->GetDeclaration();
    declarator->Accept(this);

    TypeSpecification* type_spec = declaration->GetType();
    TypeRef base_type = type_resolver_.ResolvePrimitive(type_spec);
    if (!base_type) {
        ReportError("internal error: declaration has no type specification");
        return;
    }

    StorageClass storage_class =
        declaration->GetDeclarationSpecifiers()->GetStorageClass();

    if (auto* function_declarator =
            declarator_utils::FindFunctionDeclarator(declarator)) {
        BindFunction(function_declarator, type_spec, storage_class, false, declarator);
        return;
    }

    auto* id_declarator = declarator_utils::FindIdentifierDeclarator(declarator);
    if (!id_declarator) {
        ReportError("declaration has unknown declarator type");
        return;
    }

    TypeRef declared_type = type_resolver_.ResolveObjectType(base_type, declarator);
    if (!declared_type) {
        ReportError("cannot resolve declaration type for '" + id_declarator->GetId() +
                    "'");
        return;
    }

    if (in_file_scope_) {
        BindFileScopeObject(id_declarator, declared_type, storage_class);
    } else {
        BindBlockScopeObject(id_declarator, declared_type, storage_class);
    }
}

void DeclarationTypeChecker::Visit(IdentifierDeclarator* declarator) {
    std::string id = declarator->GetId();
    SymbolInfo* info = symbol_table_.FindByUniqueName(id);
    if (!info || (info->type && info->type->IsFunction())) {
        ReportError("id declarator'" + declarator->GetId() + "' is not a variable");
    }
}

void DeclarationTypeChecker::Visit(FunctionDeclarator* declarator) {
    Declarator* inner = declarator->GetDeclarator();
    if (dynamic_cast<FunctionDeclarator*>(inner)) {
        ReportError("function declarator'" + declarator->GetId() +
                    "' cannot be a function");
        return;
    }

    auto* id_declarator = declarator_utils::FindIdentifierDeclarator(inner);
    std::string name = id_declarator->GetId();
    SymbolInfo* info = symbol_table_.FindByUniqueName(name);
    if (!info || (info->type && !info->type->IsFunction())) {
        ReportError("function declarator'" + declarator->GetId() +
                    "' is already declared as a variable");
    }
}

void DeclarationTypeChecker::Visit(ParameterDeclaration* declaration) {
    StorageClass storage_class =
        declaration->GetDeclarationSpecifiers()->GetStorageClass();

    Declarator* declarator = declaration->GetDeclarator();
    declarator->Accept(this);

    TypeRef type = type_resolver_.ResolveObjectType(declaration->GetType(), declarator);
    if (!type) {
        ReportError("internal error: parameter declaration has no declarator");
        return;
    }

    if (auto* function_declarator = dynamic_cast<FunctionDeclarator*>(declarator)) {
        ReportError("parameter '" + function_declarator->GetId() +
                    "' cannot be a function");
        return;
    }

    auto* id_declarator = declarator_utils::FindIdentifierDeclarator(declarator);
    if (!id_declarator) {
        ReportError("unknown parameter declarator type");
        return;
    }

    std::string name = id_declarator->GetId();
    if (storage_class != StorageClass::None) {
        ReportError("storage class specifier is not allowed on parameter '" + name + "'");
    }

    SymbolInfo* info = symbol_table_.FindByUniqueName(name);
    if (!info) {
        ReportError("internal error: symbol not found for parameter '" + name + "'");
        return;
    } else if (info->type) {
        ReportError("parameter '" + name + "' already has a type");
        return;
    }
    info->type = type;
}

void DeclarationTypeChecker::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
}

void DeclarationTypeChecker::Visit(ReturnStatement* statement) {}

void DeclarationTypeChecker::Visit(ExpressionStatement* statement) {}

void DeclarationTypeChecker::Visit(SelectionStatement* statement) {
    statement->GetThenStatement()->Accept(this);
    if (statement->HasElseStatement()) {
        statement->GetElseStatement()->Accept(this);
    }
}

void DeclarationTypeChecker::Visit(WhileStatement* statement) {
    statement->GetBody()->Accept(this);
}

void DeclarationTypeChecker::Visit(ForStatement* statement) {
    BaseElement* init = statement->GetInit();
    if (!dynamic_cast<Declaration*>(init) && !dynamic_cast<Expression*>(init)) {
        ReportError("for statement has invalid initialization");
        return;
    }

    if (auto* declaration = dynamic_cast<Declaration*>(init)) {
        if (declaration->GetDeclarationSpecifiers()->GetStorageClass() !=
            StorageClass::None) {
            ReportError("storage class specifier is not allowed in for loop header");
            return;
        }
        if (dynamic_cast<FunctionDeclarator*>(declaration->GetDeclaration())) {
            ReportError("function declaration in for loop header");
            return;
        }
        declaration->Accept(this);
    }

    statement->GetBody()->Accept(this);
}

bool DeclarationTypeChecker::BindFunction(FunctionDeclarator* function_declarator,
                                          TypeSpecification* return_type_spec,
                                          StorageClass storage_class, bool is_definition,
                                          Declarator* full_declarator) {
    Declarator* declarator_for_type =
        full_declarator ? full_declarator : function_declarator;
    TypeRef function_type = type_resolver_.ResolveFunctionType(
        declarator_for_type, function_declarator, return_type_spec);
    if (!function_type) {
        return false;
    }

    std::string function_name = function_declarator->GetId();
    SymbolInfo* info = RequireSymbol(function_name);
    if (!info) {
        return false;
    }

    auto new_linkage = (storage_class == StorageClass::Static)
                           ? SymbolInfo::LinkageKind::Internal
                           : SymbolInfo::LinkageKind::External;

    if (info->type) {
        if (!info->type->Equals(function_type)) {
            ReportError("conflicting types for function '" + function_name + "'");
            return false;
        }
        if (new_linkage == SymbolInfo::LinkageKind::Internal &&
            info->linkage == SymbolInfo::LinkageKind::External) {
            ReportError("static declaration of '" + function_name +
                        "' follows non-static declaration");
            return false;
        }
        if (is_definition && info->is_defined) {
            ReportError("function '" + function_name + "' is already defined");
            return false;
        }
    } else {
        info->type = function_type;
        info->linkage = new_linkage;
    }

    if (is_definition) {
        info->is_defined = true;
    }
    return true;
}

bool DeclarationTypeChecker::BindFileScopeObject(IdentifierDeclarator* id_declarator,
                                                 TypeRef declared_type,
                                                 StorageClass storage_class) {
    std::string name = id_declarator->GetId();
    SymbolInfo* info = RequireSymbol(name);
    if (!info) {
        return false;
    }

    SymbolInfo::InitialValue new_init_state;
    std::optional<NumericConstant> new_init_constant;

    if (id_declarator->HasInitializer()) {
        new_init_state = SymbolInfo::InitialValue::Initial;
    } else if (storage_class == StorageClass::Extern) {
        new_init_state = SymbolInfo::InitialValue::NoInitializer;
    } else {
        new_init_state = SymbolInfo::InitialValue::Tentative;
    }

    auto new_linkage = (storage_class == StorageClass::Static)
                           ? SymbolInfo::LinkageKind::Internal
                           : SymbolInfo::LinkageKind::External;

    if (info->type) {
        if (!info->type->Equals(declared_type)) {
            ReportError("conflicting types for '" + name + "'");
            return false;
        }
        if (storage_class == StorageClass::Extern) {
            new_linkage = info->linkage;
        } else if (info->linkage != new_linkage) {
            ReportError("conflicting linkage for '" + name + "'");
            return false;
        }
        if (info->init_state == SymbolInfo::InitialValue::Initial &&
            new_init_state == SymbolInfo::InitialValue::Initial) {
            ReportError("redefinition of '" + name + "'");
            return false;
        }
        info->linkage = new_linkage;
        if (new_init_state > info->init_state) {
            info->init_state = new_init_state;
            info->init_constant = new_init_constant;
        }
    } else {
        info->type = declared_type;
        info->linkage = new_linkage;
        info->init_state = new_init_state;
        info->init_constant = new_init_constant;
    }

    info->duration = SymbolInfo::StorageDuration::Static;
    return true;
}

bool DeclarationTypeChecker::BindBlockScopeObject(IdentifierDeclarator* id_declarator,
                                                  TypeRef declared_type,
                                                  StorageClass storage_class) {
    std::string name = id_declarator->GetId();
    SymbolInfo* info = RequireSymbol(name);
    if (!info) {
        return false;
    }

    if (storage_class == StorageClass::Extern) {
        if (id_declarator->HasInitializer()) {
            ReportError("initializer on local extern variable declaration");
            if (!info->type) {
                info->type = declared_type;
                info->init_state = SymbolInfo::InitialValue::NoInitializer;
                info->duration = SymbolInfo::StorageDuration::Static;
            }
            return false;
        }
        if (info->type) {
            if (info->type->IsFunction()) {
                ReportError("function '" + name + "' redeclared as variable");
                return false;
            }
            if (!info->type->Equals(declared_type)) {
                ReportError("conflicting types for '" + name + "'");
                return false;
            }
        } else {
            info->type = declared_type;
            info->init_state = SymbolInfo::InitialValue::NoInitializer;
            info->duration = SymbolInfo::StorageDuration::Static;
        }
        return true;
    }

    if (storage_class == StorageClass::Static) {
        info->init_state = SymbolInfo::InitialValue::Initial;
        if (!id_declarator->HasInitializer()) {
            info->init_constant = 0;
        }
        info->type = declared_type;
        info->duration = SymbolInfo::StorageDuration::Static;
        return true;
    }

    info->type = declared_type;
    info->duration = SymbolInfo::StorageDuration::Automatic;
    return true;
}

SymbolInfo* DeclarationTypeChecker::RequireSymbol(const std::string& unique_name) {
    SymbolInfo* info = symbol_table_.FindByUniqueName(unique_name);
    if (!info) {
        ReportError("internal error: symbol not found for '" + unique_name + "'");
        return nullptr;
    }
    return info;
}
