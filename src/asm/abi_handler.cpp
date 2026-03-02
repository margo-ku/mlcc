#include "include/asm/abi_handler.h"

#include "include/asm/operands.h"

void ABIHandler::Reset() {
    next_gp_register_number_ = 0;
    next_simd_register_number_ = 0;
    next_stack_offset_ = 0;
}

ABIHandler::ArgLocation ABIHandler::NextCallArgument(TypeRef type) {
    ASMOperand::Size size = static_cast<ASMOperand::Size>(type->Size());

    if (type->IsFloatingPoint()) {
        if (next_simd_register_number_ < kMaxSIMDRegs) {
            auto reg = std::make_shared<Register>(
                "d" + std::to_string(next_simd_register_number_++));
            return ArgLocation{.kind = ArgLocation::Kind::Register, .operand = reg};
        }
    } else {
        if (next_gp_register_number_ < kMaxGPRegs) {
            std::string prefix = type->Size() == 8 ? "x" : "w";
            auto reg = std::make_shared<Register>(
                prefix + std::to_string(next_gp_register_number_++));
            return ArgLocation{.kind = ArgLocation::Kind::Register, .operand = reg};
        }
    }

    auto sp = std::make_shared<Register>("sp");
    int offset = next_stack_offset_;
    next_stack_offset_ += kSlotSize;
    auto mem = std::make_shared<MemoryOperand>(sp, offset, size);
    return ArgLocation{.kind = ArgLocation::Kind::Stack, .operand = mem};
}

ABIHandler::ArgLocation ABIHandler::NextFormalParameter(TypeRef type) {
    ASMOperand::Size size = static_cast<ASMOperand::Size>(type->Size());

    if (type->IsFloatingPoint()) {
        if (next_simd_register_number_ < kMaxSIMDRegs) {
            auto reg = std::make_shared<Register>(
                "d" + std::to_string(next_simd_register_number_++));
            return ArgLocation{.kind = ArgLocation::Kind::Register, .operand = reg};
        }
    } else {
        if (next_gp_register_number_ < kMaxGPRegs) {
            std::string prefix = type->Size() == 8 ? "x" : "w";
            auto reg = std::make_shared<Register>(
                prefix + std::to_string(next_gp_register_number_++));
            return ArgLocation{.kind = ArgLocation::Kind::Register, .operand = reg};
        }
    }

    auto x29 = std::make_shared<Register>("x29");
    int offset = kFrameRecordSize + next_stack_offset_;
    next_stack_offset_ += kSlotSize;
    auto mem = std::make_shared<MemoryOperand>(x29, offset, size);
    return ArgLocation{.kind = ArgLocation::Kind::Stack, .operand = mem};
}

std::shared_ptr<Register> ABIHandler::GetReturnRegister(TypeRef return_type) const {
    if (return_type->IsFloatingPoint()) {
        return std::make_shared<Register>("d0");
    }
    size_t size = return_type->Size();
    std::string prefix = (size == 8) ? "x" : "w";
    return std::make_shared<Register>(prefix + "0");
}

size_t ABIHandler::GetStackArgsSize() const { return RoundUp(next_stack_offset_, 16); }

constexpr size_t ABIHandler::RoundUp(size_t value, size_t align) {
    return (value + (align - 1)) & ~(align - 1);
}