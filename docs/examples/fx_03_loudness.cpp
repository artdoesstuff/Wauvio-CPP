// Measure and set loudness (LUFS), the way streaming platforms do.
#include "wauvio/fx/ext.hpp"
#include <cstdio>
using namespace wauvio;

int main() {
    instruments::Trumpet t; music::Arrangement a;
    a.add(t, {{67, 1.0, music::Dynamics::mf}, {72, 1.0, music::Dynamics::f}, {71, 2.0, music::Dynamics::mp}});
    StereoBuffer audio = a.render();

    fx::LoudnessReport before = fx::measure_loudness(audio, 44100);
    std::printf("before: %.1f LUFS integrated, true peak %.1f dBTP\n", before.integrated_lufs, before.true_peak_db);

    // Gain (and a limiter if needed) to reach -14 LUFS without exceeding -1 dBTP.
    fx::LoudnessReport after = fx::normalize_loudness(audio, 44100, -14.0, -1.0);
    std::printf("after:  %.1f LUFS integrated, true peak %.1f dBTP\n", after.integrated_lufs, after.true_peak_db);

    fx::write_wav(audio, "fx_03_loudness.wav");
}
