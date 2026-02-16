#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>

#include "include/types/type.h"

class NumericConstant {
public:
    using Storage = std::variant<int, long, unsigned int, unsigned long, double>;
    enum class Kind { Int32, Int64, UInt32, UInt64, Double };

    NumericConstant(int value);
    NumericConstant(long value);
    NumericConstant(unsigned int value);
    NumericConstant(unsigned long value);
    NumericConstant(double value);
    explicit NumericConstant(Storage value);

    Kind GetKind() const;
    bool IsSigned() const;
    bool Is64Bit() const;
    bool IsFloatingPoint() const;

    int64_t AsInt64() const;
    uint64_t AsUInt64() const;
    double AsDouble() const;

    std::string ToString() const;
    void CastTo(TypeRef type);

    template <typename T>
    bool Holds() const {
        static_assert(std::is_same_v<T, int> || std::is_same_v<T, long> ||
                      std::is_same_v<T, unsigned int> ||
                      std::is_same_v<T, unsigned long> || std::is_same_v<T, double>);
        return std::holds_alternative<T>(value_);
    }

    template <typename T>
    T Get() const {
        static_assert(std::is_same_v<T, int> || std::is_same_v<T, long> ||
                      std::is_same_v<T, unsigned int> ||
                      std::is_same_v<T, unsigned long> || std::is_same_v<T, double>);
        return std::get<T>(value_);
    }

    const Storage& GetStorage() const;

private:
    Storage value_;
};