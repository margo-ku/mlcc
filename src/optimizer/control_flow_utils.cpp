#include "include/optimizer/control_flow_utils.h"

#include <unordered_set>

namespace cfg::transforms {

void RemoveUnreachableBlocks(ControlFlowGraph& cfg) {
    cfg.MarkAliveBlocks();
    std::unordered_set<size_t> ids_to_remove;
    for (size_t index = 2; index < cfg.GetBlockCount(); ++index) {
        if (!cfg.GetBlock(index).alive) {
            ids_to_remove.insert(index);
        }
    }
    cfg.RemoveBlocks(ids_to_remove);
}

void RemoveRedundantGotos(ControlFlowGraph& cfg) {
    for (size_t index = 2; index < cfg.GetBlockCount(); ++index) {
        auto& block = cfg.GetBlock(index);
        if (block.instructions.empty()) {
            continue;
        }
        size_t instr_index = block.instructions.size() - 1;
        if (block.instructions[instr_index].GetOp() != TACInstruction::OpCode::GoTo) {
            continue;
        }

        if (*cfg.GetSuccessors(index).begin() == index + 1) {
            block.instructions.pop_back();
        }
    }
}

void RemoveRedundantLabels(ControlFlowGraph& cfg) {
    for (size_t index = 2; index < cfg.GetBlockCount(); ++index) {
        auto& block = cfg.GetBlock(index);
        if (block.instructions.empty()) {
            continue;
        }
        size_t instr_index = 0;
        if (block.instructions[instr_index].GetOp() != TACInstruction::OpCode::Label) {
            continue;
        }

        // to do: refactor
        const auto predecessors = cfg.GetPredecessors(index);
        if (predecessors.size() == 1) {
            size_t prev_index = *predecessors.begin();
            if (prev_index >= 2 && prev_index == index - 1) {
                const auto& prev_block = cfg.GetBlock(prev_index);
                if (!prev_block.instructions.empty()) {
                    auto op = prev_block.instructions[prev_block.instructions.size() - 1]
                                  .GetOp();
                    if (op != TACInstruction::OpCode::GoTo &&
                        op != TACInstruction::OpCode::If &&
                        op != TACInstruction::OpCode::IfFalse) {
                        std::vector<TACInstruction> new_instructions;
                        for (size_t instr_index = 1;
                             instr_index < block.instructions.size(); ++instr_index) {
                            new_instructions.push_back(block.instructions[instr_index]);
                        }
                        block.instructions = std::move(new_instructions);
                    }
                }
            }
        }
    }
}

void RemoveEmptyBlocks(ControlFlowGraph& cfg) {
    std::unordered_set<size_t> ids_to_remove;
    for (size_t index = 2; index < cfg.GetBlockCount(); ++index) {
        if (cfg.GetBlock(index).instructions.empty()) {
            ids_to_remove.insert(index);
        }
    }
    cfg.RemoveBlocks(ids_to_remove);
}

}  // namespace cfg::transforms