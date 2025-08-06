#include "include/asm/instructions.h"

#include <cassert>

std::vector<std::shared_ptr<ASMOperand>> ASMInstruction::GetOperands() const {
    return {};
}

void ASMInstruction::SetOperands(
    const std::vector<std::shared_ptr<ASMOperand>>& new_operands) {}

///////////////////////////////////////////////

LabelInstruction::LabelInstruction(const std::string& label) : label_(label) {}

std::string LabelInstruction::ToString() const { return label_ + ":"; }

bool LabelInstruction::IsFunction() const { return !label_.empty() && label_[0] == '_'; }

///////////////////////////////////////////////

GlobalDirective::GlobalDirective(const std::string& name) : name_(name) {}

std::string GlobalDirective::ToString() const { return ".globl " + name_; }

///////////////////////////////////////////////

MovInstruction::MovInstruction(std::shared_ptr<ASMOperand> dst,
                               std::shared_ptr<ASMOperand> src)
    : dst_(dst), src_(src) {}

std::string MovInstruction::ToString() const {
    return "mov " + dst_->ToString() + ", " + src_->ToString();
}

std::vector<std::shared_ptr<ASMOperand>> MovInstruction::GetOperands() const {
    return {dst_, src_};
}

void MovInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    dst_ = ops[0];
    src_ = ops[1];
}

///////////////////////////////////////////////

BinaryInstruction::BinaryInstruction(BinaryOp op, std::shared_ptr<ASMOperand> dst,
                                     std::shared_ptr<ASMOperand> lhs,
                                     std::shared_ptr<ASMOperand> rhs)
    : op_(op), dst_(dst), lhs_(lhs), rhs_(rhs) {}

std::string BinaryInstruction::ToString() const {
    std::string opcode;
    switch (op_) {
        case BinaryOp::Add:
            opcode = "add";
            break;
        case BinaryOp::Sub:
            opcode = "sub";
            break;
        case BinaryOp::Mul:
            opcode = "mul";
            break;
        case BinaryOp::SDiv:
            opcode = "sdiv";
            break;
        case BinaryOp::And:
            opcode = "and";
            break;
        case BinaryOp::Orr:
            opcode = "orr";
            break;
        case BinaryOp::Eor:
            opcode = "eor";
            break;
    }
    return opcode + " " + dst_->ToString() + ", " + lhs_->ToString() + ", " +
           rhs_->ToString();
}

std::vector<std::shared_ptr<ASMOperand>> BinaryInstruction::GetOperands() const {
    return {dst_, lhs_, rhs_};
}

void BinaryInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 3);
    dst_ = ops[0];
    lhs_ = ops[1];
    rhs_ = ops[2];
}

///////////////////////////////////////////////

UnaryInstruction::UnaryInstruction(UnaryOp op, std::shared_ptr<ASMOperand> dst,
                                   std::shared_ptr<ASMOperand> operand)
    : op_(op), dst_(dst), operand_(operand) {}

std::string UnaryInstruction::ToString() const {
    std::string opcode;
    switch (op_) {
        case UnaryOp::Neg:
            opcode = "neg";
            break;
        case UnaryOp::Mvn:
            opcode = "mvn";
            break;
    }
    return opcode + " " + dst_->ToString() + ", " + operand_->ToString();
}

std::vector<std::shared_ptr<ASMOperand>> UnaryInstruction::GetOperands() const {
    return {dst_, operand_};
}

void UnaryInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    dst_ = ops[0];
    operand_ = ops[1];
}

///////////////////////////////////////////////

CompareInstruction::CompareInstruction(std::shared_ptr<ASMOperand> lhs,
                                       std::shared_ptr<ASMOperand> rhs)
    : lhs_(lhs), rhs_(rhs) {}

std::string CompareInstruction::ToString() const {
    return "cmp " + lhs_->ToString() + ", " + rhs_->ToString();
}

std::vector<std::shared_ptr<ASMOperand>> CompareInstruction::GetOperands() const {
    return {lhs_, rhs_};
}

void CompareInstruction::SetOperands(
    const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    lhs_ = ops[0];
    rhs_ = ops[1];
}

///////////////////////////////////////////////

CSetInstruction::CSetInstruction(std::shared_ptr<ASMOperand> dst, Condition cond)
    : dst_(dst), cond_(cond) {}

std::string CSetInstruction::ToString() const {
    std::string cond;
    switch (cond_) {
        case Condition::Eq:
            cond = "eq";
            break;
        case Condition::Ne:
            cond = "ne";
            break;
        case Condition::Lt:
            cond = "lt";
            break;
        case Condition::Le:
            cond = "le";
            break;
        case Condition::Gt:
            cond = "gt";
            break;
        case Condition::Ge:
            cond = "ge";
            break;
    }
    return "cset " + dst_->ToString() + ", " + cond;
}

std::vector<std::shared_ptr<ASMOperand>> CSetInstruction::GetOperands() const {
    return {dst_};
}

void CSetInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 1);
    dst_ = ops[0];
}

///////////////////////////////////////////////

BranchInstruction::BranchInstruction(BranchType type, const std::string& label,
                                     Condition cond)
    : type_(type), label_(label), cond_(cond) {}

std::string BranchInstruction::ToString() const {
    if (type_ == BranchType::Unconditional) {
        return "b " + label_;
    }

    std::string cond;
    switch (cond_) {
        case Condition::Eq:
            cond = "eq";
            break;
        case Condition::Ne:
            cond = "ne";
            break;
        case Condition::Lt:
            cond = "lt";
            break;
        case Condition::Le:
            cond = "le";
            break;
        case Condition::Gt:
            cond = "gt";
            break;
        case Condition::Ge:
            cond = "ge";
            break;
    }

    return "b" + cond + " " + label_;
}

///////////////////////////////////////////////

RetInstruction::RetInstruction() {}

std::string RetInstruction::ToString() const { return "ret"; }

///////////////////////////////////////////////

LoadInstruction::LoadInstruction(std::shared_ptr<ASMOperand> dst,
                                 std::shared_ptr<ASMOperand> address)
    : dst_(dst), address_(address) {}

std::string LoadInstruction::ToString() const {
    return "ldr " + dst_->ToString() + ", " + address_->ToString();
}

///////////////////////////////////////////////

StoreInstruction::StoreInstruction(std::shared_ptr<ASMOperand> src,
                                   std::shared_ptr<ASMOperand> address)
    : src_(src), address_(address) {}

std::string StoreInstruction::ToString() const {
    return "str " + src_->ToString() + ", " + address_->ToString();
}

///////////////////////////////////////////////

StorePairInstruction::StorePairInstruction(std::shared_ptr<ASMOperand> src1,
                                           std::shared_ptr<ASMOperand> src2,
                                           std::shared_ptr<ASMOperand> address)
    : src1_(std::move(src1)), src2_(std::move(src2)), address_(std::move(address)) {}

std::string StorePairInstruction::ToString() const {
    return "stp " + src1_->ToString() + ", " + src2_->ToString() + ", " +
           address_->ToString();
}

////////////////////////////////////////////////////

LoadPairInstruction::LoadPairInstruction(std::shared_ptr<ASMOperand> dst1,
                                         std::shared_ptr<ASMOperand> dst2,
                                         std::shared_ptr<ASMOperand> address)
    : dst1_(std::move(dst1)), dst2_(std::move(dst2)), address_(std::move(address)) {}

std::string LoadPairInstruction::ToString() const {
    return "ldp " + dst1_->ToString() + ", " + dst2_->ToString() + ", " +
           address_->ToString();
}