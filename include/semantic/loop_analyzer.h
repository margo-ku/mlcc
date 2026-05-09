#pragma once

#include <stack>

#include "include/semantic/semantic_visitor.h"
#include "include/semantic/symbol_table.h"

class LoopAnalyzer : public SemanticVisitor {
public:
    explicit LoopAnalyzer(SymbolTable& symbol_table);
    virtual ~LoopAnalyzer();
    void Visit(JumpStatement* statement) override;
    void Visit(WhileStatement* statement) override;
    void Visit(ForStatement* statement) override;

private:
    std::stack<std::string> loop_ids_;
    size_t loop_id_counter_ = 0;
    SymbolTable& symbol_table_;

    std::string GenerateLoopId();
};