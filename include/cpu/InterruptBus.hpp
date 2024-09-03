#pragma once

#include <cstdint>
#include <utility>
#include "cpu/CPU.hpp"


class InterruptBus {
    CPU &cpu;

    public:
    explicit InterruptBus(CPU &cpu) : cpu(cpu) { }

    enum class INTERRUPT : uint8_t {
        V_BLANK_INTERRUPT =     1,
        STAT_INTERRUPT =        1 << 1,
        TIMER_INTERRUPT =       1 << 2,
        SERIAL_INTERRUPT =      1 << 3,
        JOYPAD_INTERRUPT =      1 << 4
    };

    void request_interrupt(INTERRUPT intrrupt) {
        cpu.IF(cpu.IF() | std::to_underlying(intrrupt));
    }

};