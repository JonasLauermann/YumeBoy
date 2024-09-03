#pragma once

#include <array>
#include <cstdint>
#include <memory>

class YumeBoy;

class APU {
    YumeBoy &yume_boy_; // Reference to Emulator
    uint32_t time_;     // the amount of time the apu has been running for (in T-cycles / 2^22 Hz)

    /* Registers
       From https://gbdev.io/pandocs/Audio_Registers.html:
       Audio registers are named following a NRxy scheme, where x is the channel number (or 5 for “global” registers), and y is the register’s ID within the channel. Since many registers share common properties, a notation is often used where e.g. NRx2 is used to designate NR12, NR22, NR32, and NR42 at the same time, for simplicity. */

    /* Sound Channel 1 — Pulse with period sweep */
    /* 0xFF10 — NR10: Channel 1 sweep */
    uint8_t NR10 = 0x0;
    /* 0xFF11 — NR11: Channel 1 length timer & duty cycle */
    uint8_t NR11 = 0x0;
    /* 0xFF12 — NR12: Channel 1 volume & envelope */
    uint8_t NR12 = 0x0;
    /* 0xFF13 — NR13: Channel 1 period low [write-only] */
    uint8_t NR13 = 0x0;
    /* 0xFF14 — NR14: Channel 1 period high & control */
    uint8_t NR14 = 0x0;

    /* Sound Channel 2 — Pulse */
    /* 0xFF16 — NR21: Channel 2 length timer & duty cycle */
    uint8_t NR21 = 0x0;
    /* 0xFF17 — NR22: Channel 2 volume & envelope */
    uint8_t NR22 = 0x0;
    /* 0xFF18 — NR23: Channel 2 period low [write-only] */
    uint8_t NR23 = 0x0;
    /* 0xFF19 — NR24: Channel 2 period high & control */
    uint8_t NR24 = 0x0;

    /* Sound Channel 3 — Wave output */
    /* 0xFF1A — NR30: Channel 3 DAC enable */
    uint8_t NR30 = 0x0;
    /* 0xFF1B — NR31: Channel 3 length timer [write-only] */
    uint8_t NR31 = 0x0;
    /* 0xFF1C — NR32: Channel 3 output level */
    uint8_t NR32 = 0x0;
    /* 0xFF1D — NR33: Channel 3 period low [write-only] */
    uint8_t NR33 = 0x0;
    /* 0xFF1E — NR34: Channel 3 period high & control */
    uint8_t NR34 = 0x0;

    /* 0xFF30–0xFF3F — Wave pattern RAM

       Wave RAM is 16 bytes long; each byte holds two “samples”, each 4 bits.

       As CH3 plays, it reads wave RAM left to right, upper nibble first. That is, $FF30’s upper nibble, $FF30’s lower nibble, $FF31’s upper nibble, and so on. */
    std::array<uint8_t, 16> wave_pattern_ram;

    /* Sound Channel 4 — Noise */
    /* 0xFF20 — NR41: Channel 4 length timer [write-only] */
    uint8_t NR41 = 0x0;
    /* 0xFF21 — NR42: Channel 4 volume & envelope */
    uint8_t NR42 = 0x0;
    /* 0xFF22 — NR43: Channel 4 frequency & randomness */
    uint8_t NR43 = 0x0;
    /* 0xFF23 — NR44: Channel 4 control */
    uint8_t NR44 = 0x0;

    /* Global control registers */
    /* 0xFF24 — NR50: Master volume & VIN panning */
    uint8_t NR50 = 0x0;
    /* 0xFF25 — NR51: Sound panning */
    uint8_t NR51 = 0x0;
    /* 0xFF26 — NR52: Audio master control */
    uint8_t NR52 = 0x0;

    public:
    APU() = delete;
    explicit APU(YumeBoy &yume_boy) : yume_boy_(yume_boy) { }

    /* Runs the APU until it reaches the next "stable" state. Returns the amount of time spent. */
    uint32_t tick();
};