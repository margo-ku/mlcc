#include "include/optimizer/control_flow_graph.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <queue>
#include <unordered_set>

namespace cfg {

static const std::unordered_set<TACInstruction::OpCode> block_start_opcodes = {
    TACInstruction::OpCode::Label};

static const std::unordered_set<TACInstruction::OpCode> block_end_opcodes = {
    TACInstruction::OpCode::GoTo, TACInstruction::OpCode::If,
    TACInstruction::OpCode::IfFalse, TACInstruction::OpCode::Return};

ControlFlowGraph::ControlFlowGraph() {}

void ControlFlowGraph::BuildBlocks(const std::vector<TACInstruction>& instructions) {
    AddInitialBlocks();
    Block current_block;
    bool is_block_started = false;
    for (size_t index = 0; index < instructions.size(); ++index) {
        auto& instr = instructions[index];
        auto op = instr.GetOp();
        if (block_start_opcodes.contains(op) && is_block_started) {
            blocks_.push_back(current_block);
            is_block_started = false;
        }

        if (!is_block_started) {
            current_block = Block();
            current_block.id = blocks_.size();
            is_block_started = true;
        }

        if (block_start_opcodes.contains(op)) {
            current_block.label = instr.GetLabel();
            label_to_block_[current_block.label] = current_block.id;
        }

        current_block.instructions.push_back(instr);

        if (block_end_opcodes.contains(op)) {
            blocks_.push_back(current_block);
            is_block_started = false;
        }
    }

    if (is_block_started) {
        blocks_.push_back(current_block);
        is_block_started = false;
    }
}

void ControlFlowGraph::BuildEdges() {
    successors_.clear();
    predecessors_.clear();
    successors_.resize(blocks_.size());
    predecessors_.resize(blocks_.size());

    if (blocks_.size() == 2) {
        AddEdge(entry_index, exit_index);
        return;
    }

    AddEdge(entry_index, 2);
    for (size_t index = 2; index < blocks_.size(); ++index) {
        assert(blocks_[index].instructions.size() - 1 >= 0);
        size_t instr_index = blocks_[index].instructions.size() - 1;
        auto& instr = blocks_[index].instructions[instr_index];
        auto op = instr.GetOp();
        if (op == TACInstruction::OpCode::GoTo) {
            size_t next_index = label_to_block_[instr.GetLabel()];
            AddEdge(index, next_index);
        } else if (op == TACInstruction::OpCode::If ||
                   op == TACInstruction::OpCode::IfFalse) {
            if (index + 1 < blocks_.size()) {
                AddEdge(index, index + 1);
            } else {
                AddEdge(index, exit_index);
            }

            size_t next_index = label_to_block_[instr.GetLabel()];
            AddEdge(index, next_index);
        } else if (op == TACInstruction::OpCode::Return) {
            AddEdge(index, exit_index);
        } else {
            if (index + 1 < blocks_.size()) {
                AddEdge(index, index + 1);
            } else {
                AddEdge(index, exit_index);
            }
        }
    }
}

const Block& ControlFlowGraph::GetBlock(size_t id) const { return blocks_[id]; }

Block& ControlFlowGraph::GetBlock(size_t id) { return blocks_[id]; }

size_t ControlFlowGraph::GetBlockCount() const { return blocks_.size(); }

std::optional<size_t> ControlFlowGraph::FindBlockByLabel(const std::string& label) const {
    auto it = label_to_block_.find(label);
    if (it == label_to_block_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void ControlFlowGraph::MarkAliveBlocks() {
    for (auto& block : blocks_) {
        block.alive = false;
    }

    std::queue<size_t> queue;
    std::unordered_set<size_t> used;

    queue.push(entry_index);
    used.insert(entry_index);
    while (!queue.empty()) {
        size_t index = queue.front();
        queue.pop();
        blocks_[index].alive = true;

        for (size_t child : successors_[index]) {
            if (!used.contains(child)) {
                queue.push(child);
                used.insert(child);
            }
        }
    }
}

void ControlFlowGraph::RemoveBlocks(const std::unordered_set<size_t>& ids_to_remove) {
    std::vector<Block> new_blocks;
    for (size_t index = 0; index < blocks_.size(); ++index) {
        if (ids_to_remove.contains(index) && index >= 2) {
            continue;
        }
        blocks_[index].id = new_blocks.size();
        new_blocks.push_back(std::move(blocks_[index]));
    }
    blocks_ = std::move(new_blocks);
    label_to_block_.clear();
    for (size_t index = 0; index < blocks_.size(); ++index) {
        if (!blocks_[index].label.empty()) {
            label_to_block_[blocks_[index].label] = index;
        }
    }
    BuildEdges();
}

void ControlFlowGraph::Clear() {
    blocks_.clear();
    label_to_block_.clear();
    successors_.clear();
    predecessors_.clear();
}

std::vector<TACInstruction> ControlFlowGraph::GetInstructions() const {
    std::vector<TACInstruction> instructions;
    for (const auto& block : blocks_) {
        for (const auto& instr : block.instructions) {
            instructions.push_back(instr);
        }
    }
    return instructions;
}

void ControlFlowGraph::AddInitialBlocks() {
    Block entry;
    entry.id = entry_index;
    entry.is_entry = true;

    Block exit;
    exit.id = exit_index;
    exit.is_exit = true;

    blocks_.push_back(entry);
    blocks_.push_back(exit);
}

void ControlFlowGraph::AddEdge(size_t from, size_t to) {
    if (from >= successors_.size() || to >= predecessors_.size()) {
        assert(false && "Invalid edge index");
    }
    successors_[from].insert(to);
    predecessors_[to].insert(from);
}

void ControlFlowGraph::Print(std::ostream& out) {
    out << "Control Flow Graph:\n" << std::endl;
    for (size_t index = 0; index < blocks_.size(); ++index) {
        const auto& block = blocks_[index];
        out << "Block " << block.id;
        if (block.is_entry) {
            out << " [entry]";
        }

        if (block.is_exit) {
            out << " [exit]";
        }

        if (!block.label.empty()) {
            out << " (label: " << block.label << ")";
        }

        out << "\n";
        for (const auto& instr : block.instructions) {
            out << "    " << instr.ToString() << "\n";
        }

        out << "    Successors: ";
        for (size_t child : successors_[index]) {
            out << child << ' ';
        }
        out << "\n";

        out << "    Predecessors: ";
        for (size_t parent : predecessors_[index]) {
            out << parent << ' ';
        }
        out << "\n";
        out << "\n";
    }
}

const std::set<size_t>& ControlFlowGraph::GetSuccessors(size_t id) const {
    return successors_[id];
}

const std::set<size_t>& ControlFlowGraph::GetPredecessors(size_t id) const {
    return predecessors_[id];
}

}  // namespace cfg