#pragma once

#include <cstdint>


/* Represents the state of an `Instruction` object. It saves all information to construct a new instruction during the load operation that is in the same state as the saved Instruction. It does not matter if it is a `MultiCycleInstruction` or not`. */
struct InstructionSaveState {
    const uint8_t opcode;
    const bool extended;

    // MultiCycleInstruction
    const uint8_t cycle;
    const uint8_t temp_u8;
    const uint16_t temp_u16;
};