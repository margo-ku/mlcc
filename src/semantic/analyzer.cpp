#include "include/semantic/analyzer.h"

#include "include/semantic/loop_analyzer.h"
#include "include/semantic/symbol_resolver.h"
#include "include/semantic/symbol_table.h"
#include "include/semantic/type_checker.h"

SemanticAnalyzer::SemanticAnalyzer(SymbolTable& symbol_table)
    : symbol_resolver_(symbol_table),
      type_checker_(symbol_table),
      loop_analyzer_(symbol_table) {}

SemanticAnalyzer::~SemanticAnalyzer() {}

void SemanticAnalyzer::Analyze(TranslationUnit* translation_unit) {
    RunPass(translation_unit, symbol_resolver_, symbol_resolver_.GetErrors()) &&
        RunPass(translation_unit, type_checker_, type_checker_.GetErrors()) &&
        RunPass(translation_unit, loop_analyzer_, loop_analyzer_.GetErrors());
}

bool SemanticAnalyzer::RunPass(TranslationUnit* translation_unit, Visitor& pass,
                               const std::vector<std::string>& errors) {
    pass.Visit(translation_unit);
    AppendErrors(errors);
    return errors.empty();
}

void SemanticAnalyzer::AppendErrors(const std::vector<std::string>& errors) {
    errors_.insert(errors_.end(), errors.begin(), errors.end());
}

bool SemanticAnalyzer::HasErrors() const { return !errors_.empty(); }

const std::vector<std::string>& SemanticAnalyzer::GetErrors() const { return errors_; }