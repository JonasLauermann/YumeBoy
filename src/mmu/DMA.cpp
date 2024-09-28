#include <mmu/DMA.hpp>
#include <savestate/DMASaveState.hpp>

void DMA::tick()
{
    if (dma_pending) {  // DMA is delayed by one M-cycle
        dma_pending = false;
        dma_running = true;
        return;
    }

    if (not dma_running) return;

    last_byte = mmu.read_memory(uint16_t((DMA_ << 8) | next_byte_addr));
    mmu.write_memory(0xFE00 | next_byte_addr, last_byte);
    next_byte_addr++;

    if (next_byte_addr > 0x9F) {
        dma_running = false;
    }
}
DMASaveState DMA::save_state() const
{
    DMASaveState s = {
        DMA_,

        dma_pending,
        dma_running,

        next_byte_addr,
        last_byte,
    };
    return s;
}

void DMA::load_state(DMASaveState state)
{
    DMA_ = state.DMA_;

    dma_pending = state.dma_pending;
    dma_running = state.dma_running;

    next_byte_addr = state.next_byte_addr;
    last_byte = state.last_byte;
}