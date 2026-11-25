// Restyle a whole MIDI file with a genre preset. It is reversible, and deterministic for a given seed.
#include "wauvio/midi.hpp"
using namespace wauvio;

int main() {
    { music::Arrangement a; instruments::ElectricPiano ep; instruments::BassGuitar bg;
      music::Melody m; for (int n : {60, 64, 67, 72, 67, 64, 60, 55}) m.push_back({n, 0.25});
      a.add(ep, m); a.add(bg, {{36, 1.0}, {41, 1.0}});
      midi::track::save_midi(a, "demo3.mid"); }

    midi::track::MidiMusic song = midi::track::load_midi("demo3.mid");

    midi::genre::GenreApplyOptions go;
    go.allow_instrumentation = true;        // let the preset swap in genre-typical instruments
    go.seed = 3;                            // different seed = different (but repeatable) humanisation

    song.setGenre(midi::genre::presets::jazz(), go);
    render(song, "midi_03_genre_jazz.wav");

    song.setGenre(midi::genre::presets::metal(), go);   // starts again from the original, not from the jazz version
    render(song, "midi_03_genre_metal.wav");

    song.clearGenre();                                  // back to the file as loaded
    render(song, "midi_03_genre_original.wav");
}
