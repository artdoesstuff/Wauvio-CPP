// Polyphony: VoiceManager plays several notes at once with a shared sound design.
#include "wauvio.hpp"
using namespace wauvio;

int main() {
    synth::VoiceManager vm;
    vm.shape1   = osc::WaveShape::Sawtooth;   // two oscillators per voice...
    vm.shape2   = osc::WaveShape::Square;
    vm.detune_hz = 3.0;                       // ...slightly detuned for width
    vm.adsr     = env::ADSR(0.05, 0.2, 0.6, 0.6);

    // C major chord. Notes are given as frequencies; midi_to_freq converts note numbers.
    vm.note_on(theory::midi_to_freq(60), 0.8);   // C4
    vm.note_on(theory::midi_to_freq(64), 0.8);   // E4
    vm.note_on(theory::midi_to_freq(67), 0.8);   // G4

    Buffer chord = vm.render(3.0);
    normalize(chord);
    save_wav(chord, "core_04_chord_voices.wav");
}
