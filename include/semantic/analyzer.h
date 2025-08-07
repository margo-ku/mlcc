#pragma once

#include <vector>

#include "include/semantic/loop_analyzer.h"
#include "include/semantic/symbol_resolver.h"
#include "include/semantic/type_checker.h"

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();
    void Analyze(TranslationUnit* translation_unit);

    bool HasErrors() const;
    const std::vector<std::string>& GetErrors() const;
    void UpdateErrors();

private:
    std::vector<std::string> errors_;
    SymbolResolver symbol_resolver_;
    TypeChecker type_checker_;
    LoopAnalyzer loop_analyzer_;
};