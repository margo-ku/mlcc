#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

class ConstantPool {
public:
    std::string RegisterDouble(double value);

    const std::vector<std::pair<std::string, double>>& GetDoubleConstants() const;

private:
    std::vector<std::pair<std::string, double>> double_constants_;
    std::unordered_map<uint64_t, std::string> double_name_cache_;
    size_t next_id_ = 0;

    static uint64_t DoubleToBits(double value);
};
