#include "include/semantic/symbol_table.h"

void SymbolTable::EnterScope() { scopes_.emplace_back(); }

void SymbolTable::ExitScope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

bool SymbolTable::Declare(const std::string& original_name, const SymbolInfo& info) {
    if (IsInCurrentScope(original_name)) {
        return false;
    }
    scopes_.back()[original_name] = info;
    all_symbols_[info.name] = info;
    return true;
}

void SymbolTable::Register(const SymbolInfo& info) { all_symbols_[info.name] = info; }

std::optional<SymbolInfo> SymbolTable::Lookup(const std::string& original_name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(original_name);
        if (found != it->end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

bool SymbolTable::IsInCurrentScope(const std::string& name) const {
    if (scopes_.empty()) {
        return false;
    }
    return scopes_.back().find(name) != scopes_.back().end();
}

std::string SymbolTable::GenerateUniqueName(const std::string& base) {
    int count = name_counters_[base]++;
    if (scopes_.size() == 1) {
        return base;
    }
    return base + "." + std::to_string(count);
}

SymbolInfo* SymbolTable::FindByOriginalName(const std::string& original_name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->contains(original_name)) {
            return &((*it)[original_name]);
        }
    }
    return nullptr;
}

SymbolInfo* SymbolTable::FindByUniqueName(const std::string& unique_name) {
    if (all_symbols_.contains(unique_name)) {
        return &all_symbols_[unique_name];
    }
    return nullptr;
}