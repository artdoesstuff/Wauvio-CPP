// Use a SoundFont (.sf2) as the instrument source for a MIDI file.
// Pass the path to any SF2 on the command line:  ./midi_05_soundfont GeneralUser.sf2 song.mid
#include "wauvio/sf2.hpp"
#include "wauvio/midi.hpp"
using namespace wauvio;

int main(int argc, char** argv) {
    if (argc < 3) { std::printf("usage: %s soundfont.sf2 song.mid\n", argv[0]); return 0; }

    auto sf = std::make_shared<sf2::SoundFont>(argv[1]);        // throws sf2::Sf2ParseError on a bad file
    std::printf("loaded '%s' with %zu presets\n", sf->name().c_str(), sf->list_presets().size());

    midi::track::LoadOptions opts;
    opts.resolver.set_bank_provider(sf);                         // SF2 presets now win over the built-in instruments
    auto song = midi::track::load_midi(argv[2], opts);
    render(song, "midi_05_soundfont.wav");
}
