// A sustained atmosphere: three textures layered for twenty seconds.
#include "../../wauvio_sfx.hpp"
#include <filesystem>
#include <iostream>

int main() {
    namespace sfx = wauvio::sfx;
    std::filesystem::create_directories("sfx_examples_out");

    sfx::Layered bed;
    bed.seed(3);
    bed.add("detuned_atmosphere", sfx::LayerOpts().dur(20).set("pitch", 41).set("brightness", 0.35));                 // warm pad underneath
    bed.add("granular_cloud",     sfx::LayerOpts().dur(20).gain(-9).set("pitch", 77).set("density", 0.4));           // bell grains on top
    bed.add("air_texture",        sfx::LayerOpts().dur(20).gain(-14));                                                // a little hiss

    sfx::RenderSettings settings;
    settings.peak_db = -6.0;
    sfx::Rendered audio = bed.render(settings);
    audio.save("sfx_examples_out/atmosphere.wav");
    std::cout << "atmosphere: " << audio.seconds() << " s\n";
}
