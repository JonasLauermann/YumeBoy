#pragma once

#include <cstdint>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


/* Represents the state of a `DMA` object. */
struct DMASaveState {
    uint8_t DMA_;

    bool dma_pending;
    bool dma_running;

    uint8_t next_byte_addr;
    uint8_t last_byte;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & DMA_;

        ar & dma_pending;
        ar & dma_running;

        ar & next_byte_addr;
        ar & last_byte;
    }
};
