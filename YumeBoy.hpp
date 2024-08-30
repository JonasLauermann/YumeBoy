#pragma once

#include "CPU.hpp"
#include "Cartridge.hpp"
#include "Memory.hpp"
#include "MemoryStub.hpp"
#include "PPU.hpp"
#include "Joypad.hpp"

#include <memory>


/** Stores all components of the emulator and facilitates communication between components. */
class YumeBoy {
    int64_t time_budget = 0;    // the time budget of the CPU/PPU. A positive budget indicates that the PPU is running behind and vice versa.

    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<Cartridge> cartridge_;
    std::unique_ptr<PPU> ppu_;
    std::unique_ptr<MemorySTUB> audio_;
    std::unique_ptr<Memory> hram_;
    std::unique_ptr<Memory> wram_;
    std::unique_ptr<MemorySTUB> link_cable_;
    std::unique_ptr<Joypad> joypad_;

    public:
    explicit YumeBoy(const std::string& filepath) {
        cpu_ = std::make_unique<CPU>(*this);
        cartridge_ = CartridgeFactory::Create(filepath);
        ppu_ = std::make_unique<PPU>(*this);
        audio_ = std::make_unique<MemorySTUB>("Audio", 0xFF10, 0xFF26);
        hram_ = std::make_unique<Memory>(0xFF80, 0xFFFE);
        wram_ = std::make_unique<Memory>(0xc000, 0xDFFF);
        link_cable_ = std::make_unique<MemorySTUB>("Serial Data Transfer (Link Cable)", 0xFF01, 0xFF02);
        joypad_ = std::make_unique<Joypad>(*this);
    }

    void tick() {
        if (time_budget <= 0) {
            time_budget += cpu_->tick();
            joypad_->update_joypad_state();
        } else {
            time_budget -= ppu_->tick();
        }
    }

    uint8_t read_memory(uint16_t addr) {
        // Cartridge ROM
        if (addr <= 0x7FFF)
            return cartridge_->read_rom(addr);
        else if (0x8000 <= addr and addr <= 0x9FFF)
            return ppu_->read_vram(addr);
        else if (0xA000 <= addr and addr <= 0xBFFF)
            return cartridge_->read_ram(addr);
        else if (0xC000 <= addr and addr <= 0xDFFF)
            return wram_->read_memory(addr);
        else if (0xE000 <= addr and addr <= 0xFDFF)
            // TODO: Echo RAM (Nintendo says use of this area is prohibited)
            throw std::runtime_error("Echo RAM not implemented");
        else if (0xFE00 <= addr and addr <= 0xFE9F)
            return ppu_->read_oam_ram(addr);
        else if (0xFEA0 <= addr and addr <= 0xFEFF)
            // Not Usable (Nintendo says use of this area is prohibited)
            return 0xFF;
        else if (0xFF00 == addr)
            return joypad_->P1();
        else if (0xFF01 <= addr and addr <= 0xFF02)
            // TODO Serial transfer
            return link_cable_->read_memory(addr);
        else if (addr == 0xFF04)
            return cpu_->timer_divider()->DIV();
        else if (addr == 0xFF05)
            return cpu_->timer_divider()->TIMA();
        else if (addr == 0xFF06)
            return cpu_->timer_divider()->TMA();
        else if (addr == 0xFF07)
            return cpu_->timer_divider()->TAC();
        else if (addr == 0xFF0F)
            return cpu_->IF();
        else if (0xFF10 <= addr and addr <= 0xFF26)
            return audio_->read_memory(addr);
        else if (0xFF40 <= addr and addr <= 0xFF4B)
            return ppu_->read_lcd_register(addr);
        else if (addr == 0xFF50)
            return cartridge_->boot_room_enabled();
        else if (0xFF51 <= addr and addr <= 0xFF7F)
            // Unused
            return 0xFF;
        else if (0xFF80 <= addr and addr <= 0xFFFE)
            return hram_->read_memory(addr);
        else if (addr == 0xFFFF)
            return cpu_->IE();
        else {
            std::cerr << std::format("Address {:#04X} is read from which is undocumented! Returning 0xFF.\n", addr);
            return 0xFF;    // https://www.reddit.com/r/EmuDev/comments/6yi3hh/game_boy_how_do_undocumented_io_registers_behave/
        }
    }

    void write_memory(uint16_t addr, uint8_t value) {
        // Cartridge ROM
        if (addr <= 0x7FFF)
            cartridge_->write_rom(addr, value);
        else if (0x8000 <= addr and addr <= 0x9FFF)
            ppu_->write_vram(addr, value);
        else if (0xA000 <= addr and addr <= 0xBFFF)
            cartridge_->write_ram(addr, value);
        else if (0xC000 <= addr and addr <= 0xDFFF)
            wram_->write_memory(addr, value);
        else if (0xE000 <= addr and addr <= 0xFDFF)
            // TODO: Echo RAM (Nintendo says use of this area is prohibited)
            throw std::runtime_error("Echo RAM not implemented");
        else if (0xFE00 <= addr and addr <= 0xFE9F)
            ppu_->write_oam_ram(addr, value);
        else if (0xFEA0 <= addr and addr <= 0xFEFF)
            // Not Usable (Nintendo says use of this area is prohibited)
            return;
        else if (0xFF00 == addr)
            joypad_->P1(value);
        else if (0xFF01 <= addr and addr <= 0xFF02)
            // TODO Serial transfer
            link_cable_->write_memory(addr, value);
        else if (addr == 0xFF04)
            cpu_->timer_divider()->DIV(value);
        else if (addr == 0xFF05)
            cpu_->timer_divider()->TIMA(value);
        else if (addr == 0xFF06)
            cpu_->timer_divider()->TMA(value);
        else if (addr == 0xFF07)
            cpu_->timer_divider()->TAC(value);
        else if (addr == 0xFF0F)
            cpu_->IF(value);
        else if (0xFF10 <= addr and addr <= 0xFF26)
            audio_->write_memory(addr, value);
        else if (0xFF40 <= addr and addr <= 0xFF4B)
            ppu_->write_lcd_register(addr, value);
        else if (addr == 0xFF50)
            cartridge_->boot_room_enabled(value);
        else if (0xFF51 <= addr and addr <= 0xFF7F)
            // Unused
            return;
        else if (0xFF80 <= addr and addr <= 0xFFFE)
            hram_->write_memory(addr, value);
        else if (addr == 0xFFFF)
            cpu_->IE(value);
        else
            std::cerr << std::format("Address {:#04X} is written to which is undocumented! Ignoring write operation.\n", addr);
    }

    enum class INTERRUPT : uint8_t {
        V_BLANK_INTERRUPT =     1,
        STAT_INTERRUPT =        1 << 1,
        TIMER_INTERRUPT =       1 << 2,
        SERIAL_INTERRUPT =      1 << 3,
        JOYPAD_INTERRUPT =      1 << 4
    };

    void request_interrupt(INTERRUPT intrrupt) {
        cpu_->IF(cpu_->IF() | static_cast<uint8_t>(intrrupt));
    }

    void dump_tilemap() {
        // advance emulation until PPU is no longer in PIXEL_TRANSFER mode
        while ((read_memory(0xFF41) & 0b11) == 3)
            tick();

        const size_t SIZE = 128 * 3 * 64 * 3;  // tiles per block * 3 blocks * num pixels per tile * 3 color channels (RGB)
        std::array<uint8_t, SIZE> image_data;

        std::ofstream tilemap_file;		//output stream object
        tilemap_file.open("tilemap.ppm");

        if (not tilemap_file.is_open())
        {
            std::cout << "Unable to create/open tilemap.ppm" << std::endl;
            return;
        }

        const int width = 16 * 8;
        const int height = 24 * 8;
        static_assert(SIZE == width * height * 3);

        //Image header - Need this to start the image properties
        tilemap_file << "P3" << std::endl;                      //Declare that you want to use ASCII colour values
        tilemap_file << width << " " << height << std::endl;    //Declare w & h
        tilemap_file << "255" << std::endl;                     //Declare max colour ID
        

        // get palette data
        uint8_t palette = read_memory(0xFF48); // OBJ palette 0 data

        int pixel = 0;
        //Image Painter - sets the background and the diagonal line to the array
        for (unsigned int row = 0; row < height; row++) {
            for (unsigned int col = 0; col < width; col += 8) {
                // calculate current tile ID
                uint16_t tile_id = ((uint16_t(row) / 8) * 0x10) + (uint16_t(col) / 8);

                // fetch 2 bytes representing a single row of a single tile
                uint8_t tile_row = row % 8;
                auto tile_data_addr = uint16_t(0x8000 + (tile_id << 4));
                uint8_t lower_byte = read_memory(tile_data_addr + (tile_row * 2));
                uint8_t higher_byte = read_memory(tile_data_addr + (tile_row * 2) + 1);

                // convert to color using palette
                for (int i = 7; i >= 0; --i) {
                    uint8_t tile_color = (((higher_byte << 1) >> i) & 0b10) | ((lower_byte >> i) & 0b1);
                    uint8_t c = (palette >> (2 * tile_color)) & 0b11;
                    uint8_t r, g, b;

                    switch (c)
                    {
                    case 0: // WHITE
                        r = 233;
                        g = 239;
                        b = 236;
                        break;

                    case 1: // LIGHT_GRAY
                        r = 160;
                        g = 160;
                        b = 139;
                        break;

                    case 2: // DARK_GRAY
                        r = 85;
                        g = 85;
                        b = 104;
                        break;

                    case 3:  // BLACK
                        r = 33;
                        g = 30;
                        b = 32;
                        break;

                    default:
                        std::unreachable();
                    }

                    // write row data to array
                    image_data[(pixel + (7 - i)) * 3] = r;
                    image_data[(pixel + (7 - i)) * 3 + 1] = g;
                    image_data[(pixel + (7 - i)) * 3 + 2] = b;
                }
                pixel += 8;
            }
        }
        

        //Image Body - outputs image_data array to the .ppm file, creating the image
        for (int x = 0; x < SIZE; x += 3) {
            int r = image_data[x];		//Sets value as an integer, not a character value
            int g = image_data[x+1];		//Sets value as an integer, not a character value
            int b = image_data[x+2];		//Sets value as an integer, not a character value
            tilemap_file << r << " " << g << " " << b << " " << std::endl;		//Sets 3 bytes of colour to each pixel	
        }

        tilemap_file.close();
    }

};