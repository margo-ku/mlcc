#include "include/semantic/expression_type_checker.h"

#include <memory>

#include "include/ast/declarations.h"
#include "include/ast/expressions.h"
#include "include/semantic/declarator_utils.h"
#include "include/types/function_type.h"
#include "include/types/pointer_type.h"
#include "include/types/primitive_type.h"

namespace {

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

ExpressionTypeChecker::ExpressionTypeChecker(SymbolTable& symbol_table)
    : symbol_table_(symbol_table),
      type_resolver_([this](const std::string& message) { ReportError(message); }),
      conversions_(type_resolver_,
                   [this](const std::string& message) { ReportError(message); }) {}

void ExpressionTypeChecker::Visit(TranslationUnit* translation_unit) {
    in_file_scope_ = true;
    for (auto& declaration : translation_unit->GetExternalDeclarations()) {
        declaration->Accept(this);
    }
}

void ExpressionTypeChecker::Visit(FunctionDefinition* function) {
    auto* function_declarator =
        declarator_utils::FindFunctionDeclarator(function->GetDeclarator());

    SymbolInfo* info = symbol_table_.FindByUniqueName(function_declarator->GetId());
    auto function_type = std::dynamic_pointer_cast<FunctionType>(info->type);

    TypeRef saved_return_type = current_return_type_;
    bool saved_file_scope = in_file_scope_;
    current_return_type_ = function_type->GetReturnType();
    in_file_scope_ = false;

    function->GetBody()->Accept(this);

    current_return_type_ = saved_return_type;
    in_file_scope_ = saved_file_scope;
}

void ExpressionTypeChecker::Visit(Declaration* declaration) {
    Declarator* declarator = declaration->GetDeclaration();
    if (declarator_utils::FindFunctionDeclarator(declarator)) {
        return;
    }

    auto* id_declarator = declarator_utils::FindIdentifierDeclarator(declarator);
    if (!id_declarator->HasInitializer()) {
        return;
    }

    SymbolInfo* info = symbol_table_.FindByUniqueName(id_declarator->GetId());
    CheckInitializerForObject(
        id_declarator, info,
        declaration->GetDeclarationSpecifiers()->GetStorageClass());
}

void ExpressionTypeChecker::Visit(IdExpression* expression) {
    std::string name = expression->GetId();
    SymbolInfo* info = symbol_table_.FindByUniqueName(name);
    expression->SetTypeRef(info->type);
}

void ExpressionTypeChecker::Visit(PrimaryExpression* expression) {
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

void ExpressionTypeChecker::Visit(UnaryExpression* expression) {
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

std::optional<ConversionRules::BinaryOpResult>
ExpressionTypeChecker::CheckBinaryOperands(BinaryExpression::BinaryOperator op,
                                           TypeRef left_type, TypeRef right_type,
                                           Expression* left_expr,
                                           Expression* right_expr) {
    switch (ClassifyBinaryOp(op)) {
        case BinaryOpClass::Integral:
            if (!left_type->IsIntegral() || !right_type->IsIntegral()) {
                ReportError(
                    "operands of bitwise/modulo operation must be integral types");
                return std::nullopt;
            }
            {
                TypeRef common = conversions_.CommonArithmeticType(left_type, right_type);
                return ConversionRules::BinaryOpResult{common, common};
            }

        case BinaryOpClass::Arithmetic:
            if (!left_type->IsArithmetic() || !right_type->IsArithmetic()) {
                ReportError("operands of arithmetic operation must be arithmetic types");
                return std::nullopt;
            }
            {
                TypeRef common = conversions_.CommonArithmeticType(left_type, right_type);
                return ConversionRules::BinaryOpResult{common, common};
            }

        case BinaryOpClass::Comparison:
            if (!left_type->IsArithmetic() || !right_type->IsArithmetic()) {
                ReportError("operands of comparison must be arithmetic types");
                return std::nullopt;
            }
            return ConversionRules::BinaryOpResult{
                conversions_.CommonArithmeticType(left_type, right_type),
                PrimitiveType::GetInt32()};

        case BinaryOpClass::Equality:
            if (left_type->IsPointer() || right_type->IsPointer()) {
                TypeRef common = conversions_.CommonPointerType(left_expr, right_expr);
                if (!common) return std::nullopt;
                return ConversionRules::BinaryOpResult{common, PrimitiveType::GetInt32()};
            }
            if (!left_type->IsArithmetic() || !right_type->IsArithmetic()) {
                ReportError(
                    "operands of equality comparison must be arithmetic or pointer "
                    "types");
                return std::nullopt;
            }
            return ConversionRules::BinaryOpResult{
                conversions_.CommonArithmeticType(left_type, right_type),
                PrimitiveType::GetInt32()};

        case BinaryOpClass::Logical:
            if (!conversions_.IsScalar(left_type) || !conversions_.IsScalar(right_type)) {
                ReportError("operands of logical operation must be scalar types");
                return std::nullopt;
            }
            return ConversionRules::BinaryOpResult{nullptr, PrimitiveType::GetInt32()};
    }
}

void ExpressionTypeChecker::Visit(BinaryExpression* expression) {
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
        auto wrapped_left = conversions_.ConvertToCommonType(
            expression->ExtractLeftExpression(), result->operand_type);
        if (!wrapped_left) {
            return;
        }
        expression->SetLeftExpression(std::move(wrapped_left));

        auto wrapped_right = conversions_.ConvertToCommonType(
            expression->ExtractRightExpression(), result->operand_type);
        if (!wrapped_right) {
            return;
        }
        expression->SetRightExpression(std::move(wrapped_right));
    }

    expression->SetTypeRef(result->result_type);
}

void ExpressionTypeChecker::Visit(ConditionalExpression* expression) {
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
        common_type = conversions_.CommonPointerType(expression->GetLeftExpression(),
                                                    expression->GetRightExpression());
    } else {
        if (!left_type->IsArithmetic() || !right_type->IsArithmetic()) {
            ReportError("conditional expression operands must be arithmetic types");
            return;
        }
        common_type = conversions_.CommonArithmeticType(left_type, right_type);
    }
    if (!common_type) {
        return;
    }

    std::unique_ptr<Expression> wrapped_left_expr =
        conversions_.ConvertToCommonType(expression->ExtractLeftExpression(), common_type);
    if (!wrapped_left_expr) {
        return;
    }
    expression->SetLeftExpression(std::move(wrapped_left_expr));

    std::unique_ptr<Expression> wrapped_right_expr = conversions_.ConvertToCommonType(
        expression->ExtractRightExpression(), common_type);
    if (!wrapped_right_expr) {
        return;
    }
    expression->SetRightExpression(std::move(wrapped_right_expr));

    expression->SetTypeRef(common_type);
}

void ExpressionTypeChecker::Visit(AssignmentExpression* expression) {
    Expression* left_expression = expression->GetLeftExpression();
    if (!conversions_.IsLValue(left_expression)) {
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
        conversions_.ConvertByAssignment(expression->ExtractRightExpression(), left_type);
    if (!converted_right_expr) {
        return;
    }
    expression->SetRightExpression(std::move(converted_right_expr));

    expression->SetTypeRef(left_type);
}

void ExpressionTypeChecker::Visit(CastExpression* expression) {
    if (!CheckTypeNameExpressions(expression->GetTypeName())) {
        return;
    }

    Expression* inner = expression->GetExpression();
    if (!inner->GetTypeRef()) {
        inner->Accept(this);
    }

    TypeRef from_type = inner->GetTypeRef();
    TypeRef to_type = type_resolver_.ResolveTypeName(expression->GetTypeName());

    if (!from_type || !to_type) {
        ReportError("invalid types in cast expression");
        return;
    }

    if (!conversions_.CanExplicitCast(from_type, to_type)) {
        ReportError("cannot cast expression of type '" + from_type->ToString() +
                    "' to '" + to_type->ToString() + "'");
        return;
    }

    expression->SetTypeRef(to_type);
}

void ExpressionTypeChecker::Visit(ReturnStatement* statement) {
    if (statement->HasExpression()) {
        statement->GetExpression()->Accept(this);

        if (current_return_type_) {
            std::unique_ptr<Expression> converted =
                conversions_.ConvertByAssignment(statement->ExtractExpression(),
                                                 current_return_type_);
            if (!converted) {
                return;
            }
            statement->SetExpression(std::move(converted));
        }
    }
}

void ExpressionTypeChecker::Visit(ForStatement* statement) {
    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);
}

void ExpressionTypeChecker::Visit(FunctionCallExpression* expression) {
    Expression* function_expression = expression->GetFunction();
    function_expression->Accept(this);

    TypeRef function_type = function_expression->GetTypeRef();
    if (!function_type || !function_type->IsFunction()) {
        ReportError("called expression is not a function");
        return;
    }

    auto* func_type = dynamic_cast<FunctionType*>(function_type.get());

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
                conversions_.ConvertByAssignment(std::move(args[index]), param_type);
            if (!converted) {
                return;
            }
            args[index] = std::move(converted);
        }
    }

    expression->SetTypeRef(func_type->GetReturnType());
}

void ExpressionTypeChecker::Visit(SubscriptExpression* expression) {
    // to do
}

void ExpressionTypeChecker::Visit(SingleInitializer* initializer) {
    initializer->GetExpression()->Accept(this);
    initializer->SetTypeRef(initializer->GetExpression()->GetTypeRef());
}

void ExpressionTypeChecker::Visit(CompoundInitializer* initializer) {
    for (auto& item : initializer->GetInitializers()) {
        item->Accept(this);
    }
}

void ExpressionTypeChecker::Visit(AddressExpression* expression) {
    expression->GetExpression()->Accept(this);
    if (!conversions_.IsLValue(expression->GetExpression())) {
        ReportError("address operator requires an lvalue operand");
        return;
    }

    TypeRef new_type =
        std::make_shared<PointerType>(expression->GetExpression()->GetTypeRef());
    expression->SetTypeRef(new_type);
}

void ExpressionTypeChecker::Visit(DereferenceExpression* expression) {
    expression->GetExpression()->Accept(this);
    TypeRef pointer_type = expression->GetExpression()->GetTypeRef();
    if (!pointer_type || !pointer_type->IsPointer()) {
        ReportError("dereference operator requires a pointer operand");
        return;
    }

    auto dereferenced_type = std::dynamic_pointer_cast<PointerType>(pointer_type);
    expression->SetTypeRef(dereferenced_type->GetBaseType());
}

bool ExpressionTypeChecker::CheckInitializerForObject(IdentifierDeclarator* declarator,
                                                      SymbolInfo* info,
                                                      StorageClass storage_class) {
    std::string name = declarator->GetId();
    auto* single_init = declarator_utils::AsSingleInitializer(declarator->GetInitializer());
    if (!single_init) {
        if (in_file_scope_) {
            ReportError("file scope variable '" + name +
                        "' has unsupported non-scalar initializer");
        } else if (storage_class == StorageClass::Static) {
            ReportError("local static '" + name +
                        "' has unsupported non-scalar initializer");
        } else {
            ReportError("local variable '" + name +
                        "' has unsupported non-scalar initializer");
        }
        return false;
    }

    if (info->HasStaticDuration()) {
        auto* primary = dynamic_cast<PrimaryExpression*>(single_init->GetExpression());
        if (!primary) {
            if (in_file_scope_) {
                ReportError("initializer of file scope variable '" + name +
                            "' is not a constant expression");
            } else {
                ReportError("non-constant initializer on local static variable '" + name +
                            "'");
            }
            return false;
        }

        primary->Accept(this);
        if (in_file_scope_) {
            TypeRef init_type = primary->GetTypeRef();
            if (init_type && !init_type->Equals(info->type)) {
                auto converted = conversions_.ConvertConstantInitializer(
                    single_init->ExtractExpression(), info->type);
                if (!converted) {
                    return false;
                }
                single_init->SetExpression(std::move(converted));
            }
            info->init_constant =
                dynamic_cast<PrimaryExpression*>(single_init->GetExpression())
                    ->GetValue();
        } else {
            info->init_constant = primary->GetValue();
            TypeRef init_type = primary->GetTypeRef();
            if (init_type && !init_type->Equals(info->type)) {
                auto converted = conversions_.ConvertByAssignment(
                    single_init->ExtractExpression(), info->type);
                if (!converted) {
                    return false;
                }
                single_init->SetExpression(std::move(converted));
            }
        }
        return true;
    }

    Expression* init = single_init->GetExpression();
    init->Accept(this);
    TypeRef init_type = init->GetTypeRef();
    if (init_type && !init_type->Equals(info->type)) {
        auto converted =
            conversions_.ConvertByAssignment(single_init->ExtractExpression(), info->type);
        if (!converted) {
            return false;
        }
        single_init->SetExpression(std::move(converted));
    }
    return true;
}

bool ExpressionTypeChecker::CheckTypeNameExpressions(TypeName* type_name) {
    if (!type_name->HasAbstractDeclarator()) {
        return true;
    }
    return CheckAbstractDeclaratorExpressions(type_name->GetAbstractDeclarator());
}

bool ExpressionTypeChecker::CheckAbstractDeclaratorExpressions(
    AbstractDeclarator* declarator) {
    if (auto* pointer_declarator = dynamic_cast<PointerAbstractDeclarator*>(declarator)) {
        if (pointer_declarator->HasBase()) {
            return CheckAbstractDeclaratorExpressions(pointer_declarator->GetBase());
        }
        return true;
    }

    if (auto* array_declarator = dynamic_cast<ArrayAbstractDeclarator*>(declarator)) {
        if (array_declarator->GetSize()) {
            array_declarator->GetSize()->Accept(this);
            if (!array_declarator->GetSize()->GetTypeRef() ||
                !array_declarator->GetSize()->GetTypeRef()->IsIntegral()) {
                ReportError("array abstract declarator size must be integral");
                return false;
            }
        }
        if (array_declarator->HasBase()) {
            return CheckAbstractDeclaratorExpressions(array_declarator->GetBase());
        }
        return true;
    }

    return true;
}
