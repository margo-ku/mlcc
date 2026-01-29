#pragma once

#include <queue>
#include <vector>

#include "allocator.h"
#include "include/optimizer/asm_optimizer.h"
#include "include/semantic/symbol_table.h"
#include "include/visitors/tac_visitor.h"
#include "instructions.h"
#include "operands.h"

class LinearIRBuilder {
public:
    explicit LinearIRBuilder(
        const std::vector<std::vector<TACInstruction>>& tac_instructions,
        SymbolTable& symbol_table);

    void Build();
    void Print(std::ostream& out) const;

private:
    std::vector<std::vector<TACInstruction>> tac_instructions_;
    std::vector<std::vector<std::shared_ptr<ASMInstruction>>> asm_instructions_;
    std::queue<std::shared_ptr<ASMOperand>> pending_args_;
    FrameStackAllocator stack_allocator_;
    TempRegisterAllocator reg_allocator_;
    ASMOptimizer optimizer_;
    SymbolTable& symbol_table_;

    std::string exit_label_ = "exit";
    std::string current_function_name_;
    int param_index_ = 0;
    int current_param_count_ = 0;

    void LowerInstruction(const TACInstruction& instr);
    void ResolveOperands();

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
    void LowerExtend(const TACInstruction& instr);
    void LowerTruncate(const TACInstruction& instr);

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
    std::shared_ptr<Register> GetReturnRegister() const;

    std::vector<std::shared_ptr<ASMInstruction>> MakeLoadImmediateInstrs(
        std::shared_ptr<ASMOperand> dst, uint64_t value);
};