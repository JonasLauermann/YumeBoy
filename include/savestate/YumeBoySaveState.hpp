#pragma once

#include <cstdint>
#include <memory>
#include <savestate/CPUSaveState.hpp>
#include <savestate/CartridgeSaveState.hpp>
#include <savestate/PPUSaveState.hpp>
#include <savestate/LCDSaveState.hpp>
#include <savestate/MemorySTUBSaveState.hpp>
#include <savestate/RAMSaveState.hpp>
#include <savestate/JoypadSaveState.hpp>
#include <savestate/TimerSaveState.hpp>
#include <savestate/DMASaveState.hpp>

/* Represents the state of a `YumeBoy` object. The goal of these separate state structs is to achieve a tree-like hierachy for easy serialization. */
struct YumeBoySaveState {
    const uint64_t ticks;
    const std::string& filepath;

    // the MMU's state can be reconstructed
    const CPUSaveState cpu_;
    const CartridgeSaveState cartridge_;
    const PPUSaveState ppu_;
    const LCDSaveState lcd_;
    const MemorySTUBSaveState audio_;
    const RAMSaveState hram_;
    const RAMSaveState wram_;
    const MemorySTUBSaveState link_cable_;
    const JoypadSaveState joypad_;
    const TimerSaveState timer_;
    // InterruptBus has no state
    const DMASaveState dma_;
    // the DMA_Memory's state can be reconstructed
};