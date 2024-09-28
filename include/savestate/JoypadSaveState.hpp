#pragma once

#include <cstdint>
#include <vector>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>


/* Represents the state of a `Joypad` object. */
struct JoypadSaveState {
    bool select_buttons = false;
    bool select_dpad = false;

    bool start_button = false;
    bool select_button = false;
    bool b_button = false;
    bool a_button = false;

    bool down_dpad = false;
    bool up_dpad = false;
    bool left_dpad = false;
    bool right_dpad = false;

    private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, [[maybe_unused]] const unsigned int version)
    {
        ar & select_buttons;
        ar & select_dpad;

        ar & start_button;
        ar & select_button;
        ar & b_button;
        ar & a_button;

        ar & down_dpad;
        ar & up_dpad;
        ar & left_dpad;
        ar & right_dpad;
    }
};