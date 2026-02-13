#pragma once

#include "include/optimizer/control_flow_graph.h"
#include "include/tac/instruction.h"

class TACOptimizer {
public:
    void Optimize(std::vector<std::vector<TACInstruction>>& instructions);

private:
    bool FoldConstants(std::vector<std::vector<TACInstruction>>& instructions);
    bool PropagateCopies(std::vector<std::vector<TACInstruction>>& instructions);
    bool EliminateDeadStores(std::vector<std::vector<TACInstruction>>& instructions);
    bool EliminateUnreachableCode(std::vector<std::vector<TACInstruction>>& instructions);

    bool IsConstant(const TACOperand& operand);
    long long EvaluateBinaryOp(TACInstruction::OpCode op, long long lhs, long long rhs);
    long long EvaluateUnaryOp(TACInstruction::OpCode op, long long operand);
    bool TryFoldBinary(const TACInstruction& in, TACInstruction& out);
    bool TryFoldUnary(const TACInstruction& in, TACInstruction& out);
    bool TryFoldCondition(const TACInstruction& in, TACInstruction& out, bool& changed);

    static void BuildControlFlowGraph(std::vector<TACInstruction>& instructions,
                                      cfg::ControlFlowGraph& cfg);

    std::vector<cfg::ControlFlowGraph> cf_graphs_;
};