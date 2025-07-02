#pragma once
#include <map>
#include <vector>

struct VariableInfo {
    int offset;
};

struct FrameInfo {
    std::vector<std::map<std::string, VariableInfo>> variables;
    int total_size = 0;
};