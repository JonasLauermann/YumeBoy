#pragma once

#include <cstdint>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


/* Represents the state of a `Timer` object. */
struct TimerSaveState {
    uint16_t system_counter;
    uint8_t TAC_;
    bool old_tac_bit;
    uint8_t TIMA_;
    uint8_t tima_overflow_delay;
    uint8_t TMA_;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & system_counter;
        ar & TAC_;
        ar & old_tac_bit;
        ar & TIMA_;
        ar & tima_overflow_delay;
        ar & TMA_;
    }
};