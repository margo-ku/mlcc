#pragma once

#include <set>
#include <string>
#include <unordered_map>

#include "operands.h"

struct Frame {
    std::unordered_map<std::string, int> offsets_;
    int current_offset_ = 0;
};

class FrameStackAllocator {
public:
    FrameStackAllocator();

    int GetOffset(const std::string& name, int size = 8);

    int GetTotalFrameSize() const;
    int GetAlignedFrameSize(int alignment = 16) const;

    void PushFrame();
    void PopFrame();

private:
    std::vector<Frame> frames_;
};

///////////////////////////////////////////////

class TempRegisterAllocator {
public:
    TempRegisterAllocator();

    std::shared_ptr<Register> Allocate();
    void Free(const std::shared_ptr<Register>& reg);

private:
    std::set<std::string> registers_;
};