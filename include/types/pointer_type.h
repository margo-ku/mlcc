#pragma once

#include "type.h"

class PointerType : public Type {
public:
    explicit PointerType(TypeRef base_type);

    size_t Size() const override;
    size_t Alignment() const override;
    bool IsPointer() const override;
    std::string ToString() const override;
    bool Equals(const TypeRef& other) const override;

    TypeRef GetBaseType() const;

private:
    TypeRef base_type_;
};