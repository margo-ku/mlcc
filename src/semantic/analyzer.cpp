#include "include/semantic/analyzer.h"

#include "include/semantic/loop_analyzer.h"
#include "include/semantic/symbol_resolver.h"
#include "include/semantic/symbol_table.h"
#include "include/semantic/type_checker.h"

SemanticAnalyzer::SemanticAnalyzer(SymbolTable& symbol_table)
    : symbol_table_(symbol_table),
      symbol_resolver_(symbol_table),
      type_checker_(symbol_table),
      loop_analyzer_(symbol_table) {}

SemanticAnalyzer::~SemanticAnalyzer() {}

void SemanticAnalyzer::Analyze(TranslationUnit* translation_unit) {
    symbol_resolver_.Visit(translation_unit);
    type_checker_.Visit(translation_unit);
    loop_analyzer_.Visit(translation_unit);

    UpdateErrors();
}

void SemanticAnalyzer::UpdateErrors() {
    errors_.insert(errors_.end(), symbol_resolver_.GetErrors().begin(),
                   symbol_resolver_.GetErrors().end());
    errors_.insert(errors_.end(), type_checker_.GetErrors().begin(),
                   type_checker_.GetErrors().end());
    errors_.insert(errors_.end(), loop_analyzer_.GetErrors().begin(),
                   loop_analyzer_.GetErrors().end());
}

bool SemanticAnalyzer::HasErrors() const { return !errors_.empty(); }

const std::vector<std::string>& SemanticAnalyzer::GetErrors() const { return errors_; }