#pragma once

#include <functional>
#include <memory>
#include <string>

#include "include/ast/declarations.h"
#include "include/types/type.h"

class TypeResolver {
public:
    using DiagnosticReporter = std::function<void(const std::string&)>;

    explicit TypeResolver(DiagnosticReporter report_error);

    TypeRef ResolvePrimitive(TypeSpecification* type);
    TypeRef ResolveObjectType(TypeRef base_type, Declarator* declarator);
    TypeRef ResolveObjectType(TypeSpecification* base_spec, Declarator* declarator);
    TypeRef ResolveTypeName(TypeName* type_name);
    TypeRef ResolveAbstractDeclaratorType(TypeRef base_type,
                                          AbstractDeclarator* declarator);
    TypeRef ResolveFunctionType(Declarator* full_declarator,
                                FunctionDeclarator* function_declarator,
                                TypeSpecification* return_type_spec);
    std::unique_ptr<TypeName> BuildTypeName(TypeRef type);

private:
    void ReportError(const std::string& message);

    DiagnosticReporter report_error_;
};
