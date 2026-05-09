#include "include/semantic/loop_analyzer.h"

LoopAnalyzer::LoopAnalyzer(SymbolTable& symbol_table) : symbol_table_(symbol_table) {}

LoopAnalyzer::~LoopAnalyzer() {}

void LoopAnalyzer::Visit(JumpStatement* statement) {
    if (loop_ids_.empty()) {
        ReportError("jump statement outside of loop");
        return;
    }
    statement->SetLabel(loop_ids_.top());
}

void LoopAnalyzer::Visit(WhileStatement* statement) {
    std::string loop_id = GenerateLoopId();
    statement->SetLabel(loop_id);
    loop_ids_.push(loop_id);

    statement->GetCondition()->Accept(this);
    statement->GetBody()->Accept(this);

    loop_ids_.pop();
}

void LoopAnalyzer::Visit(ForStatement* statement) {
    std::string loop_id = GenerateLoopId();
    statement->SetLabel(loop_id);
    loop_ids_.push(loop_id);

    statement->GetInit()->Accept(this);
    statement->GetCondition()->Accept(this);
    statement->GetIncrement()->Accept(this);
    statement->GetBody()->Accept(this);

    loop_ids_.pop();
}

std::string LoopAnalyzer::GenerateLoopId() {
    return "loop." + std::to_string(loop_id_counter_++);
}