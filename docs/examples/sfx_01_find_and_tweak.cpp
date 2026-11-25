// Find a preset, see what it can do, change its parameters, render it.
#include "wauvio/sfx.hpp"
#include <cstdio>
namespace sfx = wauvio::sfx;

int main() {
    // 1. Search the catalog (ids, names, tags and descriptions are searched).
    for (const sfx::Preset* p : sfx::search("riser"))
        std::printf("%-22s %s\n", p->id.c_str(), p->name.c_str());

    // 2. find() looks at a preset's metadata: every parameter with its default and allowed range.
    const sfx::Preset* info = sfx::find("noise_riser");
    for (const sfx::ParamSpec& s : info->params)
        std::printf("  %-12s default %-6g range %g..%g  %s\n", s.name.c_str(), s.def, s.min, s.max, s.description.c_str());

    //    make() gives you an Effect: the preset plus its current parameter values, ready to tweak and render.
    sfx::Effect e = sfx::make("noise_riser");

    // 3. set() validates: a value outside the allowed range throws sfx::Error (nothing is silently clamped).
    e.set("duration", 4.0).set("intensity", 0.8).set("brightness", 0.5);
    e.seed(2024);                    // same seed = same audio every time
    try { e.set("intensity", 5.0); } catch (const sfx::Error& err) { std::printf("rejected: %s\n", err.what()); }

    // 4. Render and save. Output is stereo 44.1 kHz, peak-normalised to -1 dBFS unless you change RenderSettings.
    sfx::Rendered audio = e.render();
    std::printf("%.2f s, peak %.1f dB\n", audio.seconds(), audio.stats().peak_db());
    audio.save("sfx_01_find_and_tweak.wav");
}
