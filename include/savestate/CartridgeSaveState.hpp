#pragma once

#include <cstdint>
#include <vector>

/* Represents the state of a `CartridgeSaveState` object. */
struct CartridgeSaveState {
    // rom_bytes_ are unnecessary to save
    const std::vector<uint8_t> ram_bytes_;

    const uint8_t boot_rom_enabled_;

    const uint8_t CARTRIDGE_TYPE;
    const uint8_t ROM_SIZE;
    const uint8_t RAM_SIZE;
    
    // MBC1
    const uint8_t RAM_enabled;
    const uint8_t ROM_bank_number;
    const uint8_t RAM_bank_number;
    
    const bool banking_mode_select;
};