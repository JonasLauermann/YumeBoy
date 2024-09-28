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

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/unique_ptr.hpp>

/* Represents the state of a `YumeBoy` object. The goal of these separate state structs is to achieve a tree-like hierachy for easy serialization. */
struct YumeBoySaveState {
    uint64_t ticks;
    std::string filepath;

    // the MMU's state can be reconstructed
    CPUSaveState cpu_;
    CartridgeSaveState cartridge_;
    PPUSaveState ppu_;
    LCDSaveState lcd_;
    MemorySTUBSaveState audio_;
    RAMSaveState hram_;
    RAMSaveState wram_;
    MemorySTUBSaveState link_cable_;
    JoypadSaveState joypad_;
    TimerSaveState timer_;
    // InterruptBus has no state
    DMASaveState dma_;
    // the DMA_Memory's state can be reconstructed

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & ticks;
        ar & filepath;

        ar & cpu_;
        ar & cartridge_;
        ar & ppu_;
        ar & lcd_;
        ar & audio_;
        ar & hram_;
        ar & wram_;
        ar & link_cable_;
        ar & joypad_;
        ar & timer_;
        ar & dma_;
    }
};