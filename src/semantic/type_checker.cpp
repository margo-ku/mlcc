#include "include/semantic/type_checker.h"

#include "include/ast/declarations.h"
#include "include/ast/expressions.h"
#include "include/semantic/symbol_table.h"
#include "include/types/function_type.h"
#include "include/types/primitive_type.h"
#include "include/types/type.h"

TypeChecker::TypeChecker(SymbolTable& symbol_table) : symbol_table_(symbol_table) {}

TypeChecker::~TypeChecker() {}

void TypeChecker::Visit(TranslationUnit* translation_unit) {
    in_file_scope_ = true;
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

    if (auto func_declarator =
            dynamic_cast<FunctionDeclarator*>(function->GetDeclarator())) {
        if (!ProcessFunctionDeclaration(
                func_declarator, function->GetReturnType(),
                function->GetDeclarationSpecifiers()->GetStorageClass(), true)) {
            return;
        }
        if (func_declarator->HasParameters()) {
            func_declarator->GetParameters()->Accept(this);
        }
    } else {
        ReportError("function definition has non-function declarator");
        return;
    }

    current_return_type_ = ResolvePrimitiveType(function->GetReturnType());
    bool saved_file_scope = in_file_scope_;
    in_file_scope_ = false;
    function->GetBody()->Accept(this);
    in_file_scope_ = saved_file_scope;
    current_return_type_ = nullptr;
}

void TypeChecker::Visit(DeclarationSpecifiers* decl_specs) {}

void TypeChecker::Visit(TypeSpecification* type) {}

void TypeChecker::Visit(Declaration* declaration) {
    if (!declaration->GetDeclarationSpecifiers()->HasTypeSpecifier()) {
        ReportError("declaration must have at least one type specifier");
        return;
    }

    Declarator* declarator = declaration->GetDeclaration();
    declarator->Accept(this);

    TypeSpecification* type_spec = declaration->GetType();
    TypeRef type = ResolvePrimitiveType(type_spec);
    if (!type) {
        ReportError("internal error: declaration has no type specification");
        return;
    }

    StorageClass storage_class =
        declaration->GetDeclarationSpecifiers()->GetStorageClass();

    if (auto func_declarator = dynamic_cast<FunctionDeclarator*>(declarator)) {
        if (!ProcessFunctionDeclaration(func_declarator, type_spec, storage_class,
                                        false)) {
            return;
        }
    } else if (auto id_declarator = dynamic_cast<IdentifierDeclarator*>(declarator)) {
        SymbolInfo* info = symbol_table_.FindByUniqueName(id_declarator->GetId());
        if (!info) {
            ReportError("internal error: symbol not found for '" +
                        id_declarator->GetId() + "'");
            return;
        }
        bool ok = in_file_scope_
                      ? ProcessFileScopeVariable(id_declarator, type, storage_class)
                      : ProcessBlockScopeVariable(id_declarator, type, storage_class);
        if (!ok) {
            return;
        }
    } else {
        ReportError("declaration has unknown declarator type");
        return;
    }
}

void TypeChecker::Visit(Expression* expression) {}

void TypeChecker::Visit(IdExpression* expression) {
    std::string name = expression->GetId();
    if (SymbolInfo* info = symbol_table_.FindByUniqueName(name)) {
        if (!info->type) {
            ReportError("type of '" + name + "' is not set");
            return;
        }
        expression->SetTypeRef(info->type);
    } else {
        ReportError("there is no symbol_info about '" + name + "' in symbol table");
    }
}

void TypeChecker::Visit(PrimaryExpression* expression) {
    switch (expression->GetValue().GetKind()) {
        case IntegralConstant::Kind::Int32:
            expression->SetTypeRef(PrimitiveType::GetInt32());
            break;
        case IntegralConstant::Kind::Int64:
            expression->SetTypeRef(PrimitiveType::GetInt64());
            break;
        case IntegralConstant::Kind::UInt32:
            expression->SetTypeRef(PrimitiveType::GetUInt32());
            break;
        case IntegralConstant::Kind::UInt64:
            expression->SetTypeRef(PrimitiveType::GetUInt64());
            break;
    }
}

void TypeChecker::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);

    TypeRef operand_type = expression->GetExpression()->GetTypeRef();
    if (!operand_type || !operand_type->IsIntegral()) {
        ReportError("unary operator requires integral operand");
        return;
    }

    switch (expression->GetOp()) {
        case UnaryExpression::UnaryOperator::Minus:
        case UnaryExpression::UnaryOperator::Plus:
        case UnaryExpression::UnaryOperator::BinaryNot:
            expression->SetTypeRef(operand_type);
            break;
        case UnaryExpression::UnaryOperator::Not:
            expression->SetTypeRef(PrimitiveType::GetInt32());
            break;
    }
}

void TypeChecker::Visit(BinaryExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);

    TypeRef left_type = expression->GetLeftExpression()->GetTypeRef();
    TypeRef right_type = expression->GetRightExpression()->GetTypeRef();

    if (!left_type || !left_type->IsIntegral()) {
        ReportError("left operand of binary expression must be an integral type");
        return;
    }
    if (!right_type || !right_type->IsIntegral()) {
        ReportError("right operand of binary expression must be an integral type");
        return;
    }

    TypeRef common_type = GetCommonType(left_type, right_type);

    std::unique_ptr<Expression> wrapped_left_expr =
        WrapWithCast(expression->ExtractLeftExpression(), common_type);
    if (!wrapped_left_expr) {
        return;
    }
    expression->SetLeftExpression(std::move(wrapped_left_expr));

    std::unique_ptr<Expression> wrapped_right_expr =
        WrapWithCast(expression->ExtractRightExpression(), common_type);
    if (!wrapped_right_expr) {
        return;
    }
    expression->SetRightExpression(std::move(wrapped_right_expr));

    switch (expression->GetOp()) {
        case BinaryExpression::BinaryOperator::And:
        case BinaryExpression::BinaryOperator::Or:
        case BinaryExpression::BinaryOperator::Less:
        case BinaryExpression::BinaryOperator::Greater:
        case BinaryExpression::BinaryOperator::LessEqual:
        case BinaryExpression::BinaryOperator::GreaterEqual:
        case BinaryExpression::BinaryOperator::Equal:
        case BinaryExpression::BinaryOperator::NotEqual: {
            expression->SetTypeRef(PrimitiveType::GetInt32());
            break;
        }
        default: {
            expression->SetTypeRef(common_type);
            break;
        }
    }
}

void TypeChecker::Visit(ConditionalExpression* expression) {
    expression->GetCondition()->Accept(this);
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);

    TypeRef cond_type = expression->GetCondition()->GetTypeRef();
    TypeRef left_type = expression->GetLeftExpression()->GetTypeRef();
    TypeRef right_type = expression->GetRightExpression()->GetTypeRef();

    if (!cond_type || !cond_type->IsIntegral()) {
        ReportError("conditional expression condition must be an integral type");
        return;
    }
    if (!left_type || !left_type->IsIntegral()) {
        ReportError("conditional expression left operand must be an integral type");
        return;
    }
    if (!right_type || !right_type->IsIntegral()) {
        ReportError("conditional expression right operand must be an integral type");
        return;
    }

    TypeRef common_type = GetCommonType(left_type, right_type);

    std::unique_ptr<Expression> wrapped_left_expr =
        WrapWithCast(expression->ExtractLeftExpression(), common_type);
    if (!wrapped_left_expr) {
        return;
    }
    expression->SetLeftExpression(std::move(wrapped_left_expr));

    std::unique_ptr<Expression> wrapped_right_expr =
        WrapWithCast(expression->ExtractRightExpression(), common_type);
    if (!wrapped_right_expr) {
        return;
    }
    expression->SetRightExpression(std::move(wrapped_right_expr));

    expression->SetTypeRef(common_type);
}

void TypeChecker::Visit(AssignmentExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);

    TypeRef left_type = expression->GetLeftExpression()->GetTypeRef();
    TypeRef right_type = expression->GetRightExpression()->GetTypeRef();

    if (!left_type || !left_type->IsIntegral()) {
        ReportError("left operand of assignment must be an integral type");
        return;
    }
    if (!right_type || !right_type->IsIntegral()) {
        ReportError("right operand of assignment must be an integral type");
        return;
    }

    if (!right_type->Equals(left_type)) {
        std::unique_ptr<Expression> wrapped_right_expr =
            WrapWithCast(expression->ExtractRightExpression(), left_type);
        if (!wrapped_right_expr) {
            return;
        }
        expression->SetRightExpression(std::move(wrapped_right_expr));
    }

    expression->SetTypeRef(left_type);
}

void TypeChecker::Visit(CastExpression* expression) {
    Expression* inner = expression->GetExpression();
    if (!inner->GetTypeRef()) {
        inner->Accept(this);
    }

    TypeRef from_type = inner->GetTypeRef();
    TypeRef to_type = ResolvePrimitiveType(expression->GetType());

    if (!from_type || !to_type) {
        ReportError("invalid types in cast expression");
        return;
    }

    if (!CanCast(from_type, to_type)) {
        ReportError("cannot cast expression of type '" + from_type->ToString() +
                    "' to '" + to_type->ToString() + "'");
        return;
    }

    expression->SetTypeRef(to_type);
}

void TypeChecker::Visit(CompoundStatement* statement) {
    statement->GetBody()->Accept(this);
}

void TypeChecker::Visit(ReturnStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);

        if (current_return_type_) {
            TypeRef expr_type = statement->GetExpression()->GetTypeRef();
            if (expr_type && !expr_type->Equals(current_return_type_)) {
                std::unique_ptr<Expression> wrapped =
                    WrapWithCast(statement->ExtractExpression(), current_return_type_);
                if (wrapped) {
                    statement->SetExpression(std::move(wrapped));
                }
            }
        }
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
        ReportError("for statement has invalid initialization");
        return;
    }

    if (auto decl = dynamic_cast<Declaration*>(init)) {
        if (decl->GetDeclarationSpecifiers()->GetStorageClass() != StorageClass::None) {
            ReportError("storage class specifier is not allowed in for loop header");
            return;
        }
        if (dynamic_cast<FunctionDeclarator*>(decl->GetDeclaration())) {
            ReportError("function declaration in for loop header");
            return;
        }
    }
    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);
}

void TypeChecker::Visit(ParameterDeclaration* declaration) {
    StorageClass storage_class =
        declaration->GetDeclarationSpecifiers()->GetStorageClass();

    Declarator* declarator = declaration->GetDeclarator();
    declarator->Accept(this);

    TypeSpecification* type_spec = declaration->GetType();
    TypeRef type = ResolvePrimitiveType(type_spec);
    if (!type) {
        ReportError("internal error: parameter declaration has no declarator");
        return;
    }

    if (auto func_declarator = dynamic_cast<FunctionDeclarator*>(declarator)) {
        ReportError("parameter '" + func_declarator->GetId() + "' cannot be a function");
        return;
    } else if (auto id_declarator = dynamic_cast<IdentifierDeclarator*>(declarator)) {
        std::string name = id_declarator->GetId();

        if (storage_class != StorageClass::None) {
            ReportError("storage class specifier is not allowed on parameter '" + name +
                        "'");
        }

        SymbolInfo* info = symbol_table_.FindByUniqueName(name);
        if (!info) {
            ReportError("internal error: symbol not found for parameter '" + name + "'");
            return;
        }
        if (info->type) {
            ReportError("parameter '" + name + "' already has a type");
            return;
        }
        info->type = type;
    } else {
        ReportError("unknown parameter declarator type");
        return;
    }
}

void TypeChecker::Visit(ParameterList* list) {
    for (auto& parameter : list->GetParameters()) {
        parameter->Accept(this);
    }
}

void TypeChecker::Visit(FunctionCallExpression* expression) {
    Expression* function_expression = expression->GetFunction();
    if (!function_expression) {
        ReportError("function call has no function expression");
        return;
    }
    function_expression->Accept(this);

    TypeRef function_type = function_expression->GetTypeRef();
    if (!function_type) {
        ReportError("cannot resolve type of function in call expression");
        return;
    }

    FunctionType* func_type = dynamic_cast<FunctionType*>(function_type.get());
    if (!function_type) {
        ReportError("called expression is not a function");
        return;
    }

    size_t expected = func_type->GetParameterCount();
    size_t provided = 0;
    if (expression->HasArguments() && expression->GetArguments()) {
        provided = expression->GetArguments()->GetArguments().size();
    }

    if (provided != expected) {
        ReportError("function '" + func_type->ToString() + "' expects " +
                    std::to_string(expected) + " arguments, but " +
                    std::to_string(provided) + " were provided");
        return;
    }

    if (provided > 0) {
        auto& args = expression->GetArguments()->GetArguments();
        for (size_t index = 0; index < provided; ++index) {
            args[index]->Accept(this);
            TypeRef arg_type = args[index]->GetTypeRef();
            TypeRef param_type = func_type->GetParamTypes()[index];
            if (!arg_type || !arg_type->IsIntegral()) {
                ReportError("argument " + std::to_string(index) +
                            " of function call must be an integral type");
                return;
            }

            if (!arg_type->Equals(param_type)) {
                std::unique_ptr<Expression> wrapped =
                    WrapWithCast(std::move(args[index]), param_type);
                if (!wrapped) {
                    return;
                }
                args[index] = std::move(wrapped);
            }
        }
    }

    expression->SetTypeRef(func_type->GetReturnType());
}
void TypeChecker::Visit(ArgumentExpressionList* list) {
    for (auto& argument : list->GetArguments()) {
        argument->Accept(this);
    }
}

void TypeChecker::Visit(IdentifierDeclarator* declarator) {
    std::string id = declarator->GetId();
    SymbolInfo* info = symbol_table_.FindByUniqueName(id);
    if (!info || info->type && !info->type->IsIntegral()) {
        ReportError("id declarator'" + declarator->GetId() +
                    "' is not an integral variable");
    }
}

void TypeChecker::Visit(FunctionDeclarator* declarator) {
    Declarator* inner = declarator->GetDeclarator();
    if (auto func_decl = dynamic_cast<FunctionDeclarator*>(inner)) {
        ReportError("function declarator'" + declarator->GetId() +
                    "' cannot be a function");
        return;
    }

    auto id_decl = dynamic_cast<IdentifierDeclarator*>(inner);
    std::string name = id_decl->GetId();
    SymbolInfo* info = symbol_table_.FindByUniqueName(name);
    if (!info || info->type && info->type->IsIntegral()) {
        ReportError("function declarator'" + declarator->GetId() +
                    "' is an integral variable");
    }
}

////////////////////////////////////////////////////////////////////

const std::vector<std::string>& TypeChecker::GetErrors() const { return errors_; }

void TypeChecker::ReportError(const std::string& message) { errors_.push_back(message); }

bool TypeChecker::CanCast(TypeRef from, TypeRef to) {
    if (!from || !to) {
        return false;
    }

    if (from->Equals(to)) {
        return true;
    }

    // All integral types can be cast to each other
    return from->IsIntegral() && to->IsIntegral();
}

std::unique_ptr<Expression> TypeChecker::WrapWithCast(
    std::unique_ptr<Expression> expression, TypeRef target_type) {
    TypeRef from_type = expression->GetTypeRef();
    if (!from_type) {
        expression->Accept(this);
        from_type = expression->GetTypeRef();
    }

    if (!from_type->Equals(target_type)) {
        if (CanCast(from_type, target_type)) {
            auto cast_expr = std::make_unique<CastExpression>(
                std::move(GetTypeSpecification(target_type)), std::move(expression));

            cast_expr->SetTypeRef(target_type);
            return cast_expr;
        } else {
            ReportError("cannot cast expression of type '" + from_type->ToString() +
                        "' to '" + target_type->ToString() + "'");
            return nullptr;
        }
    }
    return expression;
}

TypeRef TypeChecker::GetCommonType(TypeRef type1, TypeRef type2) {
    if (type1->Equals(type2)) {
        return type1;
    }
    if (type1->Size() == type2->Size()) {
        return type1->IsSigned() ? type1 : type2;
    }
    return type1->Size() > type2->Size() ? type1 : type2;
}

TypeRef TypeChecker::ResolvePrimitiveType(TypeSpecification* type) {
    if (!type) {
        ReportError("type specification is empty");
        return nullptr;
    }

    switch (type->GetType()) {
        case TypeSpecification::Type::Int:
            return PrimitiveType::GetInt32();
        case TypeSpecification::Type::Long:
            return PrimitiveType::GetInt64();
        case TypeSpecification::Type::UInt:
            return PrimitiveType::GetUInt32();
        case TypeSpecification::Type::ULong:
            return PrimitiveType::GetUInt64();
    }
}

std::unique_ptr<TypeSpecification> TypeChecker::GetTypeSpecification(TypeRef type) {
    if (!type) {
        return nullptr;
    }
    if (type->Equals(PrimitiveType::GetInt32())) {
        return std::make_unique<TypeSpecification>(TypeSpecification::Type::Int);
    } else if (type->Equals(PrimitiveType::GetInt64())) {
        return std::make_unique<TypeSpecification>(TypeSpecification::Type::Long);
    } else if (type->Equals(PrimitiveType::GetUInt32())) {
        return std::make_unique<TypeSpecification>(TypeSpecification::Type::UInt);
    } else if (type->Equals(PrimitiveType::GetUInt64())) {
        return std::make_unique<TypeSpecification>(TypeSpecification::Type::ULong);
    }
    return nullptr;
}

TypeRef TypeChecker::ResolveFunctionType(Declarator* declarator,
                                         TypeSpecification* return_type_spec) {
    if (!return_type_spec) {
        ReportError("type specification of return type is empty");
        return nullptr;
    }
    TypeRef return_type = ResolvePrimitiveType(return_type_spec);
    if (!return_type) {
        return nullptr;
    }

    std::vector<TypeRef> param_types;
    if (auto func_declarator = dynamic_cast<FunctionDeclarator*>(declarator)) {
        if (func_declarator->HasParameters()) {
            ParameterList* list = func_declarator->GetParameters();
            if (list) {
                const auto& params = list->GetParameters();
                for (const auto& param_ptr : params) {
                    if (!param_ptr) {
                        continue;
                    }
                    ParameterDeclaration* param = param_ptr.get();
                    if (!param) {
                        continue;
                    }

                    TypeSpecification* param_type_spec = param->GetType();
                    if (!param) {
                        ReportError("Parameter has no type");
                        return nullptr;
                    }
                    TypeRef param_type = ResolvePrimitiveType(param_type_spec);
                    if (!param_type) {
                        ReportError("Parameter has invalid type");
                        return nullptr;
                    }
                    param_types.push_back(param_type);
                }
            }
        }
    }
    return std::make_shared<FunctionType>(return_type, std::move(param_types));
}

bool TypeChecker::ProcessFunctionDeclaration(FunctionDeclarator* func_declarator,
                                             TypeSpecification* return_type_spec,
                                             StorageClass storage_class,
                                             bool is_definition) {
    TypeRef func_type = ResolveFunctionType(func_declarator, return_type_spec);
    if (!func_type) {
        return false;
    }

    std::string func_name = func_declarator->GetId();
    SymbolInfo* info = symbol_table_.FindByUniqueName(func_name);
    if (!info) {
        ReportError("internal error: symbol not found for '" + func_name + "'");
        return false;
    }

    auto new_linkage = (storage_class == StorageClass::Static)
                           ? SymbolInfo::LinkageKind::Internal
                           : SymbolInfo::LinkageKind::External;

    if (info->type) {
        if (!info->type->Equals(func_type)) {
            ReportError("conflicting types for function '" + func_name + "'");
            return false;
        }
        if (new_linkage == SymbolInfo::LinkageKind::Internal &&
            info->linkage == SymbolInfo::LinkageKind::External) {
            ReportError("static declaration of '" + func_name +
                        "' follows non-static declaration");
            return false;
        }
        if (is_definition && info->is_defined) {
            ReportError("function '" + func_name + "' is already defined");
            return false;
        }
    } else {
        info->type = func_type;
        info->linkage = new_linkage;
    }

    if (is_definition) {
        info->is_defined = true;
    }
    return true;
}

bool TypeChecker::ProcessFileScopeVariable(IdentifierDeclarator* id_declarator,
                                           TypeRef declared_type,
                                           StorageClass storage_class) {
    std::string name = id_declarator->GetId();
    SymbolInfo* info = symbol_table_.FindByUniqueName(name);
    if (!info) {
        ReportError("internal error: symbol not found for '" + name + "'");
        return false;
    }

    SymbolInfo::InitialValue new_init_state;
    std::optional<IntegralConstant> new_init_constant;

    if (id_declarator->HasInitializer()) {
        auto* primary = dynamic_cast<PrimaryExpression*>(id_declarator->GetInitializer());
        if (!primary) {
            ReportError("initializer of file scope variable '" + name +
                        "' is not a constant expression");
            return false;
        }
        primary->Accept(this);
        new_init_state = SymbolInfo::InitialValue::Initial;
        new_init_constant = primary->GetValue();

        TypeRef init_type = primary->GetTypeRef();
        if (init_type && !init_type->Equals(declared_type)) {
            auto wrapped =
                WrapWithCast(id_declarator->ExtractInitializer(), declared_type);
            if (!wrapped) {
                return false;
            }
            id_declarator->SetInitializer(std::move(wrapped));
        }
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

bool TypeChecker::ProcessBlockScopeVariable(IdentifierDeclarator* id_declarator,
                                            TypeRef declared_type,
                                            StorageClass storage_class) {
    std::string name = id_declarator->GetId();
    SymbolInfo* info = symbol_table_.FindByUniqueName(name);
    if (!info) {
        ReportError("internal error: symbol not found for '" + name + "'");
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
            if (!info->type->IsIntegral()) {
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
        if (id_declarator->HasInitializer()) {
            auto* primary =
                dynamic_cast<PrimaryExpression*>(id_declarator->GetInitializer());
            if (!primary) {
                ReportError("non-constant initializer on local static variable '" + name +
                            "'");
                return false;
            }
            primary->Accept(this);
            info->init_state = SymbolInfo::InitialValue::Initial;
            info->init_constant = primary->GetValue();

            TypeRef init_type = primary->GetTypeRef();
            if (init_type && !init_type->Equals(declared_type)) {
                auto wrapped =
                    WrapWithCast(id_declarator->ExtractInitializer(), declared_type);
                if (!wrapped) {
                    return false;
                }
                id_declarator->SetInitializer(std::move(wrapped));
            }
        } else {
            info->init_state = SymbolInfo::InitialValue::Initial;
            info->init_constant = 0;
        }
        info->type = declared_type;
        info->duration = SymbolInfo::StorageDuration::Static;
        return true;
    }

    info->type = declared_type;
    info->duration = SymbolInfo::StorageDuration::Automatic;
    if (id_declarator->HasInitializer()) {
        Expression* init = id_declarator->GetInitializer();
        init->Accept(this);
        TypeRef init_type = init->GetTypeRef();
        if (init_type && !init_type->Equals(declared_type)) {
            auto wrapped =
                WrapWithCast(id_declarator->ExtractInitializer(), declared_type);
            if (!wrapped) {
                return false;
            }
            id_declarator->SetInitializer(std::move(wrapped));
        }
    }
    return true;
}