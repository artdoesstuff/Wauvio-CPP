// Making it sound like a performance: vibrato, ornaments, per-note tweaks, a persona and automation lanes.
#include "wauvio/vocal/v3.hpp"
using namespace wauvio::vocal::v3;

int main() {
    Singer s;
    s.set_tempo(80);
    s.natural();                       // slight human imperfections (timing, pitch, tone), tuned for this voice
    s.auto_expression();               // rule-based vibrato and dynamics; only fills fields you have not set
    s.persona("soft", 0.8);            // a ready-made "look" (Yune ships: normal, soft)

    // Note-level control: note() returns a NoteRef; every setter returns it again, so calls chain.
    s.note(69, 480, "さ");
    s.note(72, 480, "く").breathy(0.3);
    s.note(76, 1440, "ら").vibrato(0.45, 5.5, /*delay*/0.3, /*fade-in*/0.4).ornament("fall", 1.0);
    s.rest(240);
    s.note(74, 480, "さ").glide_from(71);          // slide up into the note
    s.note(72, 1920, "く").portamento(0.08);

    // Lanes: automation over time (absolute seconds). This one swells the volume then fades it.
    s.lane("volume", {{0.0, 0.7}, {3.0, 1.0}, {6.0, 0.5}});

    s.save("vocal_02_expressive.wav");
}
