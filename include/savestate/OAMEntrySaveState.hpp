#pragma once

#include <cstdint>


/* Represents the state of a `OAM_entry` object. */
struct OAMEntrySaveState {
    const uint8_t y;
    const uint8_t x;
    const uint8_t tile_id;
    const uint8_t flags;
};