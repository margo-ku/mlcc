#include "include/optimizer/tac_optimizer.h"

void TACOptimizer::Optimize(std::vector<std::vector<TACInstruction>>& instructions) {
    FoldConstants(instructions);
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

bool TACOptimizer::FoldConstants(
    std::vector<std::vector<TACInstruction>>& function_instructions) {
    bool changed = false;
    std::vector<std::vector<TACInstruction>> new_function_instructions;

    for (const auto& instructions : function_instructions) {
        std::vector<TACInstruction> new_instructions;
        for (const auto& instruction : instructions) {
            auto op = instruction.GetOp();
            auto instr = instruction;

            if (op == TACInstruction::OpCode::Add || op == TACInstruction::OpCode::Sub ||
                op == TACInstruction::OpCode::Mul || op == TACInstruction::OpCode::Div ||
                op == TACInstruction::OpCode::Mod ||
                op == TACInstruction::OpCode::Equal ||
                op == TACInstruction::OpCode::NotEqual ||
                op == TACInstruction::OpCode::Less ||
                op == TACInstruction::OpCode::LessEqual ||
                op == TACInstruction::OpCode::Greater ||
                op == TACInstruction::OpCode::GreaterEqual ||
                op == TACInstruction::OpCode::BitwiseAnd ||
                op == TACInstruction::OpCode::BitwiseXor ||
                op == TACInstruction::OpCode::BitwiseOr ||
                op == TACInstruction::OpCode::LeftShift ||
                op == TACInstruction::OpCode::RightShift) {
                if (IsConstant(instruction.GetLhs()) &&
                    IsConstant(instruction.GetRhs())) {
                    int lhs = std::stoi(instruction.GetLhs());
                    int rhs = std::stoi(instruction.GetRhs());
                    int result = EvaluateBinaryOp(op, lhs, rhs);

                    instr = TACInstruction(TACInstruction::OpCode::Assign,
                                           instruction.GetDst(), std::to_string(result));
                    changed = true;
                }
            } else if (op == TACInstruction::OpCode::Plus ||
                       op == TACInstruction::OpCode::Minus ||
                       op == TACInstruction::OpCode::Not ||
                       op == TACInstruction::OpCode::BinaryNot) {
                if (IsConstant(instruction.GetLhs())) {
                    int operand = std::stoi(instruction.GetLhs());
                    int result = EvaluateUnaryOp(op, operand);

                    instr = TACInstruction(TACInstruction::OpCode::Assign,
                                           instruction.GetDst(), std::to_string(result));
                    changed = true;
                }
            } else if (op == TACInstruction::OpCode::If) {
                if (IsConstant(instruction.GetLhs())) {
                    int condition = std::stoi(instruction.GetLhs());
                    changed = true;
                    if (condition != 0) {
                        instr = TACInstruction(TACInstruction::OpCode::GoTo,
                                               instruction.GetLabel());
                    } else {
                        continue;
                    }
                }
            } else if (op == TACInstruction::OpCode::IfFalse) {
                if (IsConstant(instruction.GetLhs())) {
                    int condition = std::stoi(instruction.GetLhs());
                    changed = true;
                    if (condition == 0) {
                        instr = TACInstruction(TACInstruction::OpCode::GoTo,
                                               instruction.GetLabel());
                    } else {
                        continue;
                    }
                }
            }
            new_instructions.push_back(instr);
        }
        new_function_instructions.push_back(std::move(new_instructions));
    }

    function_instructions = std::move(new_function_instructions);
    return changed;
}