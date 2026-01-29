#pragma once

#include <set>
#include <string>
#include <unordered_map>

#include "operands.h"

struct Frame {
    int current_offset = 0;
    std::unordered_map<std::string, int> offsets;
    int alignment = 16;
};

class FrameStackAllocator {
public:
    FrameStackAllocator();

    void PushFrame();
    void PopFrame();

    int GetLocalOffset(const std::string& name, int size);
    int GetArgumentOffset(std::string name, int size) const;
    int GetArgumentOffsetForCaller(int index, int size = 8) const;

    int ReserveStackArguments(size_t arg_count);
    int GetTotalFrameSize() const;
    int GetAlignedFrameSize(int alignment = 16) const;

private:
    std::vector<Frame> frames_;
};

///////////////////////////////////////////////

class TempRegisterAllocator {
public:
    TempRegisterAllocator();

    std::shared_ptr<Register> Allocate(ASMOperand::Size size = ASMOperand::Size::Byte4);
    void Free(const std::shared_ptr<Register>& reg);

private:
    std::set<int> available_regs_;
};