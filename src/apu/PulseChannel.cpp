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
void PulseChannel<WithSweep>::length_tick()
{
    if (not length_enabled())
        return;

    --length_timer;

    if (length_timer == 0)
        channel_enabled = false;
}

template <bool WithSweep>
uint16_t PulseChannel<WithSweep>::calculate_next_frequency()
requires WithSweep
{
    // calculate new frequency
    uint16_t new_frequency = shadow_frequency >> sweep_freq_shift();
    new_frequency += freq_decrease() ? shadow_frequency - new_frequency : shadow_frequency + new_frequency;

    // overflow check
    if (new_frequency > 2047)
        channel_enabled = false;

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
void PulseChannel<WithSweep>::envelope_tick()
{
    if (envelope_period() == 0)
        return;

    if (period_timer > 0)
        --period_timer;

    if (period_timer > 0)
        return;
    
    period_timer = envelope_period();

    if ((current_volume < 0xF and envelope_upwards()) or (current_volume > 0x00 and not envelope_upwards()))
        current_volume += envelope_upwards() ? 1 : -1;
}

template <bool WithSweep>
void PulseChannel<WithSweep>::trigger()
{
    // set internal registers
    channel_enabled = true;

    period_timer = envelope_period();
    current_volume = initial_volume();

    shadow_frequency = frequency();
    if constexpr (WithSweep) {
        sweep_timer = sweep_period() == 0 ? sweep_period() : 8;
        sweep_enabled = sweep_period() != 0 or sweep_freq_shift() != 0;
        if (sweep_freq_shift() != 0)
            calculate_next_frequency(); // overflow check
    }
    
    length_timer = 64 - (NRX1_ & 0b111111);
}

template <bool WithSweep>
float_t PulseChannel<WithSweep>::tick()
{
    // Frame Sequencer: increase DIV_APU every time DIV's bit 4 (5 in double-speed mode) goes from 1 to 0
    bool div_bit = apu_.mem_.read_memory(0xFF04) & (1 << 4);
    if (prev_DIV_bit and not div_bit) {
        DIV_APU = (DIV_APU + 1) % 8;

        // sound length
        if (DIV_APU % 2 == 0)
            length_tick();

        // sweep (channel 1 only)
        if constexpr (WithSweep) {
            if (DIV_APU % 4 == 2)
                sweep_tick();
        }

        // envelop
        if (DIV_APU == 7)
            envelope_tick();
    }
    prev_DIV_bit = div_bit;

    // duty
    if (frequency_timer-- == 0) {
        frequency_timer = (2048 - frequency()) * 4;
        wave_duty_position = (wave_duty_position + 1) % 8;
    }

    // DAC conversion
    if ((NRX2_ & 0xF8) == 0)  // DAC is disabled
        return 0;

    int8_t dac_input = ((wave_duty_table[wave_duty_pattern()] >> (7 - wave_duty_position)) & 1) ? current_volume : -current_volume;

    float_t dac_output = float_t(dac_input / 15.0f);

    return dac_output;
}

template <bool WithSweep>
PulseChannelSaveState PulseChannel<WithSweep>::save_state() const
{
    PulseChannelSaveState s = {
        NR10_,
        NRX1_,
        NRX2_,
        NRX3_,
        NRX4_,

        frequency_timer,

        wave_duty_position,

        DIV_APU,
        prev_DIV_bit,

        channel_enabled,
        period_timer,
        current_volume,
        sweep_enabled,
        shadow_frequency,
        sweep_timer,
        length_timer,
    };
    return s;
}

template <bool WithSweep>
void PulseChannel<WithSweep>::load_state(PulseChannelSaveState state)
{
    if constexpr (WithSweep)
        NR10_ = state.NR10_;
    
    NRX1_ = state.NRX1_;
    NRX2_ = state.NRX2_;
    NRX3_ = state.NRX3_;
    NRX4_ = state.NRX4_;

    frequency_timer = state.frequency_timer;

    wave_duty_position = state.wave_duty_position;

    DIV_APU = state.DIV_APU;
    prev_DIV_bit = state.prev_DIV_bit;

    channel_enabled = state.channel_enabled;
    period_timer = state.period_timer;
    current_volume = state.current_volume;
    sweep_enabled = state.sweep_enabled;
    shadow_frequency = state.shadow_frequency;
    sweep_timer = state.sweep_timer;
    length_timer = state.length_timer;
}

template class PulseChannel<true>; // instantiate PulseChannel with sweep
template class PulseChannel<false>; // instantiate PulseChannel without sweep
