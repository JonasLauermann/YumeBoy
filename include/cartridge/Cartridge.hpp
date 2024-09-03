#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include <fstream>
#include "mmu/Memory.hpp"

/** Represents the read-only memory_ of game cartridges */
class Cartridge : public Memory
{
    std::vector<uint8_t> rom_bytes_;
    std::vector<uint8_t> ram_bytes_;

    uint8_t boot_rom_enabled_ = 0x0;

protected:
    const uint8_t CARTRIDGE_TYPE;
    const uint8_t ROM_SIZE;
    const uint8_t RAM_SIZE;

    uint8_t rom_bytes(uint32_t addr);
    uint8_t ram_bytes(uint32_t addr);
    void ram_bytes(uint32_t addr, uint8_t value);

    virtual uint8_t read_rom(uint16_t addr) = 0;
    virtual void write_rom(uint16_t addr [[maybe_unused]], uint8_t value [[maybe_unused]]) { /* Writing to ROM is not possible by default. */ };

    virtual uint8_t read_ram(uint16_t addr) = 0;
    virtual void write_ram(uint16_t addr, uint8_t value) = 0;

    uint8_t boot_rom_enabled() const { return boot_rom_enabled_; }
    void boot_rom_enabled(uint8_t value) { boot_rom_enabled_ = value; }

public:
    Cartridge(std::vector<uint8_t> rom_bytes, std::vector<uint8_t> ram_bytes, uint8_t carrtidge_type, uint8_t rom_size, uint8_t ram_size) : rom_bytes_(std::move(rom_bytes)), ram_bytes_(std::move(ram_bytes)), CARTRIDGE_TYPE(carrtidge_type), ROM_SIZE(rom_size), RAM_SIZE(ram_size) {}

    bool contains_address(uint16_t addr) const override {
        return (addr <= 0x7FFF) or (0xA000 <= addr and addr <= 0xBFFF) or (addr == 0xFF50);
    }

    uint8_t read_memory(uint16_t addr) override {
        if (addr <= 0x7FFF)
            return read_rom(addr);
        else if (0xA000 <= addr and addr <= 0xBFFF)
            return read_ram(addr);
        else if (addr == 0xFF50)
            return boot_rom_enabled();
        else
            std::unreachable();
    }

    void write_memory(uint16_t addr, uint8_t value) override {
        if (addr <= 0x7FFF)
            write_rom(addr, value);
        else if (0xA000 <= addr and addr <= 0xBFFF)
            write_ram(addr, value);
        else if (addr == 0xFF50)
            boot_rom_enabled(value);
        else
            std::unreachable();
    }
};

struct CartridgeFactory
{
    static std::unique_ptr<Cartridge> Create(const std::string &filepath, bool skip_bootrom);
};