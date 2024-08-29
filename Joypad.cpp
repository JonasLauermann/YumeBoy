#include "Joypad.hpp"

#include "YumeBoy.hpp"
#include "SDL_events.h"

uint8_t Joypad::P1()
{
    uint8_t inv_P1 = 0;
    inv_P1 |= (state_.select_dpad << 5) + (state_.select_buttons << 4);
    
    if (state_.select_dpad)
        inv_P1 |= (state_.down_dpad << 3) + (state_.up_dpad << 2) + (state_.left_dpad << 1) + state_.right_dpad;

    if (state_.select_buttons)
        inv_P1 |= (state_.start_button << 3) + (state_.select_button << 2) + (state_.b_button << 1) + state_.a_button;

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
        yume_boy_.request_interrupt(YumeBoy::INTERRUPT::JOYPAD_INTERRUPT);
}

void Joypad::update_joypad_state()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                uint8_t P1_ = P1();
                bool old_combined_input_lines = (P1_ & 0b1000) and (P1_ & 0b0100) and (P1_ & 0b0010) and (P1_ & 0b0001);

                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_Z:
                    state_.b_button = event.type == SDL_KEYDOWN;
                    break;
                
                case SDL_SCANCODE_X:
                    state_.a_button = event.type == SDL_KEYDOWN;
                    break;
                
                case SDL_SCANCODE_RETURN:
                    state_.start_button = event.type == SDL_KEYDOWN;
                    break;
                
                case SDL_SCANCODE_BACKSPACE:
                    state_.select_button = event.type == SDL_KEYDOWN;
                    break;
                
                case SDL_SCANCODE_DOWN:
                    state_.down_dpad = event.type == SDL_KEYDOWN;
                    break;
                
                case SDL_SCANCODE_UP:
                    state_.up_dpad = event.type == SDL_KEYDOWN;
                    break;
                
                case SDL_SCANCODE_LEFT:
                    state_.left_dpad = event.type == SDL_KEYDOWN;
                    break;
                
                case SDL_SCANCODE_RIGHT:
                    state_.right_dpad = event.type == SDL_KEYDOWN;
                    break;
                
                // DEBUG keys
                case SDL_SCANCODE_1:
                    if (event.type == SDL_KEYDOWN)
                        yume_boy_.dump_tilemap();
                    break;
                
                default:
                    break;
                }
                // Falling edge detector
                P1_ = P1();
                if (bool new_combined_input_lines = (P1_ & 0b1000) and (P1_ & 0b0100) and (P1_ & 0b0010) and (P1_ & 0b0001); old_combined_input_lines and not new_combined_input_lines)
                    yume_boy_.request_interrupt(YumeBoy::INTERRUPT::JOYPAD_INTERRUPT);
                break;
            }

            default:
                break;
        }
    }
}
