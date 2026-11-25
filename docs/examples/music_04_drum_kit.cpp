// Drum kits map MIDI note numbers (General MIDI) to drum sounds.
#include "wauvio/ext.hpp"
using namespace wauvio;

int main() {
    instruments::Kit808 kit;                    // also: IndustrialKit, and others in the catalog
    const int KICK = 36, SNARE = 38, HAT = 42;

    // render_pattern takes (time in seconds, midi note) pairs.
    std::vector<std::pair<double,int>> hits;
    const double step = 0.125;                   // an eighth note at 120 BPM
    for (int bar = 0; bar < 4; ++bar)
        for (int i = 0; i < 8; ++i) {
            double t = (bar * 8 + i) * step;
            hits.push_back({t, HAT});
            if (i % 4 == 0) hits.push_back({t, KICK});
            if (i % 4 == 2) hits.push_back({t, SNARE});
        }

    StereoBuffer loop = kit.render_pattern(hits, 4.5, music::Dynamics::f);
    normalize(loop);
    save_wav_stereo(loop, "music_04_drum_kit.wav");
}
