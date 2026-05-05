#pragma once
#include <memory>

class Type;
using TypeRef = std::shared_ptr<Type>;

class Type {
public:
    enum class Kind {
        Primitive,
        Function,
        Pointer  //, Void, Array, Struct, etc.
    };

    explicit Type(Kind kind) : kind_(kind) {}
    virtual ~Type() = default;

    Kind GetKind() const noexcept { return kind_; }
    virtual size_t Size() const = 0;
    virtual size_t Alignment() const = 0;

    virtual bool IsIntegral() const { return false; }
    virtual bool IsArithmetic() const { return false; }
    virtual bool IsSigned() const { return false; }
    virtual bool IsInt() const { return false; }
    virtual bool IsLong() const { return false; }
    virtual bool IsFloatingPoint() const { return false; }
    virtual bool IsFunction() const { return false; }
    virtual bool IsPointer() const { return false; }

    virtual std::string ToString() const = 0;

    virtual bool Equals(const TypeRef& other) const { return this == other.get(); }

private:
    Kind kind_;
};