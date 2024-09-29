#include <apu/PulseChannel.hpp>

#include <array>
#include <apu/APU.hpp>
#include <savestate/PulseChannelSaveState.hpp>


const std::array<uint8_t, 4> wave_duty_table = {
    0b11111110, // 12.5% Duty Cycle
    0b01111110, // 25% Duty Cycle
    0b01111000, // 50% Duty Cycle
    0b10000001, // 75% Duty Cycle
};

template <bool WithSweep>
uint16_t PulseChannel<WithSweep>::calculate_next_frequency()
requires WithSweep
{
    // calculate new frequency
    uint16_t new_frequency = shadow_frequency >> sweep_freq_shift();
    new_frequency += freq_decrease() ? shadow_frequency - new_frequency : shadow_frequency + new_frequency;

    // overflow check
    if (new_frequency > 2047)
        AudioChannel::disable();

    return new_frequency;
}

template <bool WithSweep>
void PulseChannel<WithSweep>::sweep_tick()
requires WithSweep
{
    if (sweep_timer > 0)
        --sweep_timer;

    if (sweep_timer > 0)
        return;

    sweep_timer = sweep_period() > 0 ? sweep_period() : 8;
    
    if (not sweep_enabled or sweep_period() == 0)
        return;
    
    uint16_t new_frequency = calculate_next_frequency();

    if (new_frequency <= 2047 and sweep_freq_shift() > 0) {
        frequency(new_frequency);
        shadow_frequency = new_frequency;

        // another overflow check
        calculate_next_frequency();
    }
}

template <bool WithSweep>
void PulseChannel<WithSweep>::trigger()
{
    AudioChannel::trigger();

    if constexpr (WithSweep) {
        shadow_frequency = frequency();
        sweep_timer = sweep_period() == 0 ? sweep_period() : 8;
        sweep_enabled = sweep_period() != 0 or sweep_freq_shift() != 0;
        if (sweep_freq_shift() != 0)
            calculate_next_frequency(); // overflow check
    }
}

template <bool WithSweep>
float_t PulseChannel<WithSweep>::tick()
{
    // Frame Sequencer
    if (DIV_APU_tick()) {
        // sound length
        if (DIV_APU() % 2 == 0)
            length_tick();

        // sweep (channel 1 only)
        if constexpr (WithSweep) {
            if (DIV_APU() % 4 == 2)
                sweep_tick();
        }

        // envelop
        if (DIV_APU() == 7)
            envelope_tick();
    }

    // duty
    if (frequency_timer-- == 0) {
        frequency_timer = (2048 - frequency()) * 4;
        wave_duty_position = (wave_duty_position + 1) % 8;
    }

    // DAC conversion
    if ((NRX2() & 0xF8) == 0)  // DAC is disabled
        return 0;

    int8_t dac_input = ((wave_duty_table[wave_duty_pattern()] >> (7 - wave_duty_position)) & 1) ? current_volume() : -current_volume();

    float_t dac_output = float_t(dac_input / 15.0f);

    return dac_output;
}

template <bool WithSweep>
PulseChannelSaveState PulseChannel<WithSweep>::save_state() const
{
    PulseChannelSaveState s = {
        AudioChannel::save_state(),

        NR10_,

        frequency_timer,

        sweep_enabled,
        shadow_frequency,
        sweep_timer,

        wave_duty_position,
    };
    return s;
}

template <bool WithSweep>
void PulseChannel<WithSweep>::load_state(PulseChannelSaveState state)
{
    AudioChannel::load_state(state.base);

    if constexpr (WithSweep)
        NR10_ = state.NR10_;

    frequency_timer = state.frequency_timer;

    sweep_enabled = state.sweep_enabled;
    shadow_frequency = state.shadow_frequency;
    sweep_timer = state.sweep_timer;

    wave_duty_position = state.wave_duty_position;
}

template class PulseChannel<true>; // instantiate PulseChannel with sweep
template class PulseChannel<false>; // instantiate PulseChannel without sweep
