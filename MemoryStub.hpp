#pragma once

#include <cstdint>
#include <iostream>


/** A Memory STUB used as placeholder for missing memory_ components. */
class MemorySTUB {
    std::vector<uint8_t> memory_;
    const uint16_t begin_memory_range_;
    const uint16_t end_memory_range_;   // both begin and end are included in range
    std::string name_; // name of the component not yet implemented

    public:
    MemorySTUB(std::string name, uint16_t begin_memory_range, uint16_t end_memory_range)
    : begin_memory_range_(begin_memory_range), end_memory_range_(end_memory_range), name_(name) {
        memory_.resize(end_memory_range - begin_memory_range + 1, 0);
    }

    uint8_t read_memory(uint16_t addr)
    {
        std::cerr << std::format("Address {:#06X}  is read from {} which is not implemented and uses a STUB!\n", addr, name_);
        assert(begin_memory_range_ <= addr and addr <= end_memory_range_);
        return memory_[addr - begin_memory_range_];
    }

    void write_memory(uint16_t addr, uint8_t value)
    {
        std::cerr << std::format("Value {:#04X} is written to address {:#06X} in {} which is not implemented and uses a STUB!\n", value, addr, name_);
        assert(begin_memory_range_ <= addr and addr <= end_memory_range_);
        memory_[addr - begin_memory_range_] = value;
    }
};