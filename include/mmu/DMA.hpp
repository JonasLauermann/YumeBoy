#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include "mmu/Memory.hpp"
#include "mmu/MMU.hpp"


class DMA_Memory;

/* Implements the OAM Transfer via Direct Memory Access.
   https://hacktix.github.io/GBEDG/dma/#oam-dma
   https://gbdev.io/pandocs/OAM_DMA_Transfer.html */
class DMA : public Memory {
    friend DMA_Memory;
    MMU &mmu;

    /** 0xFF46 — DMA: OAM DMA source address & start.
     * Writing to this register starts a DMA transfer from ROM or RAM to OAM (Object Attribute Memory). */
    uint8_t DMA_ = 0x0;

    bool dma_pending = false;
    bool dma_running = false;

    uint8_t next_byte_addr = 0x00;
    uint8_t last_byte = 0x00;

    public:
    explicit DMA(MMU &mmu) : mmu(mmu) { }

    /* performs an M-cycle. */
    void tick();

    bool contains_address(uint16_t addr) const override {
        return addr == 0xFF46;
    }

    uint8_t read_memory(uint16_t addr) override {
        assert(addr = 0xFF46);
        return DMA_;
    }

    void write_memory(uint16_t addr, uint8_t value) override {
        assert(addr = 0xFF46);
        DMA_ = value;
        dma_pending = true;
        next_byte_addr = 0x00;
    }
};


/* Implements a "decorator" for the MMU that handels the Memory Bus' behaviour during a OAM Direct Memory Access Transfer. (Not a "real" decorator since I didn't follow the decorator pattern) */
class DMA_Memory : public MMU {
    MMU &mmu;
    DMA &dma;

    public:
    DMA_Memory(MMU &mmu, DMA &dma) : mmu(mmu), dma(dma) { }

    uint8_t read_memory(uint16_t addr) override
    {
        if (dma.dma_running and (addr < 0xFF80)) return dma.last_byte;

        return mmu.read_memory(addr);
    }

    void write_memory(uint16_t addr, uint8_t value) override
    {
        if ((not dma.dma_running) or (addr >= 0xFF80))
            mmu.write_memory(addr, value);
    }
};