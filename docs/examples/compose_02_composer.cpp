// The Composer: describe the song you want with chained settings, then compose().
// The same seed always produces the same song; change it for a different one.
#include "wauvio/compose/midi.hpp"       // compose::save_midi
#include "wauvio/compose/fx.hpp"         // compose::save_mastered
#include <iostream>
using namespace wauvio::compose;

int main() {
    ComposedSong song = Composer()
        .key(PitchClass::F, ScaleType::Major)
        .tempo(112.0)
        .meter(4, 4)
        .mood(Mood::Jazzy)                    // also picks the default instruments and drum pattern
        .genre("jazz", 0.75)                  // genre styling (see the Genre system); 0 = none, 1 = full
        .progression("Jazz Turnaround")       // a built-in progression (see the Compose reference for the list)
        .motif("Syncopated Hook")             // a built-in motif the melody is built from
        .accompaniment(AccompanimentPattern::Alberti)
        .bass_pattern(BassPattern::Walking)
        .seed(101)
        .compose();

    song.save("compose_02_composer.wav");
    save_midi(song, "compose_02_composer.mid");                                       // export the notes
    save_mastered(song, "compose_02_composer_mastered.wav",
                  wauvio::fx::master_for_genre("jazz"));                              // loudness-targeted master
    std::cout << song.summary() << "\n";
}
