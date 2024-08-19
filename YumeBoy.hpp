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
    std::unique_ptr<Memory> wram_;
    std::unique_ptr<MemorySTUB> link_cable_;

    public:
    YumeBoy(const std::string& filepath) {
        cpu_ = std::make_unique<CPU>(*this);
        cartridge_ = CartridgeFactory::Create(filepath);
        ppu_ = std::make_unique<PPU>(*this);
        audio_ = std::make_unique<MemorySTUB>("Audio", 0xFF10, 0xFF26);
        hram_ = std::make_unique<Memory>(0xFF80, 0xFFFE);
        wram_ = std::make_unique<Memory>(0xc000, 0xDFFF);
        link_cable_ = std::make_unique<MemorySTUB>("Serial Data Transfer (Link Cable)", 0xFF01, 0xFF02);
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
            return cartridge_->read_rom(addr);
        else if (0x8000 <= addr and addr <= 0x9FFF)
            return ppu_->read_vram(addr);
        else if (0xA000 <= addr and addr <= 0xBFFF)
            // TODO: Cartridge RAM
            throw std::runtime_error("Cartridge RAM not implemented");
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
        else if (0xFF01 <= addr and addr <= 0xFF02)
            // TODO Serial transfer
            return link_cable_->read_memory(addr);
        else if (addr == 0xFF04)
            return cpu_->timer_divider.DIV();
        else if (addr == 0xFF05)
            return cpu_->timer_divider.TIMA();
        else if (addr == 0xFF06)
            return cpu_->timer_divider.TMA();
        else if (addr == 0xFF07)
            return cpu_->timer_divider.TAC();
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
        else
            throw std::runtime_error("Memory address out of range.");
    }

    void write_memory(uint16_t addr, uint8_t value) {
        // Cartridge ROM
        if (addr <= 0x7FFF)
            cartridge_->write_rom(addr, value);
        else if (0x8000 <= addr and addr <= 0x9FFF)
            ppu_->write_vram(addr, value);
        else if (0xA000 <= addr and addr <= 0xBFFF)
            // TODO: Cartridge RAM
            throw std::runtime_error("Cartridge RAM not implemented");
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
        else if (0xFF01 <= addr and addr <= 0xFF02)
            // TODO Serial transfer
            link_cable_->write_memory(addr, value);
        else if (addr == 0xFF04)
            cpu_->timer_divider.DIV(value);
        else if (addr == 0xFF05)
            cpu_->timer_divider.TIMA(value);
        else if (addr == 0xFF06)
            cpu_->timer_divider.TMA(value);
        else if (addr == 0xFF07)
            cpu_->timer_divider.TAC(value);
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
            throw std::runtime_error("Memory address out of range.");
    }

    enum INTERRUPT : uint8_t {
        V_BLANK_INTERRUPT =     1,
        STAT_INTERRUPT =        1 << 1,
        TIMER_INTERRUPT =       1 << 2,
        SERIAL_INTERRUPT =      1 << 3,
        JOYPAD_INTERRUPT =      1 << 4
    };

    void request_interrupt(INTERRUPT intrrupt) {
        cpu_->IF(cpu_->IF() | intrrupt);
    }

};