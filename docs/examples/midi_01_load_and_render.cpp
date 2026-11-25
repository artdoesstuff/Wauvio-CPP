// Load a MIDI file, render it with Wauvio's built-in General MIDI instruments, and save a WAV.
#include "wauvio/midi.hpp"
using namespace wauvio;

int main() {
    // (Self-contained demo: first write a small MIDI file. Skip this part if you have your own .mid.)
    {
        music::Arrangement a;
        instruments::Trumpet tp; instruments::UprightBass bs;
        a.add(tp, {{72, 0.5}, {74, 0.5}, {76, 1.0}, {72, 1.0}});
        a.add(bs, {{48, 1.0}, {43, 1.0}, {48, 1.0}});
        midi::track::save_midi(a, "demo.mid");
    }

    // load_midi gives you a MidiMusic: one "part" per track/channel, each with an instrument picked from its
    // General MIDI program number. (This demo file stores program 0 = piano for every part; real files vary.)
    midi::track::MidiMusic song = midi::track::load_midi("demo.mid");
    std::printf("%zu parts, %.1f seconds, %.0f BPM\n", song.part_count(), song.duration_seconds, song.initial_bpm);
    for (size_t i = 0; i < song.part_count(); ++i)
        std::printf("  part %zu: %s (GM program %d)\n", i, song.part(i).name.c_str(), song.part(i).gm_program);

    render(song, "midi_01_load_and_render.wav");
}
