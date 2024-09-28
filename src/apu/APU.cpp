#include <apu/APU.hpp>

#include <savestate/APUSaveState.hpp>


void APU::tick()
{
   float_t right_out = 0;
   float_t left_out = 0;
   if (NR52 & (1 << 7)) {
      // tick channels
      float_t chl1 = channel1->tick();
      float_t chl2 = channel2->tick();
      float_t chl3 = 0; // TODO
      float_t chl4 = 0; // TODO

      // Mixer
      right_out += NR51 & 1 ? chl1 : 0;
      right_out += NR51 & (1 << 1) ? chl2 : 0;
      right_out += NR51 & (1 << 2) ? chl3 : 0;
      right_out += NR51 & (1 << 3) ? chl4 : 0;
      right_out /= 4.0f;

      left_out += NR51 & (1 << 4) ? chl1 : 0;
      left_out += NR51 & (1 << 5) ? chl2 : 0;
      left_out += NR51 & (1 << 6) ? chl3 : 0;
      left_out += NR51 & (1 << 7) ? chl4 : 0;
      left_out /= 4.0f;

      // Amplifier
      right_out *= right_volume() / 8.0f;
      left_out *= left_volume() / 8.0f;
   }

   // check if sample should be queued
   if (sample_time == 4194304 / SAMPLE_RATE) {
      sample_time = 0;
      samples[pushed_samples] = left_out;
      samples[pushed_samples + 1] = right_out;
      pushed_samples += 2;

      if (pushed_samples == SAMPLE_PER_BUFFER) {
         SDL_PutAudioStreamData(stream.get(), samples.data(), sizeof(samples));
         pushed_samples = 0;  // clearing the array itself should not be necessary
      }
   } else {
      ++sample_time;
   }

}

APUSaveState APU::save_state() const
{
   APUSaveState s = {
      sample_time,
      samples,
      pushed_samples,

      channel1->save_state(),
      channel2->save_state(),
      // TODO channel 3
      // TODO channel 4

      NR50,
      NR51,
      NR52,
   };
   return s;
}

void APU::load_state(APUSaveState apu_state)
{
   sample_time = apu_state.sample_time;
   samples = apu_state.samples;
   pushed_samples = apu_state.pushed_samples;

   channel1->load_state(apu_state.channel1);
   channel2->load_state(apu_state.channel2);
   // TODO channel 3
   // TODO channel 4
   
   NR50 = apu_state.NR50;
   NR51 = apu_state.NR51;
   NR52 = apu_state.NR52;
}