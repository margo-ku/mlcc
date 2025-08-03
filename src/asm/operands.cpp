#include "include/asm/operands.h"

#include <sstream>

Register::Register(std::string name) : name_(name) {}

std::string Register::ToString() const { return name_; }

///////////////////////////////////////////////

Immediate::Immediate(int value) : value_(value) {}

std::string Immediate::ToString() const { return "#" + std::to_string(value_); }

///////////////////////////////////////////////

Pseudo::Pseudo(const std::string& name) : name_(name) {}

std::string Pseudo::ToString() const { return "%" + name_; }  // debug only

const std::string& Pseudo::GetName() const { return name_; }

///////////////////////////////////////////////

MemoryOperand::MemoryOperand(std::shared_ptr<Register> base, int offset, Mode mode)
    : base_(base), offset_(offset), mode_(mode) {}

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