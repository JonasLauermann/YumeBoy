#pragma once

#include <cstdint>
#include <vector>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>


/* Represents the state of a `RAM` object. */
struct RAMSaveState {
    std::vector<uint8_t> memory_;
    uint16_t begin_memory_range_;
    uint16_t end_memory_range_;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & memory_;
        ar & begin_memory_range_;
        ar & end_memory_range_;
    }
};