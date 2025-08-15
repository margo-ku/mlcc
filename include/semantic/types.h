#pragma once

#include <string>

// to do: create h and cpp files for types
class TypeInfo {
public:
    virtual ~TypeInfo() = default;
    virtual bool IsFunction() const = 0;
    virtual bool IsVariable() const = 0;
    virtual std::string ToString() const = 0;
};

/////////////////////

class VariableType : public TypeInfo {
public:
    enum class PrimitiveType { Int };

    PrimitiveType type;

    VariableType(PrimitiveType t) : type(t) {}
    bool IsFunction() const override { return false; }
    bool IsVariable() const override { return true; }
    std::string ToString() const override { return "int"; }
};

/////////////////////

class FunctionType : public TypeInfo {
public:
    int parameter_count;
    bool is_defined = false;

    FunctionType(int params) : parameter_count(params), is_defined(false) {}
    bool IsFunction() const override { return true; }
    bool IsVariable() const override { return false; }
    std::string ToString() const override {
        return "function(" + std::to_string(parameter_count) + " params)";
    }

    void SetDefined() { is_defined = true; }
    bool IsDefined() const { return is_defined; }
};