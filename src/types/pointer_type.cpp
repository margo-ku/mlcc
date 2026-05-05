#include "include/types/pointer_type.h"

#include <string>

PointerType::PointerType(TypeRef base_type)
    : Type(Type::Kind::Pointer), base_type_(base_type) {}

size_t PointerType::Size() const { return 8; }

size_t PointerType::Alignment() const { return 8; }

bool PointerType::IsPointer() const { return true; }

std::string PointerType::ToString() const {
    return base_type_->ToString() + std::string("*");
}

bool PointerType::Equals(const TypeRef& other) const {
    if (auto other_ptr = dynamic_cast<PointerType*>(other.get())) {
        return base_type_->Equals(other_ptr->base_type_);
    }
    return false;
}

TypeRef PointerType::GetBaseType() const { return base_type_; }