#pragma once

#include <vector>

#include "allocator.h"
#include "include/visitors/tac_visitor.h"
#include "instructions.h"
#include "operands.h"

class LinearIRBuilder {
public:
    explicit LinearIRBuilder(const std::vector<TACInstruction>& tac_instructions);

    void Build();
    void Print(std::ostream& out) const;

private:
    std::vector<TACInstruction> tac_instructions_;
    std::vector<std::shared_ptr<ASMInstruction>> asm_instructions_;
    FrameStackAllocator stack_allocator_;
    TempRegisterAllocator reg_allocator_;

    std::string exit_label_ = "exit_label";

    void LowerInstruction(const TACInstruction& instr);
    void ResolveOperands();
    void RunPeepholeOptimization();

    void LowerAssign(const TACInstruction& instr);
    void LowerUnaryOp(const TACInstruction& instr);
    void LowerBinaryOp(const TACInstruction& instr);
    void LowerMod(const TACInstruction& instr);
    void LowerComparison(const TACInstruction& instr);
    void LowerBranch(const TACInstruction& instr);
    void LowerControl(const TACInstruction& instr);

    void AddFunctionPrologue(std::vector<std::shared_ptr<ASMInstruction>>& instructions,
                             int stack_size);
    void AddFunctionEpilogue(std::vector<std::shared_ptr<ASMInstruction>>& instructions,
                             int stack_size);
    void AddPrologueAndEpilogue();

    std::shared_ptr<ASMOperand> MakeOperand(const std::string& value);
    void Emit(std::shared_ptr<ASMInstruction> instr);

    bool IsPureInputInstruction(const std::shared_ptr<ASMInstruction>& instr);
};