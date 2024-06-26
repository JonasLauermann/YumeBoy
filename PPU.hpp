#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <queue>
#include <stdexcept>
#include <vector>
#include <SDL.h>

#define VRAM_BEGIN 0x8000
#define VRAM_END 0x9FFF
#define OAM_RAM_BEGIN 0xFE00
#define OAM_RAM_END 0xFE9F
#define LCD_REG_BEGIN 0xFF40
#define LCD_REG_END 0xFF4B

#define DISPLAY_WIDTH 160
#define DISPLAY_HEIGHT 144

class YumeBoy;

/** The Pixel-Processing Unit. It handles anything related to drawing the frames of games. */
class PPU {
    YumeBoy &yume_boy_;         // Reference to Emulator
    uint64_t tick_time_;        // the amount of time the ppu has run for this tick (in T-cycles / 2^22 Hz)
    uint64_t scanline_time_;    // the amount of time the ppu has run for this scanline (in T-cycles / 2^22 Hz)

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
    /** 0xFF46 — DMA: OAM DMA source address & start.
     * Writing to this register starts a DMA transfer from ROM or RAM to OAM (Object Attribute Memory). */
    uint8_t DMA = 0x0;
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

    // /* LCD
    //  * Implemented as a vector of pixels, each pixel is a number that represents one of the system colors.
    //  * 0 - White
    //  * 1 - Light Gray
    //  * 2 - Dark Gray
    //  * 3 - Black
    //  * 4 - Off */
    // std::vector<uint8_t> lcd;

    class LCD {
        using pixel_buffer_t = std::array<uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT * 4>;

        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *pixel_matrix_texture;
        pixel_buffer_t pixel_buffer;
        pixel_buffer_t::iterator buffer_it;

        bool power_;

        public:
        enum Color : uint8_t {
            WHITE = 0,
            LIGHT_GRAY = 1,
            DARK_GRAY = 2,
            BLACK = 3
        };

        LCD(const char *title, int width, int height) : buffer_it(pixel_buffer.begin()), power_(false) {
            window = SDL_CreateWindow("YumeBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * 4, DISPLAY_HEIGHT * 4, 0);
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            pixel_matrix_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        }

        ~LCD() { SDL_DestroyWindow(window); }

        void power(bool on) { power_ = on; }

        void push_pixel(Color c);

        void update_screen();

    } lcd;


    /* one ppu cycle takes two T-cycles (2^22 Hz) */
    void dot(uint8_t cycles = 1) {
        tick_time_ += (cycles * 2);
        scanline_time_ += (cycles * 2);
    }

    enum PPU_Mode { H_Blank = 0, V_Blank = 1, OAM_Scan = 2, Pixel_Transfer = 3 } mode_;  // current Mode of the PPU

    void set_mode(PPU_Mode mode);

    /* OAM specific */
    uint16_t oam_pointer = OAM_RAM_BEGIN;

    struct OAM_entry {
        uint8_t x;
        uint8_t y;
        uint8_t tile_id;
        uint8_t flags;
    };

    std::vector<OAM_entry> scanline_sprites;

    /* Pixel FIFO & Fetcher */
    enum ColorPallet {BG, S0, S1};
    struct Pixel {
        uint8_t color;
        ColorPallet pallet;
        bool bg_priority;
    };

    std::queue<Pixel> BG_FIFO;
    std::queue<Pixel> Sprite_FIFO;

    bool pixel_fifo_stopped;
    uint8_t fifo_pushed_pixels; // current x position of the pixel fifo

    class PixelFetcher {
        uint8_t fetcher_x;   // fetcher internal coordinates
        uint8_t step;
        bool fetch_sprite_;
        bool fetch_window;
        PPU &p;

        OAM_entry oam_entry;

        uint8_t tile_id;
        uint8_t low_data;
        uint8_t high_data;

        public:
        PixelFetcher(PPU &ppu) : fetcher_x(0), step(0), fetch_sprite_(false), fetch_window(false), p(ppu) { }

        void bg_tick();

        void sprite_tick();

        void tick() {
            if (fetch_sprite_)
                sprite_tick();
            else
                bg_tick();
        }

        void fetch_sprite(OAM_entry entry);

        void reset();

    } fetcher;


public:
    PPU() = delete;
    PPU(YumeBoy &yume_boy) : yume_boy_(yume_boy), scanline_time_(0), lcd("YumeBoy", DISPLAY_WIDTH * 4, DISPLAY_HEIGHT * 4), mode_(V_Blank), pixel_fifo_stopped(false), fifo_pushed_pixels(0), fetcher(*this) {
        vram_.resize(VRAM_END - VRAM_BEGIN + 1, 0);
        oam_ram_.resize(OAM_RAM_END - OAM_RAM_BEGIN + 1, 0);
        
        SDL_Init(SDL_INIT_VIDEO);
        set_mode(OAM_Scan);
    }

    ~PPU() {
        SDL_Quit();
    }

    uint8_t read_vram(uint16_t addr);
    void write_vram(uint16_t addr, uint8_t value);

    uint8_t read_oam_ram(uint16_t addr);
    void write_oam_ram(uint16_t addr, uint8_t value);

    uint8_t read_lcd_register(uint16_t addr) const;
    void write_lcd_register(uint16_t addr, uint8_t value);

    void h_blank_tick();
    void v_blank_tick();
    void oam_scan_tick();
    void pixel_transfer_tick();

    /* Runs the PPU until it reaches the next "stable" state. Returns the amount of time spent. */
    uint32_t tick() {
        // skip if LCD is turned off
        if (not (LCDC & 1 << 7)) return 1;

        /* set time of current tick to 0. */
        tick_time_ = 0;

        // Determine current mode_
        switch (mode_) {
            case H_Blank:
                h_blank_tick();
                break;
            case V_Blank:
                v_blank_tick();
                break;
            case OAM_Scan: 
                oam_scan_tick();
                break;
            case Pixel_Transfer:
                pixel_transfer_tick();
                break;
            default:
                throw std::runtime_error("Unknown PPU mode");
        }
        return tick_time_;
    }
};