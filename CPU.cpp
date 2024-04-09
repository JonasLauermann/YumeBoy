#include <iostream>
#include "CPU.hpp"

#include "YumeBoy.hpp"

uint8_t CPU::fetch_byte()
{
    m_cycle();
    return yume_boy_.read_memory(PC++);
}

void CPU::LD_register(uint8_t &target, uint16_t addr)
{
    m_cycle();
    target = yume_boy_.read_memory(addr);
}

void CPU::LD_memory(uint16_t addr, uint8_t value)
{
    m_cycle();
    yume_boy_.write_memory(addr, value);
}

void CPU::PUSH(uint16_t value)
{
    m_cycle();  // internal cycle
    m_cycle();
    yume_boy_.write_memory(--SP, value >> 8);
    m_cycle();
    yume_boy_.write_memory(--SP, value & 0xFF);
}

uint16_t CPU::POP()
{
    m_cycle();
    uint8_t lower = yume_boy_.read_memory(SP++);
    m_cycle();
    uint8_t higher = yume_boy_.read_memory(SP++);
    return lower | (higher << 8);
}

void CPU::SUB(uint8_t const &source)
{
    z(A == source);
    n(true);
    h((A & 0xF) < (source & 0xF));
    c(A < source);
    A -= source;
}

void CPU::SBC(uint8_t const &source)
{
    z(A == (source + c()));
    n(true);
    h((A & 0xF) < ((source + c()) & 0xF));
    c(A < (source + c()));
    A -= source + c();
}

void CPU::CP_register(uint8_t const &target)
{
    z(A == target);
    n(true);
    h((A & 0xF) < (target & 0xF));
    c(A < target);
}

void CPU::CP_memory(uint16_t addr)
{
    m_cycle();
    uint8_t mem_value = yume_boy_.read_memory(addr);
    z(A == mem_value);
    n(true);
    h((A & 0xF) < (mem_value & 0xF));
    c(A < mem_value);
}

void CPU::RL(uint8_t &target)
{
    uint8_t old_carry = c();
    c(target & 1 << 7);
    target = (target << 1) | old_carry;
    z(target == 0);
    n(false);
    h(false);
}

uint32_t CPU::tick()
{
    /* set time of current tick to 0. */
    time_ = 0;

    /* Used to realize the delayed effect of the *EI* cpu instruction.
     *    1 := EI was just executed, do nothing and decrement by one.
     *    0 := EI was executed in the previous fetch-execute cycle, enable interrupts and decrement by one.
     *   -1 := Do nothing. */
    int8_t ei_delay = -1;   // TODO: move out of method since it is no longer an infinite loop

    // fetch program counter
    uint8_t opcode = fetch_byte();
    switch (opcode) {
        case 0x00:  // NOP
            break;

        case 0x03: { // increment the contents of register pair BC by 1.
            m_cycle();
            BC(BC() + 1);
            break;
        }

        case 0x04: { // increment the contents of register B by 1.
            h((B & 0xF) == 0xF);
            ++B;
            z(B == 0);
            n(false);
            break;
        }

        case 0x05: { // decrement the contents of register B by 1.
            h((B & 0xF) == 0);
            --B;
            z(B == 0);
            n(true);
            break;
        }

        case 0x06: { // load the 8-bit immediate operand d8 into register B.
            B = fetch_byte();
            break;
        }

        case 0x0B: { // decrement the contents of register BC by 1.
            m_cycle();
            BC(BC() - 1);
            break;
        }

        case 0x0C: { // increment the contents of register C by 1.
            h((C & 0xF) == 0xF);
            ++C;
            z(C == 0);
            n(false);
            break;
        }

        case 0x0D: { // decrement the contents of register C by 1.
            h((C & 0xF) == 0);
            --C;
            z(C == 0);
            n(true);
            break;
        }

        case 0x0E: { // load the 8-bit immediate operand d8 into register C.
            C = fetch_byte();
            break;
        }

        case 0x11: { // load the 2 bytes of immediate data into register pair DE.
            DE(fetch_byte() | fetch_byte() << 8);
            break;
        }

        case 0x13: { // increment the contents of register pair DE by 1.
            m_cycle();
            DE(DE() + 1);
            break;
        }

        case 0x15: { // decrement the contents of register D by 1.
            h((D & 0xF) == 0);
            --D;
            z(D == 0);
            n(true);
            break;
        }

        case 0x16: { // load the 8-bit immediate operand d8 into register D.
            D = fetch_byte();
            break;
        }

        case 0x17: { // rotate the contents of register A to the left.
            RL(A);
            break;
        }

        case 0x18: { // jump s8 steps from the current address in the program counter (PC).
            int8_t offset = fetch_byte();
            m_cycle();
            PC += offset;
            break;
        }

        case 0x19: { // add the contents of register pair DE to the contents of register pair HL, and store the results in register pair HL.
            m_cycle(); // internal cycle
            n(false);
            h((HL() & 0xFFF) + (DE() & 0xFFF) > 0xFFF);
            c(HL() + DE() > 0xFFFF);
            HL(HL() + DE());
            break;
        }

        case 0x1A: { // load the 8-bit contents of memory specified by register pair DE into register A.
            LD_register(A, DE());
            break;
        }

        case 0x1D: { // decrement the contents of register E by 1.
            h((E & 0xF) == 0);
            --E;
            z(E == 0);
            n(true);
            break;
        }

        case 0x1E: { // load the 8-bit immediate operand d8 into register E.
            E = fetch_byte();
            break;
        }

        case 0x20: { // if zero flag is false, perform relative jump using the next byte as a signed offset
            int8_t offset = fetch_byte();
            if (z()) break;
            m_cycle();
            PC += offset;
            break;
        }

        case 0x21: { // load next two byte into register HL
            HL(fetch_byte() | fetch_byte() << 8);
            break;
        }

        case 0x22: { // store value in register A into the byte pointed to by HL and increment HL afterward.
            LD_memory(HL(), A);
            HL(HL() + 1);
            break;
        }

        case 0x23: { // increment the contents of register pair HL by 1.
            m_cycle();
            HL(HL() + 1);
            break;
        }

        case 0x24: { // increment the contents of register H by 1.
            h((H & 0xF) == 0xF);
            ++H;
            z(H == 0);
            n(false);
            break;
        }

        case 0x28: { // if the z flag is true, jump s8 steps from the current address stored in the program counter (PC).
            int8_t offset = fetch_byte();
            if (not z()) break;
            m_cycle();
            PC += offset;
            break;
        }

        case 0x31: { // load next two byte into register SP
            SP = fetch_byte() | fetch_byte() << 8;
            break;
        }

        case 0x32: { // store value in register A into the byte pointed to by HL and decrement HL afterward.
            LD_memory(HL(), A);
            HL(HL() - 1);
            break;
        }

        case 0x33: { // increment the contents of register pair SP by 1.
            m_cycle();
            ++SP;
            break;
        }

        case 0x3C: { // increment the contents of register A by 1.
            h((A & 0xF) == 0xF);
            ++A;
            z(A == 0);
            n(false);
            break;
        }

        case 0x3D: { // decrement the contents of register A by 1.
            h((A & 0xF) == 0);
            --A;
            z(A == 0);
            n(true);
            break;
        }

        case 0x3E: { // load the 8-bit immediate operand d8 into register A.
            A = fetch_byte();
            break;
        }

        case 0x45: { // load the contents of register L into register B.
            B = L;
            break;
        }

        case 0x4F: { // load the contents of register A into register C.
            C = A;
            break;
        }

        case 0x52: { // load the contents of register D into register D.
            D = D;
            break;
        }

        case 0x54: { // load the contents of register H into register D.
            D = H;
            break;
        }

        case 0x57: { // load the contents of register A into register D.
            D = A;
            break;
        }

        case 0x66: { // load the 8-bit contents of memory specified by register pair HL into register H.
            LD_register(H, HL());
            break;
        }

        case 0x67: { // load the contents of register A into register H.
            H = A;
            break;
        }

        case 0x6E: { // load the 8-bit contents of memory specified by register pair HL into register L.
            LD_register(L, HL());
            break;
        }

        case 0x73: { // store the contents of register E in the memory location specified by register pair HL.
            LD_memory(HL(), E);
            break;
        }

        case 0x77: { // store the contents of register A in the memory location specified by register pair HL.
            LD_memory(HL(), A);
            break;
        }

        case 0x78: { // load the contents of register B into register A.
            A = B;
            break;
        }

        case 0x7B: { // load the contents of register E into register A.
            A = E;
            break;
        }

        case 0x7C: { // load the contents of register H into register A.
            A = H;
            break;
        }

        case 0x7D: { // load the contents of register L into register A.
            A = L;
            break;
        }

        case 0x86: { // add the contents of memory specified by register pair HL to the contents of register A, and store the results in register A.
            m_cycle();
            auto mem = yume_boy_.read_memory(HL());
            n(false);
            h((A & 0xF) + (mem & 0xF) > 0xF);
            c(A + mem > 0xFF);
            A += mem;
            z(A == 0);
            break;
        }

        case 0x90: { // subtract the contents of register B from the contents of register A, and store the results in register A.
            SUB(B);
            break;
        }

        case 0x99: { // subtract the contents of register C and the carry flag from the contents of register A, and store the results in register A.
            SBC(C);
            break;
        }

        case 0x9F: { // subtract the contents of register A and the carry flag from the contents of register A, and store the results in register A.
            SBC(A);
            break;
        }

        case 0xAF: { // A XOR A (effectively sets register A to 0)
            A = 0;
            z(true);
            n(false);
            h(false);
            c(false);
            break;
        }

        case 0xB9: { // compare the contents of register C and the contents of register A by calculating A - C, and set the Z flag if they are equal.
            CP_register(C);
            break;
        }

        case 0xBB: { // compare the contents of register E and the contents of register A by calculating A - E, and set the Z flag if they are equal.
            CP_register(E);
            break;
        }

        case 0xBE: { // compare the contents of memory specified by register pair HL and the contents of register A by calculating A - (HL), and set the Z flag if they are equal.
            CP_memory(HL());
            break;
        }

        case 0xC1: { // pop the contents from the memory stack into register pair into register pair BC.
            BC(POP());
            break;
        }

        case 0xC5: { // push the contents of register pair BC onto the memory stack.
            PUSH(BC());
            break;
        }

        case 0xC9: { // RET - Pop from the memory stack the program counter PC value pushed when the subroutine was called, returning control to the source program.
            PC = POP();
            m_cycle();  // internal cycle
            break;
        }

        case 0xCB: { // Prefix -> fetch next byte to complete opcode
            opcode = fetch_byte();
            switch (opcode) {
                case 0x11: { // rotate the contents of register C to the left.
                    RL(C);
                    break;
                }

                case 0x7C: { // test bit 7 in register H, set the zero flag if bit not set.
                    z(not(H & 1 << 7));
                    n(false);
                    h(true);
                    break;
                }

                default:
                    std::cerr << "Unknown CPU instruction: 0xCB" << std::hex << std::uppercase << (int) opcode
                              << std::endl;
                    exit(-1);
            }
            break;
        }

        /* If the Z flag is 1, the program counter PC value corresponding to the memory location of the instruction
         * following the CALL instruction is pushed to the 2 bytes following the memory byte specified by the stack
         * pointer SP. The 16-bit immediate operand a16 is then loaded into PC. */
        case 0xCC: {
            uint16_t target_addr = (fetch_byte() | fetch_byte() << 8);
            if (not z()) { break; }
            PUSH(PC);
            PC = target_addr;
            break;
        }

        /* CALL - In memory, push the program counter PC value corresponding to the address following the CALL
         * instruction to the 2 bytes following the byte specified by the current stack pointer SP. Then load the 16-bit
         * immediate operand a16 into PC. */
        case 0xCD: {
            uint16_t target_addr = (fetch_byte() | fetch_byte() << 8);
            PUSH(PC);
            PC = target_addr;
            break;
        }

        case 0xD1: { // pop the contents from the memory stack into register pair into register pair DE.
            DE(POP());
            break;
        }

        case 0xE0: { // store the contents of register A in the internal RAM, port register, or mode_ register at the address in the range 0xFF00-0xFFFF specified by the 8-bit immediate operand a8.
            LD_memory(0xFF00 + fetch_byte(), A);
            break;
        }

        case 0xE2: { // store the contents of register A in the internal RAM, port register, or mode_ register at the address in the range 0xFF00-0xFFFF specified by register C.
            LD_memory(0xFF00 + C, A);
            break;
        }

        case 0xE5: { // push the contents of register pair HL onto the memory stack.
            PUSH(HL());
            break;
        }

        case 0xF0: { // load into register A the contents of the internal RAM, port register, or mode_ register at the address in the range 0xFF00-0xFFFF specified by the 8-bit immediate operand a8.
            LD_register(A, 0xFF00 + fetch_byte());
            break;
        }

        case 0xFE: { // compare the contents of register A and the contents of the 8-bit immediate operand d8 by calculating A - d8, and set the Z flag if they are equal.
            uint8_t op_value = fetch_byte();
            z(A == op_value);
            n(true);
            h((A & 0xF) < (op_value & 0xF));
            c(A < op_value);
            break;
        }

        default: {
            std::cerr << "Unknown CPU instruction: 0x" << std::hex << std::uppercase << (int) opcode << std::endl;
            exit(-1);
        }
    }

    /* Interrupt Handling */
    // check if interrupts should be enabled or disabled
    if (ei_delay == 1) { --ei_delay; }
    else if (ei_delay == 0) { IME = true; --ei_delay; }

    // check if an interrupt was requested and if the specific interrupt is enabled
    assert(not (IF & 0xE0) and "Interrupt flag set for invalid bits");
    uint8_t req_intrrupt = IE & IF;
    if (IME and req_intrrupt) {
        // if multiple interrupts were requested at the same time, handle lower bit interrupts first
        uint8_t interrupt_bit = 0;
        while (not (req_intrrupt & 0x1)) { req_intrrupt = req_intrrupt >> 1; interrupt_bit++; }

        // reset interrupt bit and disable interrupt handling
        assert(IF & 0x1 << interrupt_bit);
        IF ^= 0x1 << interrupt_bit;
        IME = false;

        // transfer control to the interrupt handler
        m_cycle();
        // push PC to stack.
        PUSH(PC);
        // set PC to handler address.
        m_cycle();
        PC = 0x40 + (0x8 * interrupt_bit);
    }

    return time_;
}