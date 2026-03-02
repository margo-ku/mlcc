#pragma once

#include <set>
#include <string>
#include <unordered_map>

#include "operands.h"

struct Frame {
    int current_offset = 0;
    std::unordered_map<std::string, int> offsets;
};

class FrameStackAllocator {
public:
    FrameStackAllocator();

    void PushFrame();
    void PopFrame();

    int GetLocalOffset(const std::string& name, int size);
    int GetTotalFrameSize() const;
    int GetAlignedFrameSize(int alignment = 16) const;

private:
    std::vector<Frame> frames_;
};

///////////////////////////////////////////////

class TempRegisterAllocator {
public:
    TempRegisterAllocator();

    std::shared_ptr<Register> AllocateGP(ASMOperand::Size size = ASMOperand::Size::Byte4);
    std::shared_ptr<Register> AllocateSIMD();
    void Free(const std::shared_ptr<Register>& reg);

private:
    std::set<int> available_gp_regs_;
    std::set<int> available_simd_regs_;
};