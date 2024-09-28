#pragma once

#include <cstdint>


/* Represents the state of a `Timer` object. */
struct TimerSaveState {
    const uint16_t system_counter;
    const uint8_t TAC_;
    const bool old_tac_bit;
    const uint8_t TIMA_;
    const uint8_t tima_overflow_delay;
    const uint8_t TMA_;
};