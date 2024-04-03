#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include <fstream>


/** Represents the read-only memory_ of game cartridges */
class Cartridge {
    std::vector<uint8_t> rom_bytes_;

    uint8_t boot_rom_enabled = 0x0;

    public:
    Cartridge(const std::string& filepath) {
        // Open the file in binary mode
        std::ifstream file(filepath, std::ios::binary);

        // Check if the file was opened successfully
        if (!file.is_open()) {
            throw std::runtime_error("Error opening file: " + filepath);
        }

        // TODO support more cartridge types other than ROM ONLY (https://gbdev.io/pandocs/The_Cartridge_Header.html#0147--cartridge-type)
        // Get the file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        assert(fileSize == 1 << 15);

        // Allocate memory_ for the byte array
        rom_bytes_.reserve(fileSize);

        // Read the file contents into the byte array
        rom_bytes_.insert(rom_bytes_.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        // TODO support more cartridge types other than ROM ONLY (https://gbdev.io/pandocs/The_Cartridge_Header.html#0147--cartridge-type)
        assert(rom_bytes_.size() == 1 << 15);
        assert(rom_bytes_[0x0147] == 0x00);

        // Close the file
        file.close();
    }

    uint8_t read_memory(uint16_t addr);

    uint8_t boot_room_enabled() { return boot_rom_enabled;}
    void boot_room_enabled(uint8_t value) { boot_rom_enabled = value;}


};