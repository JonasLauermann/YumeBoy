#pragma once

#include <cstdint>
#include <memory>
#include "cpu/instructions/Instruction.hpp"
#include "cpu/states.hpp"
#include "mmu/Memory.hpp"


class YumeBoy;
class InterruptBus;

class CPU : public Memory {
    friend InterruptBus;
    friend Instruction;
    friend MultiCycleInstruction;

    #define INSTRUCTION(op, name, superclass) friend class name;
    #include "cpu/instructions/opcodes.tbl"
    #include "cpu/instructions/extended_opcodes.tbl"
    #undef INSTRUCTION

    MMU &mem_;

    CPU_STATES state = CPU_STATES::FetchOpcode;
    std::unique_ptr<Instruction> instruction;

    // Registers
    uint8_t A = 0x0;
    uint8_t B = 0x0;
    uint8_t C = 0x0;
    uint8_t D = 0x0;
    uint8_t E = 0x0;
    uint8_t H = 0x0;
    uint8_t L = 0x0;

    uint16_t SP = 0x0;
    uint16_t PC = 0x0;

    /* Flags:
     * Bit 7: z (Zero flag)
     * Bit 6: n (Subtraction Flag [BCD])
     * Bit 5: h (Half Carry flag [BCD])
     * Bit 4: c (Carry flag)
     * Bit 3-0: unused (always zero) */
    uint8_t F = 0x0;

    bool z() const { return F & 1 << 7; }
    bool n() const { return F & 1 << 6; }
    bool h() const { return F & 1 << 5; }
    bool c() const { return F & 1 << 4; }

    void z(bool b) { F = b ? F | (1 << 7) : F & ~(1 << 7); }
    void n(bool b) { F = b ? F | (1 << 6) : F & ~(1 << 6); }
    void h(bool b) { F = b ? F | (1 << 5) : F & ~(1 << 5); }
    void c(bool b) { F = b ? F | (1 << 4) : F & ~(1 << 4); }

    // 16-bit registers helper methods
    uint16_t AF() const { return uint16_t((A << 8) | F); }
    uint16_t BC() const { return uint16_t((B << 8) | C); }
    uint16_t DE() const { return uint16_t((D << 8) | E); }
    uint16_t HL() const { return uint16_t((H << 8) | L); }

    void AF(uint16_t x) {
        A = x >> 8;
        F = x & 0xF0;   // bits 0-3 are always zero
    }
    void BC(uint16_t x) {
        B = x >> 8;
        C = x & 0xFF;
    }
    void DE(uint16_t x) {
        D = x >> 8;
        E = x & 0xFF;
    }
    void HL(uint16_t x) {
        H = x >> 8;
        L = x & 0xFF;
    }

    // Interrupts
    /* IME is a flag internal to the CPU that controls whether any interrupt handlers are called, regardless of the
     * contents of IE. IME cannot be read in any way, and is modified by these instructions/events only. */
    bool IME = false;   // Interrupt Master Enable Flag
    
    /* Used to realize the delayed effect of the *EI* cpu instruction. */
    bool EI_executed = false;
    /* Used to realize the delayed effect of the *EI* cpu instruction. */
    bool set_IME = false;

    /* 0xFF0F — IF – Interrupt Flags (R/W)
       Only the 5 lower bits of this register are (R/W), the others return '1' always when read.
        Bit 4 – Joypad Interrupt Requested (1=Requested)
        Bit 3 – Serial Interrupt Requested (1=Requested)
        Bit 2 – Timer Interrupt Requested (1=Requested)
        Bit 1 – LCD STAT Interrupt Requested (1=Requested)
        Bit 0 – Vertical Blank Interrupt Requested (1=Requested) */
    uint8_t IF_ = 0x0;

    /* 0xFFFF – IE – Interrupt Enable (R/W)
       All 8 bits of this register are (R/W), but only the 5 lower ones are used by the interrupt handler.
        Bit 4 – Joypad Interrupt Enable (1=Enable, 0=Disable)
        Bit 3 – Serial Interrupt Enable (1=Enable, 0=Disable)
        Bit 2 – Timer Interrupt Enable (1=Enable, 0=Disable)
        Bit 1 – LCD STAT Interrupt Enable (1=Enable, 0=Disable)
        Bit 0 – Vertical Blank Interrupt Enable (1=Enable, 0=Disable) */
    uint8_t IE_ = 0x0;

    uint8_t IF() const { return IF_; }
    uint8_t IE() const { return IE_; }
    void IF(uint8_t value) { IF_ = value; }
    void IE(uint8_t value) { IE_ = value; }

    bool HALT_bug = false;

    uint8_t fetch_byte();

    public:
    CPU() = delete;
    CPU(MMU &mem, bool fast_boot) : mem_(mem) {
        if (fast_boot) {
            // Initialize the CPU’s state to the state it should have immediately after executing the boot ROM
            A = 0x01;
            F = 0xB0;
            B = 0x00;
            C = 0x13;
            D = 0x00;
            E = 0xD8;
            H = 0x01;
            L = 0x4D;
            SP = 0xFFFE;
            PC = 0x0100;
        }
    }

    /* Runs the CPU for one M-Cycle. */
    void tick();

    bool contains_address(uint16_t addr) const override {
        return (addr == 0xFF0F) or (addr == 0xFFFF);
    }

    uint8_t read_memory(uint16_t addr) override {
        if (addr == 0xFF0F)
            return IF();
        else if (addr == 0xFFFF)
            return IE();
        else
            std::unreachable();
    }

    void write_memory(uint16_t addr, uint8_t value) override {
        if (addr == 0xFF0F)
            IF(value);
        else if (addr == 0xFFFF)
            IE(value);
        else
            std::unreachable();
    }

};