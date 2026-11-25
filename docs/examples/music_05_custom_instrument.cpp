// Design your own modeled instrument by changing its TimbreRecipe.
#include "wauvio/ext.hpp"
using namespace wauvio;

// Start from an existing family class and adjust the recipe in the constructor.
class GlassPad : public music::SynthInstrument {
public:
    GlassPad() : SynthInstrument("Glass Pad") {
        recipe.osc1_shape    = osc::WaveShape::BL_Triangle;
        recipe.osc2_shape    = osc::WaveShape::Sine;
        recipe.osc2_ratio    = 3.0;               // a twelfth above: glassy overtone
        recipe.osc_mix       = 0.45;
        recipe.detune_cents  = 9;
        recipe.envelope      = env::DAHDSR(0, 0.6, 0, 0.5, 0.8, 1.2);   // slow attack, long release
        recipe.filter_cutoff = 4200;
        recipe.vibrato_depth_cents = 6;
        recipe.stereo_width  = 0.5;
    }
};

int main() {
    GlassPad pad;
    pad.reverb(0.9, 0.35);
    music::Track t(pad);
    t.add_chord({{60, 64, 67, 71}, 4.0, music::Dynamics::mp});
    t.add_chord({{58, 62, 65, 69}, 4.0, music::Dynamics::mp});
    music::Arrangement a; a.add_track(t);
    render(a, "music_05_custom_instrument.wav");
}
