#pragma once

#include <cstdint>


#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

/* Represents the state of a `AudioChannel` object. */
struct AudioChannelSaveState {
    uint8_t NRX1_;
    uint8_t NRX2_;
    uint8_t NRX3_;
    uint8_t NRX4_;

    uint8_t DIV_APU;
    bool prev_DIV_bit;

    bool channel_enabled;
    uint8_t period_timer;
    uint8_t current_volume;
    uint8_t length_timer;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & NRX1_;
        ar & NRX2_;
        ar & NRX3_;
        ar & NRX4_;

        ar & DIV_APU;
        ar & prev_DIV_bit;

        ar & channel_enabled;
        ar & period_timer;
        ar & current_volume;
        ar & length_timer;
    }
};