#pragma once

#include <cstdint>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


/* Represents the state of a `OAM_entry` object. */
struct OAMEntrySaveState {
    uint8_t y;
    uint8_t x;
    uint8_t tile_id;
    uint8_t flags;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & y;
        ar & x;
        ar & tile_id;
        ar & flags;
    }
};