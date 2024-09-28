#pragma once

#include <cstdint>
#include "mmu/Memory.hpp"

class APU;

/* https://gbdev.io/pandocs/Audio_Registers.html#sound-channel-1--pulse-with-period-sweep */
template <bool WithSweep>
class PulseChannel {
    APU &apu_;

    /* 0xFF10 — NR10: Channel 1 sweep */
    uint8_t NR10_ = 0x0;
    /* 0xFF11 / 0xFF16 — NRX1: Channel length timer & duty cycle */
    uint8_t NRX1_ = 0x0;
    /* 0xFF12 / 0xFF17 — NRX2: Channel volume & envelope */
    uint8_t NRX2_ = 0x0;
    /* 0xFF13 / 0xFF18 — NRX3: Channel period low [write-only] */
    uint8_t NRX3_ = 0x0;
    /* 0xFF14 / 0xFF19 — NRX4: Channel period high & control */
    uint8_t NRX4_ = 0x0;

    /* The role of frequency timer is to step wave generation. Each T-cycle the frequency timer is decremented by 1. As soon as it reaches 0, it is reloaded with a value calculated using the below formula, and the wave duty position register is incremented by 1.
    https://nightshade256.github.io/2021/03/27/gb-sound-emulation.html */
    uint16_t frequency_timer = 0x0;

    uint8_t wave_duty_position = 0;
    uint8_t wave_duty_pattern() const { return (NRX1_ >> 6); }

    uint8_t DIV_APU = 0x0;
    bool prev_DIV_bit = false;

    uint16_t frequency() const {
        return uint16_t((NRX4_ & 0b111) << 8) | NRX3_;
    }

    void frequency(uint16_t value) {
        NRX4_ = (NRX4_ & 0b11111000) | ((value >> 8) & 0b111);
        NRX3_ = value & 0xFF;
    }

    // internal values
    bool channel_enabled = false;
    uint8_t period_timer = 0x0;
    uint8_t current_volume = 0x0;
    bool sweep_enabled = false;
    uint16_t shadow_frequency = 0x0;
    uint8_t sweep_timer = 0x0;
    uint8_t length_timer = 0x0; // 64 - NRX1[5-0]

    // envelope register
    uint8_t initial_volume() const { return (NRX2_ & 0b11110000) >> 4; };
    bool envelope_upwards() const { return NRX2_ & (1 << 3); };
    uint8_t envelope_period() const { return NRX2_ & 0b111; };

    // sweep register
    uint8_t sweep_period() const requires WithSweep { return (NR10_ & 0b01110000) >> 4; };
    bool freq_decrease() const requires WithSweep { return NR10_ & (1 << 3); };
    uint8_t sweep_freq_shift() const requires WithSweep { return NR10_ & 0b111; };

    // sweep helper function, calculates the next frrequency and performs overflow detection (https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frequency_Sweep)
    uint16_t calculate_next_frequency() requires WithSweep;

    // length register
    bool length_enabled() const { return NRX4_ & (1 << 6); };

    void length_tick();
    void sweep_tick() requires WithSweep;
    void envelope_tick();

    void trigger();

    public:
    explicit PulseChannel(APU &apu) : apu_(apu) { };

    float_t tick();

    uint8_t NR10() const requires WithSweep { return NR10_; };
    void NR10(uint8_t value) requires WithSweep { NR10_ = value; };
    uint8_t NRX1() const { return NRX1_ & 0b11000000; };
    void NRX1(uint8_t value) { NRX1_ = value; };
    uint8_t NRX2() const { return NRX2_; };
    void NRX2(uint8_t value) { NRX2_ = value; };
    uint8_t NRX3() const { return NRX3_; };
    void NRX3(uint8_t value) { NRX3_ = value; };
    uint8_t NRX4() const { return NRX4_ & (1 << 6); };
    void NRX4(uint8_t value) {
        NRX4_ = value;
        if (NRX4_ & (1 << 7))
            trigger();
    };
};