#pragma once

#include <cstdint>

#include <apu/APU.hpp>
#include <savestate/PulseChannelSaveState.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/array.hpp>

/* Represents the state of a `CPU` object. */
struct APUSaveState {
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
        ar & channel1;
        ar & channel2;
        // TODO channel 3
        // TODO channel 4
        
        ar & NR50;
        ar & NR51;
        ar & NR52;
    }
};