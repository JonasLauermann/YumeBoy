#pragma once

#include <cstdint>
#include <queue>
#include <memory>
#include "ppu/states.hpp"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


/* Pixel FIFO & Fetcher */
enum ColorPallet {BG, S0, S1};
struct Pixel {
    uint8_t color;
    ColorPallet pallet;
    bool bg_priority;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & color;
        ar & pallet;
        ar & bg_priority;
    }
};

class PPU;
struct OAM_entry;
struct PixelFetcherSaveState;

class PixelFetcher {
    FETCHER_STATES state = FETCHER_STATES::FetchBGTileNo;
    uint8_t fetcher_x = 0;   // fetcher internal coordinates
    bool fetch_window = false;
    PPU &p;

    std::unique_ptr<OAM_entry> oam_entry;

    uint8_t tile_id;
    uint8_t low_data;
    uint8_t high_data;
    
    bool pixel_fifo_stopped = false;

    public:
    explicit PixelFetcher(PPU &ppu) : p(ppu) { }

    void tick();

    void fetch_sprite(std::unique_ptr<OAM_entry> entry);

    void reset();

    bool fifo_stopped() const { return pixel_fifo_stopped; }

    PixelFetcherSaveState save_state() const;
    void load_state(PixelFetcherSaveState fetcher_state);

};