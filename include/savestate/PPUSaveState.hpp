#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <ppu/states.hpp>
#include <savestate/PixelFetcherSaveState.hpp>


struct Pixel;

/* Represents the state of a `PPU` object. */
struct PPUSaveState {
    const PPU_STATES state;

    const uint32_t scanline_time_;

    const std::vector<uint8_t> vram_;
    const std::vector<uint8_t> oam_ram_;

    const uint8_t LCDC;
    const uint8_t STAT;
    const uint8_t SCY;
    const uint8_t SCX;
    const uint8_t LY;
    const uint8_t LYC;
    const uint8_t BGP;
    const uint8_t OBP0;
    const uint8_t WY;
    const uint8_t WX;

    const uint16_t oam_pointer;
    const std::queue<Pixel> BG_FIFO;
    const std::queue<Pixel> Sprite_FIFO;
    const uint8_t fifo_pushed_pixels;
    const PixelFetcherSaveState fetcher;
};