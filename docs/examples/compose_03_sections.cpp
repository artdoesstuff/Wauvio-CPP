// Control the song structure yourself: an intro, verse, chorus and outro, each with its own
// length, energy and which parts play.
#include "wauvio/compose.hpp"
using namespace wauvio::compose;

static SectionSpec section(SectionRole role, const char* name, int bars, double energy,
                           const char* progression, const char* motif) {
    SectionSpec s;
    s.role = role; s.name = name; s.bars = bars; s.energy = energy;       // energy 0..1: how busy/loud
    s.progression = *find_progression(progression);                     // built-in names; returns nullptr if unknown
    s.motif       = *find_motif(motif);
    return s;
}

int main() {
    Composer c;
    c.key(PitchClass::A, ScaleType::NaturalMinor).tempo(104).mood(Mood::Melancholic).seed(5);

    SectionSpec intro = section(SectionRole::Intro, "Intro", 4, 0.3, "Jazz Turnaround", "Sigh Step");
    intro.enable_drums = false; intro.enable_bass = false;               // just chords and melody
    c.add_section(intro);
    c.add_section(section(SectionRole::Verse,  "Verse",  8, 0.55, "Jazz Turnaround", "Sigh Step"));
    c.add_section(section(SectionRole::Chorus, "Chorus", 8, 0.9,  "Jazz Turnaround", "Rising Triad"));
    SectionSpec outro = section(SectionRole::Outro, "Outro", 4, 0.25, "Jazz Turnaround", "Sigh Step");
    outro.enable_drums = false;
    c.add_section(outro);

    c.compose().save("compose_03_sections.wav");
}
