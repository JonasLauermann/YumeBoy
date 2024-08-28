#pragma once

#include <vector>
#include <cstdint>
#include <cassert>
#include <fstream>

/** Represents the read-only memory_ of game cartridges */
class Cartridge
{
    std::vector<uint8_t> rom_bytes_;
    std::vector<uint8_t> ram_bytes_;

    uint8_t boot_rom_enabled = 0x0;

protected:
    const uint8_t CARTRIDGE_TYPE;
    const uint8_t ROM_SIZE;
    const uint8_t RAM_SIZE;

    uint8_t rom_bytes(uint32_t addr);
    uint8_t ram_bytes(uint32_t addr);
    void ram_bytes(uint32_t addr, uint8_t value);

public:
    virtual ~Cartridge() = default;
    Cartridge(std::vector<uint8_t> rom_bytes, std::vector<uint8_t> ram_bytes, uint8_t carrtidge_type, uint8_t rom_size, uint8_t ram_size) : rom_bytes_(std::move(rom_bytes)), ram_bytes_(std::move(ram_bytes)), CARTRIDGE_TYPE(carrtidge_type), ROM_SIZE(rom_size), RAM_SIZE(ram_size) {}

    virtual uint8_t read_rom(uint16_t addr) = 0;
    virtual void write_rom(uint16_t addr [[maybe_unused]], uint8_t value [[maybe_unused]]) { /* Writing to ROM is not possible by default. */ };

    virtual uint8_t read_ram(uint16_t addr) = 0;
    virtual void write_ram(uint16_t addr, uint8_t value) = 0;

    uint8_t boot_room_enabled() const { return boot_rom_enabled; }
    void boot_room_enabled(uint8_t value) { boot_rom_enabled = value; }
};

class ROM_ONLY : public Cartridge
{
public:
    explicit ROM_ONLY(std::vector<uint8_t> rom_bytes) : Cartridge(std::move(rom_bytes), {}, 0x00, 0x00, 0x00) {}

    uint8_t read_rom(uint16_t addr) override;

    uint8_t read_ram(uint16_t addr [[maybe_unused]]) override { return 0xFF; };
    void write_ram(uint16_t addr [[maybe_unused]], uint8_t value [[maybe_unused]]) override { /* As the name suggests ROM_ONLY does not have any RAM to write to. */ };
};

template <bool BATTERY>
class MBC1 : public Cartridge
{
    // Registers
    uint8_t RAM_enabled = 0x00;
    uint8_t ROM_bank_number = 0x01;   // 5-bit register, range: 0x01-0x1F (0x00 is treated as 0x01)
    uint8_t RAM_bank_number = 0x00;   // 2-bit register, range: 0x00-0x03 (can be used for additional ROM banks instead of RAM -- https://gbdev.io/pandocs/MBC1.html#40005fff--ram-bank-number--or--upper-bits-of-rom-bank-number-write-only)
    bool banking_mode_select = false; // if true, uses the 2-bit register for ROM banking

    uint32_t translate_ram_addr(uint16_t addr) const
    {
        assert(0xA000 <= addr and addr <= 0xBFFF);

        if ((RAM_enabled & 0xF) != 0xA)
            return 0xFF;

        // address translation based on https://gbdev.io/pandocs/MBC1.html#a000bfff
        uint32_t phy_addr = addr & 0x1FFF;

        if (banking_mode_select)
            phy_addr |= (RAM_bank_number & 0b11) << 13;

        phy_addr -= 0xA000;
        return phy_addr;
    };

public:
    MBC1(std::vector<uint8_t> rom_bytes, std::vector<uint8_t> ram_bytes, uint8_t carrtidge_type, uint8_t rom_size, uint8_t ram_size) : Cartridge(std::move(rom_bytes), std::move(ram_bytes), carrtidge_type, rom_size, ram_size) {};
    MBC1(std::vector<uint8_t> rom_bytes, uint8_t carrtidge_type, uint8_t rom_size) : Cartridge(std::move(rom_bytes), {}, carrtidge_type, rom_size, 0x00) {};

    uint8_t read_rom(uint16_t addr) override
    {
        // TODO: https://gbdev.io/pandocs/MBC1.html#mbc1m-1-mib-multi-game-compilation-carts
        assert(addr < 0xA000);

        // address translation based on https://gbdev.io/pandocs/MBC1.html#addressing-diagrams
        uint32_t phy_addr = addr & 0x3FFF;

        if ((0x4000 <= addr) or banking_mode_select)
            phy_addr |= (RAM_bank_number & 0b11) << 19;

        if (0x4000 <= addr)
            phy_addr |= (ROM_bank_number & 0b11111) << 14;

        // only consider the relevant bits and ignore upper bits (TODO: could be done using constexprs)
        phy_addr %= 1 << (15 + ROM_SIZE);

        return Cartridge::rom_bytes(phy_addr);
    };
    void write_rom(uint16_t addr, uint8_t value) override
    {
        assert(addr < 0xA000);

        if (addr <= 0x1FFF)
            RAM_enabled = value;
        else if (0x2000 <= addr and addr <= 0x3FFF)
            ROM_bank_number = std::max(uint8_t(value & 0b11111), uint8_t(1));  // ROM Banking Number: 0x00 is treated as 0x01
        else if (0x4000 <= addr and addr <= 0x5FFF)
            RAM_bank_number = value & 0b11;
        else if (0x6000 <= addr and addr <= 0x7FFF)
            banking_mode_select = value & 0b1;
    };

    // TODO: save RAM if BATTERY is available
    uint8_t read_ram(uint16_t addr) override { return Cartridge::ram_bytes(translate_ram_addr(addr)); };
    void write_ram(uint16_t addr, uint8_t value) override { Cartridge::ram_bytes(translate_ram_addr(addr), value); };
};

struct CartridgeFactory
{
    static std::unique_ptr<Cartridge> Create(const std::string &filepath)
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

        switch (cartridge_type)
        {
        case 0x00: // ROM ONLY
            assert(rom_bytes.size() == 1 << 15);
            assert(rom_size == 0x00);
            assert(ram_size == 0x00);
            return std::make_unique<ROM_ONLY>(std::move(rom_bytes));

        case 0x01: // MBC1
            assert(rom_bytes.size() == 1ULL << (15 + rom_size));
            assert(rom_size < 0x07);
            assert(ram_size == 0x00);
            return std::make_unique<MBC1<false>>(std::move(rom_bytes), cartridge_type, rom_size);

        default:
            throw std::runtime_error(std::format("Unknown Cartridge Type {:X}!", cartridge_type));
        }
    }
};