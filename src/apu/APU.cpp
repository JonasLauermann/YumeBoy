#include <apu/APU.hpp>

#include <savestate/APUSaveState.hpp>


void APU::tick()
{
   ++sample_time;
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
   if (sample_time == (CPU_FREQUENCY / SAMPLE_RATE)) {

      sample_time = 0;
      samples[pushed_samples] = left_out;
      samples[pushed_samples + 1] = right_out;
      pushed_samples += 2;

      if (pushed_samples == SAMPLE_PER_BUFFER) {

         pushed_samples = 0;  // clearing the array itself is not necessary, we just overwrite old values
         assert(not samples.empty());
         // only add new samples if previous sample is almost used up
         if (SDL_GetAudioStreamAvailable(stream.get()) > ((SAMPLE_RATE * 2 /* stero sound */ * sizeof(samples[0]) /* single sample size */) / 16))
            return;

         // std::cerr << std::format("Pushing {} bytes\n", sizeof(samples));
         if (int available = SDL_GetAudioStreamAvailable(stream.get()); available == 0) {
            // The stream has run out of samples
            std::cerr << "No more samples available in the audio stream. This can cause crackeling\n";
         } else {
            // There are still samples available
            // std::cerr << std::format("Samples available: {} bytes\n", available);
         }

         SDL_PutAudioStreamData(stream.get(), samples.data(), sizeof(samples));
      }
   }

}

APUSaveState APU::save_state() const
{
   APUSaveState s = {
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
   channel1->load_state(apu_state.channel1);
   channel2->load_state(apu_state.channel2);
   // TODO channel 3
   // TODO channel 4
   
   NR50 = apu_state.NR50;
   NR51 = apu_state.NR51;
   NR52 = apu_state.NR52;
}