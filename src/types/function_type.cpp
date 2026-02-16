#include "include/types/function_type.h"

#include <sstream>

FunctionType::FunctionType(TypeRef return_type, std::vector<TypeRef> param_types)
    : Type(Type::Kind::Function),
      return_type_(std::move(return_type)),
      param_types_(std::move(param_types)) {}

std::size_t FunctionType::Size() const { return 0; }
std::size_t FunctionType::Alignment() const { return 0; }
bool FunctionType::IsIntegral() const { return false; }
bool FunctionType::IsArithmetic() const { return false; }
bool FunctionType::IsSigned() const { return false; }
bool FunctionType::IsInt() const { return false; }
bool FunctionType::IsLong() const { return false; }
bool FunctionType::IsFloatingPoint() const { return false; }
std::string FunctionType::ToString() const {
    std::ostringstream oss;
    oss << "function(" << param_types_.size() << ") -> ";
    if (return_type_)
        oss << return_type_->ToString();
    else
        oss << "void";
    return oss.str();
}

bool FunctionType::Equals(const TypeRef& other) const {
    if (!other) {
        return false;
    }
    if (this == other.get()) {
        return true;
    }

    auto func = std::dynamic_pointer_cast<FunctionType>(other);
    if (!func) {
        return false;
    }
    if (param_types_.size() != func->param_types_.size()) {
        return false;
    }

    if (!return_type_->Equals(func->return_type_)) {
        return false;
    }

    for (size_t idx = 0; idx < param_types_.size(); ++idx) {
        if (!param_types_[idx]->Equals(func->param_types_[idx])) {
            return false;
        }
    }
    return true;
}
