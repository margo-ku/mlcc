#pragma once
#include "include/visitors/tac_visitor.h"

class TACOptimizer {
public:
    void Optimize(std::vector<std::vector<TACInstruction>>& instructions);

private:
    bool FoldConstants(std::vector<std::vector<TACInstruction>>& instructions);
    bool AnalyzeControlFlow(std::vector<std::vector<TACInstruction>>& instructions);
    bool PropagateCopies(std::vector<std::vector<TACInstruction>>& instructions);
    bool EliminateDeadStores(std::vector<std::vector<TACInstruction>>& instructions);
    bool EliminateUnreachableCode(std::vector<std::vector<TACInstruction>>& instructions);
    bool IsConstant(const std::string& operand);
    int EvaluateBinaryOp(TACInstruction::OpCode op, int lhs, int rhs);
    int EvaluateUnaryOp(TACInstruction::OpCode op, int operand);
};