#pragma once

#include "CPU.hpp"
#include "Cartridge.hpp"
#include "Memory.hpp"
#include "MemoryStub.hpp"
#include "PPU.hpp"

#include <memory>


/** Stores all components of the emulator and facilitates communication between components. */
class YumeBoy {
    int64_t time_budget = 0;    // the time budget of the CPU/PPU. A positive budget indicates that the PPU is running behind and vice versa.

    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<Cartridge> cartridge_;
    std::unique_ptr<PPU> ppu_;
    std::unique_ptr<MemorySTUB> audio_;
    std::unique_ptr<Memory> hram_;

    public:
    YumeBoy(const std::string& filepath) {
        cpu_ = std::make_unique<CPU>(*this);
        cartridge_ = std::make_unique<Cartridge>(filepath);
        ppu_ = std::make_unique<PPU>();
        audio_ = std::make_unique<MemorySTUB>("Audio", 0xFF10, 0xFF26);
        hram_ = std::make_unique<Memory>(0xFF80, 0xFFFE);
    }

    void tick() {
        if (time_budget <= 0)
            time_budget += cpu_->tick();
        else
            time_budget -= ppu_->tick();
    }

    uint8_t read_memory(uint16_t addr) {
        // Cartridge ROM
        if (addr <= 0x7FFF)
            return cartridge_->read_memory(addr);
        else if (0x8000 <= addr and addr <= 0x9FFF)
            return ppu_->read_vram(addr);
        else if (0xA000 <= addr and addr <= 0xBFFF)
            // TODO: Cartridge RAM
            throw std::runtime_error("Cartridge RAM not implemented");
        else if (0xC000 <= addr and addr <= 0xDFFF)
            // TODO: WRAM
            throw std::runtime_error("WRAM not implemented");
        else if (0xE000 <= addr and addr <= 0xFDFF)
            // TODO: Echo RAM (Nintendo says use of this area is prohibited)
            throw std::runtime_error("Echo RAM not implemented");
        else if (0xFE00 <= addr and addr <= 0xFE9F)
            return ppu_->read_oam_ram(addr);
        else if (0xFEA0 <= addr and addr <= 0xFEFF)
            // TODO: Not Usable (Nintendo says use of this area is prohibited)
            throw std::runtime_error("Not Usable (Nintendo says use of this area is prohibited)");
        else if (0xFF00 <= addr and addr <= 0xFF7F)
            if (addr == 0xFF50)
                return cartridge_->boot_room_enabled();
            else if (addr == 0xFF0F)
                return cpu_->read_IF();
            else if (0xFF10 <= addr and addr <= 0xFF26)
                return audio_->read_memory(addr);
            else if (0xFF40 <= addr and addr <= 0xFF4B)
                return ppu_->read_lcd_register(addr);
            else
                // TODO: I/O Registers
                throw std::runtime_error("I/O Registers not implemented");
        else if (0xFF80 <= addr and addr <= 0xFFFE)
            return hram_->read_memory(addr);
        else if (addr == 0xFFFF)
            return cpu_->read_IE();
        else
            throw std::runtime_error("Memory address out of range.");
    }

    void write_memory(uint16_t addr, uint8_t value) {
        // Cartridge ROM
        if (addr <= 0x7FFF)
            throw std::runtime_error("ROM is not writable");
        else if (0x8000 <= addr and addr <= 0x9FFF)
            ppu_->write_vram(addr, value);
        else if (0xA000 <= addr and addr <= 0xBFFF)
            // TODO: Cartridge RAM
            throw std::runtime_error("Cartridge RAM not implemented");
        else if (0xC000 <= addr and addr <= 0xDFFF)
            // TODO: WRAM
            throw std::runtime_error("WRAM not implemented");
        else if (0xE000 <= addr and addr <= 0xFDFF)
            // TODO: Echo RAM (Nintendo says use of this area is prohibited)
            throw std::runtime_error("Echo RAM not implemented");
        else if (0xFE00 <= addr and addr <= 0xFE9F)
            ppu_->write_oam_ram(addr, value);
        else if (0xFEA0 <= addr and addr <= 0xFEFF)
            // TODO: Not Usable (Nintendo says use of this area is prohibited)
            throw std::runtime_error("Not Usable (Nintendo says use of this area is prohibited)");
        else if (0xFF00 <= addr and addr <= 0xFF7F)
            if (addr == 0xFF50)
                cartridge_->boot_room_enabled(value);
            else if (addr == 0xFF0F)
                cpu_->write_IF(value);
            else if (0xFF10 <= addr and addr <= 0xFF26)
                audio_->write_memory(addr, value);
            else if (0xFF40 <= addr and addr <= 0xFF4B)
                ppu_->write_lcd_register(addr, value);
            else
                // TODO: I/O Registers
                throw std::runtime_error("I/O Registers not implemented");
        else if (0xFF80 <= addr and addr <= 0xFFFE)
            hram_->write_memory(addr, value);
        else if (addr == 0xFFFF)
            cpu_->write_IE(value);
        else
            throw std::runtime_error("Memory address out of range.");
    }

};