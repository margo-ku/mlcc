#include "include/asm/allocator.h"

FrameStackAllocator::FrameStackAllocator() {}

void FrameStackAllocator::PushFrame() { frames_.emplace_back(); }

void FrameStackAllocator::PopFrame() { frames_.pop_back(); }

int FrameStackAllocator::GetLocalOffset(const std::string& name, int size) {
    auto& frame = frames_.back();

    if (!frame.offsets.contains(name)) {
        frame.current_offset += size;
        frame.offsets[name] = frame.current_offset;
    }
    return -frame.offsets.at(name);
}

int FrameStackAllocator::GetArgumentOffset(std::string name, int size) const {
    if (name.find("arg..") == std::string::npos) {
        throw std::runtime_error("Invalid argument name: " + name);
    }
    name = name.substr(5);
    int index = 0;
    try {
        index = std::stoi(name);
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Invalid argument name: " + name);
    }

    if (index < 8) {
        throw std::runtime_error("Argument should be on the stack");
    }

    return 16 + (index - 8) * 8;  // 16 (FP + LR) + offset
}

int FrameStackAllocator::GetArgumentOffsetForCaller(int index, int size) const {
    return index * 8;
}

int FrameStackAllocator::GetTotalFrameSize() const {
    return frames_.back().current_offset;
}

int FrameStackAllocator::GetAlignedFrameSize(int alignment) const {
    int size = frames_.back().current_offset;
    int padding = (alignment - (size % alignment)) % alignment;
    return size + padding;
}

int FrameStackAllocator::ReserveStackArguments(size_t arg_count) {
    int size = arg_count * 8;
    int padding = (16 - (size % 16)) % 16;
    return size + padding;
}

///////////////////////////////////////////////

TempRegisterAllocator::TempRegisterAllocator() {
    available_regs_ = {9, 10, 11, 12, 13, 14, 15};
}

std::shared_ptr<Register> TempRegisterAllocator::Allocate(ASMOperand::Size size) {
    if (available_regs_.empty()) {
        throw std::runtime_error("Out of temporary registers");
    }
    auto it = available_regs_.begin();
    int reg_num = *it;
    available_regs_.erase(it);

    std::string prefix = (size == ASMOperand::Size::Byte8) ? "x" : "w";
    return std::make_shared<Register>(prefix + std::to_string(reg_num));
}

void TempRegisterAllocator::Free(const std::shared_ptr<Register>& reg) {
    std::string name = reg->ToString();
    if (name.empty()) {
        return;
    }

    int reg_num = std::stoi(name.substr(1));
    available_regs_.insert(reg_num);
}