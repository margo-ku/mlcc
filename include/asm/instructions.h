#pragma once

#include <memory>
#include <string>
#include <vector>

#include "include/types/integral_constant.h"
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
    MovInstruction(std::shared_ptr<ASMOperand> dst, std::shared_ptr<ASMOperand> src);
    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> dst_;
    std::shared_ptr<ASMOperand> src_;
};

class MovzInstruction : public ASMInstruction {
public:
    MovzInstruction(std::shared_ptr<ASMOperand> dst, uint16_t imm16, int shift);

    std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;
    std::string ToString() const override;

private:
    std::shared_ptr<ASMOperand> dst_;
    uint16_t imm16_;
    int shift_;
};

class MovkInstruction : public ASMInstruction {
public:
    MovkInstruction(std::shared_ptr<ASMOperand> dst, uint16_t imm16, int shift);

    std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;
    std::string ToString() const override;

private:
    std::shared_ptr<ASMOperand> dst_;
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

class LoadInstruction : public ASMInstruction {
public:
    LoadInstruction(std::shared_ptr<ASMOperand> dst, std::shared_ptr<ASMOperand> address);
    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> dst_, address_;
};

class LoadPairInstruction : public ASMInstruction {
public:
    LoadPairInstruction(std::shared_ptr<ASMOperand> dst1,
                        std::shared_ptr<ASMOperand> dst2,
                        std::shared_ptr<ASMOperand> address);

    std::string ToString() const override;

private:
    std::shared_ptr<ASMOperand> dst1_;
    std::shared_ptr<ASMOperand> dst2_;
    std::shared_ptr<ASMOperand> address_;
};

class StoreInstruction : public ASMInstruction {
public:
    StoreInstruction(std::shared_ptr<ASMOperand> src,
                     std::shared_ptr<ASMOperand> address);
    std::string ToString() const override;

    virtual std::vector<std::shared_ptr<ASMOperand>> GetOperands() const override;
    virtual void SetOperands(
        const std::vector<std::shared_ptr<ASMOperand>>& new_operands) override;

private:
    std::shared_ptr<ASMOperand> src_, address_;
};

class StorePairInstruction : public ASMInstruction {
public:
    StorePairInstruction(std::shared_ptr<ASMOperand> src1,
                         std::shared_ptr<ASMOperand> src2,
                         std::shared_ptr<ASMOperand> address);

    std::string ToString() const override;

private:
    std::shared_ptr<ASMOperand> src1_;
    std::shared_ptr<ASMOperand> src2_;
    std::shared_ptr<ASMOperand> address_;
};

///////////////////////////////////////////////

class AllocateStackInstruction : public ASMInstruction {
public:
    explicit AllocateStackInstruction(std::shared_ptr<ASMOperand> size,
                                      bool final_size = false);
    std::string ToString() const override;
    void ChangeSize(std::shared_ptr<ASMOperand> size);

private:
    std::shared_ptr<ASMOperand> size_;
    bool final_size_;
};

class DeallocateStackInstruction : public ASMInstruction {
public:
    explicit DeallocateStackInstruction(std::shared_ptr<ASMOperand> size,
                                        bool final_size = false);
    std::string ToString() const override;
    void ChangeSize(std::shared_ptr<ASMOperand> size);

private:
    std::shared_ptr<ASMOperand> size_;
    bool final_size_;
};

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

class TextSectionDirective : public ASMInstruction {
public:
    TextSectionDirective() = default;
    std::string ToString() const override;
};

class DataSectionDirective : public ASMInstruction {
public:
    DataSectionDirective() = default;
    std::string ToString() const override;
};

class StaticVariableDirective : public ASMInstruction {
public:
    StaticVariableDirective(const std::string& name, IntegralConstant value, int size,
                            bool is_global);
    std::string ToString() const override;

private:
    std::string name_;
    IntegralConstant value_;
    int size_;
    bool is_global_;
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