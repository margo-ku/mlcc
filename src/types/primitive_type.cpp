#include "include/types/primitive_type.h"

#include <mutex>

PrimitiveType::PrimitiveType(Tag tag) : Type(Type::Kind::Primitive), tag_(tag) {}

PrimitiveType::Tag PrimitiveType::GetTag() const { return tag_; }

std::size_t PrimitiveType::Size() const {
    switch (tag_) {
        case Tag::Int32:
        case Tag::UInt32:
            return 4;
        case Tag::Int64:
        case Tag::UInt64:
            return 8;
    }
    return 0;
}

std::size_t PrimitiveType::Alignment() const { return Size(); }

bool PrimitiveType::IsIntegral() const { return true; }

bool PrimitiveType::IsSigned() const { return tag_ == Tag::Int32 || tag_ == Tag::Int64; }

bool PrimitiveType::IsInt() const { return tag_ == Tag::Int32 || tag_ == Tag::UInt32; }

bool PrimitiveType::IsLong() const { return tag_ == Tag::Int64 || tag_ == Tag::UInt64; }

std::string PrimitiveType::ToString() const {
    switch (tag_) {
        case Tag::Int32:
            return "int";
        case Tag::Int64:
            return "long";
        case Tag::UInt32:
            return "unsigned int";
        case Tag::UInt64:
            return "unsigned long";
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
    return ptr->tag_ == tag_;
}

//////////////////////////////////////////////////

TypeRef& PrimitiveType::GetInstance(Tag tag) {
    static TypeRef instances[4];
    static std::once_flag flags[4];
    int idx = static_cast<int>(tag);
    std::call_once(flags[idx], [&]() {
        instances[idx] = std::make_shared<PrimitiveType>(tag);
    });
    return instances[idx];
}

TypeRef PrimitiveType::GetInt32() { return GetInstance(Tag::Int32); }
TypeRef PrimitiveType::GetInt64() { return GetInstance(Tag::Int64); }
TypeRef PrimitiveType::GetUInt32() { return GetInstance(Tag::UInt32); }
TypeRef PrimitiveType::GetUInt64() { return GetInstance(Tag::UInt64); }