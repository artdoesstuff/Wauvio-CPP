// Tempo-aware effects: durations in beats, rhythmic steps locked to the tempo.
#include "../../wauvio_sfx.hpp"
#include <filesystem>
#include <iostream>

int main() {
    namespace sfx = wauvio::sfx;
    std::filesystem::create_directories("sfx_examples_out");

    for (double bpm : { 90.0, 140.0 }) {
        sfx::Effect riser = sfx::make("rhythmic_riser");
        riser.tempo(bpm).duration_beats(8);            // 8 beats, resolved to seconds when rendering
        std::cout << bpm << " bpm: 8 beats = " << riser.duration() << " s\n";
        riser.render().save("sfx_examples_out/rhythmic_riser_" + std::to_string(static_cast<int>(bpm)) + "bpm.wav");
    }

    sfx::Effect gate = sfx::make("gated_noise");       // "subdivision" is the step length in beats (0.25 = sixteenth)
    gate.tempo(120).set("subdivision", 0.5).set("swing", 0.2).duration_beats(8);
    gate.render().save("sfx_examples_out/gated_noise_eighths.wav");
}
