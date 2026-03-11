#include "include/asm/instructions.h"

#include <cassert>
#include <cstring>

#include "include/types/numeric_constant.h"

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
                               std::shared_ptr<ASMOperand> src, bool is_float)
    : variant_(Variant::Regular),
      is_float_(is_float),
      dst_(dst),
      src_(src),
      imm16_(0),
      shift_(0) {}

MovInstruction::MovInstruction(Variant variant, std::shared_ptr<ASMOperand> dst,
                               uint16_t imm16, int shift)
    : variant_(variant),
      is_float_(false),
      dst_(dst),
      src_(nullptr),
      imm16_(imm16),
      shift_(shift) {
    assert(variant != Variant::Regular);  // to do: check
}

std::string MovInstruction::ToString() const {
    if (variant_ == Variant::Regular) {
        std::string opcode = is_float_ ? "fmov" : "mov";
        return opcode + " " + dst_->ToString() + ", " + src_->ToString();
    }

    std::string op = (variant_ == Variant::MovZ) ? "movz" : "movk";
    if (shift_ == 0) {
        return op + " " + dst_->ToString() + ", #" + std::to_string(imm16_);
    }
    return op + " " + dst_->ToString() + ", #" + std::to_string(imm16_) + ", lsl #" +
           std::to_string(shift_);
}

std::vector<std::shared_ptr<ASMOperand>> MovInstruction::GetOperands() const {
    if (variant_ == Variant::Regular) {
        return {dst_, src_};
    }
    return {dst_};
}

void MovInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    if (variant_ == Variant::Regular) {
        assert(ops.size() == 2);
        dst_ = ops[0];
        src_ = ops[1];
    } else {
        assert(ops.size() == 1);
        dst_ = ops[0];
    }
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
        case BinaryOp::FAdd:
            opcode = "fadd";
            break;
        case BinaryOp::FSub:
            opcode = "fsub";
            break;
        case BinaryOp::FMul:
            opcode = "fmul";
            break;
        case BinaryOp::FDiv:
            opcode = "fdiv";
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
        case UnaryOp::FNeg:
            opcode = "fneg";
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
                                       std::shared_ptr<ASMOperand> rhs, bool is_float)
    : lhs_(lhs), rhs_(rhs), is_float_(is_float) {}

std::string CompareInstruction::ToString() const {
    std::string opcode = is_float_ ? "fcmp" : "cmp";
    return opcode + " " + lhs_->ToString() + ", " + rhs_->ToString();
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

LdrInstruction::LdrInstruction(std::shared_ptr<ASMOperand> dst,
                               std::shared_ptr<ASMOperand> address)
    : dsts_{std::move(dst)}, address_(std::move(address)) {}

LdrInstruction::LdrInstruction(std::shared_ptr<ASMOperand> dst1,
                               std::shared_ptr<ASMOperand> dst2,
                               std::shared_ptr<ASMOperand> address)
    : dsts_{std::move(dst1), std::move(dst2)}, address_(std::move(address)) {}

std::string LdrInstruction::ToString() const {
    if (dsts_.size() == 1) {
        return "ldr " + dsts_[0]->ToString() + ", " + address_->ToString();
    }
    return "ldp " + dsts_[0]->ToString() + ", " + dsts_[1]->ToString() + ", " +
           address_->ToString();
}

std::vector<std::shared_ptr<ASMOperand>> LdrInstruction::GetOperands() const {
    std::vector<std::shared_ptr<ASMOperand>> ops = dsts_;
    ops.push_back(address_);
    return ops;
}

void LdrInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == dsts_.size() + 1);
    for (size_t i = 0; i < dsts_.size(); ++i) {
        dsts_[i] = ops[i];
    }
    address_ = ops.back();
}

StrInstruction::StrInstruction(std::shared_ptr<ASMOperand> src,
                               std::shared_ptr<ASMOperand> address)
    : srcs_{std::move(src)}, address_(std::move(address)) {}

StrInstruction::StrInstruction(std::shared_ptr<ASMOperand> src1,
                               std::shared_ptr<ASMOperand> src2,
                               std::shared_ptr<ASMOperand> address)
    : srcs_{std::move(src1), std::move(src2)}, address_(std::move(address)) {}

std::string StrInstruction::ToString() const {
    if (srcs_.size() == 1) {
        return "str " + srcs_[0]->ToString() + ", " + address_->ToString();
    }
    return "stp " + srcs_[0]->ToString() + ", " + srcs_[1]->ToString() + ", " +
           address_->ToString();
}

std::vector<std::shared_ptr<ASMOperand>> StrInstruction::GetOperands() const {
    std::vector<std::shared_ptr<ASMOperand>> ops = srcs_;
    ops.push_back(address_);
    return ops;
}

void StrInstruction::SetOperands(const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == srcs_.size() + 1);
    for (size_t idx = 0; idx < srcs_.size(); ++idx) {
        srcs_[idx] = ops[idx];
    }
    address_ = ops.back();
}

///////////////////////////////////////////////

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

SectionDirective::SectionDirective(const std::string& name) : name_(name) {}

std::string SectionDirective::ToString() const { return name_; }

StaticVariableDirective::StaticVariableDirective(const std::string& name,
                                                 NumericConstant value, int size,
                                                 bool is_global)
    : name_(name), value_(value), size_(size), is_global_(is_global) {}

std::string StaticVariableDirective::ToString() const {
    std::string result;
    if (is_global_) {
        result += ".globl _" + name_ + "\n";
    }
    if (size_ == 8) {
        result += ".p2align 3\n";
    } else {
        result += ".p2align 2\n";
    }
    result += "_" + name_ + ":\n";
    if (size_ == 8) {
        if (value_.IsFloatingPoint()) {
            double d = value_.AsDouble();
            uint64_t bits;
            std::memcpy(&bits, &d, sizeof(bits));
            result += "    .quad " + std::to_string(bits);
        } else {
            result += "    .quad " + value_.ToString();
        }
    } else {
        result += "    .long " + value_.ToString();
    }
    return result;
}

DoubleConstantDirective::DoubleConstantDirective(const std::string& name, double value)
    : name_(name), value_(value) {}

std::string DoubleConstantDirective::ToString() const {
    std::string result;
    result += ".p2align 3\n";
    result += name_ + ":\n";
    uint64_t bits;
    std::memcpy(&bits, &value_, sizeof(bits));
    result += "    .quad " + std::to_string(bits);
    return result;
}

///////////////////////////////////////////////

AdrpInstruction::AdrpInstruction(std::shared_ptr<ASMOperand> dst,
                                 const std::string& symbol)
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
    return "ldr " + dst_->ToString() + ", [" + base_->ToString() + ", " + symbol_ +
           "@PAGEOFF]";
}

std::vector<std::shared_ptr<ASMOperand>> LoadGlobalInstruction::GetOperands() const {
    return {dst_, base_};
}

void LoadGlobalInstruction::SetOperands(
    const std::vector<std::shared_ptr<ASMOperand>>& ops) {
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
    return "str " + src_->ToString() + ", [" + base_->ToString() + ", " + symbol_ +
           "@PAGEOFF]";
}

std::vector<std::shared_ptr<ASMOperand>> StoreGlobalInstruction::GetOperands() const {
    return {src_, base_};
}

void StoreGlobalInstruction::SetOperands(
    const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    src_ = ops[0];
    base_ = ops[1];
}

///////////////////////////////////////////////

IntToFloatInstruction::IntToFloatInstruction(std::shared_ptr<ASMOperand> dst,
                                             std::shared_ptr<ASMOperand> src,
                                             bool is_signed)
    : dst_(dst), src_(src), is_signed_(is_signed) {}

std::string IntToFloatInstruction::ToString() const {
    std::string opcode = is_signed_ ? "scvtf" : "ucvtf";
    return opcode + " " + dst_->ToString() + ", " + src_->ToString();
}

std::vector<std::shared_ptr<ASMOperand>> IntToFloatInstruction::GetOperands() const {
    return {dst_, src_};
}

void IntToFloatInstruction::SetOperands(
    const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    dst_ = ops[0];
    src_ = ops[1];
}

///////////////////////////////////////////////

FloatToIntInstruction::FloatToIntInstruction(std::shared_ptr<ASMOperand> dst,
                                             std::shared_ptr<ASMOperand> src,
                                             bool is_signed)
    : dst_(dst), src_(src), is_signed_(is_signed) {}

std::string FloatToIntInstruction::ToString() const {
    std::string opcode = is_signed_ ? "fcvtzs" : "fcvtzu";
    return opcode + " " + dst_->ToString() + ", " + src_->ToString();
}

std::vector<std::shared_ptr<ASMOperand>> FloatToIntInstruction::GetOperands() const {
    return {dst_, src_};
}

void FloatToIntInstruction::SetOperands(
    const std::vector<std::shared_ptr<ASMOperand>>& ops) {
    assert(ops.size() == 2);
    dst_ = ops[0];
    src_ = ops[1];
}

///////////////////////////////////////////////