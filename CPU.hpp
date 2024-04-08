#pragma once

#include <cstdint>

class YumeBoy;

class CPU {
    YumeBoy &yume_boy_; // Reference to Emulator
    uint64_t time_;     // the amount of time the cpu has been running for (in T-cycles / 2^22 Hz)

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
    uint16_t AF() const { return A << 8 | F; }
    uint16_t BC() const { return B << 8 | C; }
    uint16_t DE() const { return D << 8 | E; }
    uint16_t HL() const { return H << 8 | L; }

    void AF(uint16_t x) {
        A = x >> 8;
        F = x & 0xFF;
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

    /* TODO interrupts */
    // Interrupts
    /* IME is a flag internal to the CPU that controls whether any interrupt handlers are called, regardless of the
     * contents of IE. IME cannot be read in any way, and is modified by these instructions/events only. */
    bool IME = false;   // Interrupt Master Enable Flag

    /* 0xFF0F — IF – Interrupt Flags (R/W)
       Only the 5 lower bits of this register are (R/W), the others return '1' always when read.
        Bit 4 – Joypad Interrupt Requested (1=Requested)
        Bit 3 – Serial Interrupt Requested (1=Requested)
        Bit 2 – Timer Interrupt Requested (1=Requested)
        Bit 1 – LCD STAT Interrupt Requested (1=Requested)
        Bit 0 – Vertical Blank Interrupt Requested (1=Requested) */
    uint8_t IF = 0x0;

    /* 0xFFFF – IE – Interrupt Enable (R/W)
       All 8 bits of this register are (R/W), but only the 5 lower ones are used by the interrupt handler.
        Bit 4 – Joypad Interrupt Enable (1=Enable, 0=Disable)
        Bit 3 – Serial Interrupt Enable (1=Enable, 0=Disable)
        Bit 2 – Timer Interrupt Enable (1=Enable, 0=Disable)
        Bit 1 – LCD STAT Interrupt Enable (1=Enable, 0=Disable)
        Bit 0 – Vertical Blank Interrupt Enable (1=Enable, 0=Disable) */
    uint8_t IE = 0x0;

    public:
    uint8_t read_IF() { return IF; }
    uint8_t read_IE() { return IE; }
    void write_IF(uint8_t value) { IF = value; }
    void write_IE(uint8_t value) { IE = value; }

    private:
    /* one cpu cycle takes four T-cycles (2^22 Hz) */
    void m_cycle(uint8_t cycles = 1) { time_ += (cycles * 4); }

    uint8_t fetch_byte();

    /* Load value into register from memory address */
    void LD_register(uint8_t &target, uint16_t addr);
    /* Store value in memory at given address */
    void LD_memory(uint16_t addr, uint8_t value);
    /* Push 16-Bit value to stack */
    void PUSH(uint16_t value);
    /* Pop 16-Bit value from stack */
    uint16_t POP();
    /* subtract the contents of register source from the contents of register A, and store the results in register A. */
    void SUB(uint8_t const &source);
    /* Subtract the contents of register source and the carry flag from the contents of register A, and store the results in register A. */
    void SBC(uint8_t const &source);
    /* Compare the contents of register source and the contents of register A by calculating A - source, and set the Z flag if they are equal. */
    void CP_register(uint8_t const &target);
    /* compare the contents of memory specified by addr and the contents of register A by calculating A - (addr), and set the Z flag if they are equal. */
    void CP_memory(uint16_t addr);
    /* Rotate the contents of register `target` to the left. */
    void RL(uint8_t &target);

    public:
    CPU() = delete;
    CPU(YumeBoy &yume_boy) : yume_boy_(yume_boy) { }

    /* Runs the CPU until it reaches the next "stable" state. Returns the amount of time spent. */
    uint32_t tick();

};