#include <cpu/instructions/Instruction.hpp>

#include "cpu/CPU.hpp"
#include <cstdint>
#include <utility>
#include "YumeBoy.hpp"


#define INSTRUCTION(op, name, _) \
case op: { \
    instruction = std::make_unique<name>(cpu, mem); \
    break; \
}
std::unique_ptr<Instruction> Instruction::Get(uint8_t opcode, bool extended, CPU &cpu, MMU& mem)
{
    std::unique_ptr<Instruction> instruction;
    if (extended) {
        switch (opcode)
        {
        #include "cpu/instructions/extended_opcodes.tbl"
        
        default:
            std::unreachable();
        }
    } else {
        switch (opcode)
        {
        #include "cpu/instructions/opcodes.tbl"
        
        default:
            std::unreachable();
        }
    }
    return instruction;
}
#undef INSTRUCTION

//=================================================================================================//
//  HELPER FUNCTIONS                                                                               //
//=================================================================================================//

bool Instruction::INC_R8(uint8_t &R)
{
    cpu().h((R & 0xF) == 0xF);
    ++R;
    cpu().z(R == 0);
    cpu().n(false);
    return true;
}

bool Instruction::DEC_R8(uint8_t &R)
{
    cpu().h((R & 0xF) == 0);
    --R;
    cpu().z(R == 0);
    cpu().n(true);
    return true;
}

bool Instruction::LD_R8_R8(uint8_t &R0, const uint8_t &R1) const
{
    R0 = R1;
    return true;
}

bool Instruction::ADD(const uint8_t &R)
{
    cpu().h((cpu().A & 0xF) + (R & 0xF) > 0xF);
    cpu().c(cpu().A > 0xFF - R);
    cpu().A += R;
    cpu().z(cpu().A == 0);
    cpu().n(false);
    return true;
}

bool Instruction::ADC(const uint8_t &R)
{
    uint8_t old_carry = cpu().c();
    cpu().h((cpu().A & 0xF) + (R & 0xF) + uint8_t(cpu().c()) > 0xF);
    cpu().c(cpu().A + uint8_t(cpu().c()) > 0xFF - R);
    cpu().A += R + old_carry;
    cpu().z(cpu().A == 0);
    cpu().n(false);
    return true;
}

bool Instruction::SUB(const uint8_t &R)
{
    cpu().z(cpu().A == R);
    cpu().n(true);
    cpu().h((cpu().A & 0xF) < (R & 0xF));
    cpu().c(cpu().A < R);
    cpu().A -= R;
    return true;
}

bool Instruction::SBC(const uint8_t &R)
{
    uint8_t old_carry = cpu().c();
    cpu().n(true);
    cpu().h((cpu().A & 0xF) < (R & 0xF) + old_carry);
    cpu().c(cpu().A < (R + old_carry));
    cpu().A -= R + old_carry;
    cpu().z(cpu().A == 0);
    return true;
}

bool Instruction::AND(const uint8_t &R)
{
    cpu().A &= R;
    cpu().z(cpu().A == 0);
    cpu().n(false);
    cpu().h(true);
    cpu().c(false);
    return true;
}

bool Instruction::XOR(const uint8_t &R)
{
    cpu().A ^= R;
    cpu().z(cpu().A == 0);
    cpu().n(false);
    cpu().h(false);
    cpu().c(false);
    return true;
}

bool Instruction::OR(const uint8_t &R)
{
    cpu().A |= R;
    cpu().z(cpu().A == 0);
    cpu().n(false);
    cpu().h(false);
    cpu().c(false);
    return true;
}

bool Instruction::CP(const uint8_t &R)
{
    cpu().z(cpu().A == R);
    cpu().n(true);
    cpu().h((cpu().A & 0xF) < (R & 0xF));
    cpu().c(cpu().A < R);
    return true;
}

bool Instruction::RLC(uint8_t &R)
{
    cpu().c(R & 1 << 7);
    R = uint8_t(R << 1) | uint8_t(cpu().c());
    cpu().z(R == 0);
    cpu().n(false);
    cpu().h(false);
    return true;
}

bool Instruction::RRC(uint8_t &R)
{
    cpu().c(R & 1);
    R = uint8_t((R >> 1) | uint8_t(cpu().c()) << 7);
    cpu().z(R == 0);
    cpu().n(false);
    cpu().h(false);
    return true;
}

bool Instruction::RL(uint8_t &R)
{
    uint8_t old_carry = cpu().c();
    cpu().c(R & 1 << 7);
    R = uint8_t((R << 1) | old_carry);
    cpu().z(R == 0);
    cpu().n(false);
    cpu().h(false);
    return true;
}

bool Instruction::RR(uint8_t &R)
{
    uint8_t old_carry = cpu().c();
    cpu().c(R & 1);
    R = uint8_t((R >> 1) | (old_carry << 7));
    cpu().z(R == 0);
    cpu().n(false);
    cpu().h(false);
    return true;
}

bool Instruction::SLA(uint8_t &R)
{
    cpu().c(R & 1 << 7);
    R <<= 1;
    cpu().z(R == 0);
    cpu().n(false);
    cpu().h(false);
    return true;
}

bool Instruction::SRA(uint8_t &R)
{
    cpu().c(R & 1);
    R = (R >> 1) | (R & (1 << 7));
    cpu().z(R == 0);
    cpu().n(false);
    cpu().h(false);
    return true;
}

bool Instruction::SWAP(uint8_t &R)
{
    R = ((R << 4) & 0b11110000) | (R >> 4);
    cpu().z(R == 0);
    cpu().n(false);
    cpu().h(false);
    cpu().c(false);
    return true;
}

bool Instruction::SRL(uint8_t &R)
{
    cpu().c(R & 1);
    R >>= 1;
    cpu().z(R == 0);
    cpu().n(false);
    cpu().h(false);
    return true;
}

bool Instruction::BIT(uint8_t bit, const uint8_t &R)
{
    cpu().z(not(R & 1 << bit));
    cpu().n(false);
    cpu().h(true);
    return true;
}

bool Instruction::RES(uint8_t bit, uint8_t &R) const
{
    assert(bit < 8);
    uint8_t mask = ~uint8_t(1 << bit);
    R &= mask;
    return true;
}

bool Instruction::SET(uint8_t bit, uint8_t &R) const
{
    R |= 1 << bit;
    return true;
}

bool MultiCycleInstruction::LD_R16_D16(uint8_t &higher_R, uint8_t &lower_R)
{
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            lower_R = cpu().fetch_byte();
            return false;
        }
       
        case 2: {
            higher_R = cpu().fetch_byte();
            return true;
        }
       
        default:
            std::unreachable();
    }
}

bool MultiCycleInstruction::LD_ADDR_R8(uint16_t addr, const uint8_t &R)
{
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            mem().write_memory(addr, R);
            return true;
        }
       
        default:
            std::unreachable();
    }
}

bool MultiCycleInstruction::INC_R16(uint8_t &higher_R, uint8_t &lower_R)
{
    switch (next_cycle()) {
        case 0: {
            temp_u8 = (lower_R == 0xFF);
            ++lower_R;
            return false;
        }
       
        case 1: {
            higher_R += temp_u8;
            return true;
        }
       
        default:
            std::unreachable();
    }
}

bool MultiCycleInstruction::LD_R8_D8(uint8_t &R)
{
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            R = cpu().fetch_byte();
            return true;
        }
       
        default:
            std::unreachable();
    }
}

bool MultiCycleInstruction::ADD_HL_R16(const uint8_t &higher_R, const uint8_t &lower_R)
{
    switch (next_cycle()) {
        case 0: {
            temp_u8 = (cpu().L + lower_R) > 0xFF;
            cpu().n(false);
            cpu().h((cpu().HL() & 0xFFF) + (uint16_t((higher_R << 8) | lower_R) & 0xFFF) > 0xFFF);
            cpu().c(cpu().HL() + uint16_t((higher_R << 8) | lower_R) > 0xFFFF);
            cpu().L += lower_R;
            return false;
        }
       
        case 1: {
            cpu().H += higher_R + temp_u8;
            return true;
        }
       
        default:
            std::unreachable();
    }
}

bool MultiCycleInstruction::LD_R8_ADDR(uint8_t &R, uint16_t addr)
{
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            R = mem().read_memory(addr);
            return true;
        }
       
        default:
            std::unreachable();
    }
}

bool MultiCycleInstruction::DEC_R16(uint8_t &higher_R, uint8_t &lower_R)
{
    switch (next_cycle()) {
        case 0: {
            temp_u8 = (lower_R == 0);
            --lower_R;
            return false;
        }
       
        case 1: {
            higher_R -= temp_u8;
            return true;
        }
       
        default:
            std::unreachable();
    }
}

uint8_t MultiCycleInstruction::POP()
{
    uint8_t val = mem().read_memory(cpu().SP);
    ++cpu().SP;
    return val;
}

bool MultiCycleInstruction::POP(uint8_t &higher_R, uint8_t &lower_R)
{
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            lower_R = POP();
            return false;
        }

        case 2: {
            higher_R = POP();
            return true;
        }

        default:
            std::unreachable();
    }
}

void MultiCycleInstruction::PUSH(uint8_t val)
{
    --cpu().SP;
    mem().write_memory(cpu().SP, val);
}

bool MultiCycleInstruction::PUSH(const uint8_t &higher_R, const uint8_t &lower_R)
{
    switch (next_cycle()) {
        case 0:
        case 1: {
            return false;
        }

        case 2: {
            PUSH(higher_R);
            return false;
        }

        case 3: {
            PUSH(lower_R);
            return true;
        }

        default:
            std::unreachable();
    }
}

bool MultiCycleInstruction::RST(uint8_t vector)
{
    PUSH(uint8_t(cpu().PC >> 8), uint8_t(cpu().PC & 0xFF));
    // set PC to vector address.
    if (cycle() == 4) {
        cpu().PC = 0x8 * vector;
        return true;
    }
    return false;
}

//=================================================================================================//
//  OPCODE IMPLEMENTATIONS                                                                         //
//=================================================================================================//

/* 0x00 - NOP */
bool NOP::execute() {
    return true;
};

/* 0x01 - LD BC, d16 */
bool LD_BC_d16::execute() {
    return LD_R16_D16(cpu().B, cpu().C);
};

/* 0x02 - LD [BC], A */
bool LD_$BC$_A::execute() {
    return LD_ADDR_R8(cpu().BC(), cpu().A);
};

/* 0x03 - INC BC */
bool INC_BC::execute() {
    return INC_R16(cpu().B, cpu().C);
};

/* 0x04 - INC B */
bool INC_B::execute() {
    return INC_R8(cpu().B);
};

/* 0x05 - DEC B */
bool DEC_B::execute() {
    return DEC_R8(cpu().B);
};

/* 0x06 - LD B, d8 */
bool LD_B_d8::execute() {
    return LD_R8_D8(cpu().B);
};

/* 0x07 - RLCA */
bool RLCA::execute() {
    RLC(cpu().A);
    cpu().z(false);   // unlike RLC, for some reason RLCA always resets the zero flag
    return true;
};

/* 0x08 - LD (a16), SP */
bool LD_$a16$_SP::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t(cpu().fetch_byte() << 8) | temp_u8;
            return false;
        }

        case 3: {
            mem().write_memory(temp_u16, cpu().SP & 0xFF);
            return false;
        }

        case 4: {
            mem().write_memory(temp_u16 + 1, cpu().SP >> 8);
            return true;
        }

        default:
            std::unreachable();
    }
};

/* 0x09 - ADD HL, BC */
bool ADD_HL_BC::execute() {
    return ADD_HL_R16(cpu().B, cpu().C);
};

/* 0x0A - LD A, (BC) */
bool LD_A_$BC$::execute() {
    return LD_R8_ADDR(cpu().A, cpu().BC());
};

/* 0x0B - DEC BC */
bool DEC_BC::execute() {
    return DEC_R16(cpu().B, cpu().C);
};

/* 0x0C - INC C */
bool INC_C::execute() {
    return INC_R8(cpu().C);
};

/* 0x0D - DEC C */
bool DEC_C::execute() {
    return DEC_R8(cpu().C);
};

/* 0x0E - LD C, d8 */
bool LD_C_d8::execute() {
    return LD_R8_D8(cpu().C);
};

/* 0x0F - RRCA */
bool RRCA::execute() {
    RRC(cpu().A);
    cpu().z(false);   // unlike RRC, for some reason RRCA always resets the zero flag
    return true;
};

/* 0x10 - STOP */
bool STOP::execute() {
    std::unreachable();
};

/* 0x11 - LD DE, d16 */
bool LD_DE_d16::execute() {
    return LD_R16_D16(cpu().D, cpu().E);
};

/* 0x12 - LD [DE], A */
bool LD_$DE$_A::execute() {
    return LD_ADDR_R8(cpu().DE(), cpu().A);
};

/* 0x13 - INC DE */
bool INC_DE::execute() {
    return INC_R16(cpu().D, cpu().E);
};

/* 0x14 - INC D */
bool INC_D::execute() {
    return INC_R8(cpu().D);
};

/* 0x15 - DEC D */
bool DEC_D::execute() {
    return DEC_R8(cpu().D);
};

/* 0x16 - LD D, d8 */
bool LD_D_d8::execute() {
    return LD_R8_D8(cpu().D);
};

/* 0x17 - RLA */
bool RLA::execute() {
    RL(cpu().A);
    cpu().z(false);   // unlike RL, for some reason RLA always sets the zero flag to false
    return true;
};

/* 0x18 - JR s8 */
bool JR_s8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            cpu().PC += int8_t(temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
};

/* 0x19 - ADD HL, DE */
bool ADD_HL_DE::execute() {
    return ADD_HL_R16(cpu().D, cpu().E);
};

/* 0x1A - LD A, (DE) */
bool LD_A_$DE$::execute() {
    return LD_R8_ADDR(cpu().A, cpu().DE());
};

/* 0x1B - DEC DE */
bool DEC_DE::execute() {
    return DEC_R16(cpu().D, cpu().E);
};

/* 0x1C - INC E */
bool INC_E::execute() {
    return INC_R8(cpu().E);
};

/* 0x1D - DEC E */
bool DEC_E::execute() {
    return DEC_R8(cpu().E);
};

/* 0x1E - LD E, d8 */
bool LD_E_d8::execute() {
    return LD_R8_D8(cpu().E);
};

/* 0x1F - RRA */
bool RRA::execute() {
    RR(cpu().A);
    cpu().z(false);   // unlike RR, for some reason RRA always sets the zero flag to false
    return true;
};

/* 0x20 - JR_NZ_s8 - if zero flag is false, perform relative jump using the next byte as a signed offset */
bool JR_NZ_s8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return cpu().z();
        }

        case 2: {
            cpu().PC += int8_t(temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
};

/* 0x21 - LD HL, d16 */
bool LD_HL_d16::execute() {
    return LD_R16_D16(cpu().H, cpu().L);
};

/* 0x22 - LD [HL+], A */
bool LD_$HLinc$_A::execute() {
    LD_ADDR_R8(cpu().HL(), cpu().A);
    // increment HL at the last cycle
    if (cycle() == 2) {
        cpu().HL(cpu().HL() + 1);
        return true;
    }
    return false;
};

/* 0x23 - INC HL */
bool INC_HL::execute() {
    return INC_R16(cpu().H, cpu().L);
};

/* 0x24 - INC H */
bool INC_H::execute() {
    return INC_R8(cpu().H);
};

/* 0x25 - DEC H */
bool DEC_H::execute() {
    return DEC_R8(cpu().H);
};

/* 0x26 - LD H, d8 */
bool LD_H_d8::execute() {
    return LD_R8_D8(cpu().H);
};

/* 0x27 - DAA - Adjust the accumulator (register A) too a binary-coded decimal (BCD) number after BCD addition and subtraction operations. (https://blog.ollien.com/posts/gb-daa/) */
bool DAA::execute() {
    uint8_t offset = 0x00;
    if (((cpu().A & 0xF) > 0x9 and not cpu().n()) or cpu().h())
        offset += 0x06;

    if ((cpu().A > 0x99 and not cpu().n()) or cpu().c()) {
        offset += 0x60;
        cpu().c(true);
    }
    
    if (cpu().n())
        cpu().A -= offset;
    else
        cpu().A += offset;

    cpu().z(cpu().A == 0);
    cpu().h(false);
    return true;
};

/* 0x28 - JR_Z_s8 - if zero flag is true, perform relative jump using the next byte as a signed offset */
bool JR_Z_s8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return not cpu().z();
        }

        case 2: {
            cpu().PC += int8_t(temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
};

/* 0x29 - ADD HL, HL */
bool ADD_HL_HL::execute() {
    return ADD_HL_R16(cpu().H, cpu().L);
};

/* 0x2A - LD A, [HL+] */
bool LD_A_$HLinc$::execute() {
    LD_R8_ADDR(cpu().A, cpu().HL());
    // increment HL at the last cycle
    if (cycle() == 2) {
        cpu().HL(cpu().HL() + 1);
        return true;
    }
    return false;
};

/* 0x2B - DEC HL */
bool DEC_HL::execute() {
    return DEC_R16(cpu().H, cpu().L);
};

/* 0x2C - INC L */
bool INC_L::execute() {
    return INC_R8(cpu().L);
};

/* 0x2D - DEC L */
bool DEC_L::execute() {
    return DEC_R8(cpu().L);
};

/* 0x2E - LD L, d8 */
bool LD_L_d8::execute() {
    return LD_R8_D8(cpu().L);
};

/* 0x2F - CPL - Take the one's complement (i.e., flip all bits) of the contents of register A. */
bool CPL::execute() {
    cpu().A = ~cpu().A;
    cpu().n(true);
    cpu().h(true);
    return true;
};

/* 0x30 - JR_NC_s8 - if carry flag is false, perform relative jump using the next byte as a signed offset */
bool JR_NC_s8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return cpu().c();
        }

        case 2: {
            cpu().PC += int8_t(temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
};

/* 0x31 - LD SP, d16 */
bool LD_SP_d16::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            cpu().SP = cpu().fetch_byte();
            return false;
        }
       
        case 2: {
            cpu().SP = uint16_t(cpu().fetch_byte() << 8) | (cpu().SP & 0xFF);
            return true;
        }
       
        default:
            std::unreachable();
    }
};

/* 0x32 - LD [HL-], A */
bool LD_$HLdec$_A::execute() {
    LD_ADDR_R8(cpu().HL(), cpu().A);
    // decrement HL at the last cycle
    if (cycle() == 2) {
        cpu().HL(cpu().HL() - 1);
        return true;
    }
    return false;
};

/* 0x33 - INC SP */
bool INC_SP::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            ++cpu().SP;
            return true;
        }
       
        default:
            std::unreachable();
    }
};

/* 0x34 - INC [HL] */
bool INC_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }
       
        case 2: {
            cpu().h((temp_u8 & 0xF) == 0xF);
            ++temp_u8;
            cpu().z(temp_u8 == 0);
            cpu().n(false);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }
       
        default:
            std::unreachable();
    }
};

/* 0x35 - DEC [HL] */
bool DEC_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }
       
        case 2: {
            cpu().h((temp_u8 & 0xF) == 0);
            --temp_u8;
            cpu().z(temp_u8 == 0);
            cpu().n(true);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }
       
        default:
            std::unreachable();
    }
};

/* 0x36 - LD [HL], d8 */
bool LD_$HL$_d8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }
       
        case 2: {
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }
       
        default:
            std::unreachable();
    }
};

/* 0x37 - SCF - set the carry flag. */
bool SCF::execute() {
    cpu().n(false);
    cpu().h(false);
    cpu().c(true);
    return true;
};

/* 0x28 - JR_C_s8 - if carry flag is true, perform relative jump using the next byte as a signed offset */
bool JR_C_s8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return not cpu().c();
        }

        case 2: {
            cpu().PC += int8_t(temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
};

/* 0x39 - ADD HL, SP */
bool ADD_HL_SP::execute() {
    uint8_t S = (cpu().SP >> 8);
    uint8_t P = (cpu().SP & 0xFF);
    switch (next_cycle()) {
        case 0: {
            temp_u8 = (cpu().L + P) > 0xFF;
            cpu().n(false);
            cpu().h((cpu().HL() & 0xFFF) + (uint16_t((S << 8) | P) & 0xFFF) > 0xFFF);
            cpu().c(cpu().HL() + uint16_t((S << 8) | P) > 0xFFFF);
            cpu().L += P;
            return false;
        }
       
        case 1: {
            cpu().H += S + temp_u8;
            return true;
        }
       
        default:
            std::unreachable();
    }
};

/* 0x3A - LD A, [HL-] */
bool LD_A_$HLdec$::execute() {
    LD_R8_ADDR(cpu().A, cpu().HL());
    // decrement HL at the last cycle
    if (cycle() == 2) {
        cpu().HL(cpu().HL() - 1);
        return true;
    }
    return false;
};

/* 0x3B - DEC SP */
bool DEC_SP::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }
       
        case 1: {
            --cpu().SP;
            return true;
        }
       
        default:
            std::unreachable();
    }
};

/* 0x3C - INC A */
bool INC_A::execute() {
    return INC_R8(cpu().A);
};

/* 0x3D - DEC A */
bool DEC_A::execute() {
    return DEC_R8(cpu().A);
};

/* 0x3E - LD A, d8 */
bool LD_A_d8::execute() {
    return LD_R8_D8(cpu().A);
};

/* 0x3F - CCF - flip the carry flag. */
bool CCF::execute() {
    cpu().n(false);
    cpu().h(false);
    cpu().c(!cpu().c());
    return true;
};

/* 0x40 - LD B, B */
bool LD_B_B::execute() {
    return LD_R8_R8(cpu().B, cpu().B);
}

/* 0x41 - LD B, C */
bool LD_B_C::execute() {
    return LD_R8_R8(cpu().B, cpu().C);
}

/* 0x42 - LD B, D */
bool LD_B_D::execute() {
    return LD_R8_R8(cpu().B, cpu().D);
}

/* 0x43 - LD B, E */
bool LD_B_E::execute() {
    return LD_R8_R8(cpu().B, cpu().E);
}

/* 0x44 - LD B, H */
bool LD_B_H::execute() {
    return LD_R8_R8(cpu().B, cpu().H);
}

/* 0x45 - LD B, L */
bool LD_B_L::execute() {
    return LD_R8_R8(cpu().B, cpu().L);
}

/* 0x46 - LD B, [HL] */
bool LD_B_$HL$::execute() {
    return LD_R8_ADDR(cpu().B, cpu().HL());
}

/* 0x47 - LD B, A */
bool LD_B_A::execute() {
    return LD_R8_R8(cpu().B, cpu().A);
}

/* 0x48 - LD C, B */
bool LD_C_B::execute() {
    return LD_R8_R8(cpu().C, cpu().B);
}

/* 0x49 - LD C, C */
bool LD_C_C::execute() {
    return LD_R8_R8(cpu().C, cpu().C);
}

/* 0x4A - LD C, D */
bool LD_C_D::execute() {
    return LD_R8_R8(cpu().C, cpu().D);
}

/* 0x4B - LD C, E */
bool LD_C_E::execute() {
    return LD_R8_R8(cpu().C, cpu().E);
}

/* 0x4C - LD C, H */
bool LD_C_H::execute() {
    return LD_R8_R8(cpu().C, cpu().H);
}

/* 0x4D - LD C, L */
bool LD_C_L::execute() {
    return LD_R8_R8(cpu().C, cpu().L);
}

/* 0x4E - LD C, [HL] */
bool LD_C_$HL$::execute() {
    return LD_R8_ADDR(cpu().C, cpu().HL());
}

/* 0x4F - LD C, A */
bool LD_C_A::execute() {
    return LD_R8_R8(cpu().C, cpu().A);
}

/* 0x50 - LD D, B */
bool LD_D_B::execute() {
    return LD_R8_R8(cpu().D, cpu().B);
}

/* 0x51 - LD D, C */
bool LD_D_C::execute() {
    return LD_R8_R8(cpu().D, cpu().C);
}

/* 0x52 - LD D, D */
bool LD_D_D::execute() {
    return LD_R8_R8(cpu().D, cpu().D);
}

/* 0x53 - LD D, E */
bool LD_D_E::execute() {
    return LD_R8_R8(cpu().D, cpu().E);
}

/* 0x54 - LD D, H */
bool LD_D_H::execute() {
    return LD_R8_R8(cpu().D, cpu().H);
}

/* 0x55 - LD D, L */
bool LD_D_L::execute() {
    return LD_R8_R8(cpu().D, cpu().L);
}

/* 0x56 - LD D, [HL] */
bool LD_D_$HL$::execute() {
    return LD_R8_ADDR(cpu().D, cpu().HL());
}

/* 0x57 - LD D, A */
bool LD_D_A::execute() {
    return LD_R8_R8(cpu().D, cpu().A);
}

/* 0x58 - LD E, B */
bool LD_E_B::execute() {
    return LD_R8_R8(cpu().E, cpu().B);
}

/* 0x59 - LD E, C */
bool LD_E_C::execute() {
    return LD_R8_R8(cpu().E, cpu().C);
}

/* 0x5A - LD E, D */
bool LD_E_D::execute() {
    return LD_R8_R8(cpu().E, cpu().D);
}

/* 0x5B - LD E, E */
bool LD_E_E::execute() {
    return LD_R8_R8(cpu().E, cpu().E);
}

/* 0x5C - LD E, H */
bool LD_E_H::execute() {
    return LD_R8_R8(cpu().E, cpu().H);
}

/* 0x5D - LD E, L */
bool LD_E_L::execute() {
    return LD_R8_R8(cpu().E, cpu().L);
}

/* 0x5E - LD E, [HL] */
bool LD_E_$HL$::execute() {
    return LD_R8_ADDR(cpu().E, cpu().HL());
}

/* 0x5F - LD E, A */
bool LD_E_A::execute() {
    return LD_R8_R8(cpu().E, cpu().A);
}

/* 0x60 - LD H, B */
bool LD_H_B::execute() {
    return LD_R8_R8(cpu().H, cpu().B);
}

/* 0x61 - LD H, C */
bool LD_H_C::execute() {
    return LD_R8_R8(cpu().H, cpu().C);
}

/* 0x62 - LD H, D */
bool LD_H_D::execute() {
    return LD_R8_R8(cpu().H, cpu().D);
}

/* 0x63 - LD H, E */
bool LD_H_E::execute() {
    return LD_R8_R8(cpu().H, cpu().E);
}

/* 0x64 - LD H, H */
bool LD_H_H::execute() {
    return LD_R8_R8(cpu().H, cpu().H);
}

/* 0x65 - LD H, L */
bool LD_H_L::execute() {
    return LD_R8_R8(cpu().H, cpu().L);
}

/* 0x66 - LD H, [HL] */
bool LD_H_$HL$::execute() {
    return LD_R8_ADDR(cpu().H, cpu().HL());
}

/* 0x67 - LD H, A */
bool LD_H_A::execute() {
    return LD_R8_R8(cpu().H, cpu().A);
}

/* 0x68 - LD L, B */
bool LD_L_B::execute() {
    return LD_R8_R8(cpu().L, cpu().B);
}

/* 0x69 - LD L, C */
bool LD_L_C::execute() {
    return LD_R8_R8(cpu().L, cpu().C);
}

/* 0x6A - LD L, D */
bool LD_L_D::execute() {
    return LD_R8_R8(cpu().L, cpu().D);
}

/* 0x6B - LD L, E */
bool LD_L_E::execute() {
    return LD_R8_R8(cpu().L, cpu().E);
}

/* 0x6C - LD L, H */
bool LD_L_H::execute() {
    return LD_R8_R8(cpu().L, cpu().H);
}

/* 0x6D - LD L, L */
bool LD_L_L::execute() {
    return LD_R8_R8(cpu().L, cpu().L);
}

/* 0x6E - LD L, [HL] */
bool LD_L_$HL$::execute() {
    return LD_R8_ADDR(cpu().L, cpu().HL());
}

/* 0x6F - LD L, A */
bool LD_L_A::execute() {
    return LD_R8_R8(cpu().L, cpu().A);
}

/* 0x70 - LD [HL], B */
bool LD_$HL$_B::execute() {
    return LD_ADDR_R8(cpu().HL(), cpu().B);
}

/* 0x71 - LD [HL], C */
bool LD_$HL$_C::execute() {
    return LD_ADDR_R8(cpu().HL(), cpu().C);
}

/* 0x72 - LD [HL], D */
bool LD_$HL$_D::execute() {
    return LD_ADDR_R8(cpu().HL(), cpu().D);
}

/* 0x73 - LD [HL], E */
bool LD_$HL$_E::execute() {
    return LD_ADDR_R8(cpu().HL(), cpu().E);
}

/* 0x74 - LD [HL], H */
bool LD_$HL$_H::execute() {
    return LD_ADDR_R8(cpu().HL(), cpu().H);
}

/* 0x75 - LD [HL], L */
bool LD_$HL$_L::execute() {
    return LD_ADDR_R8(cpu().HL(), cpu().L);
}

/* 0x76 - HALT - (https://rgbds.gbdev.io/docs/v0.7.0/gbz80.7#HALT) */
bool HALT::execute() {
    cpu().state = CPU_STATES::HaltMode;
    cpu().HALT_bug = true;
    return true;
}

/* 0x77 - LD [HL], A */
bool LD_$HL$_A::execute() {
    return LD_ADDR_R8(cpu().HL(), cpu().A);
}

/* 0x78 - LD A, B */
bool LD_A_B::execute() {
    return LD_R8_R8(cpu().A, cpu().B);
}

/* 0x79 - LD A, C */
bool LD_A_C::execute() {
    return LD_R8_R8(cpu().A, cpu().C);
}

/* 0x7A - LD A, D */
bool LD_A_D::execute() {
    return LD_R8_R8(cpu().A, cpu().D);
}

/* 0x7B - LD A, E */
bool LD_A_E::execute() {
    return LD_R8_R8(cpu().A, cpu().E);
}

/* 0x7C - LD A, H */
bool LD_A_H::execute() {
    return LD_R8_R8(cpu().A, cpu().H);
}

/* 0x7D - LD A, L */
bool LD_A_L::execute() {
    return LD_R8_R8(cpu().A, cpu().L);
}

/* 0x7E - LD A, [HL] */
bool LD_A_$HL$::execute() {
    return LD_R8_ADDR(cpu().A, cpu().HL());
}

/* 0x7F - LD A, A */
bool LD_A_A::execute() {
    return LD_R8_R8(cpu().A, cpu().A);
}

/* 0x80 - ADD A, B */
bool ADD_A_B::execute() {
    return ADD(cpu().B);
}

/* 0x81 - ADD A, C */
bool ADD_A_C::execute() {
    return ADD(cpu().C);
}

/* 0x82 - ADD A, D */
bool ADD_A_D::execute() {
    return ADD(cpu().D);
}

/* 0x83 - ADD A, E */
bool ADD_A_E::execute() {
    return ADD(cpu().E);
}

/* 0x84 - ADD A, H */
bool ADD_A_H::execute() {
    return ADD(cpu().H);
}

/* 0x85 - ADD A, L */
bool ADD_A_L::execute() {
    return ADD(cpu().L);
}

/* 0x86 - ADD A, [HL] */
bool ADD_A_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            ADD(mem().read_memory(cpu().HL()));
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0x87 - ADD A, A */
bool ADD_A_A::execute() {
    return ADD(cpu().A);
}

/* 0x88 - ADC A, B */
bool ADC_A_B::execute() {
    return ADC(cpu().B);
}

/* 0x89 - ADC A, C */
bool ADC_A_C::execute() {
    return ADC(cpu().C);
}

/* 0x8A - ADC A, D */
bool ADC_A_D::execute() {
    return ADC(cpu().D);
}

/* 0x8B - ADC A, E */
bool ADC_A_E::execute() {
    return ADC(cpu().E);
}

/* 0x8C - ADC A, H */
bool ADC_A_H::execute() {
    return ADC(cpu().H);
}

/* 0x8D - ADC A, L */
bool ADC_A_L::execute() {
    return ADC(cpu().L);
}

/* 0x8E - ADC A, [HL] */
bool ADC_A_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            ADC(mem().read_memory(cpu().HL()));
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0x8F - ADC A, A */
bool ADC_A_A::execute() {
    return ADC(cpu().A);
}

/* 0x90 - SUB A, B */
bool SUB_A_B::execute() {
    return SUB(cpu().B);
}

/* 0x91 - SUB A, C */
bool SUB_A_C::execute() {
    return SUB(cpu().C);
}

/* 0x92 - SUB A, D */
bool SUB_A_D::execute() {
    return SUB(cpu().D);
}

/* 0x93 - SUB A, E */
bool SUB_A_E::execute() {
    return SUB(cpu().E);
}

/* 0x94 - SUB A, H */
bool SUB_A_H::execute() {
    return SUB(cpu().H);
}

/* 0x95 - SUB A, L */
bool SUB_A_L::execute() {
    return SUB(cpu().L);
}

/* 0x96 - SUB A, [HL] */
bool SUB_A_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            SUB(mem().read_memory(cpu().HL()));
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0x97 - SUB A, A */
bool SUB_A_A::execute() {
    return SUB(cpu().A);
}

/* 0x98 - SBC A, B */
bool SBC_A_B::execute() {
    return SBC(cpu().B);
}

/* 0x99 - SBC A, C */
bool SBC_A_C::execute() {
    return SBC(cpu().C);
}

/* 0x9A - SBC A, D */
bool SBC_A_D::execute() {
    return SBC(cpu().D);
}

/* 0x9B - SBC A, E */
bool SBC_A_E::execute() {
    return SBC(cpu().E);
}

/* 0x9C - SBC A, H */
bool SBC_A_H::execute() {
    return SBC(cpu().H);
}

/* 0x9D - SBC A, L */
bool SBC_A_L::execute() {
    return SBC(cpu().L);
}

/* 0x9E - SBC A, [HL] */
bool SBC_A_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            SBC(mem().read_memory(cpu().HL()));
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0x9F - SBC A, A */
bool SBC_A_A::execute() {
    return SBC(cpu().A);
}

/* 0xA0 - AND A, B */
bool AND_A_B::execute() {
    return AND(cpu().B);
}

/* 0xA1 - AND A, C */
bool AND_A_C::execute() {
    return AND(cpu().C);
}

/* 0xA2 - AND A, D */
bool AND_A_D::execute() {
    return AND(cpu().D);
}

/* 0xA3 - AND A, E */
bool AND_A_E::execute() {
    return AND(cpu().E);
}

/* 0xA4 - AND A, H */
bool AND_A_H::execute() {
    return AND(cpu().H);
}

/* 0xA5 - AND A, L */
bool AND_A_L::execute() {
    return AND(cpu().L);
}

/* 0xA6 - AND A, [HL] */
bool AND_A_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            AND(mem().read_memory(cpu().HL()));
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xA7 - AND A, A */
bool AND_A_A::execute() {
    return AND(cpu().A);
}

/* 0xA8 - XOR A, B */
bool XOR_A_B::execute() {
    return XOR(cpu().B);
}

/* 0xA9 - XOR A, C */
bool XOR_A_C::execute() {
    return XOR(cpu().C);
}

/* 0xAA - XOR A, D */
bool XOR_A_D::execute() {
    return XOR(cpu().D);
}

/* 0xAB - XOR A, E */
bool XOR_A_E::execute() {
    return XOR(cpu().E);
}

/* 0xAC - XOR A, H */
bool XOR_A_H::execute() {
    return XOR(cpu().H);
}

/* 0xAD - XOR A, L */
bool XOR_A_L::execute() {
    return XOR(cpu().L);
}

/* 0xAE - XOR A, [HL] */
bool XOR_A_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            XOR(mem().read_memory(cpu().HL()));
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xAF - XOR A, A */
bool XOR_A_A::execute() {
    return XOR(cpu().A);
}

/* 0xB0 - OR A, B */
bool OR_A_B::execute() {
    return OR(cpu().B);
}

/* 0xB1 - OR A, C */
bool OR_A_C::execute() {
    return OR(cpu().C);
}

/* 0xB2 - OR A, D */
bool OR_A_D::execute() {
    return OR(cpu().D);
}

/* 0xB3 - OR A, E */
bool OR_A_E::execute() {
    return OR(cpu().E);
}

/* 0xB4 - OR A, H */
bool OR_A_H::execute() {
    return OR(cpu().H);
}

/* 0xB5 - OR A, L */
bool OR_A_L::execute() {
    return OR(cpu().L);
}

/* 0xB6 - OR A, [HL] */
bool OR_A_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            OR(mem().read_memory(cpu().HL()));
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xB7 - OR A, A */
bool OR_A_A::execute() {
    return OR(cpu().A);
}

/* 0xB8 - CP A, B */
bool CP_A_B::execute() {
    return CP(cpu().B);
}

/* 0xB9 - CP A, C */
bool CP_A_C::execute() {
    return CP(cpu().C);
}

/* 0xBA - CP A, D */
bool CP_A_D::execute() {
    return CP(cpu().D);
}

/* 0xBB - CP A, E */
bool CP_A_E::execute() {
    return CP(cpu().E);
}

/* 0xBC - CP A, H */
bool CP_A_H::execute() {
    return CP(cpu().H);
}

/* 0xBD - CP A, L */
bool CP_A_L::execute() {
    return CP(cpu().L);
}

/* 0xBE - CP A, [HL] */
bool CP_A_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            CP(mem().read_memory(cpu().HL()));
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xBF - CP A, A */
bool CP_A_A::execute() {
    return CP(cpu().A);
}

/* 0xC0 - RET NZ - If the Z flag is 0, control is returned to the source program by popping from the memory stack the program counter PC value that was pushed to the stack when the subroutine was called.

The contents of the address specified by the stack pointer SP are loaded in the lower-order byte of PC, and the contents of SP are incremented by 1. The contents of the address specified by the new SP value are then loaded in the higher-order byte of PC, and the contents of SP are incremented by 1 again. (The value of SP is 2 larger than before instruction execution.) The next instruction is fetched from the address specified by the content of PC (as usual). */
bool RET_NZ::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            return cpu().z();
        }

        case 2: {
            temp_u8 = POP();
            return false;
        }

        case 3: {
            temp_u16 = uint16_t((POP() << 8) | temp_u8);
            return false;
        }

        case 4: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xC1 - POP BC */
bool POP_BC::execute() {
    return POP(cpu().B, cpu().C);
}

/* 0xC2 - JP NZ, a16 */
bool JP_NZ_a16::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((cpu().fetch_byte() << 8) | temp_u8);
            return cpu().z();
        }

        case 3: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xC3 - JP a16 */
bool JP_a16::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((cpu().fetch_byte() << 8)) | temp_u8;
            return false;
        }

        case 3: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xC4 - CALL NZ, a16 */
bool CALL_NZ_a16::execute() {
    switch (next_cycle()) {
        case 0:
        case 3: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((cpu().fetch_byte() << 8)) | temp_u8;
            return cpu().z();
        }

        case 4: {
            PUSH(cpu().PC >> 8);
            return false;
        }

        case 5: {
            PUSH(cpu().PC & 0xFF);
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xC5 - PUSH BC */
bool PUSH_BC::execute() {
    return PUSH(cpu().B, cpu().C);
}

/* 0xC6 - ADD A, d8 */
bool ADD_A_d8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            ADD(cpu().fetch_byte());
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xC7 - RST 0 */
bool RST_0::execute() {
    return RST(0);
}

/* 0xC8 - RET Z - If the Z flag is 1, control is returned to the source program by popping from the memory stack the program counter PC value that was pushed to the stack when the subroutine was called. */
bool RET_Z::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            return not cpu().z();
        }

        case 2: {
            temp_u8 = POP();
            return false;
        }

        case 3: {
            temp_u16 = uint16_t((POP() << 8)) | temp_u8;
            return false;
        }

        case 4: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xC9 - RET - Control is returned to the source program by popping from the memory stack the program counter PC value that was pushed to the stack when the subroutine was called. */
bool RET::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = POP();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((POP() << 8)) | temp_u8;
            return false;
        }

        case 3: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCA - JP Z, a16 */
bool JP_Z_a16::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((cpu().fetch_byte() << 8)) | temp_u8;
            return not cpu().z();
        }

        case 3: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCC - CALL Z, a16 */
bool CALL_Z_a16::execute() {
    switch (next_cycle()) {
        case 0:
        case 3: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((cpu().fetch_byte() << 8)) | temp_u8;
            return not cpu().z();
        }

        case 4: {
            PUSH(cpu().PC >> 8);
            return false;
        }

        case 5: {
            PUSH(cpu().PC & 0xFF);
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCD - CALL a16 */
bool CALL_a16::execute() {
    switch (next_cycle()) {
        case 0:
        case 3: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((cpu().fetch_byte() << 8)) | temp_u8;
            return false;
        }

        case 4: {
            PUSH(cpu().PC >> 8);
            return false;
        }

        case 5: {
            PUSH(cpu().PC & 0xFF);
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCE - ADC A, d8 */
bool ADC_A_d8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            ADC(cpu().fetch_byte());
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCF - RST 1 */
bool RST_1::execute() {
    return RST(1);
}

/* 0xD0 - RET NC */
bool RET_NC::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            return cpu().c();
        }

        case 2: {
            temp_u8 = POP();
            return false;
        }

        case 3: {
            temp_u16 = uint16_t(POP() << 8) | temp_u8;
            return false;
        }

        case 4: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xD1 - POP DE */
bool POP_DE::execute() {
    return POP(cpu().D, cpu().E);
}

/* 0xD2 - JP NC, a16 */
bool JP_NC_a16::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t(cpu().fetch_byte() << 8) | temp_u8;
            return cpu().c();
        }

        case 3: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xD4 - CALL NC, a16 */
bool CALL_NC_a16::execute() {
    switch (next_cycle()) {
        case 0:
        case 3: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((cpu().fetch_byte() << 8)) | temp_u8;
            return cpu().c();
        }

        case 4: {
            PUSH(cpu().PC >> 8);
            return false;
        }

        case 5: {
            PUSH(cpu().PC & 0xFF);
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xD5 - PUSH DE */
bool PUSH_DE::execute() {
    return PUSH(cpu().D, cpu().E);
}

/* 0xD6 - SUB A, d8 */
bool SUB_A_d8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            SUB(cpu().fetch_byte());
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xD7 - RST 2 */
bool RST_2::execute() {
    return RST(2);
}

/* 0xD8 - RET C - If the C flag is 1, control is returned to the source program by popping from the memory stack the program counter PC value that was pushed to the stack when the subroutine was called. */
bool RET_C::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            return not cpu().c();
        }

        case 2: {
            temp_u8 = POP();
            return false;
        }

        case 3: {
            temp_u16 = uint16_t((POP() << 8)) | temp_u8;
            return false;
        }

        case 4: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xD9 - RETI - Used when an interrupt-service routine finishes. The address for the return from the interrupt is loaded in the program counter PC. The master interrupt enable flag is returned to its pre-interrupt status.

The contents of the address specified by the stack pointer SP are loaded in the lower-order byte of PC, and the contents of SP are incremented by 1. The contents of the address specified by the new SP value are then loaded in the higher-order byte of PC, and the contents of SP are incremented by 1 again. (THe value of SP is 2 larger than before instruction execution.) The next instruction is fetched from the address specified by the content of PC (as usual). */
bool RETI::execute() {
    switch (next_cycle()) {
        case 0: {
            cpu().IME = true; // as far as I can tell RETI does not have a one-instruction delay like EI
            return false;
        }

        case 1: {
            temp_u8 = POP();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((POP() << 8)) | temp_u8;
            return false;
        }

        case 3: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCA - JP C, a16 */
bool JP_C_a16::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((cpu().fetch_byte() << 8)) | temp_u8;
            return not cpu().c();
        }

        case 3: {
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCC - CALL C, a16 */
bool CALL_C_a16::execute() {
    switch (next_cycle()) {
        case 0:
        case 3: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t((cpu().fetch_byte() << 8)) | temp_u8;
            return not cpu().c();
        }

        case 4: {
            PUSH(cpu().PC >> 8);
            return false;
        }

        case 5: {
            PUSH(cpu().PC & 0xFF);
            cpu().PC = temp_u16;
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xDE - SBC A, d8 */
bool SBC_A_d8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            SBC(cpu().fetch_byte());
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xDF - RST 3 */
bool RST_3::execute() {
    return RST(3);
}

/* 0xE0 - LD [a8] A */
bool LD_$a8$_A::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            mem().write_memory(0xFF00 | temp_u8, cpu().A);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xE1 - POP HL */
bool POP_HL::execute() {
    return POP(cpu().H, cpu().L);
}

/* 0xE2 - LD [C] A */
bool LD_$C$_A::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            mem().write_memory(0xFF00 | cpu().C, cpu().A);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xE5 - PUSH HL */
bool PUSH_HL::execute() {
    return PUSH(cpu().H, cpu().L);
}

/* 0xE6 - AND A, d8 */
bool AND_A_d8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            AND(cpu().fetch_byte());
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xE7 - RST 4 */
bool RST_4::execute() {
    return RST(4);
}

/* 0xE8 - ADD SP, s8 */
bool ADD_SP_s8::execute() {
    switch (next_cycle()) {
        case 0:
        case 2: {
            return false;
        }
       
        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }
       
        case 3: {
            cpu().c((cpu().SP & 0xFF) > 0xFF - temp_u8);
            cpu().h((cpu().SP & 0xF) > 0x0F - (temp_u8 & 0x0F));
            cpu().z(false);
            cpu().n(false);
            cpu().SP += int8_t(temp_u8);
            return true;
        }
       
        default:
            std::unreachable();
    }
};

/* 0xE9 - JP HL */
bool JP_HL::execute() {
    cpu().PC = cpu().HL();
    return true;
}

/* 0xEA - LD [a16] A */
bool LD_$a16$_A::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t(cpu().fetch_byte() << 8) | temp_u8;
            return false;
        }

        case 3: {
            mem().write_memory(temp_u16, cpu().A);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xEE - XOR A, d8 */
bool XOR_A_d8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            XOR(cpu().fetch_byte());
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xEF - RST 5 */
bool RST_5::execute() {
    return RST(5);
}

/* 0xF0 - LD A [a8] */
bool LD_A_$a8$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            cpu().A = mem().read_memory(0xFF00 | temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xF1 - POP AF */
bool POP_AF::execute() {
    bool res = POP(cpu().A, cpu().F);
    if (cycle() == 2)
        cpu().F &= 0xF0;    // bits 3-0 are not writeable
    return res;
}

/* 0xF2 - LD A [C] */
bool LD_A_$C$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            cpu().A = mem().read_memory(0xFF00 | cpu().C);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xF3 - DI - Reset the interrupt master enable (IME) flag and prohibit maskable interrupts.

Even if a DI instruction is executed in an interrupt routine, the IME flag is set if a return is performed with a RETI instruction. */
bool DI::execute() {
    cpu().IME = false;
    cpu().EI_executed = false;
    cpu().set_IME = false;
    return true;
}

/* 0xF5 - PUSH AF */
bool PUSH_AF::execute() {
    return PUSH(cpu().A, cpu().F);
}

/* 0xF6 - OR A, d8 */
bool OR_A_d8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            OR(cpu().fetch_byte());
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xF7 - RST 6 */
bool RST_6::execute() {
    return RST(6);
}

/* 0xF8 - LD HL, SP+s8 */
bool LD_HL_SPs8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            cpu().c((cpu().SP & 0xFF) > 0xFF - temp_u8);
            cpu().h((cpu().SP & 0xF) > 0x0F - (temp_u8 & 0x0F));
            cpu().n(false);
            cpu().z(false);
            cpu().HL(uint16_t(cpu().SP + int8_t(temp_u8)));
            return true;
        }

        default:
            std::unreachable();
    }
};

/* 0xF9 - LD SP, HL */
bool LD_SP_HL::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            cpu().SP = cpu().HL();
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xFA - LD A [a16] */
bool LD_A_$a16$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = cpu().fetch_byte();
            return false;
        }

        case 2: {
            temp_u16 = uint16_t(cpu().fetch_byte() << 8) | temp_u8;
            return false;
        }

        case 3: {
            cpu().A = mem().read_memory(temp_u16);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xFB - EI - Set the interrupt master enable (IME) flag and enable maskable interrupts. This instruction can be used in an interrupt routine to enable higher-order interrupts.

The IME flag is reset immediately after an interrupt occurs. The IME flag reset remains in effect if coontrol is returned from the interrupt routine by a RET instruction. However, if an EI instruction is executed in the interrupt routine, control is returned with IME = 1. */
bool EI::execute() {
    cpu().EI_executed = true;
    return true;
}

/* 0xFE - CP A, d8 */
bool CP_A_d8::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            CP(cpu().fetch_byte());
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xFF - RST 7 */
bool RST_7::execute() {
    return RST(7);
}

//=====================================================================================//
//  16-bit opcodes                                                                     //
//=====================================================================================//

/* 0xCB00 - RLC B */
bool RLC_B::execute() {
    return RLC(cpu().B);
}

/* 0xCB01 - RLC C */
bool RLC_C::execute() {
    return RLC(cpu().C);
}

/* 0xCB02 - RLC D */
bool RLC_D::execute() {
    return RLC(cpu().D);
}

/* 0xCB03 - RLC E */
bool RLC_E::execute() {
    return RLC(cpu().E);
}

/* 0xCB04 - RLC H */
bool RLC_H::execute() {
    return RLC(cpu().H);
}

/* 0xCB05 - RLC L */
bool RLC_L::execute() {
    return RLC(cpu().L);
}

/* 0xCB06 - RLC [HL] */
bool RLC_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RLC(temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB07 - RLC A */
bool RLC_A::execute() {
    return RLC(cpu().A);
}

/* 0xCB08 - RRC B */
bool RRC_B::execute() {
    return RRC(cpu().B);
}

/* 0xCB09 - RRC C */
bool RRC_C::execute() {
    return RRC(cpu().C);
}

/* 0xCB0A - RRC D */
bool RRC_D::execute() {
    return RRC(cpu().D);
}

/* 0xCB0B - RRC E */
bool RRC_E::execute() {
    return RRC(cpu().E);
}

/* 0xCB0C - RRC H */
bool RRC_H::execute() {
    return RRC(cpu().H);
}

/* 0xCB0D - RRC L */
bool RRC_L::execute() {
    return RRC(cpu().L);
}

/* 0xCB0E - RRC [HL] */
bool RRC_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RRC(temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB0F - RRC A */
bool RRC_A::execute() {
    return RRC(cpu().A);
}

/* 0xCB10 - RL B */
bool RL_B::execute() {
    return RL(cpu().B);
}

/* 0xCB11 - RL C */
bool RL_C::execute() {
    return RL(cpu().C);
}

/* 0xCB12 - RL D */
bool RL_D::execute() {
    return RL(cpu().D);
}

/* 0xCB13 - RL E */
bool RL_E::execute() {
    return RL(cpu().E);
}

/* 0xCB14 - RL H */
bool RL_H::execute() {
    return RL(cpu().H);
}

/* 0xCB15 - RL L */
bool RL_L::execute() {
    return RL(cpu().L);
}

/* 0xCB16 - RL [HL] */
bool RL_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RL(temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB17 - RL A */
bool RL_A::execute() {
    return RL(cpu().A);
}

/* 0xCB18 - RR B */
bool RR_B::execute() {
    return RR(cpu().B);
}

/* 0xCB19 - RR C */
bool RR_C::execute() {
    return RR(cpu().C);
}

/* 0xCB1A - RR D */
bool RR_D::execute() {
    return RR(cpu().D);
}

/* 0xCB1B - RR E */
bool RR_E::execute() {
    return RR(cpu().E);
}

/* 0xCB1C - RR H */
bool RR_H::execute() {
    return RR(cpu().H);
}

/* 0xCB1D - RR L */
bool RR_L::execute() {
    return RR(cpu().L);
}

/* 0xCB1E - RR [HL] */
bool RR_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RR(temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB1F - RR A */
bool RR_A::execute() {
    return RR(cpu().A);
}

/* 0xCB20 - SLA B */
bool SLA_B::execute() {
    return SLA(cpu().B);
}

/* 0xCB21 - SLA C */
bool SLA_C::execute() {
    return SLA(cpu().C);
}

/* 0xCB22 - SLA D */
bool SLA_D::execute() {
    return SLA(cpu().D);
}

/* 0xCB23 - SLA E */
bool SLA_E::execute() {
    return SLA(cpu().E);
}

/* 0xCB24 - SLA H */
bool SLA_H::execute() {
    return SLA(cpu().H);
}

/* 0xCB25 - SLA L */
bool SLA_L::execute() {
    return SLA(cpu().L);
}

/* 0xCB26 - SLA [HL] */
bool SLA_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SLA(temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB27 - SLA A */
bool SLA_A::execute() {
    return SLA(cpu().A);
}

/* 0xCB28 - SRA B */
bool SRA_B::execute() {
    return SRA(cpu().B);
}

/* 0xCB29 - SRA C */
bool SRA_C::execute() {
    return SRA(cpu().C);
}

/* 0xCB2A - SRA D */
bool SRA_D::execute() {
    return SRA(cpu().D);
}

/* 0xCB2B - SRA E */
bool SRA_E::execute() {
    return SRA(cpu().E);
}

/* 0xCB2C - SRA H */
bool SRA_H::execute() {
    return SRA(cpu().H);
}

/* 0xCB2D - SRA L */
bool SRA_L::execute() {
    return SRA(cpu().L);
}

/* 0xCB2E - SRA [HL] */
bool SRA_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SRA(temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB2F - SRA A */
bool SRA_A::execute() {
    return SRA(cpu().A);
}

/* 0xCB30 - SWAP B */
bool SWAP_B::execute() {
    return SWAP(cpu().B);
}

/* 0xCB31 - SWAP C */
bool SWAP_C::execute() {
    return SWAP(cpu().C);
}

/* 0xCB32 - SWAP D */
bool SWAP_D::execute() {
    return SWAP(cpu().D);
}

/* 0xCB33 - SWAP E */
bool SWAP_E::execute() {
    return SWAP(cpu().E);
}

/* 0xCB34 - SWAP H */
bool SWAP_H::execute() {
    return SWAP(cpu().H);
}

/* 0xCB35 - SWAP L */
bool SWAP_L::execute() {
    return SWAP(cpu().L);
}

/* 0xCB36 - SWAP [HL] */
bool SWAP_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SWAP(temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB37 - SWAP A */
bool SWAP_A::execute() {
    return SWAP(cpu().A);
}

/* 0xCB38 - SRL B */
bool SRL_B::execute() {
    return SRL(cpu().B);
}

/* 0xCB39 - SRL C */
bool SRL_C::execute() {
    return SRL(cpu().C);
}

/* 0xCB3A - SRL D */
bool SRL_D::execute() {
    return SRL(cpu().D);
}

/* 0xCB3B - SRL E */
bool SRL_E::execute() {
    return SRL(cpu().E);
}

/* 0xCB3C - SRL H */
bool SRL_H::execute() {
    return SRL(cpu().H);
}

/* 0xCB3D - SRL L */
bool SRL_L::execute() {
    return SRL(cpu().L);
}

/* 0xCB3E - SRL [HL] */
bool SRL_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SRL(temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB3F - SRL A */
bool SRL_A::execute() {
    return SRL(cpu().A);
}

/* 0xCB40 - BIT 0 B */
bool BIT_0_B::execute() {
    return BIT(0, cpu().B);
}

/* 0xCB41 - BIT 0 C */
bool BIT_0_C::execute() {
    return BIT(0, cpu().C);
}

/* 0xCB42 - BIT 0 D */
bool BIT_0_D::execute() {
    return BIT(0, cpu().D);
}

/* 0xCB43 - BIT 0 E */
bool BIT_0_E::execute() {
    return BIT(0, cpu().E);
}

/* 0xCB44 - BIT 0 H */
bool BIT_0_H::execute() {
    return BIT(0, cpu().H);
}

/* 0xCB45 - BIT 0 L */
bool BIT_0_L::execute() {
    return BIT(0, cpu().L);
}

/* 0xCB46 - BIT 0 [HL] */
bool BIT_0_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            BIT(0, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB47 - BIT 0 A */
bool BIT_0_A::execute() {
    return BIT(0, cpu().A);
}

/* 0xCB48 - BIT 1 B */
bool BIT_1_B::execute() {
    return BIT(1, cpu().B);
}

/* 0xCB49 - BIT 1 C */
bool BIT_1_C::execute() {
    return BIT(1, cpu().C);
}

/* 0xCB4A - BIT 1 D */
bool BIT_1_D::execute() {
    return BIT(1, cpu().D);
}

/* 0xCB4B - BIT 1 E */
bool BIT_1_E::execute() {
    return BIT(1, cpu().E);
}

/* 0xCB4C - BIT 1 H */
bool BIT_1_H::execute() {
    return BIT(1, cpu().H);
}

/* 0xCB4D - BIT 1 L */
bool BIT_1_L::execute() {
    return BIT(1, cpu().L);
}

/* 0xCB4E - BIT 1 [HL] */
bool BIT_1_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            BIT(1, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB4F - BIT 1 A */
bool BIT_1_A::execute() {
    return BIT(1, cpu().A);
}

/* 0xCB50 - BIT 2 B */
bool BIT_2_B::execute() {
    return BIT(2, cpu().B);
}

/* 0xCB51 - BIT 2 C */
bool BIT_2_C::execute() {
    return BIT(2, cpu().C);
}

/* 0xCB52 - BIT 2 D */
bool BIT_2_D::execute() {
    return BIT(2, cpu().D);
}

/* 0xCB53 - BIT 2 E */
bool BIT_2_E::execute() {
    return BIT(2, cpu().E);
}

/* 0xCB54 - BIT 2 H */
bool BIT_2_H::execute() {
    return BIT(2, cpu().H);
}

/* 0xCB55 - BIT 2 L */
bool BIT_2_L::execute() {
    return BIT(2, cpu().L);
}

/* 0xCB56 - BIT 2 [HL] */
bool BIT_2_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            BIT(2, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB57 - BIT 2 A */
bool BIT_2_A::execute() {
    return BIT(2, cpu().A);
}

/* 0xCB58 - BIT 3 B */
bool BIT_3_B::execute() {
    return BIT(3, cpu().B);
}

/* 0xCB59 - BIT 3 C */
bool BIT_3_C::execute() {
    return BIT(3, cpu().C);
}

/* 0xCB5A - BIT 3 D */
bool BIT_3_D::execute() {
    return BIT(3, cpu().D);
}

/* 0xCB5B - BIT 3 E */
bool BIT_3_E::execute() {
    return BIT(3, cpu().E);
}

/* 0xCB5C - BIT 3 H */
bool BIT_3_H::execute() {
    return BIT(3, cpu().H);
}

/* 0xCB5D - BIT 3 L */
bool BIT_3_L::execute() {
    return BIT(3, cpu().L);
}

/* 0xCB5E - BIT 3 [HL] */
bool BIT_3_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            BIT(3, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB5F - BIT 3 A */
bool BIT_3_A::execute() {
    return BIT(3, cpu().A);
}

/* 0xCB60 - BIT 4 B */
bool BIT_4_B::execute() {
    return BIT(4, cpu().B);
}

/* 0xCB61 - BIT 4 C */
bool BIT_4_C::execute() {
    return BIT(4, cpu().C);
}

/* 0xCB62 - BIT 4 D */
bool BIT_4_D::execute() {
    return BIT(4, cpu().D);
}

/* 0xCB63 - BIT 4 E */
bool BIT_4_E::execute() {
    return BIT(4, cpu().E);
}

/* 0xCB64 - BIT 4 H */
bool BIT_4_H::execute() {
    return BIT(4, cpu().H);
}

/* 0xCB65 - BIT 4 L */
bool BIT_4_L::execute() {
    return BIT(4, cpu().L);
}

/* 0xCB66 - BIT 4 [HL] */
bool BIT_4_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            BIT(4, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB67 - BIT 4 A */
bool BIT_4_A::execute() {
    return BIT(4, cpu().A);
}

/* 0xCB68 - BIT 5 B */
bool BIT_5_B::execute() {
    return BIT(5, cpu().B);
}

/* 0xCB69 - BIT 5 C */
bool BIT_5_C::execute() {
    return BIT(5, cpu().C);
}

/* 0xCB6A - BIT 5 D */
bool BIT_5_D::execute() {
    return BIT(5, cpu().D);
}

/* 0xCB6B - BIT 5 E */
bool BIT_5_E::execute() {
    return BIT(5, cpu().E);
}

/* 0xCB6C - BIT 5 H */
bool BIT_5_H::execute() {
    return BIT(5, cpu().H);
}

/* 0xCB6D - BIT 5 L */
bool BIT_5_L::execute() {
    return BIT(5, cpu().L);
}

/* 0xCB6E - BIT 5 [HL] */
bool BIT_5_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            BIT(5, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB6F - BIT 5 A */
bool BIT_5_A::execute() {
    return BIT(5, cpu().A);
}

/* 0xCB70 - BIT 6 B */
bool BIT_6_B::execute() {
    return BIT(6, cpu().B);
}

/* 0xCB71 - BIT 6 C */
bool BIT_6_C::execute() {
    return BIT(6, cpu().C);
}

/* 0xCB72 - BIT 6 D */
bool BIT_6_D::execute() {
    return BIT(6, cpu().D);
}

/* 0xCB73 - BIT 6 E */
bool BIT_6_E::execute() {
    return BIT(6, cpu().E);
}

/* 0xCB74 - BIT 6 H */
bool BIT_6_H::execute() {
    return BIT(6, cpu().H);
}

/* 0xCB75 - BIT 6 L */
bool BIT_6_L::execute() {
    return BIT(6, cpu().L);
}

/* 0xCB76 - BIT 6 [HL] */
bool BIT_6_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            BIT(6, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB77 - BIT 6 A */
bool BIT_6_A::execute() {
    return BIT(6, cpu().A);
}

/* 0xCB78 - BIT 7 B */
bool BIT_7_B::execute() {
    return BIT(7, cpu().B);
}

/* 0xCB79 - BIT 7 C */
bool BIT_7_C::execute() {
    return BIT(7, cpu().C);
}

/* 0xCB7A - BIT 7 D */
bool BIT_7_D::execute() {
    return BIT(7, cpu().D);
}

/* 0xCB7B - BIT 7 E */
bool BIT_7_E::execute() {
    return BIT(7, cpu().E);
}

/* 0xCB7C - BIT 7 H */
bool BIT_7_H::execute() {
    return BIT(7, cpu().H);
}

/* 0xCB7D - BIT 7 L */
bool BIT_7_L::execute() {
    return BIT(7, cpu().L);
}

/* 0xCB7E - BIT 7 [HL] */
bool BIT_7_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            BIT(7, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB7F - BIT 7 A */
bool BIT_7_A::execute() {
    return BIT(7, cpu().A);
}

/* 0xCB80 - RES 0 B */
bool RES_0_B::execute() {
    return RES(0, cpu().B);
}

/* 0xCB81 - RES 0 C */
bool RES_0_C::execute() {
    return RES(0, cpu().C);
}

/* 0xCB82 - RES 0 D */
bool RES_0_D::execute() {
    return RES(0, cpu().D);
}

/* 0xCB83 - RES 0 E */
bool RES_0_E::execute() {
    return RES(0, cpu().E);
}

/* 0xCB84 - RES 0 H */
bool RES_0_H::execute() {
    return RES(0, cpu().H);
}

/* 0xCB85 - RES 0 L */
bool RES_0_L::execute() {
    return RES(0, cpu().L);
}

/* 0xCB86 - RES 0 [HL] */
bool RES_0_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RES(0, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB87 - RES 0 A */
bool RES_0_A::execute() {
    return RES(0, cpu().A);
}

/* 0xCB88 - RES 1 B */
bool RES_1_B::execute() {
    return RES(1, cpu().B);
}

/* 0xCB89 - RES 1 C */
bool RES_1_C::execute() {
    return RES(1, cpu().C);
}

/* 0xCB8A - RES 1 D */
bool RES_1_D::execute() {
    return RES(1, cpu().D);
}

/* 0xCB8B - RES 1 E */
bool RES_1_E::execute() {
    return RES(1, cpu().E);
}

/* 0xCB8C - RES 1 H */
bool RES_1_H::execute() {
    return RES(1, cpu().H);
}

/* 0xCB8D - RES 1 L */
bool RES_1_L::execute() {
    return RES(1, cpu().L);
}

/* 0xCB8E - RES 1 [HL] */
bool RES_1_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RES(1, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB8F - RES 1 A */
bool RES_1_A::execute() {
    return RES(1, cpu().A);
}

/* 0xCB90 - RES 2 B */
bool RES_2_B::execute() {
    return RES(2, cpu().B);
}

/* 0xCB91 - RES 2 C */
bool RES_2_C::execute() {
    return RES(2, cpu().C);
}

/* 0xCB92 - RES 2 D */
bool RES_2_D::execute() {
    return RES(2, cpu().D);
}

/* 0xCB93 - RES 2 E */
bool RES_2_E::execute() {
    return RES(2, cpu().E);
}

/* 0xCB94 - RES 2 H */
bool RES_2_H::execute() {
    return RES(2, cpu().H);
}

/* 0xCB95 - RES 2 L */
bool RES_2_L::execute() {
    return RES(2, cpu().L);
}

/* 0xCB96 - RES 2 [HL] */
bool RES_2_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RES(2, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB97 - RES 2 A */
bool RES_2_A::execute() {
    return RES(2, cpu().A);
}

/* 0xCB98 - RES 3 B */
bool RES_3_B::execute() {
    return RES(3, cpu().B);
}

/* 0xCB99 - RES 3 C */
bool RES_3_C::execute() {
    return RES(3, cpu().C);
}

/* 0xCB9A - RES 3 D */
bool RES_3_D::execute() {
    return RES(3, cpu().D);
}

/* 0xCB9B - RES 3 E */
bool RES_3_E::execute() {
    return RES(3, cpu().E);
}

/* 0xCB9C - RES 3 H */
bool RES_3_H::execute() {
    return RES(3, cpu().H);
}

/* 0xCB9D - RES 3 L */
bool RES_3_L::execute() {
    return RES(3, cpu().L);
}

/* 0xCB9E - RES 3 [HL] */
bool RES_3_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RES(3, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCB9F - RES 3 A */
bool RES_3_A::execute() {
    return RES(3, cpu().A);
}

/* 0xCBA0 - RES 4 B */
bool RES_4_B::execute() {
    return RES(4, cpu().B);
}

/* 0xCBA1 - RES 4 C */
bool RES_4_C::execute() {
    return RES(4, cpu().C);
}

/* 0xCBA2 - RES 4 D */
bool RES_4_D::execute() {
    return RES(4, cpu().D);
}

/* 0xCBA3 - RES 4 E */
bool RES_4_E::execute() {
    return RES(4, cpu().E);
}

/* 0xCBA4 - RES 4 H */
bool RES_4_H::execute() {
    return RES(4, cpu().H);
}

/* 0xCBA5 - RES 4 L */
bool RES_4_L::execute() {
    return RES(4, cpu().L);
}

/* 0xCBA6 - RES 4 [HL] */
bool RES_4_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RES(4, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBA7 - RES 4 A */
bool RES_4_A::execute() {
    return RES(4, cpu().A);
}

/* 0xCBA8 - RES 5 B */
bool RES_5_B::execute() {
    return RES(5, cpu().B);
}

/* 0xCBA9 - RES 5 C */
bool RES_5_C::execute() {
    return RES(5, cpu().C);
}

/* 0xCBAA - RES 5 D */
bool RES_5_D::execute() {
    return RES(5, cpu().D);
}

/* 0xCBAB - RES 5 E */
bool RES_5_E::execute() {
    return RES(5, cpu().E);
}

/* 0xCBAC - RES 5 H */
bool RES_5_H::execute() {
    return RES(5, cpu().H);
}

/* 0xCBAD - RES 5 L */
bool RES_5_L::execute() {
    return RES(5, cpu().L);
}

/* 0xCBAE - RES 5 [HL] */
bool RES_5_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RES(5, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBAF - RES 5 A */
bool RES_5_A::execute() {
    return RES(5, cpu().A);
}

/* 0xCBB0 - RES 6 B */
bool RES_6_B::execute() {
    return RES(6, cpu().B);
}

/* 0xCBB1 - RES 6 C */
bool RES_6_C::execute() {
    return RES(6, cpu().C);
}

/* 0xCBB2 - RES 6 D */
bool RES_6_D::execute() {
    return RES(6, cpu().D);
}

/* 0xCBB3 - RES 6 E */
bool RES_6_E::execute() {
    return RES(6, cpu().E);
}

/* 0xCBB4 - RES 6 H */
bool RES_6_H::execute() {
    return RES(6, cpu().H);
}

/* 0xCBB5 - RES 6 L */
bool RES_6_L::execute() {
    return RES(6, cpu().L);
}

/* 0xCBB6 - RES 6 [HL] */
bool RES_6_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RES(6, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBB7 - RES 6 A */
bool RES_6_A::execute() {
    return RES(6, cpu().A);
}

/* 0xCBB8 - RES 7 B */
bool RES_7_B::execute() {
    return RES(7, cpu().B);
}

/* 0xCBB9 - RES 7 C */
bool RES_7_C::execute() {
    return RES(7, cpu().C);
}

/* 0xCBBA - RES 7 D */
bool RES_7_D::execute() {
    return RES(7, cpu().D);
}

/* 0xCBBB - RES 7 E */
bool RES_7_E::execute() {
    return RES(7, cpu().E);
}

/* 0xCBBC - RES 7 H */
bool RES_7_H::execute() {
    return RES(7, cpu().H);
}

/* 0xCBBD - RES 7 L */
bool RES_7_L::execute() {
    return RES(7, cpu().L);
}

/* 0xCBBE - RES 7 [HL] */
bool RES_7_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            RES(7, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBBF - RES 7 A */
bool RES_7_A::execute() {
    return RES(7, cpu().A);
}

/* 0xCBC0 - SET 0 B */
bool SET_0_B::execute() {
    return SET(0, cpu().B);
}

/* 0xCBC1 - SET 0 C */
bool SET_0_C::execute() {
    return SET(0, cpu().C);
}

/* 0xCBC2 - SET 0 D */
bool SET_0_D::execute() {
    return SET(0, cpu().D);
}

/* 0xCBC3 - SET 0 E */
bool SET_0_E::execute() {
    return SET(0, cpu().E);
}

/* 0xCBC4 - SET 0 H */
bool SET_0_H::execute() {
    return SET(0, cpu().H);
}

/* 0xCBC5 - SET 0 L */
bool SET_0_L::execute() {
    return SET(0, cpu().L);
}

/* 0xCBC6 - SET 0 [HL] */
bool SET_0_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SET(0, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBC7 - SET 0 A */
bool SET_0_A::execute() {
    return SET(0, cpu().A);
}

/* 0xCBC8 - SET 1 B */
bool SET_1_B::execute() {
    return SET(1, cpu().B);
}

/* 0xCBC9 - SET 1 C */
bool SET_1_C::execute() {
    return SET(1, cpu().C);
}

/* 0xCBCA - SET 1 D */
bool SET_1_D::execute() {
    return SET(1, cpu().D);
}

/* 0xCBCB - SET 1 E */
bool SET_1_E::execute() {
    return SET(1, cpu().E);
}

/* 0xCBCC - SET 1 H */
bool SET_1_H::execute() {
    return SET(1, cpu().H);
}

/* 0xCBCD - SET 1 L */
bool SET_1_L::execute() {
    return SET(1, cpu().L);
}

/* 0xCBCE - SET 1 [HL] */
bool SET_1_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SET(1, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBCF - SET 1 A */
bool SET_1_A::execute() {
    return SET(1, cpu().A);
}

/* 0xCBD0 - SET 2 B */
bool SET_2_B::execute() {
    return SET(2, cpu().B);
}

/* 0xCBD1 - SET 2 C */
bool SET_2_C::execute() {
    return SET(2, cpu().C);
}

/* 0xCBD2 - SET 2 D */
bool SET_2_D::execute() {
    return SET(2, cpu().D);
}

/* 0xCBD3 - SET 2 E */
bool SET_2_E::execute() {
    return SET(2, cpu().E);
}

/* 0xCBD4 - SET 2 H */
bool SET_2_H::execute() {
    return SET(2, cpu().H);
}

/* 0xCBD5 - SET 2 L */
bool SET_2_L::execute() {
    return SET(2, cpu().L);
}

/* 0xCBD6 - SET 2 [HL] */
bool SET_2_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SET(2, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBD7 - SET 2 A */
bool SET_2_A::execute() {
    return SET(2, cpu().A);
}

/* 0xCBD8 - SET 3 B */
bool SET_3_B::execute() {
    return SET(3, cpu().B);
}

/* 0xCBD9 - SET 3 C */
bool SET_3_C::execute() {
    return SET(3, cpu().C);
}

/* 0xCBDA - SET 3 D */
bool SET_3_D::execute() {
    return SET(3, cpu().D);
}

/* 0xCBDB - SET 3 E */
bool SET_3_E::execute() {
    return SET(3, cpu().E);
}

/* 0xCBDC - SET 3 H */
bool SET_3_H::execute() {
    return SET(3, cpu().H);
}

/* 0xCBDD - SET 3 L */
bool SET_3_L::execute() {
    return SET(3, cpu().L);
}

/* 0xCBDE - SET 3 [HL] */
bool SET_3_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SET(3, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBDF - SET 3 A */
bool SET_3_A::execute() {
    return SET(3, cpu().A);
}

/* 0xCBE0 - SET 4 B */
bool SET_4_B::execute() {
    return SET(4, cpu().B);
}

/* 0xCBE1 - SET 4 C */
bool SET_4_C::execute() {
    return SET(4, cpu().C);
}

/* 0xCBE2 - SET 4 D */
bool SET_4_D::execute() {
    return SET(4, cpu().D);
}

/* 0xCBE3 - SET 4 E */
bool SET_4_E::execute() {
    return SET(4, cpu().E);
}

/* 0xCBE4 - SET 4 H */
bool SET_4_H::execute() {
    return SET(4, cpu().H);
}

/* 0xCBE5 - SET 4 L */
bool SET_4_L::execute() {
    return SET(4, cpu().L);
}

/* 0xCBE6 - SET 4 [HL] */
bool SET_4_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SET(4, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBE7 - SET 4 A */
bool SET_4_A::execute() {
    return SET(4, cpu().A);
}

/* 0xCBE8 - SET 5 B */
bool SET_5_B::execute() {
    return SET(5, cpu().B);
}

/* 0xCBE9 - SET 5 C */
bool SET_5_C::execute() {
    return SET(5, cpu().C);
}

/* 0xCBEA - SET 5 D */
bool SET_5_D::execute() {
    return SET(5, cpu().D);
}

/* 0xCBEB - SET 5 E */
bool SET_5_E::execute() {
    return SET(5, cpu().E);
}

/* 0xCBEC - SET 5 H */
bool SET_5_H::execute() {
    return SET(5, cpu().H);
}

/* 0xCBED - SET 5 L */
bool SET_5_L::execute() {
    return SET(5, cpu().L);
}

/* 0xCBEE - SET 5 [HL] */
bool SET_5_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SET(5, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBEF - SET 5 A */
bool SET_5_A::execute() {
    return SET(5, cpu().A);
}

/* 0xCBF0 - SET 6 B */
bool SET_6_B::execute() {
    return SET(6, cpu().B);
}

/* 0xCBF1 - SET 6 C */
bool SET_6_C::execute() {
    return SET(6, cpu().C);
}

/* 0xCBF2 - SET 6 D */
bool SET_6_D::execute() {
    return SET(6, cpu().D);
}

/* 0xCBF3 - SET 6 E */
bool SET_6_E::execute() {
    return SET(6, cpu().E);
}

/* 0xCBF4 - SET 6 H */
bool SET_6_H::execute() {
    return SET(6, cpu().H);
}

/* 0xCBF5 - SET 6 L */
bool SET_6_L::execute() {
    return SET(6, cpu().L);
}

/* 0xCBF6 - SET 6 [HL] */
bool SET_6_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SET(6, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBF7 - SET 6 A */
bool SET_6_A::execute() {
    return SET(6, cpu().A);
}

/* 0xCBF8 - SET 7 B */
bool SET_7_B::execute() {
    return SET(7, cpu().B);
}

/* 0xCBF9 - SET 7 C */
bool SET_7_C::execute() {
    return SET(7, cpu().C);
}

/* 0xCBFA - SET 7 D */
bool SET_7_D::execute() {
    return SET(7, cpu().D);
}

/* 0xCBFB - SET 7 E */
bool SET_7_E::execute() {
    return SET(7, cpu().E);
}

/* 0xCBFC - SET 7 H */
bool SET_7_H::execute() {
    return SET(7, cpu().H);
}

/* 0xCBFD - SET 7 L */
bool SET_7_L::execute() {
    return SET(7, cpu().L);
}

/* 0xCBFE - SET 7 [HL] */
bool SET_7_$HL$::execute() {
    switch (next_cycle()) {
        case 0: {
            return false;
        }

        case 1: {
            temp_u8 = mem().read_memory(cpu().HL());
            return false;
        }

        case 2: {
            SET(7, temp_u8);
            mem().write_memory(cpu().HL(), temp_u8);
            return true;
        }

        default:
            std::unreachable();
    }
}

/* 0xCBFF - SET 7 A */
bool SET_7_A::execute() {
    return SET(7, cpu().A);
}