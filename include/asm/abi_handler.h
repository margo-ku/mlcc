#pragma once
#include <memory>

#include "include/asm/operands.h"
#include "include/types/type.h"

class ABIHandler {
public:
    struct ArgLocation {
        enum class Kind { Register, Stack };
        Kind kind;
        std::shared_ptr<ASMOperand> operand;
    };

    void Reset();

    ArgLocation NextCallArgument(TypeRef type);     // caller-side
    ArgLocation NextFormalParameter(TypeRef type);  // callee-side

    std::shared_ptr<Register> GetReturnRegister(TypeRef return_type) const;

    size_t GetStackArgsSize() const;

private:
    static constexpr size_t kMaxGPRegs = 8;
    static constexpr size_t kMaxSIMDRegs = 8;
    static constexpr int kSlotSize = 8;
    static constexpr int kFrameRecordSize = 16;

    size_t next_gp_register_number_ = 0;
    size_t next_simd_register_number_ = 0;
    int next_stack_offset_ = 0;

    static constexpr size_t RoundUp(size_t value, size_t align);
};