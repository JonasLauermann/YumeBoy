#pragma once

#include <cstdint>
#include <string>
#include <savestate/RAMSaveState.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


/* Represents the state of a `MemorySTUB` object. */
struct MemorySTUBSaveState {
    std::string name_;
    RAMSaveState base;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & name_;
        ar & base;
    }
};