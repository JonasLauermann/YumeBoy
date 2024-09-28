#pragma once

#include <cstdint>
#include <ppu/LCD.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/array.hpp>


/* Represents the state of a `LCD` object. */
struct LCDSaveState {
    // window is reconstructed
    // renderer is reconstructed
    // pixel_matrix_texture is reconstructed

    LCD::pixel_buffer_t pixel_buffer;
    ptrdiff_t buffer_it;

    bool power_;

    uint64_t next_frame;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & pixel_buffer;
        ar & buffer_it;

        ar & power_;

        ar & next_frame;
    }
};