#pragma once

#include <vector>

#include "include/semantic/loop_analyzer.h"
#include "include/semantic/symbol_resolver.h"
#include "include/semantic/symbol_table.h"
#include "include/semantic/type_checker.h"

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(SymbolTable& symbol_table);
    ~SemanticAnalyzer();
    void Analyze(TranslationUnit* translation_unit);

    bool HasErrors() const;
    const std::vector<std::string>& GetErrors() const;

private:
    bool RunPass(TranslationUnit* translation_unit, Visitor& pass,
                 const std::vector<std::string>& errors);
    void AppendErrors(const std::vector<std::string>& errors);

    SymbolResolver symbol_resolver_;
    TypeChecker type_checker_;
    LoopAnalyzer loop_analyzer_;
    std::vector<std::string> errors_;
};