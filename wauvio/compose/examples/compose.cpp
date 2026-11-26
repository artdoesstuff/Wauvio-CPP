//   g++ -std=c++17 -O2 compose.cpp -o compose && ./compose

#include "../../compose.hpp"
#include "../midi.hpp"
#include "../fx.hpp"

#include <iostream>

int main() {
    using namespace wauvio::compose;

    // Minimal 8-bar draft
    auto draft = compose_draft(PitchClass::D, ScaleType::NaturalMinor, 124.0, Mood::Energetic, 8, /*seed=*/7);
    draft.save("compose_draft.wav");
    std::cout << "===== Quick Draft =====\n" << draft.summary() << "\n";

    // Full multi-section song with genre styling, MIDI export, and loudness mastering
    auto jazz = Composer()
        .key(PitchClass::F, ScaleType::Major)
        .tempo(112.0)
        .mood(Mood::Jazzy)
        .genre("jazz", 0.75)
        .progression("Jazz Turnaround")
        .motif("Syncopated Hook")
        .accompaniment(AccompanimentPattern::Alberti)
        .bass_pattern(BassPattern::Walking)
        .seed(101)
        .compose();

    jazz.save("compose_jazz.wav");
    save_midi(jazz, "compose_jazz.mid");
    save_mastered(jazz, "compose_jazz_mastered.wav", wauvio::fx::master_for_genre("jazz"));
    std::cout << "===== Jazz Song =====\n" << jazz.summary() << "\n";

    // Load custom progression & motif from *.wcp
    auto from_wcp = compose_from_wcp("examples.wcp", "Verse", "Hook", Mood::Energetic, 42);
    from_wcp.save("compose_wcp.wav");
    std::cout << "===== From .wcp =====\n" << from_wcp.summary() << "\n";

    return 0;
}