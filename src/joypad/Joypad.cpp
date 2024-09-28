#include "joypad/Joypad.hpp"

#include "YumeBoy.hpp"
#include "SDL3/SDL_events.h"
#include <savestate/JoypadSaveState.hpp>

uint8_t Joypad::P1() const
{
    uint8_t inv_P1 = 0;
    inv_P1 |= (uint8_t(state_.select_dpad) << 5) + (uint8_t(state_.select_buttons) << 4);
    
    if (state_.select_dpad)
        inv_P1 |= (uint8_t(state_.down_dpad) << 3) + (uint8_t(state_.up_dpad) << 2) + (uint8_t(state_.left_dpad) << 1) + uint8_t(state_.right_dpad);

    if (state_.select_buttons)
        inv_P1 |= (uint8_t(state_.start_button) << 3) + (uint8_t(state_.select_button) << 2) + (uint8_t(state_.b_button) << 1) + uint8_t(state_.a_button);

    return ~inv_P1;
}

void Joypad::P1(uint8_t value)
{
    uint8_t P1_ = P1();
    bool old_combined_input_lines = (P1_ & 0b1000) and (P1_ & 0b0100) and (P1_ & 0b0010) and (P1_ & 0b0001);

    // only Bit 5 and 6 are writable
    state_.select_dpad = value & (1 << 5);
    state_.select_buttons = value & (1 << 4);

    // Falling edge detector
    P1_ = P1();
    bool new_combined_input_lines = (P1_ & 0b1000) and (P1_ & 0b0100) and (P1_ & 0b0010) and (P1_ & 0b0001);
    if (old_combined_input_lines and not new_combined_input_lines)
        interrupts.request_interrupt(InterruptBus::INTERRUPT::JOYPAD_INTERRUPT);
}

void Joypad::update_joypad_state()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_KEY_DOWN: {
                
                switch (event.key.scancode)
                {
#ifndef NDEBUG
                case SDL_SCANCODE_1:
                    yume_boy_.dump_tilemap();
                    break;
                
                case SDL_SCANCODE_2:
                    yume_boy_.screenshot();
                    break;
#endif
                
                case SDL_SCANCODE_3:
                    yume_boy_.save_state();
                    break;
                
                case SDL_SCANCODE_4:
                    yume_boy_.load_state();
                    break;
                default:
                    break;
                }
            } // do not break in outer switch block
            case SDL_EVENT_KEY_UP: {
                uint8_t P1_ = P1();
                bool old_combined_input_lines = (P1_ & 0b1000) and (P1_ & 0b0100) and (P1_ & 0b0010) and (P1_ & 0b0001);

                switch (event.key.scancode)
                {
                case SDL_SCANCODE_Z:
                    state_.b_button = event.type == SDL_EVENT_KEY_DOWN;
                    break;
                
                case SDL_SCANCODE_X:
                    state_.a_button = event.type == SDL_EVENT_KEY_DOWN;
                    break;
                
                case SDL_SCANCODE_RETURN:
                    state_.start_button = event.type == SDL_EVENT_KEY_DOWN;
                    break;
                
                case SDL_SCANCODE_BACKSPACE:
                    state_.select_button = event.type == SDL_EVENT_KEY_DOWN;
                    break;
                
                case SDL_SCANCODE_DOWN:
                    state_.down_dpad = event.type == SDL_EVENT_KEY_DOWN;
                    break;
                
                case SDL_SCANCODE_UP:
                    state_.up_dpad = event.type == SDL_EVENT_KEY_DOWN;
                    break;
                
                case SDL_SCANCODE_LEFT:
                    state_.left_dpad = event.type == SDL_EVENT_KEY_DOWN;
                    break;
                
                case SDL_SCANCODE_RIGHT:
                    state_.right_dpad = event.type == SDL_EVENT_KEY_DOWN;
                    break;
                
                default:
                    break;
                }
                // Falling edge detector
                P1_ = P1();
                if (bool new_combined_input_lines = (P1_ & 0b1000) and (P1_ & 0b0100) and (P1_ & 0b0010) and (P1_ & 0b0001); old_combined_input_lines and not new_combined_input_lines)
                    interrupts.request_interrupt(InterruptBus::INTERRUPT::JOYPAD_INTERRUPT);
                break;
            }

            default:
                break;
        }
    }
}

JoypadSaveState Joypad::save_state() const {
    JoypadSaveState s = {
        state_.select_buttons,
        state_.select_dpad,
        
        state_.start_button,
        state_.select_button,
        state_.b_button,
        state_.a_button,
        
        state_.down_dpad,
        state_.up_dpad,
        state_.left_dpad,
        state_.right_dpad,
    };
    return s;
}

void Joypad::load_state(JoypadSaveState state)
{
        state_.select_buttons   = state.select_buttons;
        state_.select_dpad      = state.select_dpad;

        state_.start_button     = state.start_button;
        state_.select_button    = state.select_button;
        state_.b_button         = state.b_button;
        state_.a_button         = state.a_button;

        state_.down_dpad        = state.down_dpad;
        state_.up_dpad          = state.up_dpad;
        state_.left_dpad        = state.left_dpad;
        state_.right_dpad       = state.right_dpad;
}