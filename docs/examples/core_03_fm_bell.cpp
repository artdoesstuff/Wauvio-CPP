// FM synthesis: one oscillator (the modulator) wobbles the pitch of another (the carrier).
#include "wauvio.hpp"
using namespace wauvio;

int main() {
    synth::FMSynth fm;
    fm.carrier_freq   = 440.0;
    fm.modulator_freq = 440.0 * 3.5;   // non-integer ratio = inharmonic, bell-like
    fm.mod_index      = 4.0;           // higher = brighter, more complex
    fm.amplitude      = 0.7;

    Buffer bell = fm.render(3.0);

    // A fast attack and long release makes it ring like a bell.
    env::ADSR ring(0.002, 1.0, 0.0, 1.5);
    ring.apply(bell, 3.0, 0.05);

    normalize(bell);
    save_wav(bell, "core_03_fm_bell.wav");
}
