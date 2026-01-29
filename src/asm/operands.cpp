#include "include/asm/operands.h"

#include <sstream>

ASMOperand::ASMOperand(Size size) : size_(size) {}

ASMOperand::Size ASMOperand::GetSize() const { return size_; }

///////////////////////////////////////////////

Register::Register(std::string name)
    : ASMOperand(!name.empty() && name[0] == 'x' ? Size::Byte8 : Size::Byte4),
      name_(name) {}

std::string Register::ToString() const { return name_; }

///////////////////////////////////////////////

Immediate::Immediate(long long value)
    : ASMOperand(value >= INT32_MIN && value <= INT32_MAX ? Size::Byte4 : Size::Byte8),
      value_(value) {}

std::string Immediate::ToString() const { return "#" + std::to_string(value_); }

long long Immediate::GetValue() const { return value_; }

///////////////////////////////////////////////

Pseudo::Pseudo(const std::string& name, Size size) : ASMOperand(size), name_(name) {}

std::string Pseudo::ToString() const { return "%" + name_; }  // debug only

const std::string& Pseudo::GetName() const { return name_; }

///////////////////////////////////////////////

MemoryOperand::MemoryOperand(std::shared_ptr<Register> base, int offset, Size size,
                             Mode mode)
    : ASMOperand(size), base_(base), offset_(offset), mode_(mode) {}

std::string MemoryOperand::ToString() const {
    std::ostringstream out;

    switch (mode_) {
        case Mode::Offset:
            out << "[" << base_->ToString() << ", #" << offset_ << "]";
            break;
        case Mode::PreIndexed:
            out << "[" << base_->ToString() << ", #" << offset_ << "]!";
            break;
        case Mode::PostIndexed:
            out << "[" << base_->ToString() << "], #" << offset_;
            break;
    }

    return out.str();
}