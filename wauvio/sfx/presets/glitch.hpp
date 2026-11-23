#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_glitch(Registry& R) {
                Def d{R, Category::Glitch};

                d("Bitcrushed Burst", OneShot(), "A noise-and-saw burst quantised to a few bits with sample-and-hold decimation, decaying quickly.",
                  {"glitch", "bitcrush", "burst", "lofi"}, {"duration=0.8", "bits=4", "intensity=0.6", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("src").osc(Wave::Saw, Traj::linear(72, 48)).fm(2.0, 1.0).drive(2.0).crush(c.p("bits"), 2.0 + 14.0 * c.p("intensity")).lp(c.lg(c.p("brightness"), 4000, 16000)).shape(env_hit(0.003, 0.002)).fade(0.1, 10);
                      r.add("hiss").noise(NoiseColor::White).crush(c.p("bits"), 4).shape(env_hit(0.002, 0.002)).len(0.25).gain(0.5).fade(0.1, 5);
                  });

                d("Digital Stutter", OneShot(), "A held saw tone whose buffer keeps jumping back and repeating in short slices (about 70 ms by default; the rate parameter sets the slice length).",
                  {"glitch", "stutter", "repeat", "tonal"}, {"duration=1.5", "pitch=57", "intensity=0.7", "rate=14"},
                  [](Rc r, Cc c) {
                      r.add("tone").osc(Wave::Saw, Traj::linear(c.p("pitch"), c.p("pitch") - 5)).unison(2, 10).lp(2500, 1.5, 4).shape(Traj(1.0).to(0.8, 0.9).to(1.0, 0.01, Interp::Exp))
                          .stutter(1.0 / c.p("rate"), 3, 0.35 + 0.6 * c.p("intensity")).fade(0.3, 10);
                  });

                d("Buffer Repeat", OneShot(), "A bright pluck arpeggio whose buffer is grabbed on beat-length slices and repeated, in time with the tempo.",
                  {"glitch", "buffer", "repeat", "tempo-sync"}, {"duration=2", "pitch=60", "chord=1", "subdivision=0.5", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("arp").pluck(c.p("pitch"), c.evenly(8, 0.0, 0.6), 0.25).hit_notes([&] { auto n = c.chord_notes(); n.push_back(12); n.push_back(n[1] + 12); return n; }()).hit_jitter(0.0, 0.1)
                          .stutter(c.beats(c.p("subdivision")), 2, 0.4 + 0.5 * c.p("intensity"), 0.15, 1.0).place(0, 0.5).fade(0.3, 10);
                  });

                d("Granular Glitch", OneShot(), "Tiny 5-20 ms grains of a tone, scattered in pitch and position at high density: a shattered sound.",
                  {"glitch", "granular", "scatter", "shatter"}, {"duration=1.6", "pitch=64", "density=0.7", "intensity=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("grains").grain(GrainSource::Tone, Traj::linear(80 + 220 * c.p("density"), 30), Traj::linear(6, 22), c.p("pitch")).scatter(4.0 + 20.0 * c.p("intensity"), 0.9)
                          .shape(Traj(1.0).to(0.7, 0.8).to(1.0, 0.003, Interp::Exp)).place(0, c.p("width")).fade(0.2, 10);
                  });

                d("Digital Dropout", Sustained(), "A steady tone riddled with random holes of 5-60 ms, like a failing digital stream.",
                  {"glitch", "dropout", "stream", "tonal"}, {"duration=2", "pitch=64", "density=0.6", "intensity=0.8"},
                  [](Rc r, Cc c) {
                      r.add("tone").osc(Wave::Square, c.p("pitch")).unison(2, 4).lp(5000).dropout(4.0 + 22.0 * c.p("density"), 5, 25 + 40 * c.p("intensity"), 1.0).shape(Traj(1.0).to(0.9, 1.0).to(1.0, 0.01, Interp::Exp)).fade(1, 10);
                  });

                d("Aliased Sweep", Evolving(), "A square wave sweeping upward while sample-and-hold decimation grows, so the sweep folds back on itself with metallic aliasing.",
                  {"glitch", "alias", "sweep", "metallic"}, {"duration=2.5", "pitch=48", "range=48", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("sweep").osc(Wave::Square, Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range"))).crush(16.0, Traj::linear(1.5, 3.0 + 14.0 * c.p("intensity"))).shape(Traj(0.4).to(0.8, 1.0).to(1.0, 0.003, Interp::Exp)).fade(0.5, 10);
                  });

                d("Glitch Fill", OneShot(), "Short random-pitched square blips at scattered times: a fast digital fill for gaps.",
                  {"glitch", "fill", "blips", "random"}, {"duration=1.2", "pitch=72", "range=24", "steps=10", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(1); const int n = static_cast<int>(c.p("steps"));
                      r.add("blips").osc(Wave::Square, random_stairs(g, n, c.p("pitch") - c.p("range") / 2, c.p("pitch") + c.p("range") / 2)).pulses_at(c.scattered(n, 2, 0.0, 0.92), 28.0, 0.3)
                          .crush(7.0 - 3.0 * c.p("intensity"), 1.0 + 3.0 * c.p("intensity")).place(0, 0.4).fade(0.1, 8);
                  });

                d("Rhythmic Glitch", OneShot(), "A crushed tone and noise gated by a tempo-locked 16-step pattern chosen from the seed.",
                  {"glitch", "rhythmic", "gated", "tempo-sync"}, {"duration=2", "pitch=55", "subdivision=0.25", "density=0.6", "bits=5"},
                  [](Rc r, Cc c) {
                      const auto pat = hashed_pattern(c.seed, 16, 0.35 + 0.55 * c.p("density"));
                      r.add("tone").osc(Wave::Pulse, c.p("pitch")).pw(Traj::linear(0.2, 0.5)).crush(c.p("bits"), 3).gate(pat, c.p("subdivision"), 1.0, 1.0).fade(0.2, 8);
                      r.add("noise").noise(NoiseColor::White).hp(3000).crush(c.p("bits"), 6).gate(pat, c.p("subdivision"), 1.0, 1.0).gain(0.4).place(0, 0.8).fade(0.2, 8);
                  });

                d("Micro-Stutter", OneShot(), "A gliding saw repeated in 4-10 ms slices: the repetition turns into a buzzing pitched tone.",
                  {"glitch", "micro", "stutter", "buzz"}, {"duration=1", "pitch=52", "intensity=0.8", "range=12"},
                  [](Rc r, Cc c) {
                      r.add("saw").osc(Wave::Saw, Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range"))).stutter(0.004 + 0.006 * (1.0 - c.p("intensity")), 6, 0.85).lp(6000).shape(Traj(1.0).to(0.8, 0.9).to(1.0, 0.003, Interp::Exp)).fade(0.2, 10);
                  });

                d("Glitch Tail", OneShot(), "A short blip followed by a feedback echo that is crushed and stuttered as it decays.",
                  {"glitch", "tail", "echo", "crushed"}, {"duration=0.6", "tail=1.8", "pitch=69", "bits=6", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("blip").osc(Wave::Square, Traj::linear(c.p("pitch") + 7, c.p("pitch"))).shape(env_hit(0.004, 0.002)).len(0.12).ring(c.p("tail") + std::max(0.0, c.duration - 0.12))
                          .echo(0.11, 0.55 + 0.35 * c.p("intensity"), 1.0, 0.3).crush(c.p("bits"), 3).stutter(0.07, 2, 0.4, 0.1, 1.0).place(0, 0.5).fade(0.1, 15);
                  });

                d("Digital Fragmentation", Evolving(), "A saw chord whose spectrum is progressively deleted bin by bin until only fragments remain.",
                  {"glitch", "spectral", "fragment", "disintegrate"}, {"duration=3", "pitch=50", "chord=1", "intensity=0.8", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 14, 0.9).spectral(SpectralP::Disintegrate, Traj::linear(0.0, 0.5 + 0.48 * c.p("intensity")))
                          .stutter(0.09, 2, 0.25, 0.3, 1.0).shape(Traj(1.0).to(0.7, 1.0).to(1.0, 0.01, Interp::Exp)).place(0, c.p("width")).fade(1, 15);
                  });

                d("Sample-Rate Degradation", Evolving(), "A clean chord whose sample-rate falls steadily from full to a coarse 1/40: the sound turns into aliasing.",
                  {"glitch", "degradation", "sample-rate", "lofi"}, {"duration=3", "pitch=55", "chord=4", "bits=12", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 8, 1.1).crush(c.p("bits"), Traj::exp(1.0, 8.0 + 40.0 * c.p("intensity"))).shape(Traj(1.0).to(0.8, 0.9).to(1.0, 0.01, Interp::Exp)).place(0, 0.5).fade(1, 15);
                  });

                d("Randomized Retrigger", OneShot(), "A pitched tone retriggered at jittered intervals with a different random pitch on each repeat.",
                  {"glitch", "retrigger", "random", "pitched"}, {"duration=1.6", "pitch=60", "range=19", "rate=12", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(3);
                      r.add("tone").osc(Wave::Triangle, random_stairs(g, 24, c.p("pitch") - c.p("range") / 2, c.p("pitch") + c.p("range") / 2)).fm(2.0 + 5.0 * c.p("intensity"), 1.0)
                          .pulses(c.p("rate"), 55.0, 1.0, 0.45).shape(Traj(1.0).to(0.85, 0.8).to(1.0, 0.004, Interp::Exp)).place(0, 0.4).fade(0.1, 10);
                  });

                d("Buffer Freeze", Sustained(), "A bright arpeggio that suddenly sticks: the last 70 ms are looped for the rest of the effect.",
                  {"glitch", "freeze", "loop", "stuck"}, {"duration=2.5", "pitch=64", "chord=0", "intensity=0.5", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("arp").pluck(c.p("pitch"), c.evenly(6, 0.0, 0.32), 0.3).hit_notes(c.chord_notes()).freeze(0.36, 0.05 + 0.05 * (1.0 - c.p("intensity"))).lp(c.lg(c.p("brightness"), 2500, 12000)).place(0, 0.4).fade(0.3, 15);
                  });

                d("Spectral Glitch", OneShot(), "A noisy tone whose phases are scrambled in pulses: the sound smears into noise and snaps back.",
                  {"glitch", "spectral", "scramble", "smear"}, {"duration=1.8", "pitch=62", "intensity=0.7", "width=0.7"},
                  [](Rc r, Cc c) {
                      Traj amt(0.0); for (int i = 1; i <= 8; ++i) amt.to(i / 9.0, (i % 2) ? c.p("intensity") : 0.0, Interp::Smooth); amt.to(1.0, 0.0, Interp::Smooth);
                      r.add("src").osc(Wave::Saw, Traj::linear(c.p("pitch"), c.p("pitch") - 7)).unison(3, 12).spectral(SpectralP::Scramble, amt).stutter(0.06, 2, 0.3).shape(Traj(1.0).to(0.8, 0.9).to(1.0, 0.01, Interp::Exp))
                          .place(0, c.p("width")).fade(0.5, 15);
                  });

                d("Broken-Signal Burst", OneShot(), "Harsh: rectified, crushed noise and a square with dropouts, as if a transmission were failing.",
                  {"glitch", "harsh", "broken", "noise"}, {"duration=1.2", "bits=3", "intensity=0.8", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).drive(2.0 + 8.0 * c.p("intensity"), DriveP::Rectify).crush(c.p("bits"), 2.0 + 6.0 * c.p("intensity")).dropout(18.0, 6, 40, 1.0)
                          .lp(c.lg(c.p("brightness"), 3000, 14000)).shape(Traj(1.0).to(0.2, 0.9).to(1.0, 0.003, Interp::Exp)).place(0, 0.8).fade(0.1, 12);
                      r.add("square").osc(Wave::Square, Traj::linear(90, 40)).crush(5.0, 5.0).dropout(12.0, 8, 50, 1.0).shape(env_hit(0.002, 0.003)).len(0.6).gain(0.6).fade(0.1, 10);
                  });
            }
        }
    }
}
