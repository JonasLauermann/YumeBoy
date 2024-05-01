#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include <fstream>


/** Represents the read-only memory_ of game cartridges */
class Cartridge {
    std::vector<uint8_t> rom_bytes_;
    std::vector<uint8_t> ram_bytes_;

    uint8_t boot_rom_enabled = 0x0;

    public:
    Cartridge(std::vector<uint8_t> rom_bytes, std::vector<uint8_t> ram_bytes) : rom_bytes_(std::move(rom_bytes)), ram_bytes_(std::move(ram_bytes)) { }

    virtual uint8_t read_rom(uint16_t addr);
    virtual void write_rom(uint16_t addr, uint8_t value) { };

    virtual uint8_t read_ram(uint16_t addr);
    virtual void write_ram(uint16_t addr, uint8_t value);

    uint8_t boot_room_enabled() { return boot_rom_enabled; }
    void boot_room_enabled(uint8_t value) { boot_rom_enabled = value; }

};

class ROM_ONLY : public Cartridge {
    public:
    ROM_ONLY(std::vector<uint8_t> rom_bytes) : Cartridge(std::move(rom_bytes), {}) { }

    uint8_t read_rom(uint16_t addr) override;

    uint8_t read_ram(uint16_t addr) override { return 0xFF; };
    void write_ram(uint16_t addr, uint8_t value) override { };
};

struct CartridgeFactory {
    static std::unique_ptr<Cartridge> Create(const std::string& filepath) {
        // Open the file in binary mode
        std::ifstream file(filepath, std::ios::binary);

        // Check if the file was opened successfully
        if (!file.is_open()) {
            throw std::runtime_error("Error opening file: " + filepath);
        }

        // TODO support more cartridge types other than ROM ONLY

        // Get the file size
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        assert(fileSize >= 1 << 15);

        // Allocate memory_ for the byte array
        std::vector<uint8_t> rom_bytes;
        rom_bytes.reserve(fileSize);

        // Read the file contents into the byte array
        rom_bytes.insert(rom_bytes.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        // Close the file
        file.close();
        
        // Determine MBC (https://gbdev.io/pandocs/The_Cartridge_Header.html#0147--cartridge-type)
        uint8_t cartridge_type = rom_bytes[0x0147];
        uint8_t rom_size = rom_bytes[0x0148];
        uint8_t ram_size = rom_bytes[0x149];
        assert(rom_size <= 8);
        assert(rom_size <= 5);

        switch (cartridge_type)
        {
        case 0x00:  // ROM ONLY
            assert(rom_bytes.size() == 1 << 15);
            return std::make_unique<ROM_ONLY>(std::move(rom_bytes));
        
        default:
            throw std::runtime_error(std::format("Unknown Cartridge Type {:X}!", cartridge_type));
        }
    }
};