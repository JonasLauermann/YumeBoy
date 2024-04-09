#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>
#include <cassert>
#include <array>
#include <queue>

#define VRAM_BEGIN 0x8000
#define VRAM_END 0x9FFF
#define OAM_RAM_BEGIN 0xFE00
#define OAM_RAM_END 0xFE9F
#define LCD_REG_BEGIN 0xFF40
#define LCD_REG_END 0xFF4B


/** The Pixel-Processing Unit. It handles anything related to drawing the frames of games. */
class PPU {
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

    /* LCD
     * Implemented as a vector of pixels, each pixel is a number that represents one of the system colors.
     * 0 - White
     * 1 - Light Gray
     * 2 - Dark Gray
     * 3 - Black
     * 4 - Off */
    std::vector<uint8_t> lcd;

    /* one ppu cycle takes two T-cycles (2^22 Hz) */
    void dot(uint8_t cycles = 1) {
        tick_time_ += (cycles * 2);
        scanline_time_ += (cycles * 2);
    }

    enum PPU_Mode { H_Blank = 0, V_Blank = 1, OAM_Scan = 2, Pixel_Transfer = 3 } mode_;  // current Mode of the PPU

    void set_mode(PPU_Mode mode) {
        switch (mode_) {
            case H_Blank: {
                assert(mode == V_Blank or mode == OAM_Scan);
                break;
            }
            case V_Blank: {
                assert(mode == OAM_Scan);
                break;
            }
            case OAM_Scan: {
                assert(mode == Pixel_Transfer);
                break;
            }
            case Pixel_Transfer: {
                assert(mode == H_Blank);
                break;
            }
            default:
                throw std::runtime_error("Unknown PPU mode");
        }
        mode_ = mode;
        STAT = (STAT & 0b11111100) | mode;
    }

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

        void bg_tick() {
            switch (step++) {
                case 0: { // Fetch Tile Number
                    // determine if a window or BG tile should be fetched
                    fetch_window = p.LCDC & 1 << 5 and p.WY <= p.LY and p.WX <= fetcher_x;
                    // determine which tile-map is in use based on LCDC bit 3 or 6 depending on if a window or BG tile is fetched
                    uint16_t tile_map_addr = p.LCDC & 1 << (3 + 3 * fetch_window) ? 0x9C00 : 0x9800;

                    uint8_t x = fetch_window ? fetcher_x - ((p.WX - 7) / 8) : ((p.SCX / 8) + fetcher_x) & 0x1F;
                    uint8_t y = fetch_window ? p.LY - (p.WY / 8) : (((p.LY + p.SCY) & 0xFF) / 8);
                    assert(x < 32 and y < 32);

                    uint16_t tile_id_addr = tile_map_addr + x + (y * 0x20);
                    tile_id = p.vram_[tile_id_addr - VRAM_BEGIN];
                    break;
                }
                case 1: { // Fetch Tile Data (Low)
                    // determine which tile-map is in use based on LCDC bit 4
                    uint16_t tile_data_addr = p.LCDC & 1 << 4 ? 0x8000 + (tile_id << 4) : 0x9000 + (((int8_t)tile_id) << 4);
                    uint8_t line_offset = fetch_window ? ((p.LY - p.WY) % 8) * 2 : ((p.LY + p.SCY) % 8) * 2;
                    low_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset];
                    break;
                }
                case 2: { // Fetch Tile Data (High)
                    // determine which tile-map is in use based on LCDC bit 4
                    uint16_t tile_data_addr = p.LCDC & 1 << 4 ? 0x8000 + (tile_id << 4) : 0x9000 + (((int8_t)tile_id) << 4);
                    uint8_t line_offset = fetch_window ? ((p.LY - p.WY) % 8) * 2 : ((p.LY + p.SCY) % 8) * 2;
                    high_data = p.vram_[tile_data_addr - VRAM_BEGIN + line_offset + 1];
                    break;
                }
                case 3: { // Push to FIFO (if empty)
                    // check if fifo is empty
                    if (not p.BG_FIFO.empty()) {
                        // stay idle for now
                        --step;
                        return;
                    }

                    for (int i = 7; i >= 0; --i) {
                        uint8_t color = ((high_data >> (i - 1)) & 0b10) | ((low_data >> i) & 0b1);
                        p.BG_FIFO.emplace(color, BG, false);
                    }

                    ++fetcher_x; // increment internal x
                    break;
                }
            }
            step %= 4;
        }

        void sprite_tick() {
            // TODO: consider 8x16 sprites
            switch (step++) {
                case 0: { // Fetch Tile Number
                    tile_id = oam_entry.tile_id;
                    break;
                }
                case 1: { // Fetch Tile Data (Low)
                    uint16_t tile_data_addr = 0x8000 + (tile_id << 4);
                    uint8_t line_offset = oam_entry.flags & 1 << 5 ? (8 - ((p.LY - oam_entry.y) % 8)) * 2 : ((p.LY - oam_entry.y) % 8) * 2;
                    low_data = p.vram_[tile_data_addr + line_offset];
                    break;
                }
                case 2: { // Fetch Tile Data (High)
                    uint16_t tile_data_addr = 0x8000 + (tile_id << 4);
                    uint8_t line_offset = oam_entry.flags & 1 << 5 ? (8 - ((p.LY - oam_entry.y) % 8)) * 2 : ((p.LY - oam_entry.y) % 8) * 2;
                    low_data = p.vram_[tile_data_addr + line_offset + 1];
                    break;
                }
                case 3: { // Push to FIFO (skip pixel if already filled)
                    for (int i = 0; i < 8; ++i) {
                        // flip pixels vertically if flag is set
                        int j = oam_entry.flags & 1 << 6 ? i : 7 - i;
                        uint8_t color = ((high_data >> (j - 1)) & 0b10) | ((low_data >> j) & 0b1);
                        ColorPallet pallet = oam_entry.flags & 1 << 4 ? S1 : S0;

                        // skip pixel if another sprite already occupies the pixel or if it is off-screen
                        if (p.Sprite_FIFO.size() > i or oam_entry.x + i < 8) continue;
                        p.Sprite_FIFO.emplace(color, pallet, oam_entry.flags & 1 << 7);
                    }
                    
                    // set mode back to BG/Window fetching
                    assert(step == 4);
                    step = 0;
                    fetch_sprite_ = false;
                    p.pixel_fifo_stopped = false;

                    break;
                }
            }
            step %= 4;
        }

        void tick() {
            if (fetch_sprite_)
                sprite_tick();
            else
                bg_tick();
        }

        void fetch_sprite(OAM_entry entry) {
            assert(fetch_sprite_);
            // reset fetcher steps and switch mode
            oam_entry = entry;
            step = 0;
            fetch_sprite_ = true;
            p.pixel_fifo_stopped = true;
        }

        void reset() {
            fetcher_x = 0;
            step = 0;
            fetch_sprite_ = false;
            fetch_window = false;
        }

    } fetcher;


public:
    PPU() : scanline_time_(0), mode_(V_Blank), lcd(160 * 144, 4), pixel_fifo_stopped(false), fifo_pushed_pixels(0), fetcher(*this) {
        vram_.resize(VRAM_END - VRAM_BEGIN + 1, 0);
        oam_ram_.resize(OAM_RAM_END - OAM_RAM_BEGIN + 1, 0);

        set_mode(OAM_Scan);
    }

    uint8_t read_vram(uint16_t addr)
    {
        assert(VRAM_BEGIN <= addr and addr <= VRAM_END);
        // VRAM is inaccessible during pixel transfer
        if (mode_ == Pixel_Transfer) return 0xFF;
        return vram_[addr - VRAM_BEGIN];
    }

    void write_vram(uint16_t addr, uint8_t value)
    {
        assert(VRAM_BEGIN <= addr and addr <= VRAM_END);
        // VRAM is inaccessible during pixel transfer
        if (mode_ == Pixel_Transfer) return;
        vram_[addr - VRAM_BEGIN] = value;
    }

    uint8_t read_oam_ram(uint16_t addr)
    {
        assert(OAM_RAM_BEGIN <= addr and addr <= OAM_RAM_END);
        // OAM RAM is inaccessible during OAM scan and pixel transfer
        if (mode_ == Pixel_Transfer or mode_ == OAM_Scan) return 0xFF;
        return oam_ram_[addr - OAM_RAM_BEGIN];
    }

    void write_oam_ram(uint16_t addr, uint8_t value)
    {
        assert(OAM_RAM_BEGIN <= addr and addr <= OAM_RAM_END);
        // OAM RAM is inaccessible during OAM scan and pixel transfer
        if (mode_ == Pixel_Transfer or mode_ == OAM_Scan) return;
        oam_ram_[addr - OAM_RAM_BEGIN] = value;
    }

    uint8_t read_lcd_register(uint16_t addr) const
    {
        assert(LCD_REG_BEGIN <= addr and addr <= LCD_REG_END);
        switch (addr) {
            case 0xFF40:
                return LCDC;
            case 0xFF41:
                return STAT;
            case 0xFF42:
                return SCY;
            case 0xFF43:
                return SCX;
            case 0xFF44:
                return LY;
            case 0xFF45:
                return LYC;
            case 0xFF46:
                return DMA;
            case 0xFF47:
                return BGP;
            case 0xFF48:
                return OBP0;
            case 0xFF49:
                return OBP1;
            case 0xFF4A:
                return WY;
            case 0xFF4B:
                return WX;
            default:
                throw std::runtime_error("Unknown LCD register address");
        }
    }

    void write_lcd_register(uint16_t addr, uint8_t value)
    {
        assert(LCD_REG_BEGIN <= addr and addr <= LCD_REG_END);
        switch (addr) {
            case 0xFF40:
                LCDC = value;
                break;
            case 0xFF41:
                STAT = (value & 0b11111000) | (STAT & 0b111); // Bits 0-3 are not writable
                break;
            case 0xFF42:
                SCY = value;
                break;
            case 0xFF43:
                SCX = value;
                break;
            case 0xFF44:
                // LY is not writeable
                break;
            case 0xFF45:
                LYC = value;
                break;
            case 0xFF46:
                DMA = value;
                break;
            case 0xFF47:
                BGP = value;
                break;
            case 0xFF48:
                OBP0 = value;
                break;
            case 0xFF49:
                OBP1 = value;
                break;
            case 0xFF4A:
                WY = value;
                break;
            case 0xFF4B:
                WX = value;
                break;
            default:
                throw std::runtime_error("Unknown LCD register address");
        }
    }

    /* Runs the PPU until it reaches the next "stable" state. Returns the amount of time spent. */
    uint32_t tick() {
        // skip if LCD is turned off
        if (not (LCDC & 1 << 7)) return 1;

        /* set time of current tick to 0. */
        tick_time_ = 0;

        // Determine current mode_
        switch (mode_) {
            case H_Blank: {
                dot();
                if (scanline_time_ == 456) {
                    /* Move to next scanline and switch mode */
                    if (++LY == 144) {
                        set_mode(V_Blank);
                        print_lcd();
                    } else {
                        assert(LY < 144);
                        set_mode(OAM_Scan);
                    }
                    // TODO LY == LYC interrupt
                    scanline_time_ = 0;
                    fifo_pushed_pixels = 0;
                }
                break;
            }
            case V_Blank: {
                dot();
                if (scanline_time_ == 456) {
                    /* Move to next scanline and switch mode if necessary */
                    assert(144 <= LY and LY <= 153);
                    if (++LY == 154) {
                        LY = 0;
                        set_mode(OAM_Scan);
                    }
                    scanline_time_ = 0;
                }
                break;
            }
            case OAM_Scan: {
                /* Scan the OAM RAM. The CPU has no access to the OAM RAM in this mode. */
                if (oam_pointer == OAM_RAM_BEGIN) scanline_sprites.clear();

                dot();
                OAM_entry e {
                    oam_ram_[oam_pointer - OAM_RAM_BEGIN],
                    oam_ram_[oam_pointer - OAM_RAM_BEGIN + 1],
                    oam_ram_[oam_pointer - OAM_RAM_BEGIN + 2],
                    oam_ram_[oam_pointer - OAM_RAM_BEGIN + 3]
                };
                oam_pointer += 4;

                // check if the OAM entry is visible on the current scanline
                if (LY + 16 <= e.y and e.y + 8 + (8 * (bool)(LCDC & 1 << 2)) <= LY + 16) {
                    if (scanline_sprites.size() < 10) scanline_sprites.push_back(e);
                }

                if (oam_pointer == OAM_RAM_END + 1) {
                    assert(scanline_time_ == 80);
                    set_mode(Pixel_Transfer);
                    oam_pointer = OAM_RAM_BEGIN;
                }
                break;
            }
            case Pixel_Transfer: {
                dot();

                // the pixel fetcher is two times slower than the rest of the ppu
                if (scanline_time_ % 2 == 0) fetcher.tick();

                // stop if the pixel fifo is empty or stopped
                if (BG_FIFO.empty() or pixel_fifo_stopped) break;

                // if a sprite is at the current x position, switch the fetcher into sprite fetch mode
                auto it = std::find_if(
                        scanline_sprites.begin(),
                        scanline_sprites.end(),
                        [&](OAM_entry e){ return e.x <= fifo_pushed_pixels; }
                );
                if (it != scanline_sprites.end()) {
                    fetcher.fetch_sprite(*it);
                    scanline_sprites.erase(it);
                }

                // if scanline is at the beginning, skip pixels based on SCX offset
                if (fifo_pushed_pixels == 0) {
                    for (int i = 0; i < SCX % 8; ++i)
                        BG_FIFO.pop();
                }

                Pixel px = BG_FIFO.front();
                BG_FIFO.pop();
                assert((px.color & 0b11) == px.color);

                // merge the BG and sprite pixels if a sprite pixel is available
                if (not Sprite_FIFO.empty()) {
                    Pixel sp = Sprite_FIFO.front();
                    Sprite_FIFO.pop();

                    // check if sprite pixel is not transparent and that sprites are enabled
                    assert((sp.color & 0b11) == sp.color);
                    if (sp.color > 0 and LCDC & 1 << 1) {
                        // check bg priority of sprite
                        px = sp.bg_priority and px.color > 0 ? px : sp;
                    }
                }

                // if BG is disabled, make BG pixels use color ID 0
                if (not (LCDC & 1) and px.pallet == BG)
                    px.color = 0;

                // retrieve color from pallet and push to LCD
                assert(px.pallet == BG or px.color != 0);
                uint8_t c;
                switch (px.pallet) {
                    case BG: {
                        c = (BGP >> (2 * px.color)) & 0b11;
                        break;
                    }
                    case S0: {
                        c = (OBP0 >> (2 * px.color)) & 0b11;
                        break;
                    }
                    case S1: {
                        c = (OBP1 >> (2 * px.color)) & 0b11;
                        break;
                    }
                }
                lcd[LY * 160 + fifo_pushed_pixels] = c;

                // if the last pixel of the scanline was pushed, move on to H-Blank mode
                if (++fifo_pushed_pixels == 160) {
                    fetcher.reset();
                    while (not BG_FIFO.empty()) BG_FIFO.pop();
                    while (not Sprite_FIFO.empty()) Sprite_FIFO.pop();
                    set_mode(H_Blank);
                }
                break;
            }
            default:
                throw std::runtime_error("Unknown PPU mode");
        }

        return tick_time_;
    }

    // prints the current state of the LCD
    void print_lcd() {
        std::array<std::string, 5> pixels{"░", "▒", "▓", "█", "⠀"};

        // print upper frame part
        std::cout << "┏";
        for (int i = 0; i < 160; ++i) std::cout << "━";
        std::cout << "┓\n";

        // print scanlines
        for (int i = 0; i < 144; ++i) {
            std::cout << "┃";
            for (int j = 0; j < 160; ++j) std::cout << pixels[lcd[i * 160 + j]];
            std::cout << "┃\n";
        }

        // print lower frame part
        std::cout << "┗";
        for (int i = 0; i < 160; ++i) std::cout << "━";
        std::cout << "┛\n";
    }
};