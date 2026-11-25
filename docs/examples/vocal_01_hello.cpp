// The smallest singing program. Lyrics are Japanese kana (or romaji); each note carries one syllable.
#include "wauvio/vocal/v3.hpp"
using namespace wauvio::vocal::v3;

int main() {
    Singer s;                          // default voice: Yune
    s.set_tempo(100);                  // timing is in ticks; 480 ticks = one quarter note

    //       MIDI note, length in ticks, lyric
    s.note(67, 480, "こ");             // G4
    s.note(69, 480, "ん");
    s.note(71, 480, "に");
    s.note(72, 480, "ち");
    s.note(74, 960, "は");

    s.save("vocal_01_hello.wav");      // mono render, normalised
}
