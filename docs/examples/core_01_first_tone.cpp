// Your first sound: one sine note with a volume envelope, saved as a WAV file.
#include "wauvio.hpp"
using namespace wauvio;

int main() {
    // 1. An oscillator makes the raw waveform.
    osc::Oscillator tone;
    tone.shape     = osc::WaveShape::Sine;
    tone.frequency = 440.0;          // Hz (concert A)
    tone.amplitude = 0.8;            // 0..1

    // 2. render(seconds, sample_rate) gives you a Buffer: a vector of float samples.
    Buffer sound = tone.render(2.0, 44100);

    // 3. An ADSR envelope shapes the volume: fade in, settle, then fade out.
    //    ADSR(attack, decay, sustain level, release)
    env::ADSR adsr(0.02, 0.10, 0.6, 0.5);
    adsr.apply(sound, /*total_seconds=*/2.0, /*note_off at*/ 1.5);

    // 4. Save it. save_wav writes 16-bit mono PCM.
    save_wav(sound, "core_01_first_tone.wav", 44100);
}
