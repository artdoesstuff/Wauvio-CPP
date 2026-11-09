#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_rhythmic(Registry& R) {
                Def d{R, Category::Rhythmic};

                d("Gated Noise", Sustained(), "White noise chopped by a tempo-locked 16-step pattern chosen from the seed.",
                  {"rhythmic", "gate", "noise", "tempo-sync"}, {"duration=2", "subdivision=0.25", "density=0.6", "brightness=0.7", "swing=0"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).hp(c.lg(1.0 - c.p("brightness"), 200, 4000)).gate(hashed_pattern(c.seed, 16, 0.3 + 0.6 * c.p("density")), c.p("subdivision"), 1.0, 1.0, c.p("swing")).place(0, 0.8).fade(0.2, 8);
                  });

                d("Rhythmic Sweep", Evolving(), "A band-passed noise sweep rising across the effect and gated on every step: a build that ticks in time.",
                  {"rhythmic", "sweep", "gated", "build"}, {"duration=3", "subdivision=0.25", "brightness=0.7", "intensity=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("sweep").noise(NoiseColor::White).bp(Traj::exp(250, c.lg(c.p("brightness"), 3500, 11000)), 1.5 + 5.0 * c.p("intensity"), 4).gate({ 1, 0.3 }, c.p("subdivision") * 0.5, 0.9, 1.5)
                          .shape(Traj::exp(0.06, 1.0)).place(0, c.p("width")).fade(1, 6);
                  });

                d("Stutter Fill", OneShot(), "Snare-like noise-and-tone hits at accelerating intervals with rising pitch: a fill into the next bar.",
                  {"rhythmic", "fill", "snare-like", "accelerating"}, {"duration=1.5", "pitch=52", "intensity=0.7", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).bp(Traj::exp(1500, c.lg(c.p("brightness"), 3000, 6500)), 1.2).pulses(Traj::exp(6, 22), 70.0, 1.0).shape(Traj::exp(0.3, 1.0)).place(0, 0.7).fade(0.1, 8);
                      r.add("tone").osc(Wave::Triangle, Traj::linear(c.p("pitch"), c.p("pitch") + 12)).pulses(Traj::exp(6, 22), 55.0, 1.0).shape(Traj::exp(0.3, 0.8)).gain(0.5).fade(0.1, 8);
                  });

                d("Syncopated Glitch", OneShot(), "A crushed pulse tone gated in a 3+3+2 syncopation with swing: the pattern is fixed, only the sound is random.",
                  {"rhythmic", "syncopated", "glitch", "swing"}, {"duration=2", "pitch=60", "subdivision=0.25", "swing=0.2", "bits=5"},
                  [](Rc r, Cc c) {
                      r.add("tone").osc(Wave::Pulse, c.p("pitch")).pw(Traj::linear(0.15, 0.5)).crush(c.p("bits"), 2.0).gate({ 1, 0, 0, 1, 0, 0, 1, 0 }, c.p("subdivision"), 1.0, 1.0, c.p("swing")).place(0, 0.5).fade(0.2, 8);
                      r.add("click").noise(NoiseColor::White).hp(4000).gate({ 0, 1, 0, 0, 1, 0, 0, 1 }, c.p("subdivision"), 1.0, 0.5, c.p("swing")).gain(0.4).place(0, 0.9).fade(0.2, 8);
                  });

                d("Rhythmic Noise Roll", Evolving(), "A noise roll whose rate rises exponentially from 4 to 40 hits per second while the filter opens.",
                  {"rhythmic", "roll", "noise", "accelerating"}, {"duration=2.5", "brightness=0.7", "intensity=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("roll").noise(NoiseColor::White).lp(Traj::exp(1200, c.lg(c.p("brightness"), 5000, 14000)), 1.0, 4).pulses(Traj::exp(4, 40), 35.0, 0.9 + 0.1 * c.p("intensity")).shape(Traj::exp(0.1, 1.0)).place(0, c.p("width")).fade(0.2, 8);
                  });

                d("Pulse Burst", OneShot(), "Square blips at the chosen step length, each with a fast decay and the pitch stepping up every other pulse.",
                  {"rhythmic", "pulse", "blips", "tempo-sync"}, {"duration=1.6", "pitch=62", "subdivision=0.25", "range=12", "intensity=0.5"},
                  [](Rc r, Cc c) {
                      r.add("pulses").osc(Wave::Square, Traj::stairs(c.p("pitch"), c.p("pitch") + c.p("range"), 4)).lp(3500 + 6000 * c.p("intensity")).pulses(c.beat_hz(c.p("subdivision")), 70.0, 1.0)
                          .shape(Traj(1.0).to(0.8, 0.9).to(1.0, 0.004, Interp::Exp)).place(0, 0.4).fade(0.1, 8);
                  });

                d("Rhythmic Filter Chop", Sustained(), "A saw chord whose low-pass is switched between two cutoffs by a square LFO locked to the tempo: a hard, stepped filter figure.",
                  {"rhythmic", "filter", "chop", "tempo-sync"}, {"duration=3", "pitch=48", "chord=1", "subdivision=0.5", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 16, 1.0).lp(c.lg(c.p("brightness"), 700, 3500), 3.0, 4).filter_lfo(lfo_beats(Mod::Shape::Square, c.p("subdivision") * 2.0, 1.4)).shape(Traj(1.0).to(0.9, 1.0).to(1.0, 0.003, Interp::Exp)).place(0, 0.7).fade(0.3, 12);
                  });

                d("Retriggered Impact", OneShot(), "A low tonal hit retriggered at shrinking intervals while its pitch falls: the impact stutters into silence.",
                  {"rhythmic", "impact", "retrigger", "accelerating"}, {"duration=2", "pitch=40", "intensity=0.7", "range=12"},
                  [](Rc r, Cc c) {
                      r.add("hit").osc(Wave::Saw, Traj::linear(c.p("pitch") + c.p("range"), c.p("pitch"))).sub(0.6).drive(2.0 + 3.0 * c.p("intensity")).lp(1500, 1.0, 4).pulses(Traj::exp(3, 24), 110.0, 1.0)
                          .shape(Traj(1.0).to(0.8, 0.9).to(1.0, 0.003, Interp::Exp)).fade(0.2, 15);
                  });

                d("Accelerating Pulses", Evolving(), "Sine blips whose rate rises from 3 to 24 per second: the pulse train speeds up until it becomes a tone-like roll.",
                  {"rhythmic", "accelerating", "pulses", "sine"}, {"duration=2.5", "pitch=72", "intensity=0.5", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("blips").osc(Wave::Sine, c.p("pitch")).fm(1.0 + 3.0 * c.p("intensity"), 2.0).pulses(Traj::exp(3, 24), 40.0, 1.0).shape(Traj::exp(0.15, 1.0)).place(0, c.p("width")).fade(0.1, 10);
                  });

                d("Decelerating Pulses", Evolving(), "Sine blips slowing from 24 to 3 per second while the level fades: a pulse train winding down.",
                  {"rhythmic", "decelerating", "pulses", "sine"}, {"duration=2.5", "pitch=69", "intensity=0.5", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("blips").osc(Wave::Sine, Traj::linear(c.p("pitch"), c.p("pitch") - 7)).fm(1.0 + 3.0 * c.p("intensity"), 2.0).pulses(Traj::exp(24, 3), 70.0, 1.0).shape(Traj::exp(1.0, 0.03)).place(0, c.p("width")).fade(0.1, 15);
                  });

                d("Rhythmic Static", Sustained(), "Crackle and hiss gated by a tempo-locked pattern: static that grooves.",
                  {"rhythmic", "static", "crackle", "tempo-sync"}, {"duration=2", "subdivision=0.25", "density=0.5", "swing=0.1", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("crackle").noise(NoiseColor::Dust, 800 + 4000 * c.p("density")).hp(1500).gate({ 1, 0, 1, 1, 0, 1, 1, 0 }, c.p("subdivision"), 1.0, 0.5, c.p("swing")).place(0, c.p("width")).fade(0.2, 8);
                      r.add("hiss").noise(NoiseColor::Blue).lp(8000).gate({ 0, 1, 0, 0, 1, 0, 0, 1 }, c.p("subdivision"), 1.0, 1.0, c.p("swing")).gain(0.3).place(0, c.p("width")).fade(0.2, 8);
                  });

                d("Beat-Synced Tremolo Burst", OneShot(), "A bright saw chord chopped by a full-depth tremolo locked to the tempo, with a swell-and-cut envelope.",
                  {"rhythmic", "tremolo", "burst", "tempo-sync"}, {"duration=1.5", "pitch=60", "chord=0", "subdivision=0.25", "depth=1"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 14, 0.9).lp(7000, 1.0, 4).trem(c.beat_hz(c.p("subdivision") * 2.0), c.p("depth")).shape(Traj(0.1).to(0.5, 1.0, Interp::Smooth).to(1.0, 0.003, Interp::Exp)).place(0, 0.8).fade(0.5, 12);
                  });

                d("Polyrhythmic Texture", Sustained(), "Three pings looping over the same bar in 3, 4 and 5 steps: the pulses drift against each other.",
                  {"rhythmic", "polyrhythm", "texture", "tonal"}, {"duration=4", "pitch=72", "scale=5", "swing=0", "width=1"},
                  [](Rc r, Cc c) {
                      const auto n = c.scale_walk(3, 0, 2); const double bar = c.beats(4.0);
                      const int divs[3] = { 3, 4, 5 };
                      for (int k = 0; k < 3; ++k)
                          r.add("ping" + std::to_string(divs[k])).osc(Wave::Triangle, c.p("pitch") + n[static_cast<size_t>(k)]).fm(2.0, 3.0).pulses(divs[k] / bar, 220.0, 1.0).lp(6000).gain(0.9 - 0.15 * k).place(-0.7 + 0.7 * k, c.p("width")).fade(0.5, 12);
                  });

                d("Rhythmic Riser", Evolving(), "A noise-and-tone riser gated by an accelerating pulse train: a build whose chop rate rises from 4 to 36 per second.",
                  {"rhythmic", "riser", "accelerating", "build"}, {"duration=3.5", "pitch=48", "brightness=0.7", "intensity=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).bp(Traj::exp(300, c.lg(c.p("brightness"), 4000, 12000)), 1.5, 4).pulses(Traj::exp(4, 36), 30.0, 0.85).shape(Traj::exp(0.06, 1.0)).place(0, c.p("width")).fade(1, 5);
                      r.add("tone").osc(Wave::Saw, Traj::linear(c.p("pitch") - 12, c.p("pitch") + 12)).lp(Traj::exp(400, 5000), 2.0, 4).pulses(Traj::exp(4, 36), 35.0, 0.85).shape(Traj::exp(0.06, 0.7)).gain(0.5).fade(1, 5);
                  });

                d("Rhythmic Downlifter", Evolving(), "The riser's mirror: chops slow from 36 to 4 per second while the filter and pitch fall.",
                  {"rhythmic", "downlifter", "decelerating", "fall"}, {"duration=3.5", "pitch=60", "brightness=0.7", "intensity=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).bp(Traj::exp(c.lg(c.p("brightness"), 4000, 12000), 250), 1.5, 4).pulses(Traj::exp(36, 4), 45.0, 0.85).shape(Traj::exp(1.0, 0.04)).place(0, c.p("width")).fade(1, 12);
                      r.add("tone").osc(Wave::Saw, Traj::linear(c.p("pitch") + 12, c.p("pitch") - 12)).lp(Traj::exp(5000, 300), 2.0, 4).pulses(Traj::exp(36, 4), 50.0, 0.85).shape(Traj::exp(0.8, 0.03)).gain(0.5).fade(1, 12);
                  });

                d("Broken-Beat Texture", Sustained(), "Noise ticks and dusty clicks on a hashed, swung pattern with irregular gaps: a loop that never quite repeats.",
                  {"rhythmic", "broken-beat", "texture", "swing"}, {"duration=4", "subdivision=0.25", "density=0.5", "swing=0.3", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("ticks").noise(NoiseColor::White).bp(5000, 1.5).gate(hashed_pattern(c.seed, 32, 0.25 + 0.5 * c.p("density")), c.p("subdivision"), 1.0, 0.8, c.p("swing")).pulses(c.beat_hz(c.p("subdivision")), 25.0, 0.5, 0.2).place(0, c.p("width")).fade(0.2, 10);
                      r.add("dust").noise(NoiseColor::Dust, 600).hp(2000).gate(hashed_pattern(c.seed + 1, 32, 0.4), c.p("subdivision"), 1.0, 1.0, c.p("swing")).gain(0.5).place(0, c.p("width")).fade(0.2, 10);
                  });
            }
        }
    }
}
