#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "operands.h"

class FrameStackAllocator {
public:
    FrameStackAllocator();

    int GetOffset(const std::string& name, int size = 4);

    int GetTotalFrameSize() const;
    int GetAlignedFrameSize(int alignment = 16) const;

private:
    std::unordered_map<std::string, int> offsets_;
    int current_offset_;
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