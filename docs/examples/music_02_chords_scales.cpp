// Chords and scales: build harmony without computing note numbers by hand.
#include "wauvio/ext.hpp"
using namespace wauvio;

int main() {
    instruments::Cello cello;
    instruments::Harp  harp;

    // scale::* return MIDI note numbers. build(root, steps, octaves) makes custom scales.
    auto scale = music::scale::natural_minor(57 /*A3*/, 2);      // two octaves of A minor

    music::Melody run;
    for (int n : scale) run.push_back({n, 0.2, music::Dynamics::mf});

    // A Chord is a list of notes played together.
    music::Track chords(harp);
    for (int root : {57, 53, 48, 55}) {                           // Am  F  C  G (roots)
        // third = +3 (minor) or +4 (major) semitones; here we just stack a fifth and an octave for an open sound
        chords.add_chord({{root, root + 7, root + 12, root + 16}, 1.5, music::Dynamics::mp});
    }

    music::Arrangement song;
    song.add_track(chords);
    song.add(cello, run, /*start_time=*/6.0);                      // enters after 6 seconds
    song.master_reverb(0.7, 0.2);
    render(song, "music_02_chords_scales.wav");
}
