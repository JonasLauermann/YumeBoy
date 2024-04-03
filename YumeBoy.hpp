#pragma once

#include "CPU.hpp"
#include "Cartridge.hpp"
#include "Memory.hpp"
#include "MemoryStub.hpp"

#include <memory>


/** Stores all components of the emulator and facilitates communication between components. */
class YumeBoy {
    std::unique_ptr<CPU> cpu_;
    std::unique_ptr<Cartridge> cartridge_;
    std::unique_ptr<MemorySTUB> vram_;
    std::unique_ptr<MemorySTUB> audio_;
    std::unique_ptr<MemorySTUB> lcd_;
    std::unique_ptr<Memory> hram_;

    public:
    YumeBoy(const std::string& filepath) {
        cpu_ = std::make_unique<CPU>(*this);
        cartridge_ = std::make_unique<Cartridge>(filepath);
        vram_ = std::make_unique<MemorySTUB>("VRAM", 0x8000, 0x9FFF);
        audio_ = std::make_unique<MemorySTUB>("Audio", 0xFF10, 0xFF26);
        lcd_ = std::make_unique<MemorySTUB>("LCD Display", 0xFF40, 0xFF4B);
        hram_ = std::make_unique<Memory>(0xFF80, 0xFFFE);
    }

    void start() { cpu_->start_loop(); }

    uint8_t read_memory(uint16_t addr) {
        // Cartridge ROM
        if (addr <= 0x7FFF)
            return cartridge_->read_memory(addr);
        else if (0x8000 <= addr and addr <= 0x9FFF)
            // TODO: VRAM
            return vram_->read_memory(addr);
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
            // TODO: OAM
            throw std::runtime_error("OAM not implemented");
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
                return lcd_->read_memory(addr);
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
            // TODO: VRAM
            vram_->write_memory(addr, value);
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
            // TODO: OAM
            throw std::runtime_error("OAM not implemented");
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
                lcd_->write_memory(addr, value);
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