#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "include/types/type.h"

struct SymbolInfo {
    enum class LinkageKind { External, Internal, None };
    std::string name;
    std::string original_name;
    LinkageKind linkage = LinkageKind::None;
    TypeRef type = nullptr;

    bool is_defined = false;
};

////////////////////////////////////////////////////////////

class SymbolTable {
public:
    void EnterScope();

    void ExitScope();

    bool Declare(const std::string& original_name, const SymbolInfo& info);

    void Register(const SymbolInfo& info);

    std::optional<SymbolInfo> Lookup(const std::string& original_name) const;

    bool IsInCurrentScope(const std::string& name) const;

    std::string GenerateUniqueName(const std::string& base);

    SymbolInfo* FindByOriginalName(const std::string& original_name);

    SymbolInfo* FindByUniqueName(const std::string& unique_name);

private:
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes_;
    std::unordered_map<std::string, int> name_counters_;
    std::unordered_map<std::string, SymbolInfo> all_symbols_;
};