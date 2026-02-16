#pragma once
#include "type.h"

class PrimitiveType : public Type {
public:
    enum class Tag { Int32, Int64, UInt32, UInt64, Double };

    explicit PrimitiveType(Tag tag);

    Tag GetTag() const;

    static TypeRef GetInt32();
    static TypeRef GetInt64();
    static TypeRef GetUInt32();
    static TypeRef GetUInt64();
    static TypeRef GetDouble();

    size_t Size() const override;
    size_t Alignment() const override;
    bool IsIntegral() const override;
    bool IsArithmetic() const override;
    bool IsSigned() const override;
    bool IsInt() const override;
    bool IsLong() const override;
    bool IsFloatingPoint() const override;
    std::string ToString() const override;

    bool Equals(const TypeRef& other) const override;

private:
    static TypeRef& GetInstance(Tag tag);
    Tag tag_;
};
