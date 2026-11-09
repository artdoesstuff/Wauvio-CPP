// Change parameters, seed and sample rate; list what a preset offers.
#include "../../wauvio_sfx.hpp"
#include <filesystem>
#include <iostream>

int main() {
    namespace sfx = wauvio::sfx;
    std::filesystem::create_directories("sfx_examples_out");

    const sfx::Preset* p = sfx::find("cinematic_impact");
    std::cout << p->name << ": " << p->description << "\nparameters:\n";
    for (const auto& s : p->params) std::cout << "  " << s.name << " = " << s.def << "  [" << s.min << " .. " << s.max << " " << s.unit << "]\n";

    sfx::Effect hit = sfx::make("cinematic_impact");
    hit.set("duration", 3.0).set("sub", 1.0).set("brightness", 0.3).set("tail", 4.0).seed(1234);

    sfx::RenderSettings settings;
    settings.sample_rate = 48000;                      // 8000 .. 192000
    settings.peak_db = -3.0;                           // leave some headroom
    hit.render(settings).save("sfx_examples_out/impact_dark_long.wav");

    try { hit.set("brightness", 7.0); }                // invalid values are rejected, never silently clamped
    catch (const sfx::Error& e) { std::cout << "rejected: " << e.what() << "\n"; }
}
