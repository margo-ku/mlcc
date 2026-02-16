#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/types/numeric_constant.h"
#include "include/types/type.h"

struct SymbolInfo {
    enum class LinkageKind { External, Internal, None };
    enum class StorageDuration { Static, Automatic };
    enum class InitialValue { NoInitializer, Tentative, Initial };

    std::string name;
    std::string original_name;

    LinkageKind linkage = LinkageKind::None;
    StorageDuration duration = StorageDuration::Automatic;
    InitialValue init_state = InitialValue::NoInitializer;
    std::optional<NumericConstant> init_constant;

    TypeRef type = nullptr;
    bool is_defined = false;

    bool HasLinkage() const { return linkage != LinkageKind::None; }
    bool HasStaticDuration() const { return duration == StorageDuration::Static; }
    std::string GetStringInitializer() const;

private:
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

    bool IsInFileScope() const;

    const std::unordered_map<std::string, SymbolInfo>& GetAllSymbols() const;

private:
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes_;
    std::unordered_map<std::string, int> name_counters_;
    std::unordered_map<std::string, SymbolInfo> all_symbols_;
};