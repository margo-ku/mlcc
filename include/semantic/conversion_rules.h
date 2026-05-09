#pragma once

#include <memory>

#include "include/ast/expressions.h"
#include "include/semantic/type_resolver.h"

class ConversionRules {
public:
    struct BinaryOpResult {
        TypeRef operand_type;
        TypeRef result_type;
    };

    ConversionRules(TypeResolver& type_resolver,
                    TypeResolver::DiagnosticReporter report_error);

    bool IsScalar(TypeRef type) const;
    bool IsLValue(Expression* expression) const;
    bool IsNullPointerConstant(const Expression* expression) const;
    bool CanExplicitCast(TypeRef from, TypeRef to) const;

    TypeRef CommonArithmeticType(TypeRef left, TypeRef right) const;
    TypeRef CommonPointerType(Expression* left, Expression* right);

    std::unique_ptr<Expression> ConvertByAssignment(
        std::unique_ptr<Expression> expression, TypeRef target_type);
    std::unique_ptr<Expression> ConvertToCommonType(
        std::unique_ptr<Expression> expression, TypeRef target_type);
    std::unique_ptr<Expression> ConvertConstantInitializer(
        std::unique_ptr<Expression> expression, TypeRef target_type);

private:
    void ReportError(const std::string& message);

    TypeResolver& type_resolver_;
    TypeResolver::DiagnosticReporter report_error_;
};
