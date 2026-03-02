#include "include/asm/emitter.h"

void Emitter::EmitBack(std::shared_ptr<ASMInstruction> instr) {
    instructions.push_back(std::move(instr));
}

void Emitter::EmitFront(std::shared_ptr<ASMInstruction> instr) {
    instructions.push_front(std::move(instr));
}

auto Emitter::begin() { return instructions.begin(); }
auto Emitter::end() { return instructions.end(); }

void Emitter::Replace(std::list<std::shared_ptr<ASMInstruction>>::iterator it,
                      std::vector<std::shared_ptr<ASMInstruction>>& new_instrs) {
    for (auto& instr : new_instrs) {
        instructions.insert(it, std::move(instr));
    }
    instructions.erase(it);
}

void Emitter::Clear() { instructions.clear(); }

std::vector<std::shared_ptr<ASMInstruction>> Emitter::Flatten() const {
    return {instructions.begin(), instructions.end()};
}

std::list<std::shared_ptr<ASMInstruction>>& Emitter::GetInstructions() {
    return instructions;
}

void Emitter::SetInstructions(std::list<std::shared_ptr<ASMInstruction>> instrs) {
    instructions = std::move(instrs);
}