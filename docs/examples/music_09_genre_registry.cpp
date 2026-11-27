// Look genres up by name, browse them by family or tempo, and blend or weaken them.
#include "wauvio/ext.hpp"
#include <cstdio>
using namespace wauvio;
namespace gn = wauvio::music::genre;

int main() {
    // Names are matched loosely: case, spaces and punctuation do not matter, and aliases work.
    gn::GenrePresetPtr dnb = gn::find("drum and bass");
    if (!dnb) return 1;
    std::printf("%s (%s), typically %g-%g BPM\n", dnb->name.c_str(), dnb->family.c_str(), dnb->bpm_min, dnb->bpm_max);

    for (const std::string& f : gn::families()) std::printf("family: %s (%zu genres)\n", f.c_str(), gn::in_family(f).size());
    for (auto& g : gn::for_bpm(140)) std::printf("  fits 140 BPM: %s\n", g->name.c_str());

    // Blend two genres (t = 0 is the first, 1 the second), or apply one at reduced strength.
    gn::GenrePresetPtr mix  = gn::blend(*gn::find("jazz"), *gn::find("metal"), 0.35);
    gn::GenrePresetPtr mild = gn::scaled(gn::find("rock"), 0.5);

    music::Melody m;
    for (int n : {64, 67, 71, 72, 71, 67, 64, 62}) m.push_back({n, 0.25, music::Dynamics::mf});

    int i = 0;
    for (gn::GenrePresetPtr p : { mix, mild }) {
        instruments::CleanElectricGuitar gtr;
        music::Melody styled = m;
        // Passing a tempo (last argument) also applies the genre's TIMING: swing, groove, quantise, humanise.
        gn::apply_to_melody(styled, *p, gn::Role::Melodic, &gtr, /*seed=*/4, /*bpm=*/120.0);
        gn::style_instrument(gtr, *p, gn::Role::Melodic);
        music::Arrangement a; a.add(gtr, styled);
        render(a, "music_09_genre_registry_" + std::to_string(i++) + ".wav");
    }
}
