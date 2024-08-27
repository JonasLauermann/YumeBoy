#pragma once

#include <cstdint>
#include <memory>


class YumeBoy;

class CPU {
    YumeBoy &yume_boy_; // Reference to Emulator
    uint32_t time_;     // the amount of time the cpu has been running for (in T-cycles / 2^22 Hz)

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
    

    /* Used to realize the delayed effect of the *EI* cpu instruction.
     *    1 := EI was just executed, do nothing and decrement by one.
     *    0 := EI was executed in the previous fetch-execute cycle, enable interrupts and decrement by one.
     *   -1 := Do nothing. */
    int8_t EI_delay = -1;

    class TimerDivider {
        friend CPU;
        CPU &cpu_;

        uint16_t system_counter = 0x0;  // incremented every t-cycle, upper 8-bit make up DIV register

        /* 0xFF07 - TAC - Timer control
        Bit 2   - Enable: Controls whether TIMA is incremented. Note that DIV is always counting, regardless of this bit.
        Bit 1-0 - Clock select: Controls the frequency at which TIMA is incremented.
        (https://gbdev.io/pandocs/Timer_and_Divider_Registers.html#ff07--tac-timer-control) */
        uint8_t TAC_ = 0x0;

        bool old_tac_bit = false;   // Implements the "delay" in the DIV & TAC falling edge detector.

        /* 0xFF05 - TIMA: Timer counter
        This timer is incremented at the clock frequency specified by the TAC register (0xFF07). When the value overflows (exceeds 0xFF) it is reset to the value specified in TMA (0xFF06) and an interrupt is requested. */
        uint8_t TIMA_ = 0x0;

        bool old_tima_bit = false;  // Implements the "delay" in the TIMA overflow falling edge detector.
        bool tima_overflow = false;  // Is set to true to indicate that TIMA has overflown. Used to request a timer interrupt.
        bool tima_written = false;  // Is `true` if the TIMA was written to

        /* 0xFF06 - TMA: Timer modulo
        When TIMA overflows, it is reset to the value in this register and an interrupt is requested.
        If a TMA write is executed on the same M-cycle as the content of TMA is transferred to TIMA due to a timer overflow, the old value is transferred to TIMA. */
        uint8_t TMA_ = 0x0;

        protected:
        TimerDivider() = delete;
        explicit TimerDivider(CPU &cpu) : cpu_(cpu) { }

        /* Advance the Timer state by a single tick.
           `begin_m_cycle` should be set to true to signal to the Timer that it should request a Interrupt if TIMA has overflown (see https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html#timer-overflow-behavior) 
           `end_m_cycle` should be set to true to signal to the Timer that it should release the lock on the TIMA overflow detector (see https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html#timer-overflow-behavior) */
        void tick(bool begin_m_cycle, bool end_m_cycle);

        public:

        /* 0xFF04 — DIV: Divider register
        This register is incremented at a rate of 16384Hz. Writing any value to this register resets it to $00.
        Additionally, this register is reset when executing the stop instruction, and only begins ticking again once stop mode ends.
        The value of DIV is the actual bits of the system internal counter, not a mirror, not a register that increases with the system internal counter: The actual bits of the counter mapped to memory. */
        uint8_t DIV() const { return system_counter >> 8; }
        /* DIV can be written, but its value resets to 0 no matter what the value written is. In fact, the whole system internal counter is set to 0. */
        void DIV(uint8_t) { system_counter = 0; }
        
        uint8_t TAC() const { return TAC_; }
        /* Only the lower 3 bits are (R/W) */
        void TAC(uint8_t value) { TAC_ = value & 0b111; }
        
        uint8_t TIMA() const { return TIMA_; }
        void TIMA(uint8_t value) {
            TIMA_ = value;
            tima_written = true;
        }
        
        uint8_t TMA() const { return TMA_; }
        void TMA(uint8_t value) { TMA_ = value; }

    };
    TimerDivider timer_divider_;

    public:
    uint8_t IF() const { return IF_; }
    uint8_t IE() const { return IE_; }
    void IF(uint8_t value) { IF_ = value; }
    void IE(uint8_t value) { IE_ = value; }

    TimerDivider *timer_divider() { return &timer_divider_; }

    private:
    /* one cpu cycle takes four T-cycles (2^22 Hz).
       Because of the TimerDivider updated in this method, it should be called *after* the operation performed in this M-Cycle was executed. (e.g., when a new bytes is fetched from ROM, m_cycle() should be called after `read_memory(PC++)` was called) */
    void m_cycle(uint8_t cycles = 1);

    uint8_t fetch_byte();

    /* Load value into register from memory address */
    void LD_register(uint8_t &target, uint16_t addr);
    /* Store value in memory at given address */
    void LD_memory(uint16_t addr, uint8_t value);
    /* Push 16-Bit value to stack */
    void PUSH(uint16_t value);
    /* Pop 16-Bit value from stack */
    uint16_t POP();
    /* Increment the contents of register reg by 1. */
    void INC(uint8_t &reg);
    /* Decrement the contents of register reg by 1. */
    void DEC(uint8_t &reg);
    /* Add the contents of register pair source to the contents of register pair HL, and store the results in register pair HL. */
    void ADD_HL(uint16_t const &source);
    /* Add the contents of register source to the contents of register A, and store the results in register A. */
    void ADD(uint8_t const &source);
    /* Add the contents of register `source` and the CY flag to the contents of register A, and store the results in register A. */
    void ADC(uint8_t const &source);
    /* subtract the contents of register source from the contents of register A, and store the results in register A. */
    void SUB(uint8_t const &source);
    /* Subtract the contents of register source and the carry flag from the contents of register A, and store the results in register A. */
    void SBC(uint8_t const &source);
    /* Bitwise AND between the value in source and A. */
    void AND(uint8_t const &source);
    /* Bitwise XOR between the value in source and A. */
    void XOR(uint8_t const &source);
    /* Bitwise OR between the value in source and A. */
    void OR(uint8_t const &source);
    /* Call address vec. This is a shorter and faster equivalent to CALL for suitable values of vec. */
    void RST(uint8_t vector);
    /* Compare the contents of register source and the contents of register A by calculating A - source, and set the Z flag if they are equal. */
    void CP_register(uint8_t const &target);
    /* compare the contents of memory specified by addr and the contents of register A by calculating A - (addr), and set the Z flag if they are equal. */
    void CP_memory(uint16_t addr);
    /* Rotate the contents of register `target` to the left. */
    void RL(uint8_t &target);
    /* Rotate the contents of register `target` to the right. */
    void RR(uint8_t &target);
    /* Rotate the contents of register `target` to the left. The contents of bit 7 are placed in both the CY flag and bit 0 of register `target`. */
    void RLC(uint8_t &target);
    /* Shift the contents of register `target` to the left.  */
    void SLA(uint8_t &target);
    /* Shift the contents of register `target` to the right.  */
    void SRL(uint8_t &target);
    /* Test `bit` in register `source`, set the zero flag if bit not set.  */
    void BIT(uint8_t bit, const uint8_t &source);
    /* Set `bit` in register `source` to 0. */
    void RES(uint8_t bit, uint8_t &source) const;
    /* Set `bit` in register `source` to 1. */
    void SET(uint8_t bit, uint8_t &source) const;

    /* handles 16-bit opcodes */
    void cb_opcodes();

    public:
    CPU() = delete;
    explicit CPU(YumeBoy &yume_boy) : yume_boy_(yume_boy), timer_divider_(*this) { }

    /* Runs the CPU until it reaches the next "stable" state. Returns the amount of time spent. */
    uint32_t tick();

};