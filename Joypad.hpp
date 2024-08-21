#pragma once

#include <cstdint>


class YumeBoy;

class Joypad {
    YumeBoy &yume_boy_;         // Reference to Emulator

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
    } state_;

    public:
    Joypad() = delete;
    Joypad(YumeBoy &yume_boy) : yume_boy_(yume_boy) { }


    /* 0xFF00 — P1/JOYP: Joypad
        The eight Game Boy action/direction buttons are arranged as a 2×4 matrix. Select either action or direction buttons by writing to this register, then read out the bits 0-3.
        Bit 5 - P15 Select Button Keys (0=Select)
        Bit 4 - P14 Select Direction Keys (0=Select)
        Bit 3 - P13 Input Down or Start (0=Pressed) (Read Only)
        Bit 2 - P12 Input Up or Select (0=Pressed) (Read Only)
        Bit 1 - P11 Input Left or Button B (0=Pressed) (Read Only)
        Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only)
        Bits 6 and 7 always return 1. */
    uint8_t P1();
    void P1(uint8_t value);

    /* Handles `SDL_Event`s and updates P1 accrodingly and requests Interrupts if necessary. */
    void update_joypad_state();

};