#pragma once

#include <cstdint>


/* Represents the state of a `DMA` object. */
struct DMASaveState {
    const uint8_t DMA_;

    const bool dma_pending;
    const bool dma_running;

    const uint8_t next_byte_addr;
    const uint8_t last_byte;
};
