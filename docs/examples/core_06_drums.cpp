// A drum loop. You supply the drum sounds as functions; the sequencer handles the timing.
#include "wauvio.hpp"
#include <cmath>
using namespace wauvio;

// Each "sound" is a function that returns one hit as a Buffer.
Buffer kick(int sr) {
    Buffer b(int(0.25 * sr));
    double phase = 0;
    for (size_t i = 0; i < b.size(); ++i) {
        double t = double(i) / sr;
        double freq = 50 + 120 * std::exp(-t * 30);       // pitch drops fast: the "thump"
        phase += TWO_PI * freq / sr;
        b[i] = float(std::sin(phase) * std::exp(-t * 12));
    }
    return b;
}
Buffer hat(int sr) {
    osc::NoiseGenerator n;                                // deterministic: seeded
    Buffer b = n.render(0.05);
    for (size_t i = 0; i < b.size(); ++i) b[i] *= float(std::exp(-double(i) / sr * 90)) * 0.4f;
    return b;
}

int main() {
    seq::DrumSequencer drums;
    drums.bpm = 120;
    drums.steps_per_bar = 16;

    // One bool per step (16 steps = one bar of sixteenth notes). true = hit.
    drums.add_pattern({1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,1,0}, kick, 1.0f);
    drums.add_pattern({0,0,1,0, 0,0,1,0, 0,0,1,0, 0,0,1,0}, hat,  0.6f);

    Buffer loop = drums.render(/*bars=*/4);
    normalize(loop);
    save_wav(loop, "core_06_drums.wav");
}
