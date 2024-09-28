#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <ppu/states.hpp>
#include <savestate/PixelFetcherSaveState.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/deque.hpp>    // deque must be included before queue!
#include <boost/serialization/queue.hpp>
#include <boost/serialization/vector.hpp>


struct Pixel;

/* Represents the state of a `PPU` object. */
struct PPUSaveState {
    PPU_STATES state;

    uint32_t scanline_time_;

    std::vector<uint8_t> vram_;
    std::vector<uint8_t> oam_ram_;

    uint8_t LCDC;
    uint8_t STAT;
    uint8_t SCY;
    uint8_t SCX;
    uint8_t LY;
    uint8_t LYC;
    uint8_t BGP;
    uint8_t OBP0;
    uint8_t WY;
    uint8_t WX;

    uint16_t oam_pointer;
    std::queue<Pixel> BG_FIFO;
    std::queue<Pixel> Sprite_FIFO;
    uint8_t fifo_pushed_pixels;
    PixelFetcherSaveState fetcher;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & state;

        ar & scanline_time_;

        ar & vram_;
        ar & oam_ram_;

        ar & LCDC;
        ar & STAT;
        ar & SCY;
        ar & SCX;
        ar & LY;
        ar & LYC;
        ar & BGP;
        ar & OBP0;
        ar & WY;
        ar & WX;

        ar & oam_pointer;
        ar & BG_FIFO;
        ar & Sprite_FIFO;
        ar & fifo_pushed_pixels;
        ar & fetcher;
    }
};