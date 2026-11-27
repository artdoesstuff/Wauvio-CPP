// The pieces under the Composer, used by hand: generate a progression, a motif and a melody,
// then play them with instruments of your choice.
#include "wauvio/compose.hpp"
#include <iostream>
using namespace wauvio;
using namespace wauvio::compose;

int main() {
    Key key(PitchClass::A, ScaleType::NaturalMinor);

    // 1. Harmony. Chords are Roman numerals resolved against a key.
    ProgressionOptions po; po.bars = 4; po.seed = 3; po.cadence = CadenceType::Half;
    ChordProgression prog = generate_progression(key, po);
    for (const ChordSymbol& c : prog.resolve(key)) std::cout << c.name() << " ";
    std::cout << "\n";

    // 2. A motif: a short idea described in scale steps (not fixed pitches), so it fits any key.
    MotifOptions mo; mo.contour = Contour::Hook; mo.length_beats = 4; mo.seed = 5;
    Motif hook = generate_motif(mo);
    Motif answer = invert(transpose(hook, 2));          // classic development tricks: transpose, invert, retrograde, ...

    // 3. A melody that follows the chords and develops the motif; plus a countermelody against it.
    MelodyOptions mel; mel.bpm = 100; mel.seed = 11; mel.form = PhraseForm::ABAC;
    music::Melody lead    = generate_melody(key, prog, hook, mel);
    music::Melody counter = generate_countermelody(key, prog, lead, 100.0);

    // 4. Play them like any other melody from the music layer.
    music::Arrangement a;
    instruments::SoloViolin violin; instruments::Flute flute; violin.reverb(); 
    a.add(violin, lead);
    a.add(flute, counter, 0.0, 0.6);
    a.limiter();
    render(a, "compose_05_building_blocks.wav");
    (void)answer;
}
