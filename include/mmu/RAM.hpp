#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include "mmu/Memory.hpp"


class RAM : public Memory {
    std::vector<uint8_t> memory_;
    const uint16_t begin_memory_range_;
    const uint16_t end_memory_range_;   // both begin and end are included in range

    public:
    RAM(uint16_t begin_memory_range, uint16_t end_memory_range)
    : begin_memory_range_(begin_memory_range), end_memory_range_(end_memory_range) {
        memory_.resize(end_memory_range - begin_memory_range + 1, 0);
    }

    bool contains_address(uint16_t addr) const override {
        return begin_memory_range_ <= addr and addr <= end_memory_range_;
    }

    uint8_t read_memory(uint16_t addr) override
    {
        assert(begin_memory_range_ <= addr and addr <= end_memory_range_);
        return memory_[addr - begin_memory_range_];
    }

    void write_memory(uint16_t addr, uint8_t value) override
    {
        assert(begin_memory_range_ <= addr and addr <= end_memory_range_);
        memory_[addr - begin_memory_range_] = value;
    }
};