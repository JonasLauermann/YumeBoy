#pragma once

#include "cartridge/Cartridge.hpp"


class ROM_ONLY : public Cartridge
{
    uint8_t read_rom(uint16_t addr) override {
        assert(addr <= 0x7FFF);
        return Cartridge::rom_bytes(addr);
    }

    uint8_t read_ram(uint16_t addr [[maybe_unused]]) override { return 0xFF; };
    void write_ram(uint16_t addr [[maybe_unused]], uint8_t value [[maybe_unused]]) override { /* As the name suggests ROM_ONLY does not have any RAM to write to. */ };

public:
    explicit ROM_ONLY(std::vector<uint8_t> rom_bytes) : Cartridge(std::move(rom_bytes), {}, 0x00, 0x00, 0x00) {}
};