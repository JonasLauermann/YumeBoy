#pragma once

#include <cstdint>


class Memory {
    std::vector<uint8_t> memory_;
    const uint16_t begin_memory_range_, end_memory_range_;   // both begin and end are included in range

    public:
    Memory(uint16_t begin_memory_range, uint16_t end_memory_range)
    : begin_memory_range_(begin_memory_range), end_memory_range_(end_memory_range) {
        memory_.resize(end_memory_range - begin_memory_range + 1, 0);
    }

    uint8_t read_memory(uint16_t addr)
    {
        assert(begin_memory_range_ <= addr and addr <= end_memory_range_);
        return memory_[addr - begin_memory_range_];
    }

    void write_memory(uint16_t addr, uint8_t value)
    {
        assert(begin_memory_range_ <= addr and addr <= end_memory_range_);
        memory_[addr - begin_memory_range_] = value;
    }
};