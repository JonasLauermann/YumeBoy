#include "cpu/CPU.hpp"

#include <iostream>
#include "YumeBoy.hpp"

uint8_t CPU::fetch_byte()
{
    uint8_t byte = mem_.read_memory(PC);
    ++PC;
    return byte;
}

void CPU::tick()
{
    /* Interrupt Handling */
    if (bool(state & CPU_STATES::Interruptable) and IME and (IE_ & IF_)) {
        assert(not (IF_ & 0xE0) and "Interrupt flag set for invalid bits");
        state = CPU_STATES::InterruptNOP1;
    }
    
    /* Exit HALT mode */
    if ((state == CPU_STATES::HaltMode) and (IE_ & IF_)) {
        state = CPU_STATES::FetchOpcode;
        if (HALT_bug) --PC;
    }

    switch (state)
    {
    case CPU_STATES::FetchOpcode:
    {
        uint8_t opcode = fetch_byte();
        if (opcode == 0xCB) {
            state = CPU_STATES::FetchExtOpcode;
            break;
        }
        
        instruction = Instruction::Get(opcode, false, *this, mem_);
        if (not instruction->execute()) {
            state = CPU_STATES::Execute;
        } else if (EI_executed and not set_IME) {
            set_IME = true;
        } else if (EI_executed and set_IME) {
            EI_executed = false;
            set_IME = false;
            IME = true;
        }
        break;
    }

    case CPU_STATES::FetchExtOpcode:
    {
        uint8_t opcode = fetch_byte();
        instruction = Instruction::Get(opcode, true, *this, mem_);
        if (not instruction->execute()) {
            state = CPU_STATES::Execute;
            break;
        } else if (set_IME) {
            assert (EI_executed);
            EI_executed = false;
            set_IME = false;
            IME = true;
        }
        state = CPU_STATES::FetchOpcode;
        break;
    }

    case CPU_STATES::Execute:
    {
        if (instruction->execute()) {
            state = CPU_STATES::FetchOpcode;
            if (set_IME) {
                assert (EI_executed);
                EI_executed = false;
                set_IME = false;
                IME = true;
            }
        }
        break;
    }

    case CPU_STATES::InterruptNOP1:
    {
        state = CPU_STATES::InterruptNOP2;
        break;
    }

    case CPU_STATES::InterruptNOP2:
    {
        state = CPU_STATES::InterruptPushPC1;
        break;
    }

    case CPU_STATES::InterruptPushPC1:
    {
        --SP;
        mem_.write_memory(SP, PC >> 8);
        state = CPU_STATES::InterruptPushPC2;
        break;
    }

    case CPU_STATES::InterruptPushPC2:
    {
        --SP;
        mem_.write_memory(SP, PC & 0xFF);
        state = CPU_STATES::InterruptSetPC;
        break;
    }

    case CPU_STATES::InterruptSetPC:
    {
        // if multiple interrupts were requested at the same time, handle lower bit interrupts first
        uint8_t req_interrupt = IE_ & IF_;
        uint8_t interrupt_bit = 0;
        while (not (req_interrupt & 0x1)) { req_interrupt = req_interrupt >> 1; interrupt_bit++; }

        // reset interrupt bit and disable interrupt handling
        assert(IF_ & (0x1 << interrupt_bit));
        IF_ ^= 0x1 << interrupt_bit;
        IME = false;
        
        // set PC to handler address.
        PC = 0x40 + (0x8 * interrupt_bit);

        state = CPU_STATES::FetchOpcode;
        break;
    }

    case CPU_STATES::HaltMode:
        HALT_bug = false;
    case CPU_STATES::StopMode:
        break;
    
    default:
        std::unreachable();
    }
}