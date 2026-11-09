// Compose a transition from several presets with Layered: a riser, a reverse cymbal and an impact on the downbeat.
#include "../../wauvio_sfx.hpp"
#include <filesystem>
#include <iostream>

int main() {
    namespace sfx = wauvio::sfx;
    std::filesystem::create_directories("sfx_examples_out");

    sfx::Layered transition;
    transition.tempo(128).seed(7);
    transition.add("noise_riser",     sfx::LayerOpts().dur(3.75).gain(-3).place(-0.2));                // builds for 8 beats (3.75 s at 128 bpm)
    transition.add("tonal_riser",     sfx::LayerOpts().dur(3.75).gain(-8).set("pitch", 45));           // parameters can be overridden per layer
    transition.add("reverse_cymbal",  sfx::LayerOpts().dur(1.8).at(1.95).gain(-6));                    // swells into the downbeat
    transition.add("cinematic_impact", sfx::LayerOpts().at_beats(8).dur(2.0).gain(0));                 // lands exactly on beat 8

    sfx::Rendered audio = transition.render();
    audio.save("sfx_examples_out/transition.wav");
    std::cout << "transition: " << audio.seconds() << " s, " << transition.size() << " layers\n";
}
