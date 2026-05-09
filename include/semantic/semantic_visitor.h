#pragma once
#include <vector>

#include "include/visitors/recursive_visitor.h"

class SemanticVisitor : public RecursiveVisitor {
public:
    virtual ~SemanticVisitor() = default;

    const std::vector<std::string>& GetErrors() const;

protected:
    void ReportError(const std::string& message);
    bool HasErrors() const;

private:
    std::vector<std::string> errors_;
};