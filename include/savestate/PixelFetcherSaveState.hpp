#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <ppu/states.hpp>
#include <savestate/OAMEntrySaveState.hpp>


/* Represents the state of a `PixelFetcher` object. */
struct PixelFetcherSaveState {
    const FETCHER_STATES state;

    const uint8_t fetcher_x;
    const bool fetch_window;

    OAMEntrySaveState oam_entry;

    const uint8_t tile_id;
    const uint8_t low_data;
    const uint8_t high_data;

    const bool pixel_fifo_stopped;
};