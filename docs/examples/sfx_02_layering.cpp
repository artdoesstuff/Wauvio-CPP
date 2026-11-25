// Layered: combine several effects on a timeline (riser into impact, with a reversed tail).
#include "wauvio/sfx.hpp"
namespace sfx = wauvio::sfx;

int main() {
    sfx::Layered fx;
    fx.tempo(120).seed(99);

    // Riser builds for 4 beats (2 s at 120 BPM)...
    fx.add("noise_riser", sfx::LayerOpts().dur(2.0).gain(-3));
    // ...impact lands exactly on beat 4, a bit louder and an octave lower...
    fx.add("cinematic_impact", sfx::LayerOpts().at_beats(4).transpose(-12).gain(0));
    // ...and a reversed version of the same impact leads into it, panned left.
    fx.add("cinematic_impact", sfx::LayerOpts().at_beats(2).flip().gain(-9).place(-0.6).with_seed(5));

    sfx::Rendered out = fx.render();
    out.save("sfx_02_layering.wav");
}
