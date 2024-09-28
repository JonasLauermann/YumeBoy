#include "cartridge/Cartridge.hpp"

#include <array>
#include <cartridge/RomOnly.hpp>
#include <cartridge/MBC1.hpp>
#include <iostream>


constexpr std::array<uint8_t, 256> boot_rom = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E, 
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0, 
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B, 
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9, 
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04, 
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2, 
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20, 
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17, 
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C, 
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20, 
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

uint8_t Cartridge::rom_bytes(uint32_t addr)
{
    assert(addr <= rom_bytes_.size());
    if (boot_rom_enabled_ == 0 and addr <= 0xFF)
        return boot_rom[addr];
    return rom_bytes_[addr];
}

uint8_t Cartridge::ram_bytes(uint32_t addr)
{
    assert(addr <= ram_bytes_.size());
    return ram_bytes_[addr];
}

void Cartridge::ram_bytes(uint32_t addr, uint8_t value)
{
    assert(addr <= ram_bytes_.size());
    ram_bytes_[addr] = value; 
}

CartridgeSaveState Cartridge::save_state()
{
    return {
        ram_bytes_,

        boot_rom_enabled_,

        CARTRIDGE_TYPE,
        ROM_SIZE,
        RAM_SIZE
    };
}

void Cartridge::load_state(CartridgeSaveState state)
{
    ram_bytes_ = state.ram_bytes_;
    boot_rom_enabled_ = state.boot_rom_enabled_;

    assert(CARTRIDGE_TYPE == state.CARTRIDGE_TYPE);
    assert(ROM_SIZE == state.ROM_SIZE);
    assert(RAM_SIZE == state.RAM_SIZE);
}

/*==============================================================================================================*/
/* CartridgeFactory                                                                                             */
/*==============================================================================================================*/

std::unique_ptr<Cartridge> CartridgeFactory::Create(const std::string &filepath, bool skip_bootrom)
{
    // Open the file in binary mode
    std::ifstream file(filepath, std::ios::binary);

    // Check if the file was opened successfully
    if (!file.is_open())
    {
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
    assert(ram_size <= 5);
    assert(fileSize == 1 << (15 + rom_size));
    
        
    // Allocate memory_ for the RAM byte array
    uint8_t shift = 0;
    switch (ram_size)
    {
    case 0x04:
        shift += 1;
    case 0x05:
        shift += 1;
    case 0x03:
        shift += 2;
    case 0x02:
        shift += 13;
    default:
        break;
    }
    std::vector<uint8_t> ram_bytes(1 << shift, 0x00);

    std::unique_ptr<Cartridge> cartridge;
    switch (cartridge_type)
    {
    case 0x00: { // ROM ONLY
        assert(rom_bytes.size() == 1 << 15);
        assert(rom_size == 0x00);
        assert(ram_size == 0x00);
        cartridge = std::make_unique<ROM_ONLY>(std::move(rom_bytes));
        break;
    }

    case 0x01: { // MBC1
        assert(rom_bytes.size() == 1ULL << (15 + rom_size));
        assert(rom_size < 0x07);
        assert(ram_size == 0x00);
        cartridge = std::make_unique<MBC1<false>>(std::move(rom_bytes), cartridge_type, rom_size);
        break;
    }

    case 0x02: { // MBC1 + RAM
        assert(rom_bytes.size() == 1ULL << (15 + rom_size));
        assert((rom_size < 0x05 and ram_size < 0x04) or (rom_size < 0x07 and ram_size < 0x03));

        cartridge = std::make_unique<MBC1<false>>(std::move(rom_bytes), std::move(ram_bytes), cartridge_type, rom_size, ram_size);
        break;
    }

    default:
        std::cerr << std::format("Unknown Cartridge Type {:X}!", cartridge_type) << std::endl;
        throw std::invalid_argument(std::format("Unknown Cartridge Type {:X}!", cartridge_type));
    }

    if (skip_bootrom)
        cartridge->write_memory(0xFF50, 0x01);  // disable bootrom

    return cartridge;
}