#pragma once

#include <cstdint>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


/* Represents the state of an `Instruction` object. It saves all information to construct a new instruction during the load operation that is in the same state as the saved Instruction. It does not matter if it is a `MultiCycleInstruction` or not`. */
struct InstructionSaveState {
    uint8_t opcode;
    bool extended;

    // MultiCycleInstruction
    uint8_t cycle;
    uint8_t temp_u8;
    uint16_t temp_u16;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & opcode;
        ar & extended;

        ar & cycle;
        ar & temp_u8;
        ar & temp_u16;
    }
};