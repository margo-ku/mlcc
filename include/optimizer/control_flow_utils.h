#pragma once

#include "control_flow_graph.h"

namespace cfg::transforms {

void RemoveUnreachableBlocks(ControlFlowGraph& cfg);
void RemoveRedundantGotos(ControlFlowGraph& cfg);
void RemoveRedundantLabels(ControlFlowGraph& cfg);
void RemoveEmptyBlocks(ControlFlowGraph& cfg);

}  // namespace cfg::transforms