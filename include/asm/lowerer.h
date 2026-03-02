#pragma once
#include <memory>

#include "include/asm/emitter.h"
#include "include/asm/operands.h"
#include "include/semantic/symbol_table.h"
#include "include/tac/instruction.h"

class Lowerer {
public:
    Lowerer(Emitter& emitter, SymbolTable& symbol_table)
        : emitter_(emitter), symbol_table_(symbol_table) {}

    virtual ~Lowerer() = default;

    virtual void LowerAssign(const TACInstruction& instr) = 0;
    virtual void LowerBinary(const TACInstruction& instr) = 0;
    virtual void LowerUnary(const TACInstruction& instr) = 0;
    virtual void LowerCompare(const TACInstruction& instr) = 0;
    virtual void LowerCast(const TACInstruction& instr) = 0;

    virtual void LowerCompareZero(std::shared_ptr<ASMOperand> operand) = 0;

protected:
    Emitter& emitter_;
    SymbolTable& symbol_table_;

    void Emit(std::shared_ptr<ASMInstruction> instr);
    std::shared_ptr<ASMOperand> MakeOperand(const TACOperand& op);
    bool IsSignedOperand(const TACOperand& op) const;
};