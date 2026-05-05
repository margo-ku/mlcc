#pragma once
#include <memory>
#include <vector>

#include "include/asm/allocator.h"
#include "include/asm/constant_pool.h"
#include "include/asm/instructions.h"

class OperandResolver {
public:
    using InstrPtr = std::shared_ptr<ASMInstruction>;
    using InstrList = std::vector<InstrPtr>;

    OperandResolver(FrameStackAllocator& stack_allocator,
                    TempRegisterAllocator& reg_allocator, ConstantPool& constant_pool);

    void ResolveInstruction(InstrPtr& instr, InstrList& before, InstrList& after);
    void FreeTemps();

private:
    void ResolvePseudo(std::shared_ptr<ASMOperand>& operand);

    void ResolveDataOperand(std::shared_ptr<ASMOperand>& operand, bool is_dst,
                            bool use_float_register, InstrList& before, InstrList& after);

    void ResolveMemoryOperand(std::shared_ptr<ASMOperand>& operand, bool is_dst,
                              bool use_float_register, InstrList& before,
                              InstrList& after);

    void ResolveImmediate(std::shared_ptr<ASMOperand>& operand, bool use_float_register,
                          ASMOperand::Size target_size, InstrList& before);

    void ResolveFloatImmediate(std::shared_ptr<ASMOperand>& operand, InstrList& before);

    void ResolveIndirectMemory(std::shared_ptr<ASMOperand>& operand, InstrList& before,
                               InstrList& after);

    std::shared_ptr<MemoryOperand> MaterializeLargeOffset(
        const std::shared_ptr<MemoryOperand>& memory, InstrList& before);

    InstrList MakeLoadImmediateSequence(std::shared_ptr<Register> dst, uint64_t value,
                                        bool is_32bit);

    void ResolveVirtualInstructions(InstrPtr& instr, InstrList& before, InstrList& after);

    void ResolveAddress(InstrPtr& instr, InstrList& before, InstrList& after);

    bool NeedsFloatRegister(const InstrPtr& instr, size_t operand_idx) const;
    bool IsPureInputInstruction(const InstrPtr& instr) const;
    static bool CanEncodeImm9(int offset);

    FrameStackAllocator& stack_allocator_;
    TempRegisterAllocator& reg_allocator_;
    ConstantPool& constant_pool_;

    std::vector<std::shared_ptr<Register>> temps_;
};
