// A melody with MIDI note numbers, dynamics and articulations.
#include "wauvio/ext.hpp"
using namespace wauvio;
using music::Dynamics;
using music::Articulation;

int main() {
    instruments::Flute flute;
    flute.reverb(0.7, 0.25);                       // optional per-instrument effect

    // A Note is {midi_note, duration_in_seconds, dynamics, articulation}.
    // MIDI 60 = middle C, 69 = A440. Each +1 is one semitone.
    music::Melody m = {
        {67, 0.5, Dynamics::mp},
        {69, 0.5, Dynamics::mf},
        {72, 1.0, Dynamics::f, Articulation::Legato},
        music::rest(0.5),
        {71, 0.5, Dynamics::mp, Articulation::Staccato},
        {67, 1.5, Dynamics::p},
    };

    music::Arrangement song;
    song.add(flute, m);
    render(song, "music_01_melody.wav");           // 44.1 kHz stereo 16-bit by default
}
