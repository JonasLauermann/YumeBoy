#pragma once

#include <mmu/Memory.hpp>
#include <iostream>
#include <format>


class MMU {
    std::vector<Memory *> memory_;

    public:
    virtual ~MMU() = default;
    MMU() = default;

    void add(Memory *memory)
    {
        memory_.push_back(memory);
    }

    virtual uint8_t read_memory(uint16_t addr)
    {
        auto it = std::ranges::find_if(memory_, [addr](Memory *m) { return m->contains_address(addr); });
        if(it != memory_.end())
            return (*it)->read_memory(addr);
        std::cerr << std::format("Address {:#04X} is read from which is undocumented! Returning 0xFF.\n", addr);
        return 0xFF;
    }

    virtual void write_memory(uint16_t addr, uint8_t value)
    {
        auto it = std::ranges::find_if(memory_, [addr](Memory *m) { return m->contains_address(addr); });
        if(it != memory_.end())
            (*it)->write_memory(addr, value);
        else
            std::cerr << std::format("Address {:#04X} is written to which is undocumented! Ignoring write operation.\n", addr);
    }
};