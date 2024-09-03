#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include "mmu/RAM.hpp"


/** A Memory STUB used as placeholder for missing memory_ components. */
class MemorySTUB : public RAM {
    std::string name_; // name of the component not yet implemented

    public:
    MemorySTUB(std::string const& name, uint16_t begin_memory_range, uint16_t end_memory_range)
    : RAM(begin_memory_range, end_memory_range), name_(name) { }

    uint8_t read_memory(uint16_t addr) override
    {
        std::cerr << std::format("Address {:#06X}  is read from {} which is not implemented and uses a STUB!\n", addr, name_);
        return RAM::read_memory(addr);
    }

    void write_memory(uint16_t addr, uint8_t value) override
    {
        std::cerr << std::format("Value {:#04X} is written to address {:#06X} in {} which is not implemented and uses a STUB!\n", value, addr, name_);
        RAM::write_memory(addr, value);
    }
};