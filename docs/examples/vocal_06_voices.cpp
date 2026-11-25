// Same melody, every built-in voice. Voices differ in tract (formants), range and default look.
#include "wauvio/vocal/v3.hpp"
#include <cstdio>
using namespace wauvio::vocal::v3;

int main() {
    for (const std::string& id : voice_ids()) {
        VoiceProfile voice; std::string err;
        if (!make_voice(id, voice, &err)) { std::printf("skip %s: %s\n", id.c_str(), err.c_str()); continue; }

        Singer s(voice);                        // or: s.set_profile(voice)
        s.set_tempo(100);
        s.natural();                            // uses this voice's recommended strength
        // Stay inside the voice's comfortable range (voice.model.range_low / range_high).
        int base = (voice.model.range_low + voice.model.range_high) / 2;
        s.note(base, 480, "あ"); s.note(base + 2, 480, "い"); s.note(base + 4, 480, "う"); s.note(base + 5, 960, "え");
        s.save("vocal_06_voice_" + id + ".wav");
        std::printf("%-9s %s\n", id.c_str(), voice.display.c_str());
    }
}
