#pragma once

#include <cstdint>

#include <apu/APU.hpp>
#include <savestate/PulseChannelSaveState.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/array.hpp>

/* Represents the state of a `CPU` object. */
struct APUSaveState {
    uint32_t sample_time;
    APU::sample_array_t samples;
    uint16_t pushed_samples;

    PulseChannelSaveState channel1;
    PulseChannelSaveState channel2;
    // TODO channel 3
    // TODO channel 4

    uint8_t NR50;
    uint8_t NR51;
    uint8_t NR52;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & sample_time;
        ar & samples;
        ar & pushed_samples;

        ar & channel1;
        ar & channel2;
        // TODO channel 3
        // TODO channel 4
        
        ar & NR50;
        ar & NR51;
        ar & NR52;
    }
};