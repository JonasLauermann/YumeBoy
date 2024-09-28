#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <ppu/states.hpp>
#include <savestate/OAMEntrySaveState.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


/* Represents the state of a `PixelFetcher` object. */
struct PixelFetcherSaveState {
    FETCHER_STATES state;

    uint8_t fetcher_x;
    bool fetch_window;

    OAMEntrySaveState oam_entry;

    uint8_t tile_id;
    uint8_t low_data;
    uint8_t high_data;

    bool pixel_fifo_stopped;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & state;

        ar & fetcher_x;
        ar & fetch_window;

        ar & oam_entry;

        ar & tile_id;
        ar & low_data;
        ar & high_data;

        ar & pixel_fifo_stopped;
    }
};