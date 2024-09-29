#pragma once

#include <cstdint>
#include "mmu/Memory.hpp"

class APU;
struct AudioChannelSaveState;

template <bool WithEnvelope>
class AudioChannel {
    APU &apu_;

    uint8_t NRX1_ = 0x0;
    uint8_t NRX2_ = 0x0;
    uint8_t NRX3_ = 0x0;
    uint8_t NRX4_ = 0x0;

    uint8_t DIV_APU_ = 0x0;
    bool prev_DIV_bit = false;

    // internal values
    bool channel_enabled = false;
    uint8_t period_timer = 0x0;
    uint8_t current_volume_ = 0x0;
    uint8_t length_timer = 0x0; // 64 - NRX1[5-0]

    protected:
    bool enabled() const { return channel_enabled; }
    void enable() { channel_enabled = true; }
    void disable() { channel_enabled = false; }

    bool DIV_APU_tick() {
        // increase DIV_APU every time DIV's bit 4 (5 in double-speed mode) goes from 1 to 0
        bool div_bit = apu_.mem_.read_memory(0xFF04) & (1 << 4);
        bool falling_edge = prev_DIV_bit and not div_bit;
        prev_DIV_bit = div_bit;
        if (falling_edge) {
            DIV_APU_ = (DIV_APU_ + 1) % 8;
        }
        return falling_edge;
    }
    uint8_t DIV_APU() const { return DIV_APU_; }

    uint16_t frequency() const {
        return uint16_t((NRX4_ & 0b111) << 8) | NRX3_;
    }

    void frequency(uint16_t value) {
        NRX4_ = (NRX4_ & 0b11111000) | ((value >> 8) & 0b111);
        NRX3_ = value & 0xFF;
    }

    uint8_t current_volume() const { return current_volume_; }

    // volume & envelope register
    uint8_t initial_volume() const { return (NRX2_ & 0b11110000) >> 4; };
    bool envelope_upwards() const requires WithEnvelope { return NRX2_ & (1 << 3); };
    uint8_t envelope_period() const requires WithEnvelope { return NRX2_ & 0b111; };

    // length register
    bool length_enabled() const { return NRX4_ & (1 << 6); };

    void length_tick();
    void envelope_tick() requires WithEnvelope;

    virtual void trigger();

    public:
    virtual ~AudioChannel() = default;
    explicit AudioChannel(APU &apu) : apu_(apu) { };

    virtual uint8_t NRX1() const { return NRX1_; }
    virtual uint8_t NRX2() const { return NRX2_; }
    virtual uint8_t NRX3() const { return NRX3_; }
    virtual uint8_t NRX4() const { return NRX4_; }

    virtual void NRX1(uint8_t value) { NRX1_ = value; }
    virtual void NRX2(uint8_t value) { NRX2_ = value; }
    virtual void NRX3(uint8_t value) { NRX3_ = value; }
    virtual void NRX4(uint8_t value) {
        NRX4_ = value;
        if (NRX4_ & (1 << 7))
            trigger();
    }

    virtual float_t tick() = 0;

   AudioChannelSaveState save_state() const;

   void load_state(AudioChannelSaveState state);
};