#pragma once

#include <memory>
#include <string>
#include <vector>

#include "include/types/numeric_constant.h"
#include "operands.h"

enum class BinaryOp { Add, Sub, Mul, SDiv, UDiv, And, Orr, Eor, Lsl, Asr, Lsr };
enum class UnaryOp { Neg, Mvn };
// Signed: Lt, Le, Gt, Ge
// Unsigned: Lo, Ls, Hi, Hs
enum class Condition { Eq, Ne, Lt, Le, Gt, Ge, Lo, Ls, Hi, Hs };

inline std::string ConditionToStr(Condition cond) {
    switch (cond) {
        case Condition::Eq:
            return "eq";
        case Condition::Ne:
            return "ne";
        case Condition::Lt:
            return "lt";
        case Condition::Le:
            return "le";
        case Condition::Gt:
            return "gt";
        case Condition::Ge:
            return "ge";
        case Condition::Lo:
            return "lo";
        case Condition::Ls:
            return "ls";
        case Condition::Hi:
            return "hi";
        case Condition::Hs:
            return "hs";
    }
}
enum class BranchType { Unconditional, Conditional, Call };

class ASMInstruction {
public:
    virtual std::string ToString() const = 0;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const;

    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands);
};

///////////////////////////////////////////////

class LabelInstruction : public ASMInstruction {
public:
    explicit LabelInstruction(const std::string& label);
    std::string ToString() const override;
    bool IsFunction() const;

private:
    std::string label_;
};

class GlobalDirective : public ASMInstruction {
public:
    explicit GlobalDirective(const std::string& name);
    std::string ToString() const override;

private:
    std::string name_;
};

///////////////////////////////////////////////

class MovInstruction : public ASMInstruction {
public:
    enum class Variant { Regular, MovZ, MovK };

    MovInstruction(std::shared_ptr<ASMOperand> dst, std::shared_ptr<ASMOperand> src);
    MovInstruction(Variant variant, std::shared_ptr<ASMOperand> dst, uint16_t imm16,
                   int shift = 0);

    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    Variant variant_;
    std::shared_ptr<ASMOperand> dst_;
    std::shared_ptr<ASMOperand> src_;
    uint16_t imm16_;
    int shift_;
};

///////////////////////////////////////////////

class BinaryInstruction : public ASMInstruction {
public:
    BinaryInstruction(BinaryOp op, std::shared_ptr<ASMOperand> dst,
                      std::shared_ptr<ASMOperand> lhs, std::shared_ptr<ASMOperand> rhs);
    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    BinaryOp op_;
    std::shared_ptr<ASMOperand> dst_, lhs_, rhs_;
};

class UnaryInstruction : public ASMInstruction {
public:
    UnaryInstruction(UnaryOp op, std::shared_ptr<ASMOperand> dst,
                     std::shared_ptr<ASMOperand> operand);
    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    UnaryOp op_;
    std::shared_ptr<ASMOperand> dst_, operand_;
};

///////////////////////////////////////////////

class CompareInstruction : public ASMInstruction {
public:
    CompareInstruction(std::shared_ptr<ASMOperand> lhs, std::shared_ptr<ASMOperand> rhs);
    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> lhs_, rhs_;
};

class CSetInstruction : public ASMInstruction {
public:
    CSetInstruction(std::shared_ptr<ASMOperand> dst, Condition cond);
    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> dst_;
    Condition cond_;
};

class BranchInstruction : public ASMInstruction {
public:
    BranchInstruction(BranchType type, const std::string& label,
                      Condition cond = Condition::Eq);
    std::string ToString() const override;

private:
    BranchType type_;
    std::string label_;
    Condition cond_;
};

class RetInstruction : public ASMInstruction {
public:
    RetInstruction();
    std::string ToString() const override;
};

///////////////////////////////////////////////

class LdrInstruction : public ASMInstruction {
public:
    LdrInstruction(std::shared_ptr<ASMOperand> dst, std::shared_ptr<ASMOperand> address);
    LdrInstruction(std::shared_ptr<ASMOperand> dst1, std::shared_ptr<ASMOperand> dst2,
                   std::shared_ptr<ASMOperand> address);
    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::vector<std::shared_ptr<ASMOperand>> dsts_;
    std::shared_ptr<ASMOperand> address_;
};

class StrInstruction : public ASMInstruction {
public:
    StrInstruction(std::shared_ptr<ASMOperand> src, std::shared_ptr<ASMOperand> address);
    StrInstruction(std::shared_ptr<ASMOperand> src1, std::shared_ptr<ASMOperand> src2,
                   std::shared_ptr<ASMOperand> address);
    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::vector<std::shared_ptr<ASMOperand>> srcs_;
    std::shared_ptr<ASMOperand> address_;
};

///////////////////////////////////////////////

///////////////////////////////////////////////

class ExtendInstruction : public ASMInstruction {
public:
    ExtendInstruction(std::shared_ptr<ASMOperand> dst, std::shared_ptr<ASMOperand> src,
                      bool is_signed);
    std::string ToString() const override;

    std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> dst_, src_;
    bool is_signed_;
};

class TruncateInstruction : public ASMInstruction {
public:
    TruncateInstruction(std::shared_ptr<ASMOperand> dst, std::shared_ptr<ASMOperand> src);
    std::string ToString() const override;

    std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> dst_, src_;
};

///////////////////////////////////////////////

class SectionDirective : public ASMInstruction {
public:
    explicit SectionDirective(const std::string& name);
    std::string ToString() const override;

private:
    std::string name_;
};

class StaticVariableDirective : public ASMInstruction {
public:
    StaticVariableDirective(const std::string& name, NumericConstant value, int size,
                            bool is_global);
    std::string ToString() const override;

private:
    std::string name_;
    NumericConstant value_;
    int size_;
    bool is_global_;
};

class DoubleConstantDirective : public ASMInstruction {
public:
    DoubleConstantDirective(const std::string& name, double value);
    std::string ToString() const override;

private:
    std::string name_;
    double value_;
};

///////////////////////////////////////////////

class AdrpInstruction : public ASMInstruction {
public:
    AdrpInstruction(std::shared_ptr<ASMOperand> dst, const std::string& symbol);
    std::string ToString() const override;

    std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> dst_;
    std::string symbol_;
};

class LoadGlobalInstruction : public ASMInstruction {
public:
    LoadGlobalInstruction(std::shared_ptr<ASMOperand> dst,
                          std::shared_ptr<ASMOperand> base, const std::string& symbol);
    std::string ToString() const override;

    std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> dst_;
    std::shared_ptr<ASMOperand> base_;
    std::string symbol_;
};

class StoreGlobalInstruction : public ASMInstruction {
public:
    StoreGlobalInstruction(std::shared_ptr<ASMOperand> src,
                           std::shared_ptr<ASMOperand> base, const std::string& symbol);
    std::string ToString() const override;

    std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> src_;
    std::shared_ptr<ASMOperand> base_;
    std::string symbol_;
};

///////////////////////////////////////////////