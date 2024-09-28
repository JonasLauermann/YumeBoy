#include "timer/Timer.hpp"

#include <cpu/InterruptBus.hpp>
#include <savestate/TimerSaveState.hpp>


void Timer::tick()
{
    // based on https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html#relation-between-timer-and-divider-register
    // increment system_counter
    ++system_counter;

    // determine bit selected by TAC multiplexer
    uint8_t selected_bit = 3;
    switch (TAC_ & 0b11) {
        case 0:
            selected_bit += 2;
        case 3:
            selected_bit += 2;
        case 2:
            selected_bit += 2;
        case 1:
            break;
        default:
            std::unreachable();
    }
    bool tac_bit = (system_counter & (1 << selected_bit)) and (TAC_ & 0b100);

    // DIV & TAC falling edge detector
    if (not tac_bit and old_tac_bit) {
        // when falling edge is detected => increment TIMA
        ++TIMA_;

        // TIMA overflow detector
        if (TIMA_ == 0)
            tima_overflow_delay = 4;
    }
    old_tac_bit = tac_bit;

    if (tima_overflow_delay > 0) {
        --tima_overflow_delay;
        if (tima_overflow_delay == 0) {
            TIMA_ = TMA_;
            interrupts.request_interrupt(InterruptBus::INTERRUPT::TIMER_INTERRUPT);
        }
    }
}

bool Timer::contains_address(uint16_t addr) const
{
    return (0xFF04 <= addr and addr <= 0xFF07);
}

uint8_t Timer::read_memory(uint16_t addr)
{
    if (addr == 0xFF04)
        return DIV();
    else if (addr == 0xFF05)
        return TIMA();
    else if (addr == 0xFF06)
        return TMA();
    else if (addr == 0xFF07)
        return TAC();
}

void Timer::write_memory(uint16_t addr, uint8_t value)
{
    if (addr == 0xFF04)
        DIV(value);
    else if (addr == 0xFF05)
        TIMA(value);
    else if (addr == 0xFF06)
        TMA(value);
    else if (addr == 0xFF07)
        TAC(value);
}

TimerSaveState Timer::save_state() const {
    TimerSaveState s = {
        system_counter,
        TAC_,
        old_tac_bit,
        TIMA_,
        tima_overflow_delay,
        TMA_
    };
    return s;
}

void Timer::load_state(TimerSaveState state)
{
    system_counter = state.system_counter;
    TAC_ = state.TAC_;
    old_tac_bit = state.old_tac_bit;
    TIMA_ = state.TIMA_;
    tima_overflow_delay = state.tima_overflow_delay;
    TMA_ = state.TMA_;
}
