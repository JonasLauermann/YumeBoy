#pragma once

#include <cstdint>
#include <cpu/states.hpp>
#include <savestate/InstructionSaveState.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

/* Represents the state of a `CPU` object. */
struct CPUSaveState {
    // mem_ is reconstructed

    CPU_STATES state;
    InstructionSaveState instruction;

    uint8_t A;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;

    uint16_t SP;
    uint16_t PC;

    uint8_t F;

    bool IME;
    bool EI_executed;
    bool set_IME;

    uint8_t IF_;
    uint8_t IE_;

    bool HALT_bug;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & state;
        

        ar & A;
        ar & B;
        ar & C;
        ar & D;
        ar & E;
        ar & H;
        ar & L;

        ar & SP;
        ar & PC;

        ar & F;

        ar & IME;
        ar & EI_executed;
        ar & set_IME;

        ar & IF_;
        ar & IE_;

        ar & HALT_bug;
    }
};