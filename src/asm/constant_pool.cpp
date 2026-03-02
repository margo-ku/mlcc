#include "include/asm/constant_pool.h"

std::string ConstantPool::RegisterDouble(double value) {
    uint64_t bits = DoubleToBits(value);

    if (auto it = double_name_cache_.find(bits); it != double_name_cache_.end()) {
        return it->second;
    }

    std::string name = "_dconst_" + std::to_string(next_id_++);
    double_constants_.emplace_back(name, value);
    double_name_cache_[bits] = name;
    return name;
}

const std::vector<std::pair<std::string, double>>& ConstantPool::GetDoubleConstants()
    const {
    return double_constants_;
}

uint64_t ConstantPool::DoubleToBits(double value) {
    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}
