#include "include/semantic/type_checker.h"

#include <memory>
#include <optional>

#include "include/ast/declarations.h"
#include "include/ast/expressions.h"
#include "include/semantic/symbol_table.h"
#include "include/types/function_type.h"
#include "include/types/pointer_type.h"
#include "include/types/primitive_type.h"
#include "include/types/type.h"

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

int CountPointerLevelsBeforeFunction(Declarator* declarator) {
    if (dynamic_cast<FunctionDeclarator*>(declarator)) {
        return 0;
    }
    if (auto* ptr = dynamic_cast<PointerDeclarator*>(declarator)) {
        return 1 + CountPointerLevelsBeforeFunction(ptr->GetDeclarator());
    }
    return 0;
}

enum class BinaryOpClass { Integral, Arithmetic, Comparison, Equality, Logical };

BinaryOpClass ClassifyBinaryOp(BinaryExpression::BinaryOperator op) {
    using Op = BinaryExpression::BinaryOperator;
    switch (op) {
        case Op::BitwiseAnd:
        case Op::BitwiseOr:
        case Op::BitwiseXor:
        case Op::LeftShift:
        case Op::RightShift:
        case Op::Mod:
            return BinaryOpClass::Integral;
        case Op::Plus:
        case Op::Minus:
        case Op::Mul:
        case Op::Div:
            return BinaryOpClass::Arithmetic;
        case Op::Less:
        case Op::Greater:
        case Op::LessEqual:
        case Op::GreaterEqual:
            return BinaryOpClass::Comparison;
        case Op::Equal:
        case Op::NotEqual:
            return BinaryOpClass::Equality;
        case Op::And:
        case Op::Or:
            return BinaryOpClass::Logical;
    }
}

}  // namespace

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

    auto func_declarator = GetFunctionDeclarator(function->GetDeclarator());
    if (!func_declarator) {
        ReportError("function definition has non-function declarator");
        return;
    }

    if (!ProcessFunctionDeclaration(
            func_declarator, function->GetReturnType(),
            function->GetDeclarationSpecifiers()->GetStorageClass(), true,
            function->GetDeclarator())) {
        return;
    }
    if (func_declarator->HasParameters()) {
        func_declarator->GetParameters()->Accept(this);
    }

    TypeRef base_return_type = ResolvePrimitiveType(function->GetReturnType());
    int pointer_levels = CountPointerLevelsBeforeFunction(function->GetDeclarator());
    current_return_type_ = base_return_type;
    for (int idx = 0; idx < pointer_levels; ++idx) {
        current_return_type_ = std::make_shared<PointerType>(current_return_type_);
    }

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

    if (auto func_declarator = GetFunctionDeclarator(declarator)) {
        if (!ProcessFunctionDeclaration(func_declarator, type_spec, storage_class, false,
                                        declarator)) {
            return;
        }
    } else if (auto id_declarator = GetIdentifierDeclarator(declarator)) {
        TypeRef declared_type = ResolveDeclaratorType(type, declarator);
        if (!declared_type) {
            ReportError("cannot resolve declaration type for '" + id_declarator->GetId() +
                        "'");
            return;
        }
        SymbolInfo* info = symbol_table_.FindByUniqueName(id_declarator->GetId());
        if (!info) {
            ReportError("internal error: symbol not found for '" +
                        id_declarator->GetId() + "'");
            return;
        }
        bool ok =
            in_file_scope_
                ? ProcessFileScopeVariable(id_declarator, declared_type, storage_class)
                : ProcessBlockScopeVariable(id_declarator, declared_type, storage_class);
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
        case NumericConstant::Kind::Int32:
            expression->SetTypeRef(PrimitiveType::GetInt32());
            break;
        case NumericConstant::Kind::Int64:
            expression->SetTypeRef(PrimitiveType::GetInt64());
            break;
        case NumericConstant::Kind::UInt32:
            expression->SetTypeRef(PrimitiveType::GetUInt32());
            break;
        case NumericConstant::Kind::UInt64:
            expression->SetTypeRef(PrimitiveType::GetUInt64());
            break;
        case NumericConstant::Kind::Double:
            expression->SetTypeRef(PrimitiveType::GetDouble());
            break;
    }
}

void TypeChecker::Visit(UnaryExpression* expression) {
    expression->GetExpression()->Accept(this);

    TypeRef operand_type = expression->GetExpression()->GetTypeRef();
    if (!operand_type || operand_type->IsFunction()) {
        ReportError("unary operator has invalid operand type");
        return;
    }

    switch (expression->GetOp()) {
        case UnaryExpression::UnaryOperator::BinaryNot:
            if (!operand_type->IsIntegral()) {
                ReportError("unary operator requires integral operand");
                return;
            }
            expression->SetTypeRef(operand_type);
            break;
        case UnaryExpression::UnaryOperator::Minus:
        case UnaryExpression::UnaryOperator::Plus:
            if (!operand_type->IsArithmetic()) {
                ReportError("unary operator requires arithmetic operand");
                return;
            }
            expression->SetTypeRef(operand_type);
            break;
        case UnaryExpression::UnaryOperator::Not:
            expression->SetTypeRef(PrimitiveType::GetInt32());
            break;
    }
}

std::optional<TypeChecker::BinaryOpResult> TypeChecker::CheckBinaryOperands(
    BinaryExpression::BinaryOperator op, TypeRef left_type, TypeRef right_type,
    Expression* left_expr, Expression* right_expr) {
    switch (ClassifyBinaryOp(op)) {
        case BinaryOpClass::Integral:
            if (!left_type->IsIntegral() || !right_type->IsIntegral()) {
                ReportError(
                    "operands of bitwise/modulo operation must be integral types");
                return std::nullopt;
            }
            {
                TypeRef common = GetCommonType(left_type, right_type);
                return BinaryOpResult{common, common};
            }

        case BinaryOpClass::Arithmetic:
            if (!left_type->IsArithmetic() || !right_type->IsArithmetic()) {
                ReportError("operands of arithmetic operation must be arithmetic types");
                return std::nullopt;
            }
            {
                TypeRef common = GetCommonType(left_type, right_type);
                return BinaryOpResult{common, common};
            }

        case BinaryOpClass::Comparison:
            if (!left_type->IsArithmetic() || !right_type->IsArithmetic()) {
                ReportError("operands of comparison must be arithmetic types");
                return std::nullopt;
            }
            return BinaryOpResult{GetCommonType(left_type, right_type),
                                  PrimitiveType::GetInt32()};

        case BinaryOpClass::Equality:
            if (left_type->IsPointer() || right_type->IsPointer()) {
                TypeRef common = GetCommonPointerType(left_expr, right_expr);
                if (!common) return std::nullopt;
                return BinaryOpResult{common, PrimitiveType::GetInt32()};
            }
            if (!left_type->IsArithmetic() || !right_type->IsArithmetic()) {
                ReportError(
                    "operands of equality comparison must be arithmetic or pointer "
                    "types");
                return std::nullopt;
            }
            return BinaryOpResult{GetCommonType(left_type, right_type),
                                  PrimitiveType::GetInt32()};

        case BinaryOpClass::Logical:
            if ((!left_type->IsArithmetic() && !left_type->IsPointer()) ||
                (!right_type->IsArithmetic() && !right_type->IsPointer())) {
                ReportError("operands of logical operation must be scalar types");
                return std::nullopt;
            }
            return BinaryOpResult{nullptr, PrimitiveType::GetInt32()};
    }
}

void TypeChecker::Visit(BinaryExpression* expression) {
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);

    TypeRef left_type = expression->GetLeftExpression()->GetTypeRef();
    TypeRef right_type = expression->GetRightExpression()->GetTypeRef();
    if (!left_type || !right_type) {
        ReportError("internal error: binary expression operand has no type");
        return;
    }

    auto result = CheckBinaryOperands(expression->GetOp(), left_type, right_type,
                                      expression->GetLeftExpression(),
                                      expression->GetRightExpression());
    if (!result) {
        return;
    }

    if (result->operand_type) {
        auto wrapped_left =
            WrapWithCast(expression->ExtractLeftExpression(), result->operand_type);
        if (!wrapped_left) {
            return;
        }
        expression->SetLeftExpression(std::move(wrapped_left));

        auto wrapped_right =
            WrapWithCast(expression->ExtractRightExpression(), result->operand_type);
        if (!wrapped_right) {
            return;
        }
        expression->SetRightExpression(std::move(wrapped_right));
    }

    expression->SetTypeRef(result->result_type);
}

void TypeChecker::Visit(ConditionalExpression* expression) {
    expression->GetCondition()->Accept(this);
    expression->GetLeftExpression()->Accept(this);
    expression->GetRightExpression()->Accept(this);

    TypeRef cond_type = expression->GetCondition()->GetTypeRef();
    TypeRef left_type = expression->GetLeftExpression()->GetTypeRef();
    TypeRef right_type = expression->GetRightExpression()->GetTypeRef();
    if (!cond_type || !left_type || !right_type) {
        ReportError("internal error: conditional expression operand has no type");
        return;
    }

    if (!cond_type->IsArithmetic() && !cond_type->IsPointer()) {
        ReportError(
            "conditional expression condition must be an arithmetic or pointer type");
        return;
    }

    bool has_pointer_operand = left_type->IsPointer() || right_type->IsPointer();
    TypeRef common_type = nullptr;
    if (has_pointer_operand) {
        common_type = GetCommonPointerType(expression->GetLeftExpression(),
                                           expression->GetRightExpression());
    } else {
        if (!left_type->IsArithmetic() || !right_type->IsArithmetic()) {
            ReportError("conditional expression operands must be arithmetic types");
            return;
        }
        common_type = GetCommonType(left_type, right_type);
    }
    if (!common_type) {
        return;
    }

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
    Expression* left_expression = expression->GetLeftExpression();
    if (!IsLValue(left_expression)) {
        ReportError("left operand of assignment is not assignable");
        return;
    }

    left_expression->Accept(this);
    expression->GetRightExpression()->Accept(this);

    TypeRef left_type = left_expression->GetTypeRef();
    TypeRef right_type = expression->GetRightExpression()->GetTypeRef();

    if (!left_type || !right_type || left_type->IsFunction() ||
        right_type->IsFunction()) {
        ReportError("operands of assignment must be non-function types");
        return;
    }

    std::unique_ptr<Expression> converted_right_expr =
        ConvertByAssignment(expression->ExtractRightExpression(), left_type);
    if (!converted_right_expr) {
        return;
    }
    expression->SetRightExpression(std::move(converted_right_expr));

    expression->SetTypeRef(left_type);
}

void TypeChecker::Visit(CastExpression* expression) {
    Expression* inner = expression->GetExpression();
    if (!inner->GetTypeRef()) {
        inner->Accept(this);
    }

    TypeRef from_type = inner->GetTypeRef();
    TypeRef to_type = ResolveTypeName(expression->GetTypeName());

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
            std::unique_ptr<Expression> converted =
                ConvertByAssignment(statement->ExtractExpression(), current_return_type_);
            if (!converted) {
                return;
            }
            statement->SetExpression(std::move(converted));
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
    TypeRef base_type = ResolvePrimitiveType(type_spec);
    TypeRef type = ResolveDeclaratorType(base_type, declarator);
    if (!type) {
        ReportError("internal error: parameter declaration has no declarator");
        return;
    }

    if (auto func_declarator = dynamic_cast<FunctionDeclarator*>(declarator)) {
        ReportError("parameter '" + func_declarator->GetId() + "' cannot be a function");
        return;
    } else if (auto id_declarator = GetIdentifierDeclarator(declarator)) {
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
    if (!function_type || !function_type->IsFunction()) {
        ReportError("called expression is not a function");
        return;
    }

    FunctionType* func_type = dynamic_cast<FunctionType*>(function_type.get());

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
            TypeRef param_type = func_type->GetParamTypes()[index];
            std::unique_ptr<Expression> converted =
                ConvertByAssignment(std::move(args[index]), param_type);
            if (!converted) {
                return;
            }
            args[index] = std::move(converted);
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
    if (!info || (info->type && info->type->IsFunction())) {
        ReportError("id declarator'" + declarator->GetId() + "' is not a variable");
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
    if (!info || (info->type && !info->type->IsFunction())) {
        ReportError("function declarator'" + declarator->GetId() +
                    "' is already declared as a variable");
    }
}

void TypeChecker::Visit(PointerDeclarator* declarator) {}

void TypeChecker::Visit(TypeName* type_name) {}

void TypeChecker::Visit(PointerAbstractDeclarator* declarator) {}

void TypeChecker::Visit(AddressExpression* expression) {
    expression->GetExpression()->Accept(this);
    if (!IsLValue(expression->GetExpression())) {
        ReportError("address operator requires an lvalue operand");
        return;
    }

    TypeRef new_type =
        std::make_shared<PointerType>(expression->GetExpression()->GetTypeRef());
    expression->SetTypeRef(new_type);
}

void TypeChecker::Visit(DereferenceExpression* expression) {
    expression->GetExpression()->Accept(this);
    TypeRef pointer_type = expression->GetExpression()->GetTypeRef();
    if (!pointer_type || !pointer_type->IsPointer()) {
        ReportError("dereference operator requires a pointer operand");
        return;
    }

    auto dereferenced_type = std::dynamic_pointer_cast<PointerType>(pointer_type);
    if (!dereferenced_type) {
        ReportError("internal error: pointer type expected for dereference");
        return;
    }

    expression->SetTypeRef(dereferenced_type->GetBaseType());
}

////////////////////////////////////////////////////////////////////

const std::vector<std::string>& TypeChecker::GetErrors() const { return errors_; }

void TypeChecker::ReportError(const std::string& message) { errors_.push_back(message); }

bool TypeChecker::IsLValue(Expression* expression) const {
    return dynamic_cast<IdExpression*>(expression) != nullptr ||
           dynamic_cast<DereferenceExpression*>(expression) != nullptr;
}

bool TypeChecker::CanCast(TypeRef from, TypeRef to) {
    if (!from || !to) {
        return false;
    }
    if (from->Equals(to)) {
        return true;
    }

    if (from->IsArithmetic() && to->IsArithmetic()) {
        return true;
    }

    if ((from->IsIntegral() && to->IsPointer()) ||
        (from->IsPointer() && to->IsIntegral()) ||
        (from->IsPointer() && to->IsPointer())) {
        return true;
    }

    return false;
}

std::unique_ptr<Expression> TypeChecker::PerformCompileTimeCast(
    std::unique_ptr<Expression> expression, TypeRef target_type) {
    auto* primary = dynamic_cast<PrimaryExpression*>(expression.get());
    if (!primary) {
        ReportError("cannot cast expression of type '" +
                    expression->GetTypeRef()->ToString() + "' to '" +
                    target_type->ToString() + "'");
        return nullptr;
    }
    if (target_type->IsPointer()) {
        if (!IsNullPointerConstant(primary)) {
            ReportError("cannot initialize pointer with non-zero constant");
            return nullptr;
        }
        primary->GetValue() = NumericConstant(static_cast<unsigned long>(0));
        return expression;
    }
    primary->GetValue().CastTo(target_type);
    return expression;
}

std::unique_ptr<Expression> TypeChecker::ConvertByAssignment(
    std::unique_ptr<Expression> expression, TypeRef target_type) {
    TypeRef from_type = expression->GetTypeRef();
    if (!from_type) {
        expression->Accept(this);
        from_type = expression->GetTypeRef();
    }
    if (!from_type || !target_type) {
        ReportError(
            "cannot convert expression by assignment: invalid source or target "
            "type");
        return nullptr;
    }
    if (from_type->IsFunction() || target_type->IsFunction()) {
        ReportError("cannot convert function type by assignment");
        return nullptr;
    }

    bool can_convert = from_type->Equals(target_type) ||
                       (from_type->IsArithmetic() && target_type->IsArithmetic()) ||
                       (target_type->IsPointer() && from_type->IsIntegral() &&
                        IsNullPointerConstant(expression.get()));
    if (!can_convert) {
        ReportError("cannot convert expression of type '" + from_type->ToString() +
                    "' to '" + target_type->ToString() + "' by assignment");
        return nullptr;
    }
    if (from_type->Equals(target_type)) {
        return expression;
    }

    std::unique_ptr<TypeName> type_name = GetTypeName(target_type);
    if (!type_name) {
        ReportError("cannot build type name for assignment target '" +
                    target_type->ToString() + "'");
        return nullptr;
    }

    auto cast_expr =
        std::make_unique<CastExpression>(std::move(type_name), std::move(expression));
    cast_expr->SetTypeRef(target_type);
    return cast_expr;
}

std::unique_ptr<Expression> TypeChecker::WrapWithCast(
    std::unique_ptr<Expression> expression, TypeRef target_type) {
    return ConvertByAssignment(std::move(expression), target_type);
}

TypeRef TypeChecker::GetCommonType(TypeRef type1, TypeRef type2) {
    if (type1->Equals(type2)) {
        return type1;
    }
    if (type1->IsFloatingPoint() || type2->IsFloatingPoint()) {
        return type1->IsFloatingPoint() ? type1 : type2;
    }

    if (type1->IsSigned() == type2->IsSigned()) {
        return type1->Size() >= type2->Size() ? type1 : type2;
    }

    TypeRef signed_type = type1->IsSigned() ? type1 : type2;
    TypeRef unsigned_type = type1->IsSigned() ? type2 : type1;

    if (unsigned_type->Size() >= signed_type->Size()) {
        return unsigned_type;
    }

    return signed_type;
}

TypeRef TypeChecker::GetCommonPointerType(Expression* left, Expression* right) {
    TypeRef left_type = left->GetTypeRef();
    TypeRef right_type = right->GetTypeRef();
    if (!left_type || !right_type) {
        ReportError(
            "internal error: missing operand type in pointer compatibility check");
        return nullptr;
    }

    if (left_type->IsPointer() && right_type->IsPointer() &&
        left_type->Equals(right_type)) {
        return left_type;
    }

    bool left_is_null = IsNullPointerConstant(left);
    bool right_is_null = IsNullPointerConstant(right);
    if (left_type->IsPointer() && right_is_null) {
        return left_type;
    }
    if (right_type->IsPointer() && left_is_null) {
        return right_type;
    }

    ReportError("incompatible pointer operands: '" + left_type->ToString() + "' and '" +
                right_type->ToString() + "'");
    return nullptr;
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
        case TypeSpecification::Type::Double:
            return PrimitiveType::GetDouble();
    }
}

TypeRef TypeChecker::ResolveTypeName(TypeName* type_name) {
    TypeRef base_type = ResolvePrimitiveType(type_name->GetTypeSpecification());
    if (!base_type) {
        return nullptr;
    }
    if (!type_name->HasAbstractDeclarator()) {
        return base_type;
    }
    return ResolveAbstractDeclaratorType(base_type, type_name->GetAbstractDeclarator());
}

TypeRef TypeChecker::ResolveDeclaratorType(TypeRef base_type, Declarator* declarator) {
    if (dynamic_cast<IdentifierDeclarator*>(declarator)) {
        return base_type;
    }
    if (auto* pointer_declarator = dynamic_cast<PointerDeclarator*>(declarator)) {
        TypeRef inner_type =
            ResolveDeclaratorType(base_type, pointer_declarator->GetDeclarator());
        if (!inner_type) {
            return nullptr;
        }
        return std::make_shared<PointerType>(inner_type);
    }
    if (auto* function_declarator = dynamic_cast<FunctionDeclarator*>(declarator)) {
        return ResolveDeclaratorType(base_type, function_declarator->GetDeclarator());
    }
    return nullptr;
}

TypeRef TypeChecker::ResolveAbstractDeclaratorType(TypeRef base_type,
                                                   AbstractDeclarator* declarator) {
    if (!base_type || !declarator) {
        return nullptr;
    }

    auto* pointer_declarator = dynamic_cast<PointerAbstractDeclarator*>(declarator);
    if (!pointer_declarator) {
        ReportError("unsupported abstract declarator in type name");
        return nullptr;
    }

    TypeRef resolved_base = base_type;
    if (pointer_declarator->HasBase()) {
        resolved_base =
            ResolveAbstractDeclaratorType(base_type, pointer_declarator->GetBase());
        if (!resolved_base) {
            return nullptr;
        }
    }
    return std::make_shared<PointerType>(resolved_base);
}

std::unique_ptr<TypeName> TypeChecker::GetTypeName(TypeRef type) {
    if (!type) {
        return nullptr;
    }

    size_t pointer_depth = 0;
    TypeRef base_type = type;
    while (base_type->IsPointer()) {
        auto pointer_type = std::dynamic_pointer_cast<PointerType>(base_type);
        if (!pointer_type) {
            return nullptr;
        }
        base_type = pointer_type->GetBaseType();
        ++pointer_depth;
    }

    TypeSpecifierSet specifiers;
    if (base_type->Equals(PrimitiveType::GetInt32())) {
        specifiers.Add(TypeSpecifierSet::Specifier::Int);
    } else if (base_type->Equals(PrimitiveType::GetInt64())) {
        specifiers.Add(TypeSpecifierSet::Specifier::Long);
    } else if (base_type->Equals(PrimitiveType::GetUInt32())) {
        specifiers.Add(TypeSpecifierSet::Specifier::Unsigned);
    } else if (base_type->Equals(PrimitiveType::GetUInt64())) {
        specifiers.Add(TypeSpecifierSet::Specifier::Unsigned);
        specifiers.Add(TypeSpecifierSet::Specifier::Long);
    } else if (base_type->Equals(PrimitiveType::GetDouble())) {
        specifiers.Add(TypeSpecifierSet::Specifier::Double);
    } else {
        return nullptr;
    }

    if (pointer_depth == 0) {
        return std::make_unique<TypeName>(specifiers);
    }

    std::unique_ptr<AbstractDeclarator> abstract_declarator = nullptr;
    for (size_t idx = 0; idx < pointer_depth; ++idx) {
        abstract_declarator =
            std::make_unique<PointerAbstractDeclarator>(std::move(abstract_declarator));
    }
    return std::make_unique<TypeName>(specifiers, std::move(abstract_declarator));
}

IdentifierDeclarator* TypeChecker::GetIdentifierDeclarator(Declarator* declarator) {
    if (auto* id_declarator = dynamic_cast<IdentifierDeclarator*>(declarator)) {
        return id_declarator;
    }
    if (auto* pointer_declarator = dynamic_cast<PointerDeclarator*>(declarator)) {
        return GetIdentifierDeclarator(pointer_declarator->GetDeclarator());
    }
    return nullptr;
}

bool TypeChecker::IsNullPointerConstant(const Expression* expression) const {
    auto* primary = dynamic_cast<const PrimaryExpression*>(expression);
    if (!primary) {
        return false;
    }

    NumericConstant value = primary->GetValue();
    if (value.IsFloatingPoint()) {
        return false;
    }
    return value.AsUInt64() == 0;
}

TypeRef TypeChecker::ResolveFunctionType(Declarator* full_declarator,
                                         FunctionDeclarator* func_declarator,
                                         TypeSpecification* return_type_spec) {
    if (!return_type_spec) {
        ReportError("type specification of return type is empty");
        return nullptr;
    }
    TypeRef base_return_type = ResolvePrimitiveType(return_type_spec);
    if (!base_return_type) {
        return nullptr;
    }

    std::vector<TypeRef> param_types;
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
                if (!param_type_spec) {
                    ReportError("Parameter has no type");
                    return nullptr;
                }
                TypeRef param_base_type = ResolvePrimitiveType(param_type_spec);
                TypeRef param_type =
                    ResolveDeclaratorType(param_base_type, param->GetDeclarator());
                if (!param_type) {
                    ReportError("Parameter has invalid type");
                    return nullptr;
                }
                param_types.push_back(param_type);
            }
        }
    }

    int pointer_levels = CountPointerLevelsBeforeFunction(full_declarator);
    TypeRef return_type = base_return_type;
    for (int idx = 0; idx < pointer_levels; ++idx) {
        return_type = std::make_shared<PointerType>(return_type);
    }

    return std::make_shared<FunctionType>(return_type, std::move(param_types));
}

bool TypeChecker::ProcessFunctionDeclaration(FunctionDeclarator* func_declarator,
                                             TypeSpecification* return_type_spec,
                                             StorageClass storage_class,
                                             bool is_definition,
                                             Declarator* full_declarator) {
    Declarator* declarator_for_type = full_declarator ? full_declarator : func_declarator;
    TypeRef func_type =
        ResolveFunctionType(declarator_for_type, func_declarator, return_type_spec);
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
    std::optional<NumericConstant> new_init_constant;

    if (id_declarator->HasInitializer()) {
        auto* primary = dynamic_cast<PrimaryExpression*>(id_declarator->GetInitializer());
        if (!primary) {
            ReportError("initializer of file scope variable '" + name +
                        "' is not a constant expression");
            return false;
        }
        primary->Accept(this);
        new_init_state = SymbolInfo::InitialValue::Initial;
        TypeRef init_type = primary->GetTypeRef();
        if (init_type && !init_type->Equals(declared_type)) {
            auto wrapped = PerformCompileTimeCast(id_declarator->ExtractInitializer(),
                                                  declared_type);
            if (!wrapped) {
                return false;
            }
            id_declarator->SetInitializer(std::move(wrapped));
        }
        new_init_constant =
            dynamic_cast<PrimaryExpression*>(id_declarator->GetInitializer())->GetValue();
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
                auto converted = ConvertByAssignment(id_declarator->ExtractInitializer(),
                                                     declared_type);
                if (!converted) {
                    return false;
                }
                id_declarator->SetInitializer(std::move(converted));
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
            auto converted =
                ConvertByAssignment(id_declarator->ExtractInitializer(), declared_type);
            if (!converted) {
                return false;
            }
            id_declarator->SetInitializer(std::move(converted));
        }
    }
    return true;
}
