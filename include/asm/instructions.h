#pragma once

#include <memory>
#include <string>
#include <vector>

#include "operands.h"

enum class BinaryOp { Add, Sub, Mul, SDiv };
enum class UnaryOp { Neg, Mvn };
enum class Condition { Eq, Ne, Lt, Le, Gt, Ge };
enum class BranchType { Unconditional, Conditional };

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
