#pragma once

#include <cstdint>
#include <string>
#include <savestate/RAMSaveState.hpp>


/* Represents the state of a `MemorySTUB` object. */
struct MemorySTUBSaveState {
    const std::string name_;
    const RAMSaveState base;
};