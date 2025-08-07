#include "include/semantic/analyzer.h"

SemanticAnalyzer::SemanticAnalyzer() {}

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