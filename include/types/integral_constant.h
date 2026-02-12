#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <variant>

class IntegralConstant {
public:
    using Storage = std::variant<int, long, unsigned int, unsigned long>;
    enum class Kind { Int32, Int64, UInt32, UInt64 };

    IntegralConstant(int value);
    IntegralConstant(long value);
    IntegralConstant(unsigned int value);
    IntegralConstant(unsigned long value);
    explicit IntegralConstant(Storage value);

    Kind GetKind() const;
    bool IsSigned() const;

    int64_t AsInt64() const;
    uint64_t AsUInt64() const;

    std::string ToString() const;

    template <typename T>
    bool Holds() const {
        static_assert(std::is_same_v<T, int> || std::is_same_v<T, long> ||
                      std::is_same_v<T, unsigned int> ||
                      std::is_same_v<T, unsigned long>);
        return std::holds_alternative<T>(value_);
    }

    template <typename T>
    T Get() const {
        static_assert(std::is_same_v<T, int> || std::is_same_v<T, long> ||
                      std::is_same_v<T, unsigned int> ||
                      std::is_same_v<T, unsigned long>);
        return std::get<T>(value_);
    }

    const Storage& GetStorage() const;

private:
    Storage value_;
};
