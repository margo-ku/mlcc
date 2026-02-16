#include "include/types/numeric_constant.h"

#include <iomanip>
#include <sstream>
#include <utility>

#include "include/types/type.h"

NumericConstant::NumericConstant(int value) : value_(value) {}
NumericConstant::NumericConstant(long value) : value_(value) {}
NumericConstant::NumericConstant(unsigned int value) : value_(value) {}
NumericConstant::NumericConstant(unsigned long value) : value_(value) {}
NumericConstant::NumericConstant(double value) : value_(value) {}
NumericConstant::NumericConstant(Storage value) : value_(std::move(value)) {}

NumericConstant::Kind NumericConstant::GetKind() const {
    return std::visit(
        [](auto&& v) -> Kind {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, int>) {
                return Kind::Int32;
            } else if constexpr (std::is_same_v<T, long>) {
                return Kind::Int64;
            } else if constexpr (std::is_same_v<T, unsigned int>) {
                return Kind::UInt32;
            } else if constexpr (std::is_same_v<T, unsigned long>) {
                return Kind::UInt64;
            } else {
                return Kind::Double;
            }
        },
        value_);
}

bool NumericConstant::IsSigned() const {
    auto kind = GetKind();
    return kind == Kind::Int32 || kind == Kind::Int64;
}

bool NumericConstant::Is64Bit() const {
    auto kind = GetKind();
    return kind == Kind::UInt64 || kind == Kind::Int64 || kind == Kind::Double;
}

bool NumericConstant::IsFloatingPoint() const { return GetKind() == Kind::Double; }

int64_t NumericConstant::AsInt64() const {
    return std::visit([](auto&& v) -> int64_t { return static_cast<int64_t>(v); },
                      value_);
}

uint64_t NumericConstant::AsUInt64() const {
    return std::visit([](auto&& v) -> uint64_t { return static_cast<uint64_t>(v); },
                      value_);
}

double NumericConstant::AsDouble() const {
    return std::visit([](auto&& v) -> double { return static_cast<double>(v); }, value_);
}

std::string NumericConstant::ToString() const {
    return std::visit(
        [](auto&& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, double>) {
                std::ostringstream oss;
                oss << std::setprecision(17) << v;
                return oss.str();
            } else {
                return std::to_string(v);
            }
        },
        value_);
}

const NumericConstant::Storage& NumericConstant::GetStorage() const { return value_; }

void NumericConstant::CastTo(TypeRef type) {
    if (!type) {
        throw std::runtime_error("NumericConstant::CastTo: null target type");
    }

    if (type->IsFloatingPoint()) {
        value_ = AsDouble();
        return;
    }

    if (type->IsIntegral()) {
        auto signed_value = AsInt64();
        auto unsigned_value = AsUInt64();
        if (type->IsSigned() && type->IsLong()) {
            value_ = signed_value;
        } else if (type->IsSigned()) {
            value_ = IsSigned() ? static_cast<int32_t>(signed_value)
                                : static_cast<int32_t>(unsigned_value);
        } else if (type->IsLong()) {
            value_ = unsigned_value;
        } else {
            value_ = IsSigned() ? static_cast<uint32_t>(signed_value)
                                : static_cast<uint32_t>(unsigned_value);
        }
        return;
    }

    throw std::runtime_error("NumericConstant::CastTo: invalid target type");
}
