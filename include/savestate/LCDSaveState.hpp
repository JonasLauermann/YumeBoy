#pragma once

#include <cstdint>
#include <ppu/LCD.hpp>


/* Represents the state of a `LCD` object. */
struct LCDSaveState {
    // window is reconstructed
    // renderer is reconstructed
    // pixel_matrix_texture is reconstructed

    const LCD::pixel_buffer_t pixel_buffer;
    const LCD::pixel_buffer_t::iterator buffer_it;

    const bool power_;

    const uint64_t next_frame;
};