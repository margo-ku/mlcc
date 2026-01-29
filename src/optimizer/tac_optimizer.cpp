#include "include/optimizer/tac_optimizer.h"

#include <unordered_set>

#include "include/optimizer/control_flow_utils.h"
void TACOptimizer::Optimize(std::vector<std::vector<TACInstruction>>& instructions) {
    cf_graphs_.resize(instructions.size());
    bool changed = true;
    size_t step = 0;
    while (changed) {
        changed = false;
        changed |= FoldConstants(instructions);
        changed |= EliminateUnreachableCode(instructions);
    }
}

bool TACOptimizer::EliminateUnreachableCode(
    std::vector<std::vector<TACInstruction>>& instructions_list) {
    bool changed = false;
    for (size_t index = 0; index < instructions_list.size(); ++index) {
        auto& instructions = instructions_list[index];
        auto& cfg = cf_graphs_[index];
        BuildControlFlowGraph(instructions, cfg);
        cfg::transforms::RemoveUnreachableBlocks(cfg);
        cfg::transforms::RemoveRedundantGotos(cfg);
        cfg::transforms::RemoveRedundantLabels(cfg);
        cfg::transforms::RemoveEmptyBlocks(cfg);

        auto new_instructions = cfg.GetInstructions();
        if (new_instructions != instructions) {
            changed = true;
            instructions = std::move(new_instructions);
        }
    }
    return changed;
}

void TACOptimizer::BuildControlFlowGraph(std::vector<TACInstruction>& instructions,
                                         cfg::ControlFlowGraph& cfg) {
    cfg.Clear();
    cfg.BuildBlocks(instructions);
    cfg.BuildEdges();
}

static const std::unordered_set<TACInstruction::OpCode> binaryOps = {
    TACInstruction::OpCode::Add,          TACInstruction::OpCode::Sub,
    TACInstruction::OpCode::Mul,          TACInstruction::OpCode::Div,
    TACInstruction::OpCode::Mod,          TACInstruction::OpCode::Equal,
    TACInstruction::OpCode::NotEqual,     TACInstruction::OpCode::Less,
    TACInstruction::OpCode::LessEqual,    TACInstruction::OpCode::Greater,
    TACInstruction::OpCode::GreaterEqual, TACInstruction::OpCode::BitwiseAnd,
    TACInstruction::OpCode::BitwiseXor,   TACInstruction::OpCode::BitwiseOr,
    TACInstruction::OpCode::LeftShift,    TACInstruction::OpCode::RightShift};

static const std::unordered_set<TACInstruction::OpCode> unaryOps = {
    TACInstruction::OpCode::Plus, TACInstruction::OpCode::Minus,
    TACInstruction::OpCode::Not, TACInstruction::OpCode::BinaryNot};

bool TACOptimizer::FoldConstants(
    std::vector<std::vector<TACInstruction>>& function_instructions) {
    bool changed = false;
    std::vector<std::vector<TACInstruction>> new_function_instructions;

    for (const auto& instructions : function_instructions) {
        std::vector<TACInstruction> new_instructions;
        for (const auto& instruction : instructions) {
            auto op = instruction.GetOp();
            auto instr = instruction;

            if (binaryOps.contains(op)) {
                changed |= TryFoldBinary(instruction, instr);
            } else if (unaryOps.contains(op)) {
                changed |= TryFoldUnary(instruction, instr);
            } else if (op == TACInstruction::OpCode::If ||
                       op == TACInstruction::OpCode::IfFalse) {
                if (!TryFoldCondition(instruction, instr, changed)) {
                    continue;
                }
            }
            new_instructions.push_back(instr);
        }
        new_function_instructions.push_back(std::move(new_instructions));
    }

    function_instructions = std::move(new_function_instructions);
    return changed;
}

bool TACOptimizer::TryFoldBinary(const TACInstruction& in, TACInstruction& out) {
    if (!IsConstant(in.GetLhs()) || !IsConstant(in.GetRhs())) {
        return false;
    }

    int lhs = std::stoi(in.GetLhs());
    int rhs = std::stoi(in.GetRhs());
    int result = EvaluateBinaryOp(in.GetOp(), lhs, rhs);

    out = TACInstruction(TACInstruction::OpCode::Assign, in.GetDst(),
                         std::to_string(result));
    return true;
}

bool TACOptimizer::TryFoldUnary(const TACInstruction& in, TACInstruction& out) {
    if (!IsConstant(in.GetLhs())) {
        return false;
    }

    int operand = std::stoi(in.GetLhs());
    int result = EvaluateUnaryOp(in.GetOp(), operand);

    out = TACInstruction(TACInstruction::OpCode::Assign, in.GetDst(),
                         std::to_string(result));
    return true;
}

bool TACOptimizer::TryFoldCondition(const TACInstruction& in, TACInstruction& out,
                                    bool& changed) {
    if (!IsConstant(in.GetLhs())) {
        return true;
    }

    int condition = std::stoi(in.GetLhs());
    changed = true;

    if (in.GetOp() == TACInstruction::OpCode::If) {
        if (condition != 0) {
            out = TACInstruction(TACInstruction::OpCode::GoTo, in.GetLabel());
            return true;
        }
        return false;
    } else {
        if (condition == 0) {
            out = TACInstruction(TACInstruction::OpCode::GoTo, in.GetLabel());
            return true;
        }
        return false;
    }
}

bool TACOptimizer::IsConstant(const std::string& operand) {
    try {
        std::stoi(operand);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

int TACOptimizer::EvaluateBinaryOp(TACInstruction::OpCode op, int lhs, int rhs) {
    switch (op) {
        case TACInstruction::OpCode::Add:
            return lhs + rhs;
        case TACInstruction::OpCode::Sub:
            return lhs - rhs;
        case TACInstruction::OpCode::Mul:
            return lhs * rhs;
        case TACInstruction::OpCode::Div:
            return rhs != 0 ? lhs / rhs : 0;
        case TACInstruction::OpCode::Mod:
            return rhs != 0 ? lhs % rhs : 0;
        case TACInstruction::OpCode::Equal:
            return lhs == rhs ? 1 : 0;
        case TACInstruction::OpCode::NotEqual:
            return lhs != rhs ? 1 : 0;
        case TACInstruction::OpCode::Less:
            return lhs < rhs ? 1 : 0;
        case TACInstruction::OpCode::LessEqual:
            return lhs <= rhs ? 1 : 0;
        case TACInstruction::OpCode::Greater:
            return lhs > rhs ? 1 : 0;
        case TACInstruction::OpCode::GreaterEqual:
            return lhs >= rhs ? 1 : 0;
        case TACInstruction::OpCode::BitwiseAnd:
            return lhs & rhs;
        case TACInstruction::OpCode::BitwiseXor:
            return lhs ^ rhs;
        case TACInstruction::OpCode::BitwiseOr:
            return lhs | rhs;
        case TACInstruction::OpCode::LeftShift:
            return lhs << rhs;
        case TACInstruction::OpCode::RightShift:
            return lhs >> rhs;
        default:
            return 0;
    }
}

int TACOptimizer::EvaluateUnaryOp(TACInstruction::OpCode op, int operand) {
    switch (op) {
        case TACInstruction::OpCode::Plus:
            return +operand;
        case TACInstruction::OpCode::Minus:
            return -operand;
        case TACInstruction::OpCode::Not:
            return operand ? 0 : 1;
        case TACInstruction::OpCode::BinaryNot:
            return ~operand;
        default:
            return operand;
    }
}