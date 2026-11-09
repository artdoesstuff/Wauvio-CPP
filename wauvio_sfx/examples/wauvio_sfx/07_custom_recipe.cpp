// Build your own effect from sources and processors, render it, and register it as a preset.
#include "../../wauvio_sfx.hpp"
#include <filesystem>
#include <iostream>

int main() {
    namespace sfx = wauvio::sfx;
    using sfx::Traj; using sfx::Wave; using sfx::NoiseColor;
    std::filesystem::create_directories("sfx_examples_out");

    // 1) a Recipe is plain data: layers of one source plus a chain of processors
    sfx::Recipe r;
    r.name = "my_whoosh"; r.duration = 2.0; r.tail = 1.0;
    r.add("air").noise(NoiseColor::Pink)
        .bp(Traj::exp(200, 6000), 3.0, 4)              // cutoff trajectory in Hz, exponential
        .shape(sfx::Traj(0.02).to(0.7, 1.0, sfx::Interp::Exp).to(1.0, 0.001, sfx::Interp::Exp))
        .place(0.0, 0.9).reverb(1.2, 0.5, 0.5);
    r.add("tone").osc(Wave::Saw, Traj::linear(40, 76)).unison(3, 12).lp(Traj::exp(300, 5000), 1.5, 4).gain(0.4);
    sfx::render(r, /*seed=*/5).save("sfx_examples_out/my_whoosh.wav");

    // 2) the same thing as a preset with parameters, in your own registry next to the built-in catalog
    sfx::Registry mine = sfx::builtin_registry();
    sfx::Preset p;
    p.name = "My Whoosh"; p.category = sfx::Category::Risers; p.behavior = sfx::Behavior::Evolving;
    p.description = "A pink-noise whoosh with a saw underneath, tunable brightness.";
    p.tags = { "custom", "whoosh" };
    p.params = sfx::params_from({ "duration=2", "brightness=0.6", "pitch=40" });
    p.build = [](sfx::Recipe& rec, const sfx::BuildCtx& c) {
        rec.add("air").noise(NoiseColor::Pink).bp(Traj::exp(200, c.lg(c.p("brightness"), 2000, 9000)), 3.0, 4).shape(sfx::env_swell());
        rec.add("tone").osc(Wave::Saw, Traj::linear(c.p("pitch"), c.p("pitch") + 36)).lp(4000).gain(0.4);
    };
    mine.add(p);
    sfx::Effect e(*mine.find("my_whoosh"));
    e.set("brightness", 0.9).render().save("sfx_examples_out/my_whoosh_preset.wav");
    std::cout << "registry now holds " << mine.size() << " presets (built-in catalog: " << sfx::builtin_registry().size() << ")\n";
}
