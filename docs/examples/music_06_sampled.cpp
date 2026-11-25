// Use your own audio files as an instrument. (This example first makes a sample so it is self-contained.)
#include "wauvio/ext.hpp"
using namespace wauvio;

int main() {
    // 1. Make a short sample: a plucked tone at middle C. In real use, point at any .wav file.
    osc::Oscillator o; o.shape = osc::WaveShape::BL_Sawtooth; o.frequency = theory::midi_to_freq(60);
    Buffer b = o.render(1.0, 44100);
    env::ADSR(0.002, 0.3, 0.0, 0.3).apply(b, 1.0, 0.1);
    save_wav(b, "music_06_sample_c4.wav");

    // 2. Tell the instrument which MIDI note the sample was recorded at (root) and the range it covers.
    music::SampledInstrument inst;
    inst.name = "My Sample";
    inst.load_sample_file("music_06_sample_c4.wav", /*root=*/60, /*low=*/36, /*high=*/84);

    // 3. Play it. Other notes are made by pitch-shifting the sample.
    music::Melody m = {{60, 0.5}, {64, 0.5}, {67, 0.5}, {72, 1.0}, {55, 1.0}};
    music::Arrangement a; a.add(inst, m);
    render(a, "music_06_sampled.wav");
}
