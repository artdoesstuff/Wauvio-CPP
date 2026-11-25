// Sing a MIDI karaoke-style file: notes + Lyric meta events become a vocal line.
// Usage: ./vocal_07_midi_lyrics song.mid [voice-id-or-file.wvc]
#include "wauvio/vocal/v3.hpp"
#include <cstdio>
using namespace wauvio::vocal::v3;

int main(int argc, char** argv) {
    if (argc < 2) { std::printf("usage: %s song.mid [voice]\n", argv[0]); return 0; }

    VoiceProfile voice; std::string err;
    Singer s;
    if (argc > 2 && make_voice(argv[2], voice, &err)) s.set_profile(voice);   // set the voice BEFORE adding notes

    // track = -1: use the first track that has lyrics. Notes without a lyric continue the previous vowel.
    int n = s.load_midi_file(argv[1], -1);
    std::printf("loaded %d notes\n", n);
    for (auto& warning : s.diagnostics()) std::printf("warning: %s\n", warning.c_str());

    s.natural(); s.auto_expression();
    s.save("vocal_07_midi_lyrics.wav");
}
