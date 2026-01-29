#include "include/types/primitive_type.h"

#include <mutex>

PrimitiveType::PrimitiveType(Tag tag, bool is_signed)
    : Type(Type::Kind::Primitive), tag_(tag), is_signed_(is_signed) {}

PrimitiveType::Tag PrimitiveType::GetTag() const { return tag_; }

bool PrimitiveType::IsSignedFlag() const { return is_signed_; }

std::size_t PrimitiveType::Size() const {
    switch (tag_) {
        case Tag::Int32:
            return 4;
        case Tag::Int64:
            return 8;
    }
    return 0;
}

std::size_t PrimitiveType::Alignment() const { return Size(); }

bool PrimitiveType::IsIntegral() const { return true; }

bool PrimitiveType::IsSigned() const { return is_signed_; }

bool PrimitiveType::IsInt() const { return tag_ == Tag::Int32; }

bool PrimitiveType::IsLong() const { return tag_ == Tag::Int64; }

std::string PrimitiveType::ToString() const {
    switch (tag_) {
        case Tag::Int32:
            return is_signed_ ? "int" : "unsigned int";
        case Tag::Int64:
            return is_signed_ ? "long" : "unsigned long";
    }
    return "primitive";
}

bool PrimitiveType::Equals(const TypeRef& other) const {
    if (!other) {
        return false;
    }
    if (this == other.get()) {
        return true;
    }

    auto ptr = std::dynamic_pointer_cast<PrimitiveType>(other);
    if (!ptr) {
        return false;
    }
    return ptr->tag_ == tag_ && ptr->is_signed_ == is_signed_;
}

//////////////////////////////////////////////////

TypeRef PrimitiveType::GetInt32() {
    static TypeRef inst;
    static std::once_flag once;
    std::call_once(once,
                   []() { inst = std::make_shared<PrimitiveType>(Tag::Int32, true); });
    return inst;
}

TypeRef PrimitiveType::GetInt64() {
    static TypeRef inst;
    static std::once_flag once;
    std::call_once(once,
                   []() { inst = std::make_shared<PrimitiveType>(Tag::Int64, true); });
    return inst;
}