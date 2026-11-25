// Genre presets restyle a melody (dynamics, register, articulation) and its instrument (timbre, effects).
#include "wauvio/ext.hpp"
using namespace wauvio;
namespace gn = wauvio::music::genre;

int main() {
    music::Melody m;
    for (int n : {60, 62, 64, 65, 67, 69, 71, 72}) m.push_back({n, 0.25, music::Dynamics::mf});

    for (auto preset : { gn::presets::jazz(), gn::presets::metal(), gn::presets::ambient() }) {
        instruments::CleanElectricGuitar gtr;                  // a fresh instrument per style
        music::Melody styled = m;
        gn::apply_to_melody(styled, *preset, gn::Role::Melodic, &gtr, /*seed=*/7);
        gn::style_instrument(gtr, *preset, gn::Role::Melodic);  // timbre + effect wishes

        music::Arrangement a; a.add(gtr, styled);
        render(a, "music_07_genre_" + preset->name + ".wav");
    }
}
