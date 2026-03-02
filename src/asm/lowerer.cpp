#include "include/asm/lowerer.h"

void Lowerer::Emit(std::shared_ptr<ASMInstruction> instr) {
    emitter_.EmitBack(std::move(instr));
}

std::shared_ptr<ASMOperand> Lowerer::MakeOperand(const TACOperand& op) {
    return MakeASMOperand(op, symbol_table_);
}

bool Lowerer::IsSignedOperand(const TACOperand& operand) const {
    if (operand.IsConstant()) {
        return operand.AsConstant().IsSigned();
    }
    if (auto* info = symbol_table_.FindByUniqueName(operand.AsIdentifier())) {
        if (info->type) {
            return info->type->IsSigned();
        }
    }
    return false;
}
