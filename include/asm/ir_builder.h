#pragma once

#include <queue>
#include <vector>

#include "include/asm/abi_handler.h"
#include "include/asm/allocator.h"
#include "include/asm/constant_pool.h"
#include "include/asm/emitter.h"
#include "include/asm/float_lowerer.h"
#include "include/asm/int_lowerer.h"
#include "include/semantic/symbol_table.h"
#include "include/tac/instruction.h"
#include "instructions.h"

class IRBuilder {
public:
    explicit IRBuilder(const std::vector<std::vector<TACInstruction>>& tac_instructions,
                       SymbolTable& symbol_table);

    void Build();
    void GenerateASM(std::ostream& out) const;

private:
    const std::vector<std::vector<TACInstruction>>& tac_instructions_;
    std::vector<std::vector<std::shared_ptr<ASMInstruction>>> asm_instructions_;
    SymbolTable& symbol_table_;
    FrameStackAllocator stack_allocator_;
    TempRegisterAllocator reg_allocator_;
    ConstantPool constant_pool_;
    Emitter emitter_;

    struct PendingArg {
        std::shared_ptr<ASMOperand> operand;
        TypeRef type;
    };

    struct FunctionContext {
        std::string name;
        bool is_global = false;
        std::shared_ptr<Immediate> frame_size_operand;
    };

    ABIHandler abi_;
    IntLowerer int_lowerer_;
    FloatLowerer float_lowerer_;
    std::queue<PendingArg> pending_args_;
    FunctionContext current_function_;

    void PerformLowering(const std::vector<TACInstruction>& instructions);
    void PerformResolving();
    void PerformFinalizing();

    void AddFunctionDirectives();
    void AddFunctionPrologue();
    void AddFunctionEpilogue();
    void UpdateStackSize();
    void EmitDoubleConstants();

    void LowerFunction(const TACInstruction& instr);
    void LowerParam(const TACInstruction& instr);
    void LowerCall(const TACInstruction& instr);
    void LowerControl(const TACInstruction& instr);
    void LowerBranch(const TACInstruction& instr);

    Lowerer& SelectLowerer(const TACInstruction& instr);

    void DeclareStaticVariable(const TACInstruction& instr);

    TypeRef GetType(const TACOperand& operand) const;
    TypeRef GetReturnType(const std::string& func_name) const;

    std::shared_ptr<ASMOperand> MakeOperand(const TACOperand& value);
    std::string GetCurrentExitLabel() const;
};