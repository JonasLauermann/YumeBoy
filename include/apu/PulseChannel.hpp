#pragma once

#include <cstdint>
#include "apu/AudioChannel.hpp"
#include "mmu/Memory.hpp"

class APU;
struct PulseChannelSaveState;

/* https://gbdev.io/pandocs/Audio_Registers.html#sound-channel-1--pulse-with-period-sweep */
template <bool WithSweep>
class PulseChannel : public AudioChannel<true> {
    /* 0xFF10 — NR10: Channel 1 sweep */
    uint8_t NR10_ = 0x0;

    /* The role of frequency timer is to step wave generation. Each T-cycle the frequency timer is decremented by 1. As soon as it reaches 0, it is reloaded with a value calculated using the below formula, and the wave duty position register is incremented by 1.
    https://nightshade256.github.io/2021/03/27/gb-sound-emulation.html */
    uint16_t frequency_timer = 0x0;

    // internal values
    bool sweep_enabled = false;
    uint16_t shadow_frequency = 0x0;
    uint8_t sweep_timer = 0x0;

    uint8_t wave_duty_position = 0;
    uint8_t wave_duty_pattern() const { return (AudioChannel::NRX1() >> 6); }

    // sweep register
    uint8_t sweep_period() const requires WithSweep { return (NR10_ & 0b01110000) >> 4; };
    bool freq_decrease() const requires WithSweep { return NR10_ & (1 << 3); };
    uint8_t sweep_freq_shift() const requires WithSweep { return NR10_ & 0b111; };

    // sweep helper function, calculates the next frrequency and performs overflow detection (https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frequency_Sweep)
    uint16_t calculate_next_frequency() requires WithSweep;
    
    void sweep_tick() requires WithSweep;

    void trigger() override;

    public:
    explicit PulseChannel(APU &apu) : AudioChannel(apu) { };

    float_t tick() override;

    uint8_t NR10() const requires WithSweep { return NR10_; };
    void NR10(uint8_t value) requires WithSweep { NR10_ = value; };
    uint8_t NRX1() const override { return AudioChannel::NRX1() & 0b11000000; };
    void NRX1(uint8_t value) override { AudioChannel::NRX1(value); }
    uint8_t NRX4() const override { return AudioChannel::NRX4() & (1 << 6); };
    void NRX4(uint8_t value) override { AudioChannel::NRX4(value); }

   PulseChannelSaveState save_state() const;

   void load_state(PulseChannelSaveState state);
};