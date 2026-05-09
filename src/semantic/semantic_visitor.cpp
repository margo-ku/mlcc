#include "include/semantic/semantic_visitor.h"

const std::vector<std::string>& SemanticVisitor::GetErrors() const { return errors_; }

void SemanticVisitor::ReportError(const std::string& message) {
    errors_.push_back(message);
}

bool SemanticVisitor::HasErrors() const { return !errors_.empty(); }
