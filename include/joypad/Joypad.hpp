#pragma once

#include <cstdint>
#include <cpu/InterruptBus.hpp>
#include <mmu/Memory.hpp>

class YumeBoy;
struct JoypadSaveState;

class Joypad : public Memory {
    YumeBoy &yume_boy_;
    InterruptBus &interrupts;

    struct JoypadState {
        // "selection" output pins
        bool select_buttons = false;    // not to be confused by the Select button `select_button`
        bool select_dpad = false;

        // button input pins
        bool start_button = false;
        bool select_button = false;
        bool b_button = false;
        bool a_button = false;

        // D-Pad input pins
        bool down_dpad = false;
        bool up_dpad = false;
        bool left_dpad = false;
        bool right_dpad = false;
    };
    JoypadState state_;

    /* 0xFF00 — P1/JOYP: Joypad
        The eight Game Boy action/direction buttons are arranged as a 2×4 matrix. Select either action or direction buttons by writing to this register, then read out the bits 0-3.
        Bit 5 - P15 Select Button Keys (0=Select)
        Bit 4 - P14 Select Direction Keys (0=Select)
        Bit 3 - P13 Input Down or Start (0=Pressed) (Read Only)
        Bit 2 - P12 Input Up or Select (0=Pressed) (Read Only)
        Bit 1 - P11 Input Left or Button B (0=Pressed) (Read Only)
        Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only)
        Bits 6 and 7 always return 1. */
    uint8_t P1() const;
    void P1(uint8_t value);

    public:
    Joypad() = delete;
    explicit Joypad(YumeBoy &yume_boy, InterruptBus &interrupts) : yume_boy_(yume_boy), interrupts(interrupts) { }

    bool contains_address(uint16_t addr) const override {
        return addr == 0xFF00;
    }

    uint8_t read_memory(uint16_t addr) override {
        assert(addr = 0xFF00);
        return P1();
    }

    void write_memory(uint16_t addr, uint8_t value) override {
        assert(addr = 0xFF00);
        P1(value);
    }

    /* Handles `SDL_Event`s and updates P1 accrodingly and requests Interrupts if necessary. */
    void update_joypad_state();

    JoypadSaveState save_state() const;
    void load_state(JoypadSaveState state);

};