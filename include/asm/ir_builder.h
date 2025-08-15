#pragma once

#include <vector>

#include "allocator.h"
#include "include/visitors/tac_visitor.h"
#include "instructions.h"
#include "operands.h"

class LinearIRBuilder {
public:
    explicit LinearIRBuilder(
        const std::vector<std::vector<TACInstruction>>& tac_instructions);

    void Build();
    void Print(std::ostream& out) const;

private:
    std::vector<std::vector<TACInstruction>> tac_instructions_;
    std::vector<std::vector<std::shared_ptr<ASMInstruction>>> asm_instructions_;
    FrameStackAllocator stack_allocator_;
    TempRegisterAllocator reg_allocator_;

    std::string exit_label_ = "exit";
    int param_index_ = 0;
    int current_param_count_ = 0;

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
    void LowerParam(const TACInstruction& instr);
    void LowerCall(const TACInstruction& instr);
    void LowerFunction(const TACInstruction& instr);

    void AddFunctionPrologue();
    void AddFunctionEpilogue();
    void ChangeStackSize();

    void SaveCallerRegisters() const;
    void LoadCallerRegisters() const;
    void MaterializeFormalParameters();

    std::shared_ptr<ASMOperand> MakeOperand(const std::string& value);
    void Emit(std::shared_ptr<ASMInstruction> instr);

    bool IsPureInputInstruction(const std::shared_ptr<ASMInstruction>& instr);
    std::string GetCurrentExitLabel() const;

    std::vector<std::shared_ptr<ASMInstruction>> MakeLoadImmediateInstrs(
        std::shared_ptr<ASMOperand> dst, uint64_t value);
};