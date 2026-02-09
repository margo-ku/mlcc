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

MovzInstruction::MovzInstruction(std::shared_ptr<ASMOperand> dst, uint16_t imm16,
                                 int shift)
    : dst_(dst), imm16_(imm16), shift_(shift) {}

std::vector<std::shared_ptr<ASMOperand>> MovzInstruction::GetOperands() const {
    return {dst_};
}

void MovzInstruction::SetOperands(
    const std::vector<std::shared_ptr<ASMOperand>>& new_operands) {
    assert(new_operands.size() == 1);
    dst_ = new_operands[0];
}

std::string MovzInstruction::ToString() const {
    if (shift_ == 0) {
        return "movz " + dst_->ToString() + ", #" + std::to_string(imm16_);
    }
    return "movz " + dst_->ToString() + ", #" + std::to_string(imm16_) + ", lsl #" +
           std::to_string(shift_);
}

MovkInstruction::MovkInstruction(std::shared_ptr<ASMOperand> dst, uint16_t imm16,
                                 int shift)
    : dst_(dst), imm16_(imm16), shift_(shift) {}

std::vector<std::shared_ptr<ASMOperand>> MovkInstruction::GetOperands() const {
    return {dst_};
}

void MovkInstruction::SetOperands(
    const std::vector<std::shared_ptr<ASMOperand>>& new_operands) {
    assert(new_operands.size() == 1);
    dst_ = new_operands[0];
}

std::string MovkInstruction::ToString() const {
    if (shift_ == 0) return "movk " + dst_->ToString() + ", #" + std::to_string(imm16_);
    return "movk " + dst_->ToString() + ", #" + std::to_string(imm16_) + ", lsl #" +
           std::to_string(shift_);
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
        case BinaryOp::UDiv:
            opcode = "udiv";
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
        case BinaryOp::Lsl:
            opcode = "lsl";
            break;
        case BinaryOp::Asr:
            opcode = "asr";
            break;
        case BinaryOp::Lsr:
            opcode = "lsr";
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
    return "cset " + dst_->ToString() + ", " + ConditionToStr(cond_);
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

    if (type_ == BranchType::Call) {
        return "bl " + label_;
    }

    return std::string("b.") + ConditionToStr(cond_) + " " + label_;
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

std::vector<std::shared_ptr<ASMOperand>> LoadInstruction::GetOperands() const {
    return {dst_, address_};
}

void LoadInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    dst_ = ops[0];
    address_ = ops[1];
}

///////////////////////////////////////////////

StoreInstruction::StoreInstruction(std::shared_ptr<ASMOperand> src,
                                   std::shared_ptr<ASMOperand> address)
    : src_(src), address_(address) {}

std::string StoreInstruction::ToString() const {
    return "str " + src_->ToString() + ", " + address_->ToString();
}

std::vector<std::shared_ptr<ASMOperand>> StoreInstruction::GetOperands() const {
    return {src_, address_};
}

void StoreInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    src_ = ops[0];
    address_ = ops[1];
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

///////////////////////////////////////////////

AllocateStackInstruction::AllocateStackInstruction(std::shared_ptr<ASMOperand> size,
                                                   bool final_size)
    : size_(std::move(size)), final_size_(final_size) {}

std::string AllocateStackInstruction::ToString() const {
    return "sub sp, sp, " + size_->ToString();
}

void AllocateStackInstruction::ChangeSize(std::shared_ptr<ASMOperand> size) {
    if (final_size_) {
        return;
    }
    size_ = std::move(size);
}

///////////////////////////////////////////////

DeallocateStackInstruction::DeallocateStackInstruction(std::shared_ptr<ASMOperand> size,
                                                       bool final_size)
    : size_(std::move(size)), final_size_(final_size) {}

std::string DeallocateStackInstruction::ToString() const {
    return "add sp, sp, " + size_->ToString();
}

void DeallocateStackInstruction::ChangeSize(std::shared_ptr<ASMOperand> size) {
    if (final_size_) {
        return;
    }
    size_ = std::move(size);
}

///////////////////////////////////////////////

ExtendInstruction::ExtendInstruction(std::shared_ptr<ASMOperand> dst,
                                     std::shared_ptr<ASMOperand> src, bool is_signed)
    : dst_(dst), src_(src), is_signed_(is_signed) {}

std::string ExtendInstruction::ToString() const {
    auto dst_str = dst_->ToString();
    auto src_str = src_->ToString();

    bool dst_is_x = !dst_str.empty() && dst_str[0] == 'x';
    bool src_is_w = !src_str.empty() && src_str[0] == 'w';
    assert(dst_is_x || src_is_w);

    if (dst_is_x && src_is_w) {
        std::string op = is_signed_ ? "sxtw" : "uxtw";
        return op + " " + dst_str + ", " + src_str;
    }
    return "mov " + dst_str + ", " + src_str;
}

std::vector<std::shared_ptr<ASMOperand>> ExtendInstruction::GetOperands() const {
    return {dst_, src_};
}

void ExtendInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    dst_ = ops[0];
    src_ = ops[1];
}

///////////////////////////////////////////////

TruncateInstruction::TruncateInstruction(std::shared_ptr<ASMOperand> dst,
                                         std::shared_ptr<ASMOperand> src)
    : dst_(dst), src_(src) {}

std::string TruncateInstruction::ToString() const {
    auto dst_str = dst_->ToString();
    auto src_str = src_->ToString();

    bool dst_is_w = !dst_str.empty() && dst_str[0] == 'w';
    bool src_is_x = !src_str.empty() && src_str[0] == 'x';
    assert(dst_is_w || src_is_x);

    if (dst_is_w && src_is_x) {
        src_str[0] = 'w';
    }
    return "mov " + dst_str + ", " + src_str;
}

std::vector<std::shared_ptr<ASMOperand>> TruncateInstruction::GetOperands() const {
    return {dst_, src_};
}

void TruncateInstruction::SetOperands(
    const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    dst_ = ops[0];
    src_ = ops[1];
}

///////////////////////////////////////////////

std::string TextSectionDirective::ToString() const { return ".text"; }

std::string DataSectionDirective::ToString() const { return ".data"; }

StaticVariableDirective::StaticVariableDirective(const std::string& name, long long value,
                                                 int size, bool is_global)
    : name_(name), value_(value), size_(size), is_global_(is_global) {}

std::string StaticVariableDirective::ToString() const {
    std::string result;
    if (is_global_) {
        result += ".globl _" + name_ + "\n";
    }
    result += "_" + name_ + ":\n";
    if (size_ == 8) {
        result += "    .quad " + std::to_string(value_);
    } else {
        result += "    .long " + std::to_string(value_);
    }
    return result;
}

///////////////////////////////////////////////

AdrpInstruction::AdrpInstruction(std::shared_ptr<ASMOperand> dst, const std::string& symbol)
    : dst_(dst), symbol_(symbol) {}

std::string AdrpInstruction::ToString() const {
    return "adrp " + dst_->ToString() + ", " + symbol_ + "@PAGE";
}

std::vector<std::shared_ptr<ASMOperand>> AdrpInstruction::GetOperands() const {
    return {dst_};
}

void AdrpInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 1);
    dst_ = ops[0];
}

///////////////////////////////////////////////

LoadGlobalInstruction::LoadGlobalInstruction(std::shared_ptr<ASMOperand> dst,
                                             std::shared_ptr<ASMOperand> base,
                                             const std::string& symbol)
    : dst_(dst), base_(base), symbol_(symbol) {}

std::string LoadGlobalInstruction::ToString() const {
    return "ldr " + dst_->ToString() + ", [" + base_->ToString() + ", " + symbol_ + "@PAGEOFF]";
}

std::vector<std::shared_ptr<ASMOperand>> LoadGlobalInstruction::GetOperands() const {
    return {dst_, base_};
}

void LoadGlobalInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    dst_ = ops[0];
    base_ = ops[1];
}

///////////////////////////////////////////////

StoreGlobalInstruction::StoreGlobalInstruction(std::shared_ptr<ASMOperand> src,
                                               std::shared_ptr<ASMOperand> base,
                                               const std::string& symbol)
    : src_(src), base_(base), symbol_(symbol) {}

std::string StoreGlobalInstruction::ToString() const {
    return "str " + src_->ToString() + ", [" + base_->ToString() + ", " + symbol_ + "@PAGEOFF]";
}

std::vector<std::shared_ptr<ASMOperand>> StoreGlobalInstruction::GetOperands() const {
    return {src_, base_};
}

void StoreGlobalInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    src_ = ops[0];
    base_ = ops[1];
}

///////////////////////////////////////////////