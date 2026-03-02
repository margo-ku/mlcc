#include "include/asm/operand_resolver.h"

#include <cstdlib>

OperandResolver::OperandResolver(FrameStackAllocator& stack_allocator,
                                 TempRegisterAllocator& reg_allocator,
                                 ConstantPool& constant_pool)
    : stack_allocator_(stack_allocator),
      reg_allocator_(reg_allocator),
      constant_pool_(constant_pool) {}

void OperandResolver::ResolveInstruction(const InstrPtr& instr, InstrList& before,
                                         InstrList& after) {
    auto operands = instr->GetOperands();
    if (operands.empty()) {
        return;
    }

    bool is_load = dynamic_cast<LdrInstruction*>(instr.get()) != nullptr;
    bool is_store = dynamic_cast<StrInstruction*>(instr.get()) != nullptr;
    bool is_load_store = is_load || is_store;

    for (size_t idx = 0; idx < operands.size(); ++idx) {
        if (std::dynamic_pointer_cast<Pseudo>(operands[idx])) {
            ResolvePseudo(operands[idx]);
        } else if (std::dynamic_pointer_cast<DataOperand>(operands[idx])) {
            bool is_dst = (idx == 0 && !IsPureInputInstruction(instr));
            ResolveDataOperand(operands[idx], is_dst, before, after);
        }
    }

    auto max_size = [](ASMOperand::Size lhs, ASMOperand::Size rhs) {
        return static_cast<int>(lhs) >= static_cast<int>(rhs) ? lhs : rhs;
    };

    ASMOperand::Size target_size = ASMOperand::Size::Byte4;
    for (const auto& op : operands) {
        if (!std::dynamic_pointer_cast<Immediate>(op)) {
            target_size = max_size(target_size, op->GetSize());
        }
    }

    for (size_t idx = 0; idx < operands.size(); ++idx) {
        if (auto memory = std::dynamic_pointer_cast<MemoryOperand>(operands[idx])) {
            if (memory->GetMode() == MemoryOperand::Mode::Offset) {
                operands[idx] = MaterializeLargeOffset(memory, before);
                memory = std::dynamic_pointer_cast<MemoryOperand>(operands[idx]);
            }

            bool is_memory_operand_of_load_store =
                is_load_store && (idx == operands.size() - 1);
            if (is_memory_operand_of_load_store) {
                continue;
            }

            bool is_dst = (idx == 0 && !IsPureInputInstruction(instr));
            ResolveMemoryOperand(operands[idx], is_dst, before, after);
        } else if (std::dynamic_pointer_cast<Immediate>(operands[idx])) {
            bool is_sp_arithmetic = dynamic_cast<BinaryInstruction*>(instr.get()) &&
                                    operands[0]->ToString() == "sp";
            if (is_sp_arithmetic) {
                continue;
            }
            ResolveImmediate(operands[idx], target_size, before);
        }
    }

    instr->SetOperands(operands);
}

void OperandResolver::ResolvePseudo(std::shared_ptr<ASMOperand>& operand) {
    auto pseudo = std::dynamic_pointer_cast<Pseudo>(operand);
    auto size = pseudo->GetSize();
    int offset =
        stack_allocator_.GetLocalOffset(pseudo->GetName(), static_cast<int>(size));
    auto base = std::make_shared<Register>("x29");
    operand = std::make_shared<MemoryOperand>(base, offset, size);
}

void OperandResolver::ResolveDataOperand(std::shared_ptr<ASMOperand>& operand,
                                         bool is_dst, InstrList& before, InstrList& after) {
    auto data_op = std::dynamic_pointer_cast<DataOperand>(operand);
    auto size = data_op->GetSize();
    std::string symbol = "_" + data_op->GetName();

    auto addr_reg = reg_allocator_.AllocateGP(ASMOperand::Size::Byte8);
    auto value_reg = reg_allocator_.AllocateGP(size);
    temps_.push_back(addr_reg);
    temps_.push_back(value_reg);

    before.push_back(std::make_shared<AdrpInstruction>(addr_reg, symbol));

    if (is_dst) {
        after.push_back(
            std::make_shared<StoreGlobalInstruction>(value_reg, addr_reg, symbol));
    } else {
        before.push_back(
            std::make_shared<LoadGlobalInstruction>(value_reg, addr_reg, symbol));
    }

    operand = value_reg;
}

void OperandResolver::ResolveMemoryOperand(std::shared_ptr<ASMOperand>& operand,
                                           bool is_dst, InstrList& before,
                                           InstrList& after) {
    auto memory = std::dynamic_pointer_cast<MemoryOperand>(operand);
    auto size = memory->GetSize();
    auto reg = reg_allocator_.AllocateGP(size);
    temps_.push_back(reg);

    if (is_dst) {
        after.push_back(std::make_shared<StrInstruction>(reg, memory));
    } else {
        before.push_back(std::make_shared<LdrInstruction>(reg, memory));
    }

    operand = reg;
}

void OperandResolver::ResolveImmediate(std::shared_ptr<ASMOperand>& operand,
                                       ASMOperand::Size target_size, InstrList& before) {
    auto imm = std::dynamic_pointer_cast<Immediate>(operand);
    auto value = imm->GetValue();

    auto size = static_cast<int>(target_size) >= static_cast<int>(imm->GetSize())
                    ? target_size
                    : imm->GetSize();

    auto reg = reg_allocator_.AllocateGP(size);
    temps_.push_back(reg);

    bool is_32bit = (size != ASMOperand::Size::Byte8);
    auto sequence = MakeLoadImmediateSequence(reg, value.AsUInt64(), is_32bit);
    before.insert(before.end(), sequence.begin(), sequence.end());

    operand = reg;
}

void OperandResolver::ResolveFloatImmediate(std::shared_ptr<ASMOperand>& operand,
                                            InstrList& before) {
    auto imm = std::dynamic_pointer_cast<Immediate>(operand);
    double value = imm->GetValue().AsDouble();

    std::string const_name = constant_pool_.RegisterDouble(value);

    auto addr_reg = reg_allocator_.AllocateGP(ASMOperand::Size::Byte8);
    auto value_reg = reg_allocator_.AllocateGP(ASMOperand::Size::Byte8);
    temps_.push_back(addr_reg);
    temps_.push_back(value_reg);

    before.push_back(std::make_shared<AdrpInstruction>(addr_reg, const_name));
    before.push_back(
        std::make_shared<LoadGlobalInstruction>(value_reg, addr_reg, const_name));

    operand = value_reg;
}

std::shared_ptr<MemoryOperand> OperandResolver::MaterializeLargeOffset(
    const std::shared_ptr<MemoryOperand>& memory, InstrList& before) {
    if (CanEncodeImm9(memory->GetOffset())) {
        return memory;
    }

    auto addr_reg = reg_allocator_.AllocateGP(ASMOperand::Size::Byte8);
    auto imm_reg = reg_allocator_.AllocateGP(ASMOperand::Size::Byte8);
    temps_.push_back(addr_reg);
    temps_.push_back(imm_reg);

    before.push_back(std::make_shared<MovInstruction>(addr_reg, memory->GetBase()));

    int offset = memory->GetOffset();
    uint64_t abs_offset = static_cast<uint64_t>(std::abs(offset));

    auto sequence = MakeLoadImmediateSequence(imm_reg, abs_offset, false);
    before.insert(before.end(), sequence.begin(), sequence.end());

    BinaryOp op = (offset < 0) ? BinaryOp::Sub : BinaryOp::Add;
    before.push_back(
        std::make_shared<BinaryInstruction>(op, addr_reg, addr_reg, imm_reg));

    return std::make_shared<MemoryOperand>(addr_reg, 0, memory->GetSize());
}

OperandResolver::InstrList OperandResolver::MakeLoadImmediateSequence(
    std::shared_ptr<Register> dst, uint64_t value, bool is_32bit) {
    constexpr int kBitsPerChunk = 16;
    int chunk_count = is_32bit ? 2 : 4;

    int first_nonzero_idx = -1;
    for (int idx = 0; idx < chunk_count; ++idx) {
        uint16_t chunk = (value >> (idx * kBitsPerChunk)) & 0xFFFF;
        if (chunk != 0) {
            first_nonzero_idx = idx;
            break;
        }
    }

    InstrList result;
    if (first_nonzero_idx == -1) {
        result.push_back(
            std::make_shared<MovInstruction>(MovInstruction::Variant::MovZ, dst, 0, 0));
        return result;
    }

    for (int idx = 0; idx < chunk_count; ++idx) {
        uint16_t chunk = (value >> (idx * kBitsPerChunk)) & 0xFFFF;
        int shift = idx * kBitsPerChunk;

        if (idx == first_nonzero_idx) {
            result.push_back(std::make_shared<MovInstruction>(
                MovInstruction::Variant::MovZ, dst, chunk, shift));
        } else if (chunk != 0) {
            result.push_back(std::make_shared<MovInstruction>(
                MovInstruction::Variant::MovK, dst, chunk, shift));
        }
    }

    return result;
}

bool OperandResolver::IsPureInputInstruction(const InstrPtr& instr) const {
    return dynamic_cast<CompareInstruction*>(instr.get()) != nullptr ||
           dynamic_cast<BranchInstruction*>(instr.get()) != nullptr ||
           dynamic_cast<RetInstruction*>(instr.get()) != nullptr ||
           dynamic_cast<StrInstruction*>(instr.get()) != nullptr;
}

bool OperandResolver::CanEncodeImm9(int offset) {
    return offset >= -256 && offset <= 255;
}

void OperandResolver::FreeTemps() {
    while (!temps_.empty()) {
        reg_allocator_.Free(temps_.back());
        temps_.pop_back();
    }
}
