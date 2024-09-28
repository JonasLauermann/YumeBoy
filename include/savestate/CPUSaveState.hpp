#pragma once

#include <cstdint>
#include <cpu/states.hpp>
#include <savestate/InstructionSaveState.hpp>

/* Represents the state of a `CPU` object. */
struct CPUSaveState {
    // mem_ is reconstructed

    const CPU_STATES state;
    const InstructionSaveState instruction;

    const uint8_t A;
    const uint8_t B;
    const uint8_t C;
    const uint8_t D;
    const uint8_t E;
    const uint8_t H;
    const uint8_t L;

    const uint16_t SP;
    const uint16_t PC;

    const uint8_t F;

    const bool IME;
    const bool EI_executed;
    const bool set_IME;

    const uint8_t IF_;
    const uint8_t IE_;

    const bool HALT_bug;
};