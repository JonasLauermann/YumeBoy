#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <apu/PulseChannel.hpp>
#include <mmu/MMU.hpp>
#include <mmu/Memory.hpp>
#include <SDL3/SDL.h>


class APUSaveState;

class APU : public Memory {
   template <bool WithSweep>
   friend class PulseChannel;

   static constexpr uint32_t CPU_FREQUENCY = 1 << 22;
   static constexpr uint16_t SAMPLE_RATE = 32'768; // unconventional sampling rate, but divides the CPU_FREQ without remainder. Higher sampling rates caused crackling without improving the sound quality overall.
   static constexpr uint16_t SAMPLE_PER_BUFFER = 512;
   static constexpr SDL_AudioSpec spec = {
      SDL_AUDIO_F32,    // Audio data format
      2,                // Number of channels: 1 mono, 2 stereo, etc
      SAMPLE_RATE       // sample rate: sample frames per second
   };
   public:
   using sample_array_t = std::array<float_t, SAMPLE_PER_BUFFER>;

   private:
   MMU &mem_;
   uint32_t sample_time = 0;   /* number of t-cycles since last sample was queued. */
   sample_array_t samples;
   uint16_t pushed_samples = 0;

    struct sdl_deleter
    {
        void operator()(SDL_AudioStream *p) const { SDL_DestroyAudioStream(p); }
    };
   std::unique_ptr<SDL_AudioStream, sdl_deleter> stream;

   /* Registers
      From https://gbdev.io/pandocs/Audio_Registers.html:
      Audio registers are named following a NRxy scheme, where x is the channel number (or 5 for “global” registers), and y is the register’s ID within the channel. Since many registers share common properties, a notation is often used where e.g. NRx2 is used to designate NR12, NR22, NR32, and NR42 at the same time, for simplicity. */

   /* Sound Channel 1 — Pulse with period sweep */
   std::unique_ptr<PulseChannel<true>> channel1 = std::make_unique<PulseChannel<true>>(*this);

   /* Sound Channel 2 — Pulse */
   std::unique_ptr<PulseChannel<false>> channel2 = std::make_unique<PulseChannel<false>>(*this);
   

   /* Sound Channel 3 — Wave output */
   

   /* Sound Channel 4 — Noise */
   

   /* Global control registers */
   /* 0xFF24 — NR50: Master volume & VIN panning */
   uint8_t NR50 = 0x0;
   /* 0xFF25 — NR51: Sound panning */
   uint8_t NR51 = 0x0;
   /* 0xFF26 — NR52: Audio master control */
   uint8_t NR52 = 0x0;

   /* A value of 0 is treated as a volume of 1 (very quiet), and a value of 7 is treated as a volume of 8 (no volume reduction). Importantly, the amplifier never mutes a non-silent input. */
   uint8_t right_volume() const { return (NR50 & 0b111) + 1; }
   /* A value of 0 is treated as a volume of 1 (very quiet), and a value of 7 is treated as a volume of 8 (no volume reduction). Importantly, the amplifier never mutes a non-silent input. */
   uint8_t left_volume() const { return ((NR50 >> 4) & 0b111) + 1; }

   public:
   APU() = delete;
   explicit APU(MMU &mem) : mem_(mem) {
      SDL_Init(SDL_INIT_AUDIO);

      stream = std::unique_ptr<SDL_AudioStream, sdl_deleter>(SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr), sdl_deleter());

      SDL_ResumeAudioStreamDevice(stream.get());
   }

   /* Runs the APU for an */
   void tick();

    bool contains_address(uint16_t addr) const override {
        return (0xFF10 <= addr) and (addr <= 0xFF3F);
    }

    uint8_t read_memory(uint16_t addr) override {
         if (addr == 0xFF10)
            return channel1->NR10();
         else if (addr == 0xFF11)
            return channel1->NRX1();
         else if (addr == 0xFF12)
            return channel1->NRX2();
         else if (addr == 0xFF13)
            return channel1->NRX3();
         else if (addr == 0xFF14)
            return channel1->NRX4();
         else if (addr == 0xFF15)
            return 0xFF;
         else if (addr == 0xFF16)
            return channel2->NRX1();
         else if (addr == 0xFF17)
            return channel2->NRX2();
         else if (addr == 0xFF18)
            return channel2->NRX3();
         else if (addr == 0xFF19)
            return channel2->NRX4();
         else if (addr == 0xFF1A)
            return 0xFF;  // TODO
         else if (addr == 0xFF1B)
            return 0xFF;  // TODO
         else if (addr == 0xFF1C)
            return 0xFF;  // TODO
         else if (addr == 0xFF1D)
            return 0xFF;  // TODO
         else if (addr == 0xFF1E)
            return 0xFF;  // TODO
         else if (addr == 0xFF1F)
            return 0xFF;
         else if (addr == 0xFF20)
            return 0xFF;  // TODO
         else if (addr == 0xFF21)
            return 0xFF;  // TODO
         else if (addr == 0xFF22)
            return 0xFF;  // TODO
         else if (addr == 0xFF23)
            return 0xFF;  // TODO
         else if (addr == 0xFF24)
            return NR50;
         else if (addr == 0xFF25)
            return NR51;
         else if (addr == 0xFF26)
            return NR52;
         else if (0xFF27 <= addr or addr <= 0xFF2F)
            return 0xFF;   // unmapped memory
         else if (0xFF30 <= addr or addr <= 0xFF3F)
            return 0xFF;  // TODO
         else
            std::unreachable();
    }

    void write_memory(uint16_t addr, uint8_t value) override {
         if (addr == 0xFF10)
            channel1->NR10(value);
         else if (addr == 0xFF11)
            channel1->NRX1(value);
         else if (addr == 0xFF12)
            channel1->NRX2(value);
         else if (addr == 0xFF13)
            channel1->NRX3(value);
         else if (addr == 0xFF14)
            channel1->NRX4(value);
         else if (addr == 0xFF15)
            return;
         else if (addr == 0xFF16)
            channel2->NRX1(value);
         else if (addr == 0xFF17)
            channel2->NRX2(value);
         else if (addr == 0xFF18)
            channel2->NRX3(value);
         else if (addr == 0xFF19)
            channel2->NRX4(value);
         else if (addr == 0xFF1A)
            return;  // TODO
         else if (addr == 0xFF1B)
            return;  // TODO
         else if (addr == 0xFF1C)
            return;  // TODO
         else if (addr == 0xFF1D)
            return;  // TODO
         else if (addr == 0xFF1E)
            return;  // TODO
         else if (addr == 0xFF1F)
            return;
         else if (addr == 0xFF20)
            return;  // TODO
         else if (addr == 0xFF21)
            return;  // TODO
         else if (addr == 0xFF22)
            return;  // TODO
         else if (addr == 0xFF23)
            return;  // TODO
         else if (addr == 0xFF24)
            NR50 = value;
         else if (addr == 0xFF25)
            NR51 = value;
         else if (addr == 0xFF26)
            NR52 = value;  // TODO implement audio on/off and make lower bits read-only (https://gbdev.io/pandocs/Audio_Registers.html#ff26--nr52-audio-master-control)
         else if (0xFF27 <= addr or addr <= 0xFF2F)
            return;   // unmapped memory
         else if (0xFF30 <= addr or addr <= 0xFF3F)
            return;  // TODO
         else
            std::unreachable();
    }

   APUSaveState save_state() const;

   void load_state(APUSaveState apu_state);
};