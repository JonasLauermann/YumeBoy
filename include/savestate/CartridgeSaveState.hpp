#pragma once

#include <cstdint>
#include <vector>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

/* Represents the state of a `CartridgeSaveState` object. */
struct CartridgeSaveState {
    // rom_bytes_ are unnecessary to save
    std::vector<uint8_t> ram_bytes_;

    uint8_t boot_rom_enabled_;

    uint8_t CARTRIDGE_TYPE;
    uint8_t ROM_SIZE;
    uint8_t RAM_SIZE;
    
    // MBC1
    uint8_t RAM_enabled;
    uint8_t ROM_bank_number;
    uint8_t RAM_bank_number;
    
    bool banking_mode_select;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & ram_bytes_;

        ar & boot_rom_enabled_;

        ar & CARTRIDGE_TYPE;
        ar & ROM_SIZE;
        ar & RAM_SIZE;

        ar & RAM_enabled;
        ar & ROM_bank_number;
        ar & RAM_bank_number;

        ar & banking_mode_select;
    }
};