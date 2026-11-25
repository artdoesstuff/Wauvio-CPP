// Build a sound from scratch with the recipe language: a laser "zap".
// A Recipe has layers; each layer = one source -> envelope -> processors -> level/pan.
#include "wauvio/sfx.hpp"
namespace sfx = wauvio::sfx;
using sfx::Traj;

int main() {
    sfx::Recipe r;
    r.name = "zap";
    r.duration = 0.6;
    r.tail = 0.4;                                       // extra time for reverb/echo to ring out

    // Layer 1: a sawtooth sweeping from 2 kHz down to 120 Hz (Traj describes change over the layer's length).
    auto& body = r.add("body");
    body.osc(sfx::Wave::Saw, Traj::pitch_hz(2000, 120, sfx::Interp::Exp))
        .shape(Traj::decay(0.001))                      // fast exponential fade-out
        .lp(Traj::linear(9000, 600), 2.0)               // filter closes as it falls
        .drive(0.4)
        .gain(0.8);

    // Layer 2: a quick burst of noise for the "crack" at the start.
    auto& crack = r.add("crack");
    crack.noise(sfx::NoiseColor::White)
         .shape(Traj::ar(0.02, 1.0, 0.0))
         .len(0.08)
         .hp(Traj(3000.0))
         .gain(0.4);

    r.layers[0].echo(0.12, 0.35, 0.25).reverb(0.8, 0.5, 0.2);   // processors apply in the order written

    sfx::Rendered out = sfx::render(r, /*seed=*/1);
    out.save("sfx_03_custom_recipe.wav");
}
