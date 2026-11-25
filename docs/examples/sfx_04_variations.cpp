// Same preset, different seeds = a family of related-but-different sounds (great for games: no two footsteps alike).
#include "wauvio/sfx.hpp"
#include <string>
namespace sfx = wauvio::sfx;

int main() {
    for (int i = 0; i < 5; ++i) {
        sfx::Effect e = sfx::make("cinematic_impact");
        e.set("duration", 1.0).seed(1000 + i);
        e.render().save("sfx_04_variation_" + std::to_string(i) + ".wav");
    }
}
