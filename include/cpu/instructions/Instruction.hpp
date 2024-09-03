#pragma once

#include <cstdint>
#include <mmu/MMU.hpp>

class YumeBoy;
class CPU;

/* Represents a CPU instruction, should be inhertited to implement explicit instructions. */
class Instruction {
    MMU &mem_;
    CPU &cpu_;

    protected:

    CPU & cpu() { return cpu_; }
    MMU & mem() { return mem_; }

    //=================================================================================================//
    //  HELPER FUNCTIONS                                                                               //
    //=================================================================================================//

    /* INC R8 - Increment the contents of register R8 by 1. */
    bool INC_R8(uint8_t &R);

    /* DEC R8 - Decrement the contents of register R8 by 1. */
    bool DEC_R8(uint8_t &R);

    /* LD R8, R8 - Load the contents of register R1 into register R0. */
    bool LD_R8_R8(uint8_t &R0, const uint8_t &R1) const;

    /* ADD A, R8 - Add the contents of register R to the contents of register A, and store the results in register A. */
    bool ADD(const uint8_t &R);

    /* ADC A, R8 - Add the contents of register R and the CY flag to the contents of register A, and store the results in register A. */
    bool ADC(const uint8_t &R);

    /* SUB A, R8 - Subtract the contents of register R from the contents of register A, and store the results in register A. */
    bool SUB(const uint8_t &R);

    /* SBC A, R8 - Subtract the contents of register R and the CY flag from the contents of register A, and store the results in register A. */
    bool SBC(const uint8_t &R);

    /* AND A, R8 - Take the logical AND for each bit of the contents of register R and the contents of register A, and store the results in register A. */
    bool AND(const uint8_t &R);

    /* XOR A, R8 - Take the logical exclusive-OR for each bit of the contents of register R and the contents of register A, and store the results in register A. */
    bool XOR(const uint8_t &R);

    /* OR A, R8 - Take the logical OR for each bit of the contents of register R and the contents of register A, and store the results in register A. */
    bool OR(const uint8_t &R);

    /* CP A, R8 - Compare the contents of register R and the contents of register A by calculating A - R, and set the Z flag if they are equal.
    The execution of this instruction does not affect the contents of register A. */
    bool CP(const uint8_t &R);

    /* RLC - Rotate the contents of register R8 to the left. That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy operation) are copied to bit 2. The same operation is repeated in sequence for the rest of the register. The contents of bit 7 are placed in both the CY flag and bit 0 of register R8. */
    bool RLC(uint8_t &R);

    /* RRC - Rotate the contents of register R8 to the right. That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5. The same operation is repeated in sequence for the rest of the register. The contents of bit 0 are placed in both the CY flag and bit 7 of register R8. */
    bool RRC(uint8_t &R);

    /* RL - Rotate the contents of register R8 to the left, through the carry (CY) flag. That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy operation) are copied to bit 2. The same operation is repeated in sequence for the rest of the register. The previous contents of the carry flag are copied to bit 0. */
    bool RL(uint8_t &R);

    /* RR - Rotate the contents of register R8 to the right, through the carry (CY) flag. That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy) are copied to bit 5. The same operation is repeated in sequence for the rest of the register. The previous contents of the carry flag are copied to bit 7. */
    bool RR(uint8_t &R);

    /* SLA - Shift the contents of register R to the left. That is, the contents of bit 0 are copied to bit 1, and the previous contents of bit 1 (before the copy operation) are copied to bit 2. The same operation is repeated in sequence for the rest of the register. The contents of bit 7 are copied to the CY flag, and bit 0 of register R is reset to 0. */
    bool SLA(uint8_t &R);

    /* SRA - Shift the contents of register R to the right. That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5. The same operation is repeated in sequence for the rest of the register. The contents of bit 0 are copied to the CY flag, and bit 7 of register R is unchanged. */
    bool SRA(uint8_t &R);

    /* SWAP - Shift the contents of the lower-order four bits (0-3) of register R to the higher-order four bits (4-7) of the register, and shift the higher-order four bits to the lower-order four bits. */
    bool SWAP(uint8_t &R);

    /* SRL - Shift the contents of register R to the right. That is, the contents of bit 7 are copied to bit 6, and the previous contents of bit 6 (before the copy operation) are copied to bit 5. The same operation is repeated in sequence for the rest of the register. The contents of bit 0 are copied to the CY flag, and bit 7 of register R is reset to 0. */
    bool SRL(uint8_t &R);

    /* BIT - Copy the complement of the contents of bit 0 in register R to the Z flag of the program status word (PSW). */
    bool BIT(uint8_t bit, const uint8_t &R);

    /* RES - Reset specified bit in register R to 0. */
    bool RES(uint8_t bit, uint8_t &R) const;

    /* SET - Set specified bit in register R to 1. */
    bool SET(uint8_t bit, uint8_t &R) const;

    public:
    virtual ~Instruction() = default;
    Instruction(CPU &cpu, MMU &mem) : mem_(mem), cpu_(cpu) { }

    /* Executes the instruction, returns true if execution is done. */
    virtual bool execute() = 0;

    static std::unique_ptr<Instruction> Get(uint8_t opcode, bool extended, CPU &cpu, MMU& mem);
};

/* Represents a CPU instruction that takes longer than a single m-cycle, should be inhertited to implement explicit instructions. */
class MultiCycleInstruction : public Instruction {
    uint8_t cycle_ = 0;

    protected:
    uint8_t temp_u8;
    uint16_t temp_u16;

    /* increments the cycle counter and returns the previous value. */
    uint8_t next_cycle() { return cycle_++; }

    /* Returns the current cycle. */
    uint8_t cycle() const { return cycle_; }

    //=================================================================================================//
    //  HELPER FUNCTIONS                                                                               //
    //=================================================================================================//

    /* LD R16, d16 - Load the 2 bytes of immediate data into register pair R16. */
    bool LD_R16_D16(uint8_t &higher_R, uint8_t &lower_R);

    /* LD ADDR, R8 - Store the contents of register R8 in the memory location specified by ADDR. */
    bool LD_ADDR_R8(uint16_t addr, const uint8_t &R);

    /* INC R16 - Increment the contents of register pair R16 by 1. */
    bool INC_R16(uint8_t &higher_R, uint8_t &lower_R);

    /* LD R8, d8 - Load the 8-bit immediate operand d8 into register R8. */
    bool LD_R8_D8(uint8_t &R);

    /* ADD HL, R16 - Add the contents of register pair R16 to the contents of register pair HL, and store the results in register pair HL. */
    bool ADD_HL_R16(const uint8_t &higher_R, const uint8_t &lower_R);

    /* LD R8, ADDR - Load the 8-bit contents of memory specified by ADDR into register R8. */
    bool LD_R8_ADDR(uint8_t &R, uint16_t addr);

    /* DEC R16 - Decrement the contents of register pair R16 by 1. */
    bool DEC_R16(uint8_t &higher_R, uint8_t &lower_R);

    /* POP - Pops a value from the stack and increments register SP. */
    uint8_t POP();

    /* POP R16 - Pops a 18 bit value from the stack and stores it in register pair R16. */
    bool POP(uint8_t &higher_R, uint8_t &lower_R);

    /* PUSH - Push a value to the stack and decrement register SP. */
    void PUSH(uint8_t val);

    /* PUSH R16 - Push a 18 bit register to the stack and decrement register SP. */
    bool PUSH(const uint8_t &higher_R, const uint8_t &lower_R);

    /* RST VECTOR - Call address vec. This is a shorter and faster equivalent to CALL for suitable values of vec. */
    bool RST(uint8_t vector);

    public:
    explicit MultiCycleInstruction(CPU &cpu, MMU &mem) : Instruction(cpu, mem) { };
};

#define INSTRUCTION(op, name, superclass) \
/* op - name */ \
class name : public superclass { \
    public: \
    explicit name(CPU &cpu, MMU &mem) : superclass(cpu, mem) {}; \
    \
    bool execute() override; \
};
#include "cpu/instructions/opcodes.tbl"
#include "cpu/instructions/extended_opcodes.tbl"
#undef INSTRUCTION


