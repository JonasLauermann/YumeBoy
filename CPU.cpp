#include "CPU.hpp"

#include <iostream>
#include "YumeBoy.hpp"

void CPU::m_cycle(uint8_t cycles) 
{
    uint32_t t_cycles = cycles * 4;
    time_ += t_cycles;

    // increament timer clock per T-cycle
    for ( ; t_cycles > 0; --t_cycles ) {
        timer_divider()->tick((t_cycles % 4) == 0);
    }
}

uint8_t CPU::fetch_byte()
{
    uint8_t byte = yume_boy_.read_memory(PC);
    ++PC;
    m_cycle();
    return byte;
}

void CPU::LD_register(uint8_t &target, uint16_t addr)
{
    target = yume_boy_.read_memory(addr);
    m_cycle();
}

void CPU::LD_memory(uint16_t addr, uint8_t value)
{
    yume_boy_.write_memory(addr, value);
    m_cycle();
}

void CPU::PUSH(uint16_t value)
{
    m_cycle();  // internal cycle
    --SP;
    yume_boy_.write_memory(SP, value >> 8);
    m_cycle();
    --SP;
    yume_boy_.write_memory(SP, value & 0xFF);
    m_cycle();
}

uint16_t CPU::POP()
{
    uint8_t lower = yume_boy_.read_memory(SP);
    ++SP;
    m_cycle();
    uint8_t higher = yume_boy_.read_memory(SP);
    ++SP;
    m_cycle();
    return uint16_t(lower | (higher << 8u));
}

void CPU::INC(uint8_t &reg)
{
    h((reg & 0xF) == 0xF);
    ++reg;
    z(reg == 0);
    n(false);
}

void CPU::DEC(uint8_t &reg)
{
    h((reg & 0xF) == 0);
    --reg;
    z(reg == 0);
    n(true);
}

void CPU::ADD_HL(uint16_t const &source)
{
    m_cycle(); // internal cycle
    n(false);
    h((HL() & 0xFFF) + (source & 0xFFF) > 0xFFF);
    c(HL() + source > 0xFFFF);
    HL(HL() + source);
}

void CPU::ADD(uint8_t const &source)
{
    h((A & 0xF) + (source & 0xF) > 0xF);
    c(A > 0xFF - source);
    A += source;
    z(A == 0);
    n(true);
}

void CPU::ADC(uint8_t const &source)
{
    h((A & 0xF) + (source & 0xF) + uint8_t(c()) > 0xF);
    c(A + uint8_t(c()) > 0xFF - source);
    A += source + uint8_t(c());
    z(A == 0);
    n(true);
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
    z(A == (source + uint8_t(c())));
    n(true);
    h((A & 0xF) < ((source + uint8_t(c())) & 0xF));
    c(A < (source + uint8_t(c())));
    A -= source + uint8_t(c());
}

void CPU::AND(uint8_t const &source)
{
    A &= source;
    z(A == 0);
    n(false);
    h(true);
    c(false);
}

void CPU::XOR(uint8_t const &source)
{
    A ^= source;
    z(A == 0);
    n(false);
    h(false);
    c(false);
}

void CPU::OR(uint8_t const &source)
{
    A |= source;
    z(A == 0);
    n(false);
    h(false);
    c(false);
}

void CPU::RST(uint8_t vector)
{
    // push PC to stack.
    PUSH(PC);
    // set PC to vector address.
    PC = 0x8 * vector;  // for some reason this does not cause an M-Cycle
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
    uint8_t mem_value = yume_boy_.read_memory(addr);
    m_cycle();
    z(A == mem_value);
    n(true);
    h((A & 0xF) < (mem_value & 0xF));
    c(A < mem_value);
}

void CPU::RL(uint8_t &target)
{
    uint8_t old_carry = c();
    c(target & 1 << 7);
    target = uint8_t((target << 1) | old_carry);
    z(target == 0);
    n(false);
    h(false);
}

void CPU::RLC(uint8_t &target)
{
    c(target & 1 << 7);
    target = uint8_t((target << 1) | uint8_t(c()));
    z(target == 0);
    n(false);
    h(false);
}

void CPU::SLA(uint8_t &target)
{
    c(target & 1 << 7);
    target <<= 1;
    z(target == 0);
    n(false);
    h(false);
}

void CPU::BIT(uint8_t bit, const uint8_t &source)
{
    z(not(source & 1 << bit));
    n(false);
    h(true);
}

void CPU::RES(uint8_t bit, uint8_t &source) const
{
    assert(bit < 8);
    uint8_t mask = ~uint8_t(1 << bit);
    source &= mask;
}

void CPU::SET(uint8_t bit, uint8_t &source) const
{
    source |= 1 << bit;
}


void CPU::cb_opcodes()
{
    uint8_t opcode = fetch_byte();
    switch (opcode) {
        case 0x11: { // rotate the contents of register C to the left.
            RL(C);
            break;
        }

        case 0x27: { // Shift the contents of register A to the left.
            SLA(A);
            break;
        }

        case 0x37: { // Shift the contents of the lower-order four bits (0-3) of register A to the higher-order four bits (4-7) of the register, and shift the higher-order four bits to the lower-order four bits.
            A = ((A << 4) & 0b11110000) | (A >> 4);
            z(A == 0);
            n(false);
            h(false);
            c(false);
            break;
        }

        case 0x40: { // test bit 0 in register B, set the zero flag if bit not set.
            BIT(0, B);
            break;
        }

        case 0x41: { // test bit 0 in register C, set the zero flag if bit not set.
            BIT(0, C);
            break;
        }

        case 0x42: { // test bit 0 in register D, set the zero flag if bit not set.
            BIT(0, D);
            break;
        }

        case 0x43: { // test bit 0 in register E, set the zero flag if bit not set.
            BIT(0, E);
            break;
        }

        case 0x44: { // test bit 0 in register H, set the zero flag if bit not set.
            BIT(0, H);
            break;
        }

        case 0x45: { // test bit 0 in register L, set the zero flag if bit not set.
            BIT(0, L);
            break;
        }

        case 0x46: { // test bit 0 of the byte at the memeory address specified by register pair HL, set the zero flag if bit not set.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            BIT(0, value);
            break;
        }

        case 0x47: { // test bit 0 in register A, set the zero flag if bit not set.
            BIT(0, A);
            break;
        }

        case 0x48: { // test bit 1 in register B, set the zero flag if bit not set.
            BIT(1, B);
            break;
        }

        case 0x49: { // test bit 1 in register C, set the zero flag if bit not set.
            BIT(1, C);
            break;
        }

        case 0x4A: { // test bit 1 in register D, set the zero flag if bit not set.
            BIT(1, D);
            break;
        }

        case 0x4B: { // test bit 1 in register E, set the zero flag if bit not set.
            BIT(1, E);
            break;
        }

        case 0x4C: { // test bit 1 in register H, set the zero flag if bit not set.
            BIT(1, H);
            break;
        }

        case 0x4D: { // test bit 1 in register L, set the zero flag if bit not set.
            BIT(1, L);
            break;
        }

        case 0x4E: { // test bit 1 of the byte at the memeory address specified by register pair HL, set the zero flag if bit not set.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            BIT(1, value);
            break;
        }

        case 0x4F: { // test bit 1 in register A, set the zero flag if bit not set.
            BIT(1, A);
            break;
        }

        case 0x50: { // test bit 2 in register B, set the zero flag if bit not set.
            BIT(2, B);
            break;
        }

        case 0x51: { // test bit 2 in register C, set the zero flag if bit not set.
            BIT(2, C);
            break;
        }

        case 0x52: { // test bit 2 in register D, set the zero flag if bit not set.
            BIT(2, D);
            break;
        }

        case 0x53: { // test bit 2 in register E, set the zero flag if bit not set.
            BIT(2, E);
            break;
        }

        case 0x54: { // test bit 2 in register H, set the zero flag if bit not set.
            BIT(2, H);
            break;
        }

        case 0x55: { // test bit 2 in register L, set the zero flag if bit not set.
            BIT(2, L);
            break;
        }

        case 0x56: { // test bit 2 of the byte at the memeory address specified by register pair HL, set the zero flag if bit not set.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            BIT(2, value);
            break;
        }

        case 0x57: { // test bit 2 in register A, set the zero flag if bit not set.
            BIT(2, A);
            break;
        }

        case 0x58: { // test bit 3 in register B, set the zero flag if bit not set.
            BIT(3, B);
            break;
        }

        case 0x59: { // test bit 3 in register C, set the zero flag if bit not set.
            BIT(3, C);
            break;
        }

        case 0x5A: { // test bit 3 in register D, set the zero flag if bit not set.
            BIT(3, D);
            break;
        }

        case 0x5B: { // test bit 3 in register E, set the zero flag if bit not set.
            BIT(3, E);
            break;
        }

        case 0x5C: { // test bit 3 in register H, set the zero flag if bit not set.
            BIT(3, H);
            break;
        }

        case 0x5D: { // test bit 3 in register L, set the zero flag if bit not set.
            BIT(3, L);
            break;
        }

        case 0x5E: { // test bit 3 of the byte at the memeory address specified by register pair HL, set the zero flag if bit not set.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            BIT(3, value);
            break;
        }

        case 0x5F: { // test bit 3 in register A, set the zero flag if bit not set.
            BIT(3, A);
            break;
        }

        case 0x60: { // test bit 4 in register B, set the zero flag if bit not set.
            BIT(4, B);
            break;
        }

        case 0x61: { // test bit 4 in register C, set the zero flag if bit not set.
            BIT(4, C);
            break;
        }

        case 0x62: { // test bit 4 in register D, set the zero flag if bit not set.
            BIT(4, D);
            break;
        }

        case 0x63: { // test bit 4 in register E, set the zero flag if bit not set.
            BIT(4, E);
            break;
        }

        case 0x64: { // test bit 4 in register H, set the zero flag if bit not set.
            BIT(4, H);
            break;
        }

        case 0x65: { // test bit 4 in register L, set the zero flag if bit not set.
            BIT(4, L);
            break;
        }

        case 0x66: { // test bit 4 of the byte at the memeory address specified by register pair HL, set the zero flag if bit not set.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            BIT(4, value);
            break;
        }

        case 0x67: { // test bit 4 in register A, set the zero flag if bit not set.
            BIT(4, A);
            break;
        }

        case 0x68: { // test bit 5 in register B, set the zero flag if bit not set.
            BIT(5, B);
            break;
        }

        case 0x69: { // test bit 5 in register C, set the zero flag if bit not set.
            BIT(5, C);
            break;
        }

        case 0x6A: { // test bit 5 in register D, set the zero flag if bit not set.
            BIT(5, D);
            break;
        }

        case 0x6B: { // test bit 5 in register E, set the zero flag if bit not set.
            BIT(5, E);
            break;
        }

        case 0x6C: { // test bit 5 in register H, set the zero flag if bit not set.
            BIT(5, H);
            break;
        }

        case 0x6D: { // test bit 5 in register L, set the zero flag if bit not set.
            BIT(5, L);
            break;
        }

        case 0x6E: { // test bit 5 of the byte at the memeory address specified by register pair HL, set the zero flag if bit not set.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            BIT(5, value);
            break;
        }

        case 0x6F: { // test bit 5 in register A, set the zero flag if bit not set.
            BIT(5, A);
            break;
        }

        case 0x70: { // test bit 6 in register B, set the zero flag if bit not set.
            BIT(6, B);
            break;
        }

        case 0x71: { // test bit 6 in register C, set the zero flag if bit not set.
            BIT(6, C);
            break;
        }

        case 0x72: { // test bit 6 in register D, set the zero flag if bit not set.
            BIT(6, D);
            break;
        }

        case 0x73: { // test bit 6 in register E, set the zero flag if bit not set.
            BIT(6, E);
            break;
        }

        case 0x74: { // test bit 6 in register H, set the zero flag if bit not set.
            BIT(6, H);
            break;
        }

        case 0x75: { // test bit 6 in register L, set the zero flag if bit not set.
            BIT(6, L);
            break;
        }

        case 0x76: { // test bit 6 of the byte at the memeory address specified by register pair HL, set the zero flag if bit not set.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            BIT(6, value);
            break;
        }

        case 0x77: { // test bit 6 in register A, set the zero flag if bit not set.
            BIT(6, A);
            break;
        }

        case 0x78: { // test bit 7 in register B, set the zero flag if bit not set.
            BIT(7, B);
            break;
        }

        case 0x79: { // test bit 7 in register C, set the zero flag if bit not set.
            BIT(7, C);
            break;
        }

        case 0x7A: { // test bit 7 in register D, set the zero flag if bit not set.
            BIT(7, D);
            break;
        }

        case 0x7B: { // test bit 7 in register E, set the zero flag if bit not set.
            BIT(7, E);
            break;
        }

        case 0x7C: { // test bit 7 in register H, set the zero flag if bit not set.
            BIT(7, H);
            break;
        }

        case 0x7D: { // test bit 7 in register L, set the zero flag if bit not set.
            BIT(7, L);
            break;
        }

        case 0x7E: { // test bit 7 of the byte at the memeory address specified by register pair HL, set the zero flag if bit not set.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            BIT(7, value);
            break;
        }

        case 0x7F: { // test bit 7 in register A, set the zero flag if bit not set.
            BIT(7, A);
            break;
        }

        case 0x80: { // set bit 0 in register B to 0.
            RES(0, B);
            break;
        }

        case 0x81: { // set bit 0 in register C to 0.
            RES(0, C);
            break;
        }

        case 0x82: { // set bit 0 in register D to 0.
            RES(0, D);
            break;
        }

        case 0x83: { // set bit 0 in register E to 0.
            RES(0, E);
            break;
        }

        case 0x84: { // set bit 0 in register H to 0.
            RES(0, H);
            break;
        }

        case 0x85: { // set bit 0 in register L to 0.
            RES(0, L);
            break;
        }

        case 0x86: { // set bit 0 of the byte at the memeory address specified by register pair HL to 0.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            RES(0, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0x87: { // set bit 0 in register A to 0.
            RES(0, A);
            break;
        }

        case 0x88: { // set bit 1 in register B to 0.
            RES(1, B);
            break;
        }

        case 0x89: { // set bit 1 in register C to 0.
            RES(1, C);
            break;
        }

        case 0x8A: { // set bit 1 in register D to 0.
            RES(1, D);
            break;
        }

        case 0x8B: { // set bit 1 in register E to 0.
            RES(1, E);
            break;
        }

        case 0x8C: { // set bit 1 in register H to 0.
            RES(1, H);
            break;
        }

        case 0x8D: { // set bit 1 in register L to 0.
            RES(1, L);
            break;
        }

        case 0x8E: { // set bit 1 of the byte at the memeory address specified by register pair HL to 0.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            RES(1, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0x8F: { // set bit 1 in register A to 0.
            RES(1, A);
            break;
        }

        case 0x90: { // set bit 2 in register B to 0.
            RES(2, B);
            break;
        }

        case 0x91: { // set bit 2 in register C to 0.
            RES(2, C);
            break;
        }

        case 0x92: { // set bit 2 in register D to 0.
            RES(2, D);
            break;
        }

        case 0x93: { // set bit 2 in register E to 0.
            RES(2, E);
            break;
        }

        case 0x94: { // set bit 2 in register H to 0.
            RES(2, H);
            break;
        }

        case 0x95: { // set bit 2 in register L to 0.
            RES(2, L);
            break;
        }

        case 0x96: { // set bit 2 of the byte at the memeory address specified by register pair HL to 0.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            RES(2, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0x97: { // set bit 2 in register A to 0.
            RES(2, A);
            break;
        }

        case 0x98: { // set bit 3 in register B to 0.
            RES(3, B);
            break;
        }

        case 0x99: { // set bit 3 in register C to 0.
            RES(3, C);
            break;
        }

        case 0x9A: { // set bit 3 in register D to 0.
            RES(3, D);
            break;
        }

        case 0x9B: { // set bit 3 in register E to 0.
            RES(3, E);
            break;
        }

        case 0x9C: { // set bit 3 in register H to 0.
            RES(3, H);
            break;
        }

        case 0x9D: { // set bit 3 in register L to 0.
            RES(3, L);
            break;
        }

        case 0x9E: { // set bit 3 of the byte at the memeory address specified by register pair HL to 0.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            RES(3, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0x9F: { // set bit 3 in register A to 0.
            RES(3, A);
            break;
        }

        case 0xA0: { // set bit 4 in register B to 0.
            RES(4, B);
            break;
        }

        case 0xA1: { // set bit 4 in register C to 0.
            RES(4, C);
            break;
        }

        case 0xA2: { // set bit 4 in register D to 0.
            RES(4, D);
            break;
        }

        case 0xA3: { // set bit 4 in register E to 0.
            RES(4, E);
            break;
        }

        case 0xA4: { // set bit 4 in register H to 0.
            RES(4, H);
            break;
        }

        case 0xA5: { // set bit 4 in register L to 0.
            RES(4, L);
            break;
        }

        case 0xA6: { // set bit 4 of the byte at the memeory address specified by register pair HL to 0.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            RES(4, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xA7: { // set bit 4 in register A to 0.
            RES(4, A);
            break;
        }

        case 0xA8: { // set bit 5 in register B to 0.
            RES(5, B);
            break;
        }

        case 0xA9: { // set bit 5 in register C to 0.
            RES(5, C);
            break;
        }

        case 0xAA: { // set bit 5 in register D to 0.
            RES(5, D);
            break;
        }

        case 0xAB: { // set bit 5 in register E to 0.
            RES(5, E);
            break;
        }

        case 0xAC: { // set bit 5 in register H to 0.
            RES(5, H);
            break;
        }

        case 0xAD: { // set bit 5 in register L to 0.
            RES(5, L);
            break;
        }

        case 0xAE: { // set bit 5 of the byte at the memeory address specified by register pair HL to 0.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            RES(5, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xAF: { // set bit 5 in register A to 0.
            RES(5, A);
            break;
        }

        case 0xB0: { // set bit 6 in register B to 0.
            RES(6, B);
            break;
        }

        case 0xB1: { // set bit 6 in register C to 0.
            RES(6, C);
            break;
        }

        case 0xB2: { // set bit 6 in register D to 0.
            RES(6, D);
            break;
        }

        case 0xB3: { // set bit 6 in register E to 0.
            RES(6, E);
            break;
        }

        case 0xB4: { // set bit 6 in register H to 0.
            RES(6, H);
            break;
        }

        case 0xB5: { // set bit 6 in register L to 0.
            RES(6, L);
            break;
        }

        case 0xB6: { // set bit 6 of the byte at the memeory address specified by register pair HL to 0.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            RES(6, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xB7: { // set bit 6 in register A to 0.
            RES(6, A);
            break;
        }

        case 0xB8: { // set bit 7 in register B to 0.
            RES(7, B);
            break;
        }

        case 0xB9: { // set bit 7 in register C to 0.
            RES(7, C);
            break;
        }

        case 0xBA: { // set bit 7 in register D to 0.
            RES(7, D);
            break;
        }

        case 0xBB: { // set bit 7 in register E to 0.
            RES(7, E);
            break;
        }

        case 0xBC: { // set bit 7 in register H to 0.
            RES(7, H);
            break;
        }

        case 0xBD: { // set bit 7 in register L to 0.
            RES(7, L);
            break;
        }

        case 0xBE: { // set bit 7 of the byte at the memeory address specified by register pair HL to 0.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            RES(7, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xBF: { // set bit 7 in register A to 0.
            RES(7, A);
            break;
        }

        case 0xC0: { // set bit 0 in register B to 1.
            SET(0, B);
            break;
        }

        case 0xC1: { // set bit 0 in register C to 1.
            SET(0, C);
            break;
        }

        case 0xC2: { // set bit 0 in register D to 1.
            SET(0, D);
            break;
        }

        case 0xC3: { // set bit 0 in register E to 1.
            SET(0, E);
            break;
        }

        case 0xC4: { // set bit 0 in register H to 1.
            SET(0, H);
            break;
        }

        case 0xC5: { // set bit 0 in register L to 1.
            SET(0, L);
            break;
        }

        case 0xC6: { // set bit 0 of the byte at the memeory address specified by register pair HL to 1.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            SET(0, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xC7: { // set bit 0 in register A to 1.
            SET(0, A);
            break;
        }

        case 0xC8: { // set bit 1 in register B to 1.
            SET(1, B);
            break;
        }

        case 0xC9: { // set bit 1 in register C to 1.
            SET(1, C);
            break;
        }

        case 0xCA: { // set bit 1 in register D to 1.
            SET(1, D);
            break;
        }

        case 0xCB: { // set bit 1 in register E to 1.
            SET(1, E);
            break;
        }

        case 0xCC: { // set bit 1 in register H to 1.
            SET(1, H);
            break;
        }

        case 0xCD: { // set bit 1 in register L to 1.
            SET(1, L);
            break;
        }

        case 0xCE: { // set bit 1 of the byte at the memeory address specified by register pair HL to 1.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            SET(1, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xCF: { // set bit 1 in register A to 1.
            SET(1, A);
            break;
        }

        case 0xD0: { // set bit 2 in register B to 1.
            SET(2, B);
            break;
        }

        case 0xD1: { // set bit 2 in register C to 1.
            SET(2, C);
            break;
        }

        case 0xD2: { // set bit 2 in register D to 1.
            SET(2, D);
            break;
        }

        case 0xD3: { // set bit 2 in register E to 1.
            SET(2, E);
            break;
        }

        case 0xD4: { // set bit 2 in register H to 1.
            SET(2, H);
            break;
        }

        case 0xD5: { // set bit 2 in register L to 1.
            SET(2, L);
            break;
        }

        case 0xD6: { // set bit 2 of the byte at the memeory address specified by register pair HL to 1.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            SET(2, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xD7: { // set bit 2 in register A to 1.
            SET(2, A);
            break;
        }

        case 0xD8: { // set bit 3 in register B to 1.
            SET(3, B);
            break;
        }

        case 0xD9: { // set bit 3 in register C to 1.
            SET(3, C);
            break;
        }

        case 0xDA: { // set bit 3 in register D to 1.
            SET(3, D);
            break;
        }

        case 0xDB: { // set bit 3 in register E to 1.
            SET(3, E);
            break;
        }

        case 0xDC: { // set bit 3 in register H to 1.
            SET(3, H);
            break;
        }

        case 0xDD: { // set bit 3 in register L to 1.
            SET(3, L);
            break;
        }

        case 0xDE: { // set bit 3 of the byte at the memeory address specified by register pair HL to 1.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            SET(3, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xDF: { // set bit 3 in register A to 1.
            SET(3, A);
            break;
        }

        case 0xE0: { // set bit 4 in register B to 1.
            SET(4, B);
            break;
        }

        case 0xE1: { // set bit 4 in register C to 1.
            SET(4, C);
            break;
        }

        case 0xE2: { // set bit 4 in register D to 1.
            SET(4, D);
            break;
        }

        case 0xE3: { // set bit 4 in register E to 1.
            SET(4, E);
            break;
        }

        case 0xE4: { // set bit 4 in register H to 1.
            SET(4, H);
            break;
        }

        case 0xE5: { // set bit 4 in register L to 1.
            SET(4, L);
            break;
        }

        case 0xE6: { // set bit 4 of the byte at the memeory address specified by register pair HL to 1.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            SET(4, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xE7: { // set bit 4 in register A to 1.
            SET(4, A);
            break;
        }

        case 0xE8: { // set bit 5 in register B to 1.
            SET(5, B);
            break;
        }

        case 0xE9: { // set bit 5 in register C to 1.
            SET(5, C);
            break;
        }

        case 0xEA: { // set bit 5 in register D to 1.
            SET(5, D);
            break;
        }

        case 0xEB: { // set bit 5 in register E to 1.
            SET(5, E);
            break;
        }

        case 0xEC: { // set bit 5 in register H to 1.
            SET(5, H);
            break;
        }

        case 0xED: { // set bit 5 in register L to 1.
            SET(5, L);
            break;
        }

        case 0xEE: { // set bit 5 of the byte at the memeory address specified by register pair HL to 1.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            SET(5, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xEF: { // set bit 5 in register A to 1.
            SET(5, A);
            break;
        }

        case 0xF0: { // set bit 6 in register B to 1.
            SET(6, B);
            break;
        }

        case 0xF1: { // set bit 6 in register C to 1.
            SET(6, C);
            break;
        }

        case 0xF2: { // set bit 6 in register D to 1.
            SET(6, D);
            break;
        }

        case 0xF3: { // set bit 6 in register E to 1.
            SET(6, E);
            break;
        }

        case 0xF4: { // set bit 6 in register H to 1.
            SET(6, H);
            break;
        }

        case 0xF5: { // set bit 6 in register L to 1.
            SET(6, L);
            break;
        }

        case 0xF6: { // set bit 6 of the byte at the memeory address specified by register pair HL to 1.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            SET(6, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xF7: { // set bit 6 in register A to 1.
            SET(6, A);
            break;
        }

        case 0xF8: { // set bit 7 in register B to 1.
            SET(7, B);
            break;
        }

        case 0xF9: { // set bit 7 in register C to 1.
            SET(7, C);
            break;
        }

        case 0xFA: { // set bit 7 in register D to 1.
            SET(7, D);
            break;
        }

        case 0xFB: { // set bit 7 in register E to 1.
            SET(7, E);
            break;
        }

        case 0xFC: { // set bit 7 in register H to 1.
            SET(7, H);
            break;
        }

        case 0xFD: { // set bit 7 in register L to 1.
            SET(7, L);
            break;
        }

        case 0xFE: { // set bit 7 of the byte at the memeory address specified by register pair HL to 1.
            uint8_t value = yume_boy_.read_memory(HL());
            m_cycle();
            SET(7, value);
            yume_boy_.write_memory(HL(), value);
            m_cycle();
            break;
        }

        case 0xFF: { // set bit 7 in register A to 1.
            SET(7, A);
            break;
        }

        default:
            std::cerr << "Unknown CPU instruction: 0xCB" << std::hex << std::uppercase << (int) opcode
                        << std::endl;
            exit(-1);
    }
}


uint32_t CPU::tick()
{
    /* set time of current tick to 0. */
    time_ = 0;

    /* Interrupt Handling */
    // check if interrupts should be enabled or disabled
    if (EI_delay == 1) {
        --EI_delay;
    } else if (EI_delay == 0) {
        IME = true;
        --EI_delay;
    }

    // check if an interrupt was requested and if the specific interrupt is enabled
    assert(not (IF_ & 0xE0) and "Interrupt flag set for invalid bits");
    if (uint8_t req_intrrupt = IE_ & IF_; IME and req_intrrupt) {
        // if multiple interrupts were requested at the same time, handle lower bit interrupts first
        uint8_t interrupt_bit = 0;
        while (not (req_intrrupt & 0x1)) { req_intrrupt = req_intrrupt >> 1; interrupt_bit++; }

        // reset interrupt bit and disable interrupt handling
        assert(IF_ & 0x1 << interrupt_bit);
        IF_ ^= 0x1 << interrupt_bit;
        IME = false;

        // transfer control to the interrupt handler (https://gbdev.io/pandocs/Interrupts.html#interrupt-handling)
        // push PC to stack.
        PUSH(PC);
        m_cycle();
        // set PC to handler address.
        PC = 0x40 + (0x8 * interrupt_bit);
        m_cycle();
    }

    /* Execute next Instruction */
    // fetch program counter
    switch (uint8_t opcode = fetch_byte()) {
        case 0x00:  // NOP
            break;

        case 0x01: { // load the 2 bytes of immediate data into register pair BC.
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            BC(uint16_t((upper << 8) | lower));
            break;
        }

        case 0x03: { // increment the contents of register pair BC by 1.
            BC(BC() + 1);
            m_cycle();
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

        case 0x07: { // RLCA
            RLC(A);
            z(false);   // unlike RLC, for some reason RLCA always sets the zero flag to false
            break;
        }

        case 0x09: { // add the contents of register pair BC to the contents of register pair HL, and store the results in register pair HL.
            ADD_HL(BC());
            break;
        }

        case 0x0A: { // load the 8-bit contents of memory specified by register pair BC into register A.
            LD_register(A, BC());
            break;
        }

        case 0x0B: { // decrement the contents of register BC by 1.
            BC(BC() - 1);
            m_cycle();
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
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            DE(uint16_t((upper << 8) | lower));
            break;
        }

        case 0x12: { // store the contents of register A in the memory location specified by register pair DE.
            LD_memory(DE(), A);
            break;
        }

        case 0x13: { // increment the contents of register pair DE by 1.
            DE(DE() + 1);
            m_cycle();
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
            PC += offset;
            m_cycle();
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

        case 0x1C: { // increment the contents of register E by 1.
            INC(E);
            break;
        }

        case 0x1D: { // decrement the contents of register E by 1.
            DEC(E);
            break;
        }

        case 0x1E: { // load the 8-bit immediate operand d8 into register E.
            E = fetch_byte();
            break;
        }

        case 0x20: { // if zero flag is false, perform relative jump using the next byte as a signed offset
            int8_t offset = fetch_byte();
            if (z()) break;
            PC += offset;
            m_cycle();
            break;
        }

        case 0x21: { // load next two byte into register HL
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            HL(uint16_t((upper << 8) | lower));
            break;
        }

        case 0x22: { // store value in register A into the byte pointed to by HL and increment HL afterward.
            LD_memory(HL(), A);
            HL(HL() + 1);
            break;
        }

        case 0x23: { // increment the contents of register pair HL by 1.
            HL(HL() + 1);
            m_cycle();
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
            PC += offset;
            m_cycle();
            break;
        }

        case 0x2A: { // load the contents of memory specified by register pair HL into register A, and simultaneously increment the contents of HL.
            A = yume_boy_.read_memory(HL());
            HL(HL() + 1);
            m_cycle();
            break;
        }

        case 0x2C: { // increment the contents of register L by 1.
            INC(L);
            break;
        }

        case 0x2D: { // decrement the contents of register L by 1.
            DEC(L);
            break;
        }

        case 0x2F: { // Take the one's complement (i.e., flip all bits) of the contents of register A.
            A = ~A;
            n(true);
            h(true);
            break;
        }

        case 0x31: { // load next two byte into register SP
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            SP = uint16_t((upper << 8) | lower);
            break;
        }

        case 0x32: { // store value in register A into the byte pointed to by HL and decrement HL afterward.
            LD_memory(HL(), A);
            HL(HL() - 1);
            break;
        }

        case 0x33: { // increment the contents of register pair SP by 1.
            ++SP;
            m_cycle();
            break;
        }

        case 0x34: { // increment the contents of memory specified by register pair HL by 1.
            uint8_t val = yume_boy_.read_memory(HL());
            m_cycle();
            h((val & 0xF) == 0xF);
            ++val;
            yume_boy_.write_memory(HL(), val);
            m_cycle();
            z(val == 0);
            n(false);
            break;
        }

        case 0x35: { // decrement the contents of memory specified by register pair HL by 1.
            uint8_t val = yume_boy_.read_memory(HL());
            m_cycle();
            h((val & 0xF) == 0);
            --val;
            yume_boy_.write_memory(HL(), val);
            m_cycle();
            z(val == 0);
            n(true);
            break;
        }

        case 0x36: { // store the contents of 8-bit immediate operand d8 in the memory location specified by register pair HL.
            yume_boy_.write_memory(HL(), fetch_byte());
            m_cycle();
            break;
        }

        case 0x38: { // if the CY flag is 1, jump s8 steps from the current address stored in the program counter (PC).
            int8_t offset = fetch_byte();
            if (not c()) break;
            PC += offset;
            m_cycle();
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

        case 0x40: { // load the contents of register B into register B. (does nothing)
            break;
        }

        case 0x41: { // load the contents of register C into register B.
            B = C;
            break;
        }

        case 0x42: { // load the contents of register D into register B.
            B = D;
            break;
        }

        case 0x43: { // load the contents of register E into register B.
            B = E;
            break;
        }

        case 0x44: { // load the contents of register H into register B.
            B = H;
            break;
        }

        case 0x45: { // load the contents of register L into register B.
            B = L;
            break;
        }

        case 0x46: { // load the 8-bit contents of memory specified by register pair HL into register B.
            LD_register(B, HL());
            break;
        }

        case 0x47: { // load the contents of register A into register B.
            B = A;
            break;
        }

        case 0x48: { // load the contents of register B into register C.
            C = B;
            break;
        }

        case 0x49: { // load the contents of register C into register C. (does nothing)
            break;
        }

        case 0x4A: { // load the contents of register D into register C.
            C = D;
            break;
        }

        case 0x4B: { // load the contents of register E into register C.
            C = E;
            break;
        }

        case 0x4C: { // load the contents of register H into register C.
            C = H;
            break;
        }

        case 0x4D: { // load the contents of register L into register C.
            C = L;
            break;
        }

        case 0x4E: { // load the 8-bit contents of memory specified by register pair HL into register C.
            LD_register(C, HL());
            break;
        }

        case 0x4F: { // load the contents of register A into register C.
            C = A;
            break;
        }

        case 0x50: { // load the contents of register B into register D.
            D = B;
            break;
        }

        case 0x51: { // load the contents of register C into register D.
            D = C;
            break;
        }

        case 0x52: { // load the contents of register D into register D. (does nothing)
            break;
        }

        case 0x53: { // load the contents of register E into register D.
            D = E;
            break;
        }

        case 0x54: { // load the contents of register H into register D.
            D = H;
            break;
        }

        case 0x55: { // load the contents of register L into register D.
            D = L;
            break;
        }

        case 0x56: { // load the 8-bit contents of memory specified by register pair HL into register D.
            LD_register(D, HL());
            break;
        }

        case 0x57: { // load the contents of register A into register D.
            D = A;
            break;
        }

        case 0x58: { // load the contents of register B into register E.
            E = B;
            break;
        }

        case 0x59: { // load the contents of register C into register E.
            E = C;
            break;
        }

        case 0x5A: { // load the contents of register D into register E.
            E = D;
            break;
        }

        case 0x5B: { // load the contents of register E into register E. (does nothing)
            break;
        }

        case 0x5C: { // load the contents of register H into register E.
            E = H;
            break;
        }

        case 0x5D: { // load the contents of register L into register E.
            E = L;
            break;
        }

        case 0x5E: { // load the 8-bit contents of memory specified by register pair HL into register E.
            LD_register(E, HL());
            break;
        }

        case 0x5F: { // load the contents of register A into register E.
            E = A;
            break;
        }

        case 0x60: { // load the contents of register B into register H.
            H = B;
            break;
        }

        case 0x61: { // load the contents of register C into register H.
            H = C;
            break;
        }

        case 0x62: { // load the contents of register D into register H.
            H = D;
            break;
        }

        case 0x63: { // load the contents of register E into register H.
            H = E;
            break;
        }

        case 0x64: { // load the contents of register H into register H. (does nothing)
            break;
        }

        case 0x65: { // load the contents of register L into register H.
            H = L;
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

        case 0x68: { // load the contents of register B into register L.
            L = B;
            break;
        }

        case 0x69: { // load the contents of register C into register L.
            L = C;
            break;
        }

        case 0x6A: { // load the contents of register D into register L.
            L = D;
            break;
        }

        case 0x6B: { // load the contents of register E into register L.
            L = E;
            break;
        }

        case 0x6C: { // load the contents of register H into register L.
            L = H;
            break;
        }

        case 0x6D: { // load the contents of register L into register L. (does nothing)
            break;
        }

        case 0x6E: { // load the 8-bit contents of memory specified by register pair HL into register L.
            LD_register(L, HL());
            break;
        }

        case 0x6F: { // load the contents of register A into register L.
            L = A;
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

        case 0x79: { // load the contents of register C into register A.
            A = C;
            break;
        }

        case 0x7A: { // load the contents of register D into register A.
            A = D;
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

        case 0x7E: { // load the 8-bit contents of memory specified by register pair HL into register A.
            LD_register(A, HL());
            break;
        }

        case 0x7F: { // load the contents of register A into register A. (does nothing)
            break;
        }

        case 0x80: { // ADD A, B - add the contents of register B to the contents of register A, and store the results in register A.
            ADD(B);
            break;
        }

        case 0x81: { // ADD A, C - add the contents of register C to the contents of register A, and store the results in register A.
            ADD(C);
            break;
        }

        case 0x82: { // ADD A, D - add the contents of register D to the contents of register A, and store the results in register A.
            ADD(D);
            break;
        }

        case 0x83: { // ADD A, E - add the contents of register E to the contents of register A, and store the results in register A.
            ADD(E);
            break;
        }

        case 0x84: { // ADD A, H - add the contents of register H to the contents of register A, and store the results in register A.
            ADD(H);
            break;
        }

        case 0x85: { // ADD A, L - add the contents of register L to the contents of register A, and store the results in register A.
            ADD(L);
            break;
        }

        case 0x86: { // add the contents of memory specified by register pair HL to the contents of register A, and store the results in register A.
            auto mem = yume_boy_.read_memory(HL());
            m_cycle();
            ADD(mem);
            break;
        }

        case 0x87: { // ADD A, A - add the contents of register A to the contents of register A, and store the results in register A.
            ADD(A);
            break;
        }

        case 0x89: { // ADC A, C - add the contents of register C and the CY flag to the contents of register A, and store the results in register A.
            ADC(C);
            break;
        }

        case 0x90: { // subtract the contents of register B from the contents of register A, and store the results in register A.
            SUB(B);
            break;
        }

        case 0x96: { // subtract the contents of memory specified by register pair HL from the contents of register A, and store the results in register A.
            auto mem = yume_boy_.read_memory(HL());
            m_cycle();
            SUB(mem);
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

        case 0xA0: { // A AND B: Take the logical AND for each bit of the contents of register B and the contents of register A, and store the results in register A.
            AND(B);
            break;
        }

        case 0xA1: { // A AND C
            AND(C);
            break;
        }

        case 0xA2: { // A AND D
            AND(D);
            break;
        }

        case 0xA3: { // A AND E
            AND(E);
            break;
        }

        case 0xA4: { // A AND H
            AND(H);
            break;
        }

        case 0xA5: { // A AND L
            AND(L);
            break;
        }

        case 0xA7: { // A AND A (effectively only sets flags)
            z(A == 0);
            n(false);
            h(true);
            c(false);
            break;
        }

        case 0xA8: { // A XOR B: Take the logical exclusive-OR for each bit of the contents of register B and the contents of register A, and store the results in register A.
            XOR(B);
            break;
        }

        case 0xA9: { // A XOR C
            XOR(C);
            break;
        }

        case 0xAA: { // A XOR D
            XOR(D);
            break;
        }

        case 0xAB: { // A XOR E
            XOR(D);
            break;
        }

        case 0xAC: { // A XOR H
            XOR(D);
            break;
        }

        case 0xAD: { // A XOR L
            XOR(L);
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

        case 0xB0: { // A OR B
            OR(B);
            break;
        }

        case 0xB1: { // A OR C
            OR(C);
            break;
        }

        case 0xB2: { // A OR D
            OR(D);
            break;
        }

        case 0xB3: { // A OR E
            OR(E);
            break;
        }

        case 0xB4: { // A OR H
            OR(H);
            break;
        }

        case 0xB5: { // A OR L
            OR(L);
            break;
        }

        case 0xB7: { // A OR A
            OR(A);
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

        case 0xC0: { // RET NZ - If the Z flag is false, control is returned to the source program by popping from the memory stack the program counter PC value that was pushed to the stack when the subroutine was called.
            m_cycle();  // internal cycle
            if (z()) break;
            PC = POP();
            m_cycle();  // internal cycle
            break;
        }

        case 0xC1: { // pop the contents from the memory stack into register pair into register pair BC.
            BC(POP());
            break;
        }

        case 0xC2: { // JP NZ a16 - Load the 16-bit immediate operand a16 into the program counter (PC) if the Z flag is 0. a16 specifies the address of the subsequently executed instruction.
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            PC = uint16_t((upper << 8) | lower);
            if (z()) break;
            m_cycle();  // internal
            break;
        }

        case 0xC3: { // JP a16 - Load the 16-bit immediate operand a16 into the program counter (PC). a16 specifies the address of the subsequently executed instruction.
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            PC = uint16_t((upper << 8) | lower);
            m_cycle();  // internal
            break;
        }

        case 0xC4: { // CALL NZ a16 - If the Z flag is 0, the program counter PC value corresponding to the memory location of the instruction following the CALL instruction is pushed to the 2 bytes following the memory byte specified by the stack pointer SP. The 16-bit immediate operand a16 is then loaded into PC.
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            auto target_addr = uint16_t((upper << 8) | lower);
            if (z()) { break; }
            PUSH(PC);
            PC = target_addr;
            break;
        }

        case 0xC5: { // push the contents of register pair BC onto the memory stack.
            PUSH(BC());
            break;
        }

        case 0xC6: { // Add the contents of the 8-bit immediate operand d8 to the contents of register A, and store the results in register A.
            ADD(fetch_byte());
            break;
        }

        case 0xC7: { // RST 0 - Push the current value of the program counter PC onto the memory stack, and load into PC the 1th byte of page 0 memory addresses, 0x00.
            RST(0);
            break;
        }

        case 0xC8: { // RET Z - If the Z flag is true, control is returned to the source program by popping from the memory stack the program counter PC value that was pushed to the stack when the subroutine was called.
            m_cycle();  // internal cycle
            if (not z()) break;
            PC = POP();
            m_cycle();  // internal cycle
            break;
        }

        case 0xC9: { // RET - Pop from the memory stack the program counter PC value pushed when the subroutine was called, returning control to the source program.
            PC = POP();
            m_cycle();  // internal cycle
            break;
        }

        case 0xCA: { // JP Z - If the Z flag is true, Load the 16-bit immediate operand a16 into the program counter PC.
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            auto addr = uint16_t((upper << 8) | lower);
            if (not z()) break;
            PC = addr;
            m_cycle();  // internal branch dicision (?)
            break;
        }

        case 0xCB: { // Prefix -> fetch next byte to complete opcode
            cb_opcodes();
            break;
        }

        /* If the Z flag is 1, the program counter PC value corresponding to the memory location of the instruction
         * following the CALL instruction is pushed to the 2 bytes following the memory byte specified by the stack
         * pointer SP. The 16-bit immediate operand a16 is then loaded into PC. */
        case 0xCC: {
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            auto target_addr = uint16_t((upper << 8) | lower);
            if (not z()) { break; }
            PUSH(PC);
            PC = target_addr;
            break;
        }

        /* CALL - In memory, push the program counter PC value corresponding to the address following the CALL
         * instruction to the 2 bytes following the byte specified by the current stack pointer SP. Then load the 16-bit
         * immediate operand a16 into PC. */
        case 0xCD: {
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            auto target_addr = uint16_t((upper << 8) | lower);
            PUSH(PC);
            PC = target_addr;
            break;
        }

        case 0xCF: { // RST 1 - Push the current value of the program counter PC onto the memory stack, and load into PC the 1th byte of page 0 memory addresses, 0x08.
            RST(1);
            break;
        }

        case 0xD1: { // pop the contents from the memory stack into register pair into register pair DE.
            DE(POP());
            break;
        }

        case 0xD5: { // Push the contents of register pair DE onto the memory stack.
            PUSH(DE());
            break;
        }

        case 0xD6: { // Subtract the contents of the 8-bit immediate operand d8 from the contents of register A, and store the results in register A.
            SUB(fetch_byte());
            break;
        }

        case 0xD7: { // RST 2 - Push the current value of the program counter PC onto the memory stack, and load into PC the 1th byte of page 0 memory addresses, 0x10.
            RST(2);
            break;
        }

        case 0xD9: { // RETI - Used when an interrupt-service routine finishes. The address for the return from the interrupt is loaded in the program counter PC. The master interrupt enable flag is returned to its pre-interrupt status.
            IME = true; // as far as I can tell RETI does not have a one-instruction delay like EI
            PC = POP();
            m_cycle();  // internal cycle
            break;
        }

        case 0xDF: { // RST 3 - Push the current value of the program counter PC onto the memory stack, and load into PC the 1th byte of page 0 memory addresses, 0x18.
            RST(3);
            break;
        }

        case 0xE0: { // store the contents of register A in the internal RAM, port register, or mode_ register at the address in the range 0xFF00-0xFFFF specified by the 8-bit immediate operand a8.
            LD_memory(0xFF00 + fetch_byte(), A);
            break;
        }

        case 0xE1: { // pop the contents from the memory stack into register pair into register pair HL.
            HL(POP());
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

        case 0xE6: { // Take the logical AND for each bit of the contents of 8-bit immediate operand d8 and the contents of register A, and store the results in register A.
            AND(fetch_byte());
            break;
        }

        case 0xE7: { // RST 4 - Push the current value of the program counter PC onto the memory stack, and load into PC the 1th byte of page 0 memory addresses, 0x20.
            RST(4);
            break;
        }

        case 0xE9: { // JP HL - Load the contents of register pair HL into the program counter PC.
            PC = HL();
            break;
        }

        case 0xEA: { // store the contents of register A in the internal RAM or register specified by the 16-bit immediate operand a16.
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            yume_boy_.write_memory(uint16_t((upper << 8) | lower), A);
            m_cycle();
            break;
        }

        case 0xEE: { // A XOR d8
            XOR(fetch_byte());
            break;
        }

        case 0xEF: { // RST 5 - Push the current value of the program counter PC onto the memory stack, and load into PC the 1th byte of page 0 memory addresses, 0x28.
            RST(5);
            break;
        }

        case 0xF0: { // load into register A the contents of the internal RAM, port register, or mode_ register at the address in the range 0xFF00-0xFFFF specified by the 8-bit immediate operand a8.
            LD_register(A, 0xFF00 + fetch_byte());
            break;
        }

        case 0xF1: { // pop the contents from the memory stack into register pair into register pair AF.
            AF(POP());
            break;
        }

        case 0xF3: { // DI - Reset the interrupt master enable (IME) flag and prohibit maskable interrupts.
            IME = false;
            EI_delay = -1;
            break;
        }

        case 0xF5: { // Push the contents of register pair AF onto the memory stack.
            PUSH(AF());
            break;
        }

        case 0xF7: { // RST 6 - Push the current value of the program counter PC onto the memory stack, and load into PC the 1th byte of page 0 memory addresses, 0x30.
            RST(6);
            break;
        }

        case 0xFA: { // load into register A the contents of the internal RAM or register specified by the 16-bit immediate operand a16.
            uint8_t lower = fetch_byte();
            uint8_t upper = fetch_byte();
            A = yume_boy_.read_memory(uint16_t((upper << 8) | lower));
            m_cycle();
            break;
        }

        case 0xFB: { // EI - Set the interrupt master enable (IME) flag and enable maskable interrupts. This instruction can be used in an interrupt routine to enable higher-order interrupts.
            EI_delay = 1;
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

        case 0xFF: { // RST 7 - Push the current value of the program counter PC onto the memory stack, and load into PC the 1th byte of page 0 memory addresses, 0x38.
            RST(7);
            break;
        }

        default: {
            std::cerr << "Unknown CPU instruction: 0x" << std::hex << std::uppercase << (int) opcode << std::endl;
            exit(-1);
        }
    }

    return time_;
}

//=========================================================================
//  TimerDivider
//=========================================================================


void CPU::TimerDivider::tick(bool new_m_cycle)
{
    // based on https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html#relation-between-timer-and-divider-register
    // increment system_counter
    ++system_counter;

    // request interrupt if a m_cycle has passed and the TIMA register has overflown
    if (new_m_cycle and tima_overflow) {
        // request interrupt
        cpu_.yume_boy_.request_interrupt(YumeBoy::INTERRUPT::TIMER_INTERRUPT);

        // set overflown value of TIMA to TMA
        TIMA_ = TMA_;

        tima_overflow = false;
    }

    // determine bit selected by TAC multiplexer
    uint8_t selected_bit = 3;
    switch (TAC_ & 0b11) {
        case 0:
            selected_bit += 2;
        case 3:
            selected_bit += 2;
        case 2:
            selected_bit += 2;
        case 1:
            break;
        default:
            std::unreachable();
    }
    bool tac_bit = (system_counter & (1 << selected_bit)) and (TAC_ & 0b100);

    // DIV & TAC falling edge detector
    bool tmp_tac_bit = old_tac_bit;
    old_tac_bit = tac_bit;
    if (tac_bit or not tmp_tac_bit) return;

    // when falling edge is detected => increment TIMA
    ++TIMA_;

    // writes to TIMA must block the falling edge detector (https://gbdev.io/pandocs/Timer_Obscure_Behaviour.html#write_edge)
    if (not tima_written) {
        // TIMA overflow falling edge detector
        bool tima_high_bit = TIMA_ & 1 << 7;
        tima_overflow = tima_high_bit or not old_tima_bit;
        old_tima_bit = tima_high_bit;
    } else {
        tima_overflow = false;
    }
}
