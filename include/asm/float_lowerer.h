#pragma once

#include "include/asm/lowerer.h"

class FloatLowerer : public Lowerer {
public:
    using Lowerer::Lowerer;

    void LowerAssign(const TACInstruction& instr) override;
    void LowerBinary(const TACInstruction& instr) override;
    void LowerUnary(const TACInstruction& instr) override;
    void LowerCompare(const TACInstruction& instr) override;
    void LowerCast(const TACInstruction& instr) override;

    void LowerCompareZero(std::shared_ptr<ASMOperand> operand) override;
};
