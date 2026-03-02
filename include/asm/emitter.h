#pragma once

#include <list>

#include "include/asm/instructions.h"

class Emitter {
public:
    void EmitFront(std::shared_ptr<ASMInstruction> instr);
    void EmitBack(std::shared_ptr<ASMInstruction> instr);

    void Replace(std::list<std::shared_ptr<ASMInstruction>>::iterator it,
                 std::vector<std::shared_ptr<ASMInstruction>>& new_instrs);
    void Clear();

    std::vector<std::shared_ptr<ASMInstruction>> Flatten() const;

    std::list<std::shared_ptr<ASMInstruction>>& GetInstructions();
    void SetInstructions(std::list<std::shared_ptr<ASMInstruction>> instrs);

    auto begin();
    auto end();

private:
    std::list<std::shared_ptr<ASMInstruction>> instructions;
};