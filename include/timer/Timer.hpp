#pragma once

#include <cstdint>
#include <cpu/InterruptBus.hpp>
#include <mmu/Memory.hpp>


class YumeBoy;
struct TimerSaveState;

class Timer : public Memory {
    InterruptBus &interrupts;

    uint16_t system_counter = 0x0;  // incremented every t-cycle, upper 8-bit make up DIV register

    /* 0xFF07 - TAC - Timer control
    Bit 2   - Enable: Controls whether TIMA is incremented. Note that DIV is always counting, regardless of this bit.
    Bit 1-0 - Clock select: Controls the frequency at which TIMA is incremented.
    (https://gbdev.io/pandocs/Timer_and_Divider_Registers.html#ff07--tac-timer-control) */
    uint8_t TAC_ = 0x0;

    bool old_tac_bit = false;   // Implements the "delay" in the DIV & TAC falling edge detector.

    /* 0xFF05 - TIMA: Timer counter
    This timer is incremented at the clock frequency specified by the TAC register (0xFF07). When the value overflows (exceeds 0xFF) it is reset to the value specified in TMA (0xFF06) and an interrupt is requested. */
    uint8_t TIMA_ = 0x0;

    uint8_t tima_overflow_delay = 0;  // Is set to 4 to indicate that TIMA has overflown and decremented every tick. When 0, overwrite TIMA with TMA.

    /* 0xFF06 - TMA: Timer modulo
    When TIMA overflows, it is reset to the value in this register and an interrupt is requested.
    If a TMA write is executed on the same M-cycle as the content of TMA is transferred to TIMA due to a timer overflow, the old value is transferred to TIMA. */
    uint8_t TMA_ = 0x0;

    /* 0xFF04 — DIV: Divider register
    This register is incremented at a rate of 16384Hz. Writing any value to this register resets it to $00.
    Additionally, this register is reset when executing the stop instruction, and only begins ticking again once stop mode ends.
    The value of DIV is the actual bits of the system internal counter, not a mirror, not a register that increases with the system internal counter: The actual bits of the counter mapped to memory. */
    uint8_t DIV() const { return system_counter >> 8; }
    /* DIV can be written, but its value resets to 0 no matter what the value written is. In fact, the whole system internal counter is set to 0. */
    void DIV(uint8_t) { system_counter = 0; }
    
    uint8_t TAC() const { return TAC_; }
    /* Only the lower 3 bits are (R/W) */
    void TAC(uint8_t value) { TAC_ = value & 0b111; }
    
    uint8_t TIMA() const { return TIMA_; }
    void TIMA(uint8_t value) {
        TIMA_ = value;
    }
    
    uint8_t TMA() const { return TMA_; }
    void TMA(uint8_t value) { TMA_ = value; }

    public:
    Timer() = delete;
    explicit Timer(InterruptBus &interrupts) : interrupts(interrupts) { }

    /* Advance the Timer state by a single T-Cycle. */
    void tick();

    bool contains_address(uint16_t addr) const override;
    uint8_t read_memory(uint16_t addr) override;
    void write_memory(uint16_t addr, uint8_t value) override;

    TimerSaveState save_state() const;
    void load_state(TimerSaveState state);

};