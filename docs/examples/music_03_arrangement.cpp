// A small band: several instruments, each on its own track, with start offsets and gains.
#include "wauvio/ext.hpp"
using namespace wauvio;
using music::Dynamics;

int main() {
    music::Arrangement band;
    band.tempo = music::Tempo(100);
    const double beat = band.tempo.beat_seconds();           // seconds per beat at 100 BPM

    instruments::UprightBass bass;
    instruments::AcousticGrandPiano piano;
    instruments::Trumpet trumpet;  trumpet.reverb();

    // Bass: root notes, one per bar (4 beats).
    music::Melody bassline;
    for (int n : {36, 41, 43, 36}) bassline.push_back({n, 4 * beat, Dynamics::mf});

    // Piano: eighth-note pattern.
    music::Melody comp;
    for (int bar = 0; bar < 4; ++bar)
        for (int n : {60, 64, 67, 64, 60, 64, 67, 64}) comp.push_back({n, beat / 2, Dynamics::mp});

    // Trumpet joins in bar 3.
    music::Melody lead = {{72, 2 * beat, Dynamics::f}, {71, beat, Dynamics::mf}, {67, beat, Dynamics::mf}};

    band.add(bass,    bassline, 0.0,        /*track_gain=*/0.9);
    band.add(piano,   comp,     0.0,        0.7);
    band.add(trumpet, lead,     8 * beat,   0.8);
    band.limiter();                                         // keep the master from clipping
    render(band, "music_03_arrangement.wav");
}
