#include "include/semantic/conversion_rules.h"

#include <utility>

ConversionRules::ConversionRules(TypeResolver& type_resolver,
                                 TypeResolver::DiagnosticReporter report_error)
    : type_resolver_(type_resolver), report_error_(std::move(report_error)) {}

void ConversionRules::ReportError(const std::string& message) {
    if (report_error_) {
        report_error_(message);
    }
}

bool ConversionRules::IsScalar(TypeRef type) const {
    return type && (type->IsArithmetic() || type->IsPointer());
}

bool ConversionRules::IsLValue(Expression* expression) const {
    return dynamic_cast<IdExpression*>(expression) != nullptr ||
           dynamic_cast<DereferenceExpression*>(expression) != nullptr ||
           dynamic_cast<SubscriptExpression*>(expression) != nullptr;
}

bool ConversionRules::IsNullPointerConstant(const Expression* expression) const {
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

bool ConversionRules::CanExplicitCast(TypeRef from, TypeRef to) const {
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

TypeRef ConversionRules::CommonArithmeticType(TypeRef left, TypeRef right) const {
    if (left->Equals(right)) {
        return left;
    }
    if (left->IsFloatingPoint() || right->IsFloatingPoint()) {
        return left->IsFloatingPoint() ? left : right;
    }

    if (left->IsSigned() == right->IsSigned()) {
        return left->Size() >= right->Size() ? left : right;
    }

    TypeRef signed_type = left->IsSigned() ? left : right;
    TypeRef unsigned_type = left->IsSigned() ? right : left;

    if (unsigned_type->Size() >= signed_type->Size()) {
        return unsigned_type;
    }
    return signed_type;
}

TypeRef ConversionRules::CommonPointerType(Expression* left, Expression* right) {
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

std::unique_ptr<Expression> ConversionRules::ConvertByAssignment(
    std::unique_ptr<Expression> expression, TypeRef target_type) {
    TypeRef from_type = expression->GetTypeRef();
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

    std::unique_ptr<TypeName> type_name = type_resolver_.BuildTypeName(target_type);
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

std::unique_ptr<Expression> ConversionRules::ConvertToCommonType(
    std::unique_ptr<Expression> expression, TypeRef target_type) {
    return ConvertByAssignment(std::move(expression), target_type);
}

std::unique_ptr<Expression> ConversionRules::ConvertConstantInitializer(
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
