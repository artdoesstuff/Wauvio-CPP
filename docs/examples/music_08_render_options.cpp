// RenderOptions control the output file: bit depth, loudness, reverb tails, looping.
#include "wauvio/ext.hpp"
using namespace wauvio;

int main() {
    instruments::Marimba marimba;
    marimba.reverb(0.8, 0.4);
    music::Arrangement a;
    a.add(marimba, {{60, 0.25}, {64, 0.25}, {67, 0.25}, {72, 0.75}});

    RenderOptions opt;
    opt.bit_depth        = 24;                 // 16, 24 or 32 (float)
    opt.normalize        = true;               // scale the loudest peak ...
    opt.normalize_peak   = 0.9;                // ... to this level
    opt.render_tail_sec  = 2.0;                // extra silence so the reverb is not cut off
    opt.loop_count       = 2;                  // repeat the result
    opt.dither           = DitherType::Triangular;
    opt.deterministic_seed_enabled = true;     // reproducible dither noise
    render(a, "music_08_render_options.wav", opt);
}
