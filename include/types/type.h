#pragma once
#include <memory>

class Type;
using TypeRef = std::shared_ptr<Type>;

class Type {
public:
    enum class Kind {
        Primitive,
        Function,
        // Pointer, Void, Array, Struct, etc.
    };

    explicit Type(Kind k) : kind_(k) {}
    virtual ~Type() = default;

    Kind GetKind() const noexcept { return kind_; }
    virtual size_t Size() const = 0;
    virtual size_t Alignment() const = 0;

    virtual bool IsIntegral() const = 0;
    virtual bool IsSigned() const = 0;
    virtual bool IsInt() const = 0;
    virtual bool IsLong() const = 0;

    virtual std::string ToString() const = 0;

    virtual bool Equals(const TypeRef& other) const { return this == other.get(); }

private:
    Kind kind_;
};