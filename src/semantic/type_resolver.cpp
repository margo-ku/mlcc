#include "include/semantic/type_resolver.h"

#include <utility>
#include <vector>

#include "include/semantic/declarator_utils.h"
#include "include/types/function_type.h"
#include "include/types/pointer_type.h"
#include "include/types/primitive_type.h"

TypeResolver::TypeResolver(DiagnosticReporter report_error)
    : report_error_(std::move(report_error)) {}

void TypeResolver::ReportError(const std::string& message) {
    if (report_error_) {
        report_error_(message);
    }
}

TypeRef TypeResolver::ResolvePrimitive(TypeSpecification* type) {
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

TypeRef TypeResolver::ResolveObjectType(TypeRef base_type, Declarator* declarator) {
    if (dynamic_cast<IdentifierDeclarator*>(declarator)) {
        return base_type;
    }
    if (auto* pointer_declarator = dynamic_cast<PointerDeclarator*>(declarator)) {
        TypeRef inner_type =
            ResolveObjectType(base_type, pointer_declarator->GetDeclarator());
        if (!inner_type) {
            return nullptr;
        }
        return std::make_shared<PointerType>(inner_type);
    }
    if (auto* function_declarator = dynamic_cast<FunctionDeclarator*>(declarator)) {
        return ResolveObjectType(base_type, function_declarator->GetDeclarator());
    }
    return nullptr;
}

TypeRef TypeResolver::ResolveObjectType(TypeSpecification* base_spec,
                                        Declarator* declarator) {
    TypeRef base_type = ResolvePrimitive(base_spec);
    if (!base_type) {
        return nullptr;
    }
    return ResolveObjectType(base_type, declarator);
}

TypeRef TypeResolver::ResolveTypeName(TypeName* type_name) {
    TypeRef base_type = ResolvePrimitive(type_name->GetTypeSpecification());
    if (!base_type) {
        return nullptr;
    }
    if (!type_name->HasAbstractDeclarator()) {
        return base_type;
    }
    return ResolveAbstractDeclaratorType(base_type, type_name->GetAbstractDeclarator());
}

TypeRef TypeResolver::ResolveAbstractDeclaratorType(TypeRef base_type,
                                                    AbstractDeclarator* declarator) {
    if (!base_type || !declarator) {
        return nullptr;
    }

    if (auto* array_declarator = dynamic_cast<ArrayAbstractDeclarator*>(declarator)) {
        TypeRef resolved_base = base_type;
        if (array_declarator->HasBase()) {
            resolved_base =
                ResolveAbstractDeclaratorType(base_type, array_declarator->GetBase());
            if (!resolved_base) {
                return nullptr;
            }
        }
        return std::make_shared<PointerType>(resolved_base);
    }

    if (auto* pointer_declarator = dynamic_cast<PointerAbstractDeclarator*>(declarator)) {
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

    ReportError("unsupported abstract declarator in type name");
    return nullptr;
}

TypeRef TypeResolver::ResolveFunctionType(Declarator* full_declarator,
                                          FunctionDeclarator* function_declarator,
                                          TypeSpecification* return_type_spec) {
    if (!return_type_spec) {
        ReportError("type specification of return type is empty");
        return nullptr;
    }
    TypeRef base_return_type = ResolvePrimitive(return_type_spec);
    if (!base_return_type) {
        return nullptr;
    }

    std::vector<TypeRef> param_types;
    if (function_declarator->HasParameters()) {
        ParameterList* list = function_declarator->GetParameters();
        if (list) {
            const auto& params = list->GetParameters();
            for (const auto& param_ptr : params) {
                if (!param_ptr) {
                    continue;
                }

                ParameterDeclaration* param = param_ptr.get();
                TypeSpecification* param_type_spec = param->GetType();
                if (!param_type_spec) {
                    ReportError("Parameter has no type");
                    return nullptr;
                }

                TypeRef param_type =
                    ResolveObjectType(param_type_spec, param->GetDeclarator());
                if (!param_type) {
                    ReportError("Parameter has invalid type");
                    return nullptr;
                }
                param_types.push_back(param_type);
            }
        }
    }

    int pointer_levels =
        declarator_utils::CountPointerLevelsBeforeFunction(full_declarator);
    TypeRef return_type = base_return_type;
    for (int idx = 0; idx < pointer_levels; ++idx) {
        return_type = std::make_shared<PointerType>(return_type);
    }

    return std::make_shared<FunctionType>(return_type, std::move(param_types));
}

std::unique_ptr<TypeName> TypeResolver::BuildTypeName(TypeRef type) {
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
