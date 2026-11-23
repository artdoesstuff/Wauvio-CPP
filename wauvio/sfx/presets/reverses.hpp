#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_reverses(Registry& R) {
                Def d{R, Category::Reverses};

                d("Reverse Pluck", Evolving(), "A plucked string note with a reverb tail, reversed: the string seems to grow out of silence into the pick.",
                  {"reverse", "pluck", "string", "swell"}, {"duration=1.8", "pitch=60", "brightness=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("pluck").pluck(c.p("pitch"), { 0.0 }, 1.0 - c.p("brightness")).decay_scale(2.0).len(c.duration).reverb(1.2, 0.4, 0.8).reverse().place(0, c.p("width")).fade(2, 3);
                  }, true);

                d("Reverse Bell", Evolving(), "A struck bell and its reverb, reversed: the ring swells up to the strike.",
                  {"reverse", "bell", "swell", "inharmonic"}, {"duration=2.5", "pitch=72", "decay=0.7", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("bell").modal(modes_bell(), c.p("pitch"), { 0.0 }).decay_scale(0.8 + 1.6 * c.p("decay")).len(c.duration).reverb(1.6, 0.4, 0.8).reverse().place(0, c.p("width")).fade(2, 3);
                  }, true);

                d("Reverse Noise Burst", Evolving(), "A short band-passed noise burst with a long reverb tail, reversed: a hiss that swells and stops dead.",
                  {"reverse", "noise", "burst", "swell"}, {"duration=1.8", "brightness=0.6", "decay=0.7", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).bp(c.lg(c.p("brightness"), 2000, 7000), 0.9).shape(env_hit(0.002, 0.002)).len(0.12).ring(std::max(0.0, c.duration - 0.12)).reverb(1.0 + 2.5 * c.p("decay"), 0.4, 1.4).reverse().place(0, c.p("width")).fade(2, 3);
                  }, true);

                d("Reverse Chord", Evolving(), "A plucked chord and its tail reversed: a tonal swell that ends on the chord's attack.",
                  {"reverse", "chord", "swell", "tonal"}, {"duration=2.5", "pitch=55", "chord=4", "brightness=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 8, 1.2).lp(c.lg(c.p("brightness"), 2500, 8000)).shape(env_hit(0.005, 0.003)).len(c.duration * 0.6).ring(c.duration * 0.4).reverb(2.2, 0.45, 1.0).reverse().place(0, c.p("width")).fade(2, 3);
                  }, true);

                d("Reverse Bass Swell", Evolving(), "A decaying bass note, reversed: a low swell that builds into a sudden stop.",
                  {"reverse", "bass", "swell", "low"}, {"duration=2", "pitch=34", "intensity=0.5", "brightness=0.4"},
                  [](Rc r, Cc c) {
                      r.add("bass").osc(Wave::Saw, c.p("pitch")).sub(0.7).drive(1.5 + 3.0 * c.p("intensity")).lp(c.lg(c.p("brightness"), 500, 3000), 1.5, 4).shape(Traj::decay(0.003)).reverse().fade(2, 3);
                  }, true);

                d("Reverse Percussion Flourish", Evolving(), "A quick tom-and-snare run with its tail, reversed: the hits arrive in reverse order and swell.",
                  {"reverse", "percussion", "flourish", "drum"}, {"duration=1.8", "pitch=55", "brightness=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      for (int i = 0; i < 4; ++i)
                          r.add("tom" + std::to_string(i)).osc(Wave::Sine, Traj::linear(c.p("pitch") - 5.0 * i + 10, c.p("pitch") - 5.0 * i)).shape(env_hit(0.004, 0.003)).at(i * 0.13).len(0.4).ring(0.9).reverb(1.0, 0.5, 0.7).place(-0.5 + 0.35 * i, 0.4).fade(0.2, 10);
                      r.add("snare").noise(NoiseColor::White).bp(c.lg(c.p("brightness"), 2000, 5000), 0.9).pulses_at({ 0.0, 0.1, 0.2, 0.3 }, 60.0, 0.3).shape(env_hit(0.002, 0.003)).len(0.5).ring(0.9).reverb(1.0, 0.5, 0.7).gain(0.6).place(0, 0.7).fade(0.1, 10);
                      r.reverse_all = true; r.fade_in_ms = 3.0; r.fade_out_ms = 2.0;
                  }, true);

                d("Reverb-Like Synthetic Tail", OneShot(), "A tiny noise click feeding a damped synthetic reverb with a long decay and a slow echo: the tail on its own.",
                  {"tail", "reverb-like", "decay", "ambience"}, {"duration=0.3", "tail=5", "brightness=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("click").noise(NoiseColor::White).lp(c.lg(c.p("brightness"), 2500, 12000)).shape(env_hit(0.002, 0.001)).len(0.08).ring(c.p("tail")).echo(0.37, 0.4, 0.5, 0.6).reverb(1.5 + 0.9 * c.p("tail"), 0.55, 1.5).place(0, c.p("width")).fade(0.1, 40);
                  });

                d("Infinite-Rise Illusion", Evolving(), "A Shepard-style stack: five saw layers an octave apart each glide up an octave under a bell-shaped level window over the whole 5-octave span, so the pitch seems to rise forever.",
                  {"illusion", "shepard", "riser", "endless"}, {"duration=6", "pitch=36", "brightness=0.5", "width=0.6"},
                  [](Rc r, Cc c) {
                      for (int k = 0; k < 5; ++k) {
                          const double lo = c.p("pitch") + 12.0 * k;
                          Traj window(0.001);                                                    // level depends on the layer's absolute position in the 5-octave span
                          for (int i = 1; i <= 24; ++i) { const double u = static_cast<double>(i) / 24; const double pos = (k + u) / 5.0; window.to(u, std::max(0.001, std::exp(-0.5 * std::pow((pos - 0.5) / 0.22, 2.0)))); }
                          r.add("octave" + std::to_string(k)).osc(Wave::Saw, Traj::linear(lo, lo + 12.0)).lp(c.lg(c.p("brightness"), 1800, 7000), 0.9, 4).shape(window).gain(0.8).place(-0.5 + 0.25 * k, c.p("width")).fade(20, 20);
                      }
                  });

                d("Tail Swell", Evolving(), "A short chord pluck flooded with reverb whose tail then swells in level: the wash comes up after the note has gone.",
                  {"tail", "swell", "reverb-like", "chord"}, {"duration=3.5", "pitch=55", "chord=4", "brightness=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 8, 1.1).lp(c.lg(c.p("brightness"), 2500, 9000)).shape(env_hit(0.005, 0.003)).len(0.5).ring(std::max(0.0, c.duration - 0.5)).reverb(3.2, 0.45, 2.0)
                          .after(Traj(0.2).to(0.1, 0.2).to(0.85, 1.0, Interp::Exp).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(1, 15);
                  });

                d("Tail Suction", Evolving(), "A falling, darkening noise-and-sine pull whose level rises towards the end and then vanishes: a sound drawn into a drain.",
                  {"tail", "suction", "fall", "transition"}, {"duration=2", "pitch=72", "range=48", "brightness=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::Pink).lp(Traj::exp(c.lg(c.p("brightness"), 5000, 14000), 120), 1.5, 4).shape(env_swell(0.92, 0.1)).place(0, c.p("width")).fade(2, 4);
                      r.add("sine").osc(Wave::Sine, Traj::linear(c.p("pitch"), c.p("pitch") - c.p("range"))).shape(env_swell(0.92, 0.05)).gain(0.5).fade(2, 4);
                  });

                d("Transition Vacuum", OneShot(), "A dull low-passed hiss collapses to near-silence while a muffled thud lands at the end: the room empties before the next section.",
                  {"transition", "vacuum", "silence", "fall"}, {"duration=2.2", "tail=0.8", "pitch=33", "brightness=0.4", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("hiss").noise(NoiseColor::Brown).lp(Traj::exp(c.lg(c.p("brightness"), 3000, 9000), 60), 1.0, 4).shape(Traj(1.0).to(0.6, 0.3).to(0.9, 0.003, Interp::Exp)).len(c.duration).place(0, c.p("width")).fade(1, 8);
                      r.add("thud").osc(Wave::Sine, Traj::linear(c.p("pitch") + 7, c.p("pitch"))).lp(220).shape(env_hit(0.004, 0.003)).at(c.duration * 0.88).len(0.5).ring(c.p("tail")).reverb(0.8, 0.7, 0.4).gain(0.7).fade(0.5, 25);
                  });

                d("Spectral Fade", Evolving(), "A bright, dense chord whose highs are removed progressively by a spectral tilt while a long tail rings: the colour drains out of the sound.",
                  {"fade", "spectral", "darkening", "tail"}, {"duration=4", "tail=1.5", "pitch=55", "chord=1", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 24, 0.7).spectral(SpectralP::TiltFade, Traj::linear(0.0, 1.0)).shape(Traj(1.0).to(0.8, 0.8).to(1.0, 0.003, Interp::Exp)).len(c.duration).ring(c.p("tail"))
                          .reverb(1.5 + c.p("tail"), 0.6, 0.6).place(0, c.p("width")).fade(1, 25);
                  });

                d("Pitch-Reversed Illusion", Evolving(), "A note whose pitch falls as it decays, reversed: it swells upward in pitch and level and ends on the attack.",
                  {"reverse", "pitch", "illusion", "swell"}, {"duration=2", "pitch=55", "range=12", "brightness=0.6", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("saw").osc(Wave::Saw, Traj::linear(c.p("pitch") + c.p("range"), c.p("pitch"))).unison(3, 12).lp(c.lg(c.p("brightness"), 2500, 9000), 1.2, 4).shape(Traj::decay(0.003)).len(c.duration).reverb(1.4, 0.45, 0.6).reverse().place(0, c.p("width")).fade(2, 3);
                  }, true);

                d("Decaying Shimmer", OneShot(), "A bright shimmer whose partials fade from the top down while repeats echo and darken: glitter dying out.",
                  {"shimmer", "decay", "echo", "tail"}, {"duration=2", "tail=1.5", "pitch=79", "chord=4", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("shimmer").additive(c.p("pitch"), c.chord_notes(), 8, Traj::linear(0.6, 3.0)).spread(Traj(12)).hp(700).trem(7.0, 0.3).shape(env_hit(0.01, 0.003)).len(c.duration).ring(c.p("tail")).echo(0.31, 0.55, 0.7, 0.55)
                          .place(0, c.p("width")).fade(1, 25);
                  });

                d("Reverse Glitch Cascade", Evolving(), "A chord run through bit reduction, dropouts and repeated slices, reversed: the fragments climb in on themselves.",
                  {"reverse", "glitch", "cascade", "chord"}, {"duration=2.5", "pitch=60", "chord=1", "bits=6", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 10, 1.0).crush(c.p("bits"), 2.0 + 4.0 * c.p("intensity")).dropout(12.0 + 14.0 * c.p("intensity"), 8, 60, 1.0).stutter(0.06, 3, 0.5)
                          .shape(Traj::decay(0.004)).reverse().place(0, 0.7).fade(2, 3);
                  }, true);

                d("Transition Sweep-Down", OneShot(), "A falling band-pass noise sweep and a falling sine, trailed by a darkening echo: the outgoing counterpart to a riser.",
                  {"transition", "sweep", "falling", "echo"}, {"duration=2", "tail=1.5", "pitch=79", "brightness=0.7", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).bp(Traj::exp(c.lg(c.p("brightness"), 4000, 11000), 200), 1.8, 4).shape(Traj(0.3).to(0.1, 1.0, Interp::Smooth).to(1.0, 0.003, Interp::Exp)).len(c.duration).ring(c.p("tail"))
                          .echo(0.29, 0.5, 0.7, 0.65).place(0, c.p("width")).fade(2, 25);
                      r.add("sine").osc(Wave::Sine, Traj::linear(c.p("pitch"), c.p("pitch") - 36)).shape(Traj(0.3).to(0.1, 1.0, Interp::Smooth).to(1.0, 0.003, Interp::Exp)).len(c.duration).ring(c.p("tail") * 0.5).echo(0.29, 0.45, 0.6, 0.65).gain(0.4).fade(2, 25);
                  });
            }
        }
    }
}
