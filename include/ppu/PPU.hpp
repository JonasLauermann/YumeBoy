#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <queue>
#include <stdexcept>
#include <vector>
#include <cpu/InterruptBus.hpp>
#include "ppu/PixelFetcher.hpp"
#include "ppu/states.hpp"
#include "mmu/Memory.hpp"

constexpr uint16_t VRAM_BEGIN = 0x8000;
constexpr uint16_t VRAM_END = 0x9FFF;
constexpr uint16_t OAM_RAM_BEGIN = 0xFE00;
constexpr uint16_t OAM_RAM_END = 0xFE9F;
constexpr uint16_t LCD_REG_BEGIN = 0xFF40;
constexpr uint16_t LCD_REG_END = 0xFF4B;

class YumeBoy;
class LCD;

struct OAM_entry {
    uint8_t y;
    uint8_t x;
    uint8_t tile_id;
    uint8_t flags;
};

/** The Pixel-Processing Unit. It handles anything related to drawing the frames of games. */
class PPU : public Memory {
    friend PixelFetcher;
    
    LCD &lcd;

    MMU &mem;
    InterruptBus &interrupts;
    
    PPU_STATES state = PPU_STATES::VBlank;  // current Mode of the PPU

    uint32_t scanline_time_ = 0;    // the amount of time the ppu has run for this scanline (in T-cycles / 2^22 Hz)

    std::vector<uint8_t> vram_;
    std::vector<uint8_t> oam_ram_;

    /* LCD Registers */
    /** 0xFF40 — LCDC: the main LCD Control register.
     * Its bits toggle what elements are displayed on the screen, and how.
     * Bit-7: LCD & PPU enable: 0 = Off; 1 = On
     * Bit-6: Window tile map area: 0 = 9800–9BFF; 1 = 9C00–9FFF
     * Bit-5: Window enable: 0 = Off; 1 = On
     * Bit-4: BG & Window tile data area: 0 = 8800–97FF; 1 = 8000–8FFF
     * Bit-3: BG tile map area: 0 = 9800–9BFF; 1 = 9C00–9FFF
     * Bit-2: OBJ size: 0 = 8×8; 1 = 8×16
     * Bit-1: OBJ enable: 0 = Off; 1 = On
     * Bit-0: BG & Window enable / priority [Different meaning in CGB Mode]: 0 = Off; 1 = On */
    uint8_t LCDC = 0x0;
    /** 0xFF41 — STAT: LCD status.
     * Bit-7: unused.
     * Bit-6: LYC int select (Read/Write): If set, selects the LYC == LY condition for the STAT interrupt.
     * Bit-5: Mode 2 int select (Read/Write): If set, selects the Mode 2 condition for the STAT interrupt.
     * Bit-4: Mode 1 int select (Read/Write): If set, selects the Mode 1 condition for the STAT interrupt.
     * Bit-3: Mode 0 int select (Read/Write): If set, selects the Mode 0 condition for the STAT interrupt.
     * Bit-2: LYC == LY (Read-only): Set when LY contains the same value as LYC; it is constantly updated.
     * Bit-1 & Bit-0: PPU mode_ (Read-only): Indicates the PPU’s current status. */
    uint8_t STAT = 0x0;
    /** 0xFF42 — SCY: Background viewport Y position. */
    uint8_t SCY = 0x0;
    /** 0xFF43 — SCX: Background viewport X position. */
    uint8_t SCX = 0x0;
    /** 0xFF44 — LY: LCD Y coordinate [read-only].
     * LY indicates the current horizontal line, which might be about to be drawn, being drawn, or just been drawn. */
    uint8_t LY = 0x0;
    /** 0xFF45 — LYC: LY compare.
     * The Game Boy constantly compares the value of the LYC and LY registers. When both values are identical,
     * the “LYC=LY” flag in the STAT register is set, and (if enabled) a STAT interrupt is requested. */
    uint8_t LYC = 0x0;
    /** 0xFF47 — BGP (Non-CGB Mode only): BG palette data.
     * This register assigns gray shades to the color IDs of the BG and Window tiles. */
    uint8_t BGP = 0x0;
    /** 0xFF48 — OBP0 (Non-CGB Mode only): OBJ palette 0 data.
     * These registers assigns gray shades to the color indexes of the OBJs that use the corresponding palette.
     * They work exactly like BGP, except that the lower two bits are ignored because color index 0 is transparent for OBJs. */
    uint8_t OBP0 = 0x0;
    /** 0xFF49 — OBP1 (Non-CGB Mode only): OBJ palette 1 data.
     * These registers assigns gray shades to the color indexes of the OBJs that use the corresponding palette.
     * They work exactly like BGP, except that the lower two bits are ignored because color index 0 is transparent for OBJs. */
    uint8_t OBP1 = 0x0;
    /** 0xFF4A — WY: Window Y position. (WY=0-143) */
    uint8_t WY = 0x0;
    /** 0xFF4B — WX: Window X position plus 7. (WX=7-166) */
    uint8_t WX = 0x0;

    void set_mode(PPU_STATES new_state);

    /* OAM specific */
    uint16_t oam_pointer = OAM_RAM_BEGIN;
    std::vector<std::unique_ptr<OAM_entry>> scanline_sprites;

    /* Pixel FIFO and Fetcher */
    std::queue<Pixel> BG_FIFO;
    std::queue<Pixel> Sprite_FIFO;
    uint8_t fifo_pushed_pixels = 0; // current x position of the pixel fifo
    PixelFetcher fetcher;

    /* Increment LY and update STAT register */
    void next_scanline();

    /* Returns true if a STAT interrupt should be requested */
    bool STATE_interrupt_signal() const;

    uint8_t read_vram(uint16_t addr);
    void write_vram(uint16_t addr, uint8_t value);

    uint8_t read_oam_ram(uint16_t addr);
    void write_oam_ram(uint16_t addr, uint8_t value);

    uint8_t read_lcd_register(uint16_t addr) const;
    void write_lcd_register(uint16_t addr, uint8_t value);

public:
    PPU() = delete;
    explicit PPU(LCD &lcd, MMU &mem, InterruptBus &interrupts) : lcd(lcd), mem(mem), interrupts(interrupts), fetcher(*this) {
        vram_.resize(VRAM_END - VRAM_BEGIN + 1, 0);
        oam_ram_.resize(OAM_RAM_END - OAM_RAM_BEGIN + 1, 0);
        
        set_mode(PPU_STATES::OAMScan);
    }

    bool contains_address(uint16_t addr) const override;
    uint8_t read_memory(uint16_t addr) override;
    void write_memory(uint16_t addr, uint8_t value) override;

    void h_blank_tick();
    void v_blank_tick();
    void oam_scan_tick();
    void pixel_transfer_tick();

    /* Runs the PPU for a single T-Cycle. */
    void tick();
};