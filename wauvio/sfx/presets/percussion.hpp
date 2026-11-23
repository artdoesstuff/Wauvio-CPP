#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            /// The six inharmonic square ratios of classic analogue cymbal / hat circuits.
            inline std::vector<double> cymbal_ratios() { return { 1.0, 1.483, 1.932, 2.546, 2.630, 3.900 }; }

            inline void register_percussion(Registry& R) {
                Def d{R, Category::Percussion};

                d("Synthetic Crash", OneShot(), "Six inharmonic square oscillators plus high-passed noise with a fast burst and a long exponential decay: an analogue-style crash.",
                  {"crash", "cymbal", "metallic", "analogue"}, {"duration=2.2", "pitch=80", "brightness=0.7", "decay=0.7", "width=0.9"},
                  [](Rc r, Cc c) {
                      const double f = c.lg(c.p("brightness"), 4500, 9500);
                      r.add("squares").osc(Wave::Square, c.p("pitch") - 12).stack(cymbal_ratios()).hp(f * 0.8, 1.0, 4).shape(Traj(1.0).to(0.04, 0.45, Interp::Exp).to(1.0, 0.002 + 0.01 * (1.0 - c.p("decay")), Interp::Exp)).place(0, c.p("width")).fade(0.1, 25);
                      r.add("noise").noise(NoiseColor::White).hp(f * 0.7, 1.0, 4).shape(Traj(1.0).to(0.05, 0.4, Interp::Exp).to(1.0, 0.002, Interp::Exp)).gain(0.7).place(0, 1).fade(0.1, 25);
                  });

                d("Noise Cymbal", OneShot(), "Only high-passed and band-limited noise with an exponential decay: a wideband, un-pitched cymbal.",
                  {"cymbal", "noise", "wideband", "simple"}, {"duration=1.4", "brightness=0.7", "decay=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).hp(c.lg(c.p("brightness"), 4000, 9000), 1.0, 4).lp(16000).shape(Traj(1.0).to(1.0, 0.0015 + 0.01 * (1.0 - c.p("decay")), Interp::Exp)).place(0, c.p("width")).fade(0.05, 20);
                  });

                d("Reverse Cymbal", Evolving(), "A cymbal wash with a long decay rendered backwards: it swells and cuts off right on the beat.",
                  {"cymbal", "reverse", "swell", "transition"}, {"duration=2.2", "brightness=0.7", "decay=0.7", "width=1"},
                  [](Rc r, Cc c) {
                      const double f = c.lg(c.p("brightness"), 4500, 9500);
                      r.add("squares").osc(Wave::Square, 68).stack(cymbal_ratios()).hp(f * 0.8, 1.0, 4).shape(Traj::decay(0.002 + 0.01 * (1.0 - c.p("decay")))).reverse().place(0, c.p("width")).fade(2, 2);
                      r.add("noise").noise(NoiseColor::White).hp(f * 0.7, 1.0, 4).shape(Traj::decay(0.002)).reverse().gain(0.7).place(0, 1).fade(2, 2);
                  }, true);

                d("Metallic Wash", Sustained(), "Dozens of inharmonic metal-mode strikes scattered over a few seconds: a shimmering, bell-metal wash.",
                  {"wash", "metallic", "shimmer", "cymbal-like"}, {"duration=3.5", "pitch=76", "density=0.7", "decay=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("hits").modal(modes_metal(), c.p("pitch"), c.scattered(static_cast<int>(20 + 50 * c.p("density")), 8, 0.0, 0.9)).hit_notes({ 0, 2, 5, 7, 12, 14 }).hit_jitter(0.0, 0.8).decay_scale(0.15 + 0.5 * c.p("decay")).strike(0.8).hp(2500)
                          .shape(Traj(0.4).to(0.3, 1.0, Interp::Smooth).to(1.0, 0.003, Interp::Exp)).place(0, c.p("width")).fade(2, 25);
                  });

                d("Synthetic Snare Fill", OneShot(), "Eight snare-like hits (band-passed noise over a pitched triangle tone) at accelerating gaps with rising pitch: a fill.",
                  {"snare", "fill", "drum", "accelerating"}, {"duration=1.6", "pitch=55", "intensity=0.6", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      std::vector<double> t; double x = 0.0, g = 0.2; for (int i = 0; i < 8; ++i) { t.push_back(x); x += g; g *= 0.78; } for (auto& v : t) v = v / x * 0.95;
                      r.add("noise").noise(NoiseColor::White).bp(c.lg(c.p("brightness"), 1800, 4500), 0.9).pulses_at(t, 110.0, 0.3, 1.0).shape(Traj::exp(0.5, 1.0)).place(0, 0.5).fade(0.1, 10);
                      r.add("body").osc(Wave::Triangle, Traj::linear(c.p("pitch") + 3, c.p("pitch") + 12)).pulses_at(t, 60.0, 0.3, 1.0).shape(Traj::exp(0.5, 0.9)).gain(0.5).fade(0.1, 10);
                  });

                d("Noise Snare Roll", Evolving(), "High-passed noise rolling at 10 to 50 hits per second with a steadily rising level: a classic snare-roll build.",
                  {"snare", "roll", "noise", "build"}, {"duration=2.5", "brightness=0.6", "intensity=0.6", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("roll").noise(NoiseColor::White).bp(Traj::exp(1500, c.lg(c.p("brightness"), 4000, 8000)), 0.8).pulses(Traj::exp(10, 50), 45.0, 0.95).shape(Traj::exp(0.1, 1.0)).place(0, c.p("width")).fade(0.1, 8);
                  });

                d("Electronic Tom Flourish", OneShot(), "Four sine toms stepping down in pitch, each with its own pitch dive: a quick electronic tom run.",
                  {"tom", "flourish", "drum", "descending"}, {"duration=1.2", "pitch=57", "decay=0.5", "width=0.8"},
                  [](Rc r, Cc c) {
                      for (int i = 0; i < 4; ++i) {
                          const double base = c.p("pitch") - 5.0 * i;
                          r.add("tom" + std::to_string(i)).osc(Wave::Sine, Traj::linear(base + 10, base)).drive(1.5).shape(env_hit(0.004, 0.003)).at(i * 0.14).len(0.25 + 0.5 * c.p("decay")).place(0.6 - 0.4 * i, c.p("width") * 0.5).fade(0.2, 15);
                      }
                  });

                d("Synthetic Kick Impact", OneShot(), "A sine that dives from 160 Hz towards a low note set by the pitch parameter in the first tens of milliseconds, with a click and light saturation.",
                  {"kick", "impact", "drum", "punch"}, {"duration=0.6", "pitch=33", "intensity=0.6", "transient=0.7"},
                  [](Rc r, Cc c) {
                      r.add("body").osc(Wave::Sine, Traj::pitch_hz(160, midi_to_hz(c.p("pitch")) * 1.3, Interp::Exp)).drive(1.5 + 3.0 * c.p("intensity")).shape(Traj(1.0).to(0.15, 0.7).to(1.0, 0.002, Interp::Exp)).fade(0.1, 15);
                      r.add("click").noise(NoiseColor::White).lp(6000).shape(env_hit(0.001, 0.001)).len(0.012).gain(0.5 * c.p("transient")).fade(0.02, 2);
                  });

                d("Clap Burst", OneShot(), "Four closely spaced band-passed noise bursts followed by a short room tail: a synthetic hand-clap.",
                  {"clap", "burst", "noise", "percussion"}, {"duration=0.5", "tail=0.3", "brightness=0.6", "intensity=0.6", "width=0.7"},
                  [](Rc r, Cc c) {
                      r.add("claps").noise(NoiseColor::White).bp(c.lg(c.p("brightness"), 900, 2200), 1.2 + 2.0 * c.p("intensity")).pulses_at({ 0.0, 0.025, 0.055, 0.09 }, 24.0, 0.4, 1.0).shape(Traj::decay(0.002)).len(c.duration).ring(c.p("tail"))
                          .reverb(0.5, 0.6, 0.35).place(0, c.p("width")).fade(0.05, 15);
                  });

                d("Metallic Percussion Hit", OneShot(), "A short inharmonic strike (high metal modes plus a noise click): a bell-metal tick with no sustain.",
                  {"percussion", "metallic", "hit", "short"}, {"duration=0.7", "pitch=70", "decay=0.4", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("metal").modal(modes_metal(), c.p("pitch"), { 0.0 }).decay_scale(0.15 + 0.7 * c.p("decay")).strike(1.5).hp(500).lp(c.lg(c.p("brightness"), 5000, 15000)).place(0, 0.3).fade(0.05, 15);
                  });

                d("Percussive Click Cluster", OneShot(), "A dense, jittered cloud of very short high-passed noise clicks: a crackling percussive texture.",
                  {"percussion", "clicks", "cluster", "crackle"}, {"duration=0.9", "density=0.7", "brightness=0.7", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("clicks").noise(NoiseColor::White).hp(c.lg(c.p("brightness"), 1500, 6000)).pulses(30 + 120 * c.p("density"), 4.0, 1.0, 0.8).shape(Traj(1.0).to(0.6, 0.8).to(1.0, 0.003, Interp::Exp)).place(0, c.p("width")).fade(0.05, 12);
                  });

                d("Synthetic Shaker Roll", Evolving(), "High noise pulsed at 8 to 24 per second with short decays; the level rises then drops away: a shaker roll.",
                  {"shaker", "roll", "noise", "percussion"}, {"duration=1.8", "brightness=0.7", "intensity=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("shaker").noise(NoiseColor::White).hp(c.lg(c.p("brightness"), 4500, 8500), 1.0, 4).pulses(Traj::linear(8, 24), 22.0 + 25.0 * (1.0 - c.p("intensity")), 0.9, 0.1).shape(Traj(0.2).to(0.8, 1.0, Interp::Smooth).to(1.0, 0.002, Interp::Exp))
                          .place(0, c.p("width")).fade(0.1, 15);
                  });

                d("Percussion Reverse Swell", Evolving(), "A metallic strike and a noise burst with a reverb tail, reversed: percussion that builds up into the beat.",
                  {"percussion", "reverse", "swell", "metallic"}, {"duration=2", "pitch=64", "decay=0.6", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("metal").modal(modes_metal(), c.p("pitch"), { 0.0 }).decay_scale(0.4 + 1.2 * c.p("decay")).len(c.duration).reverb(1.8, 0.45, 0.8).reverse().place(0, c.p("width")).fade(2, 3);
                      r.add("noise").noise(NoiseColor::White).bp(4500, 0.8).shape(env_hit(0.002, 0.002)).len(0.3).ring(std::max(0.0, c.duration - 0.3)).reverb(1.8, 0.4, 1.0).reverse().gain(0.6).place(0, 1).fade(2, 3);
                  }, true);

                d("Resonant Percussion Tail", OneShot(), "A woody knock that excites a comb resonator and a feedback echo: the ring is longer than the strike.",
                  {"percussion", "resonant", "tail", "wood"}, {"duration=0.7", "tail=2.2", "pitch=57", "decay=0.6", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("knock").modal(modes_wood(), c.p("pitch"), { 0.0 }).strike(2.5).len(c.duration).ring(c.p("tail")).comb(midi_to_hz(c.p("pitch")), 0.85 + 0.1 * c.p("decay"), 0.7).echo(0.17, 0.5, 0.6, 0.6)
                          .lp(c.lg(c.p("brightness"), 2500, 9000)).place(0, 0.5).fade(0.1, 30);
                  });
            }
        }
    }
}
