#pragma once

#include <cassert>
#include <cstdint>
#include <vector>


class Memory {
    
    public:
    Memory() = default;
    virtual ~Memory() = default;;

    virtual bool contains_address(uint16_t addr) const = 0;

    virtual uint8_t read_memory(uint16_t addr) = 0;

    virtual void write_memory(uint16_t addr, uint8_t value) = 0;
};