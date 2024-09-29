#pragma once

#include <cstdint>

#include <savestate/AudioChannelSaveState.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

/* Represents the state of a `PulseChannel` object. */
struct PulseChannelSaveState {
    AudioChannelSaveState base;

    uint8_t NR10_;

    uint16_t frequency_timer;

    bool sweep_enabled;
    uint16_t shadow_frequency;
    uint8_t sweep_timer;

    uint8_t wave_duty_position;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & base;

        ar & NR10_;

        ar & frequency_timer;
        ar & sweep_enabled;
        ar & shadow_frequency;
        ar & sweep_timer;

        ar & wave_duty_position;
    }
};