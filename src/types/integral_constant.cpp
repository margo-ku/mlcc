#include "include/types/integral_constant.h"

#include <utility>

IntegralConstant::IntegralConstant(int value) : value_(value) {}
IntegralConstant::IntegralConstant(long value) : value_(value) {}
IntegralConstant::IntegralConstant(unsigned int value) : value_(value) {}
IntegralConstant::IntegralConstant(unsigned long value) : value_(value) {}
IntegralConstant::IntegralConstant(Storage value) : value_(std::move(value)) {}

IntegralConstant::Kind IntegralConstant::GetKind() const {
    return std::visit(
        [](auto&& v) -> Kind {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, int>) {
                return Kind::Int32;
            } else if constexpr (std::is_same_v<T, long>) {
                return Kind::Int64;
            } else if constexpr (std::is_same_v<T, unsigned int>) {
                return Kind::UInt32;
            } else {
                return Kind::UInt64;
            }
        },
        value_);
}

bool IntegralConstant::IsSigned() const {
    auto k = GetKind();
    return k == Kind::Int32 || k == Kind::Int64;
}

int64_t IntegralConstant::AsInt64() const {
    return std::visit([](auto&& v) -> int64_t { return static_cast<int64_t>(v); },
                      value_);
}

uint64_t IntegralConstant::AsUInt64() const {
    return std::visit([](auto&& v) -> uint64_t { return static_cast<uint64_t>(v); },
                      value_);
}

std::string IntegralConstant::ToString() const {
    return std::visit([](auto&& v) -> std::string { return std::to_string(v); }, value_);
}

const IntegralConstant::Storage& IntegralConstant::GetStorage() const { return value_; }