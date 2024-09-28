#pragma once

#include <cstdint>
#include <vector>


/* Represents the state of a `Joypad` object. */
struct JoypadSaveState {
    const bool select_buttons = false;
    const bool select_dpad = false;

    const bool start_button = false;
    const bool select_button = false;
    const bool b_button = false;
    const bool a_button = false;

    const bool down_dpad = false;
    const bool up_dpad = false;
    const bool left_dpad = false;
    const bool right_dpad = false;
};