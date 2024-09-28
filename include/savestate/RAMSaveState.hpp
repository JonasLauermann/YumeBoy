#pragma once

#include <cstdint>
#include <vector>


/* Represents the state of a `RAM` object. */
struct RAMSaveState {
    const std::vector<uint8_t> memory_;
    const uint16_t begin_memory_range_;
    const uint16_t end_memory_range_;
};