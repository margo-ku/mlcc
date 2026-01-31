#pragma once
#include "type.h"

class PrimitiveType : public Type {
public:
    enum class Tag { Int32, Int64, UInt32, UInt64 };

    explicit PrimitiveType(Tag tag);

    Tag GetTag() const;

    static TypeRef GetInt32();
    static TypeRef GetInt64();
    static TypeRef GetUInt32();
    static TypeRef GetUInt64();

    size_t Size() const override;
    size_t Alignment() const override;
    bool IsIntegral() const override;
    bool IsSigned() const override;
    bool IsInt() const override;
    bool IsLong() const override;
    std::string ToString() const override;

    bool Equals(const TypeRef& other) const override;

private:
    static TypeRef& GetInstance(Tag tag);
    Tag tag_;
};
