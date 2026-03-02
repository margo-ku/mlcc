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

int FrameStackAllocator::GetTotalFrameSize() const {
    return frames_.back().current_offset;
}

int FrameStackAllocator::GetAlignedFrameSize(int alignment) const {
    int size = frames_.back().current_offset;
    int padding = (alignment - (size % alignment)) % alignment;
    return size + padding;
}

///////////////////////////////////////////////

TempRegisterAllocator::TempRegisterAllocator() {
    available_gp_regs_ = {9, 10, 11, 12, 13, 14, 15};
    available_simd_regs_ = {16, 17, 18, 19, 20, 21, 22, 23,
                            24, 25, 26, 27, 28, 29, 30, 31};
}

std::shared_ptr<Register> TempRegisterAllocator::AllocateGP(ASMOperand::Size size) {
    if (available_gp_regs_.empty()) {
        throw std::runtime_error("Out of temporary GP registers");
    }
    auto it = available_gp_regs_.begin();
    int reg_num = *it;
    available_gp_regs_.erase(it);

    std::string prefix = (size == ASMOperand::Size::Byte8) ? "x" : "w";
    return std::make_shared<Register>(prefix + std::to_string(reg_num));
}

std::shared_ptr<Register> TempRegisterAllocator::AllocateSIMD() {
    if (available_simd_regs_.empty()) {
        throw std::runtime_error("Out of temporary SIMD registers");
    }
    auto it = available_simd_regs_.begin();
    int reg_num = *it;
    available_simd_regs_.erase(it);

    return std::make_shared<Register>("d" + std::to_string(reg_num));
}

void TempRegisterAllocator::Free(const std::shared_ptr<Register>& reg) {
    std::string name = reg->ToString();
    if (name.empty()) {
        return;
    }

    char prefix = name[0];
    int reg_num = std::stoi(name.substr(1));

    if (prefix == 'd') {
        available_simd_regs_.insert(reg_num);
    } else {
        available_gp_regs_.insert(reg_num);
    }
}