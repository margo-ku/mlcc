#pragma once
#include <vector>

#include "type.h"

class FunctionType : public Type {
public:
    explicit FunctionType(TypeRef return_type, std::vector<TypeRef> param_types = {});

    TypeRef GetReturnType() const { return return_type_; }
    const std::vector<TypeRef>& GetParamTypes() const { return param_types_; }
    size_t GetParameterCount() const { return param_types_.size(); }

    size_t Size() const override;
    size_t Alignment() const override;
    bool IsIntegral() const override;
    bool IsArithmetic() const override;
    bool IsSigned() const override;
    bool IsInt() const override;
    bool IsLong() const override;
    std::string ToString() const override;
    bool IsFloatingPoint() const override;

    bool Equals(const TypeRef& other) const override;

private:
    TypeRef return_type_;
    std::vector<TypeRef> param_types_;
};
