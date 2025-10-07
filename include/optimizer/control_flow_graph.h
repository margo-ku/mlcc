#pragma once

#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "include/visitors/tac_visitor.h"

namespace cfg {

struct Block {
    std::vector<TACInstruction> instructions;
    std::string label;
    size_t id;
    bool is_entry = false;
    bool is_exit = false;
    bool alive = false;
};

class ControlFlowGraph {
public:
    ControlFlowGraph();
    void BuildBlocks(const std::vector<TACInstruction>& instructions);
    void BuildEdges();

    const Block& GetBlock(size_t id) const;
    Block& GetBlock(size_t id);
    size_t GetBlockCount() const;

    const std::set<size_t>& GetSuccessors(size_t id) const;
    const std::set<size_t>& GetPredecessors(size_t id) const;

    std::optional<size_t> FindBlockByLabel(const std::string& label) const;
    void MarkAliveBlocks();
    void RemoveBlocks(const std::unordered_set<size_t>& ids_to_remove);

    void Clear();
    std::vector<TACInstruction> GetInstructions() const;

    void Print(std::ostream& out);

private:
    std::vector<std::set<size_t>> successors_;
    std::vector<std::set<size_t>> predecessors_;
    std::vector<Block> blocks_;
    std::unordered_map<std::string, size_t> label_to_block_;

    static constexpr size_t entry_index = 0;
    static constexpr size_t exit_index = 1;

    void AddInitialBlocks();
    void AddEdge(size_t from, size_t to);
};

}  // namespace cfg