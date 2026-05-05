#include "include/asm/operands.h"

#include <sstream>

#include "include/semantic/symbol_table.h"
#include "include/tac/instruction.h"
#include "include/types/numeric_constant.h"

ASMOperand::ASMOperand(Size size) : size_(size) {}

ASMOperand::Size ASMOperand::GetSize() const { return size_; }

///////////////////////////////////////////////

Register::Register(std::string name)
    : ASMOperand(!name.empty() && (name[0] == 'x' || name[0] == 'd' || name == "sp")
                     ? Size::Byte8
                     : Size::Byte4),
      name_(name) {}

std::string Register::ToString() const { return name_; }

///////////////////////////////////////////////

Immediate::Immediate(NumericConstant value)
    : ASMOperand(value.Is64Bit() ? Size::Byte8 : Size::Byte4), value_(value) {}

std::string Immediate::ToString() const { return "#" + value_.ToString(); }

NumericConstant Immediate::GetValue() const { return value_; }

void Immediate::SetValue(NumericConstant constant) { value_ = constant; }

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

const std::shared_ptr<Register>& MemoryOperand::GetBase() const { return base_; }

int MemoryOperand::GetOffset() const { return offset_; }

MemoryOperand::Mode MemoryOperand::GetMode() const { return mode_; }

///////////////////////////////////////////////

DataOperand::DataOperand(const std::string& name, Size size)
    : ASMOperand(size), name_(name) {}

std::string DataOperand::ToString() const { return "data@" + name_; }  // debug only

const std::string& DataOperand::GetName() const { return name_; }

///////////////////////////////////////////////

IndirectMemory::IndirectMemory(std::shared_ptr<ASMOperand> pointer, Size size)
    : ASMOperand(size), pointer_(std::move(pointer)) {}

IndirectMemory::IndirectMemory(std::shared_ptr<ASMOperand> pointer, int offset, Size size)
    : ASMOperand(size), pointer_(std::move(pointer)), offset_(offset) {}

std::string IndirectMemory::ToString() const {
    std::ostringstream out;
    out << "[" << pointer_->ToString();
    if (offset_) {
        out << ", #" << offset_;
    }
    out << "]";
    return out.str();
}

const std::shared_ptr<ASMOperand>& IndirectMemory::GetPointer() const { return pointer_; }

int IndirectMemory::GetOffset() const { return offset_; }

bool IndirectMemory::HasOffset() const { return offset_ != 0; }

///////////////////////////////////////////////

std::shared_ptr<ASMOperand> MakeASMOperand(const TACOperand& tac_operand,
                                           SymbolTable& symbol_table) {
    if (tac_operand.IsConstant()) {
        return std::make_shared<Immediate>(tac_operand.AsConstant());
    }

    const std::string& identifier = tac_operand.AsIdentifier();
    if (auto* info = symbol_table.FindByUniqueName(identifier)) {
        if (info->type) {
            auto size = static_cast<ASMOperand::Size>(info->type->Size());
            if (info->HasStaticDuration()) {
                return std::make_shared<DataOperand>(identifier, size);
            }
            return std::make_shared<Pseudo>(identifier, size);
        }
    }
    throw std::runtime_error("Unknown operand: " + tac_operand.ToString());
}