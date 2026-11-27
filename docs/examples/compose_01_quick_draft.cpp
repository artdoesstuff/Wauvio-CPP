// The fastest path to music: pick a key, tempo and mood, and Wauvio composes a short draft with
// melody, chords, bass and drums, using a palette of instruments chosen for the mood.
#include "wauvio/compose.hpp"
#include <iostream>
using namespace wauvio::compose;

int main() {
    //                           root            scale                     BPM   mood            bars  seed
    ComposedSong draft = compose_draft(PitchClass::D, ScaleType::NaturalMinor, 124.0, Mood::Energetic, 8, 7);

    draft.save("compose_01_quick_draft.wav");        // render + write WAV
    std::cout << draft.summary() << "\n";           // what was generated (sections, chords, ...)
}
