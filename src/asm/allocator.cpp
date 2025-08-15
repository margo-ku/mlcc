#include "include/asm/allocator.h"

FrameStackAllocator::FrameStackAllocator() {}

void FrameStackAllocator::PushFrame() { frames_.emplace_back(); }

void FrameStackAllocator::PopFrame() { frames_.pop_back(); }

int FrameStackAllocator::GetOffset(const std::string& name, int size) {
    auto& frame = frames_.back();
    if (!frame.offsets_.contains(name)) {
        frame.current_offset_ += size;
        frame.offsets_[name] = frame.current_offset_;
    }
    return -frame.offsets_.at(name);
}

int FrameStackAllocator::GetTotalFrameSize() const {
    return frames_.back().current_offset_;
}

int FrameStackAllocator::GetAlignedFrameSize(int alignment) const {
    int padding = (alignment - (frames_.back().current_offset_ % alignment)) % alignment;
    return frames_.back().current_offset_ + padding;
}

///////////////////////////////////////////////

TempRegisterAllocator::TempRegisterAllocator() {
    registers_ = {"w9", "w10", "w11", "w12", "w13", "w14", "w15"};
}

std::shared_ptr<Register> TempRegisterAllocator::Allocate() {
    if (registers_.empty()) {
        throw std::runtime_error("Out of temporary registers");
    }
    auto it = registers_.begin();
    std::string reg = *it;
    registers_.erase(it);
    return std::make_shared<Register>(reg);
}

void TempRegisterAllocator::Free(const std::shared_ptr<Register>& reg) {
    std::string name = reg->ToString();
    if (!registers_.contains(name)) {
        registers_.insert(name);
    }
}