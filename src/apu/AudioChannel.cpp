#include <apu/AudioChannel.hpp>

#include <array>
#include <apu/APU.hpp>
#include <savestate/AudioChannelSaveState.hpp>


template <bool WithEnvelope>
void AudioChannel<WithEnvelope>::length_tick()
{
    if (not length_enabled())
        return;

    --length_timer;

    if (length_timer == 0)
        channel_enabled = false;
}

template <bool WithEnvelope>
void AudioChannel<WithEnvelope>::envelope_tick()
requires WithEnvelope
{
    if (envelope_period() == 0)
        return;

    if (period_timer > 0)
        --period_timer;

    if (period_timer > 0)
        return;
    
    period_timer = envelope_period();

    if ((current_volume_ < 0xF and envelope_upwards()) or (current_volume_ > 0x00 and not envelope_upwards()))
        current_volume_ += envelope_upwards() ? 1 : -1;
}

template <bool WithEnvelope>
void AudioChannel<WithEnvelope>::trigger()
{
    // set internal registers
    channel_enabled = true;

    if constexpr (WithEnvelope) {
        period_timer = envelope_period();
    }
    current_volume_ = initial_volume();
    
    length_timer = 64 - (NRX1_ & 0b111111);
}

template <bool WithEnvelope>
AudioChannelSaveState AudioChannel<WithEnvelope>::save_state() const
{
    AudioChannelSaveState s = {
        NRX1_,
        NRX2_,
        NRX3_,
        NRX4_,

        DIV_APU_,
        prev_DIV_bit,

        channel_enabled,
        period_timer,
        current_volume_,
        length_timer,
    };
    return s;
}

template <bool WithEnvelope>
void AudioChannel<WithEnvelope>::load_state(AudioChannelSaveState state)
{
    NRX1_ = state.NRX1_;
    NRX2_ = state.NRX2_;
    NRX3_ = state.NRX3_;
    NRX4_ = state.NRX4_;

    DIV_APU_ = state.DIV_APU;
    prev_DIV_bit = state.prev_DIV_bit;

    channel_enabled = state.channel_enabled;
    period_timer = state.period_timer;
    current_volume_ = state.current_volume;
    length_timer = state.length_timer;
}

template class AudioChannel<true>;  // instantiate AudioChannel with envelope
template class AudioChannel<false>; // instantiate AudioChannel without envelope
