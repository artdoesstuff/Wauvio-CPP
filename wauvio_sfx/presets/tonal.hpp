#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_tonal(Registry& R) {
                Def d{R, Category::Tonal};

                d("Chord Swell", Evolving(), "The chosen chord on additive saw-like partials swells in under an opening low-pass and fades into a short tail.",
                  {"chord", "swell", "pad", "tonal"}, {"duration=3.5", "tail=1.2", "pitch=48", "chord=4", "brightness=0.6", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 14, 1.1).spread(Traj(8)).lp(Traj::exp(400, c.lg(c.p("brightness"), 2500, 10000)), 1.0, 4).shape(env_swell(0.7, 0.02, 0.01)).len(c.duration).ring(c.p("tail"))
                          .reverb(1.0 + c.p("tail"), 0.5, 0.4).place(0, c.p("width")).fade(5, 20);
                  });

                d("Chord Fall", OneShot(), "The chosen chord drops an octave in pitch while the filter closes and the level decays.",
                  {"chord", "fall", "descending", "tonal"}, {"duration=2.5", "pitch=60", "chord=1", "range=12", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(Traj::linear(c.p("pitch"), c.p("pitch") - c.p("range")), c.chord_notes(), 12, 1.0).lp(Traj::exp(c.lg(c.p("brightness"), 3000, 10000), 300), 1.2, 4).shape(env_hit(0.01, 0.002)).place(0, 0.8).fade(0.5, 20);
                  });

                d("Harmonic Transition", Evolving(), "One chord cross-fades into a second chord built on another degree of the chosen scale.",
                  {"chord", "transition", "harmonic", "modulation"}, {"duration=3", "pitch=52", "chord=0", "scale=1", "width=0.9"},
                  [](Rc r, Cc c) {
                      const auto deg = scale_degrees(static_cast<int>(c.p("scale"))); const double root2 = c.p("pitch") + scale_step(deg, 3);
                      r.add("from").additive(c.p("pitch"), c.chord_notes(), 8, 1.2).lp(3500, 1.0, 4).shape(Traj(0.001).to(0.12, 1.0, Interp::Smooth).to(0.45, 1.0).to(0.65, 0.001, Interp::Smooth)).place(-0.3, c.p("width")).fade(5, 10);
                      r.add("to").additive(root2, c.chord_notes(), 8, 1.2).lp(3500, 1.0, 4).shape(Traj(0.001).to(0.35, 0.001).to(0.55, 1.0, Interp::Smooth).to(0.88, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0.3, c.p("width")).fade(5, 10);
                  });

                d("Tonal Riser Cluster", Evolving(), "One glissando voice per chord tone, each starting a different distance below its target: the cluster converges onto the chord as it rises.",
                  {"riser", "chord", "converging", "tonal"}, {"duration=3.5", "pitch=48", "chord=0", "range=12", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      const auto ch = c.chord_notes(); Rng g = c.rng(3);
                      for (size_t i = 0; i < ch.size(); ++i) {
                          const double target = c.p("pitch") + ch[i]; const double start = target - c.p("range") - g.range(0.0, 7.0);
                          r.add("voice" + std::to_string(i)).osc(Wave::Saw, Traj::linear(start, target)).unison(3, 12).lp(Traj::exp(400, c.lg(c.p("brightness"), 2500, 9000)), 1.2, 4).shape(Traj::exp(0.08, 1.0)).gain(0.8)
                              .place(-0.6 + 1.2 * static_cast<double>(i) / std::max<size_t>(1, ch.size() - 1), 0.6).fade(1, 10);
                      }
                  });

                d("Arpeggio Rise", OneShot(), "Plucked notes climbing the chosen scale, evenly spaced across the effect and ringing into each other.",
                  {"arpeggio", "rising", "pluck", "scale"}, {"duration=2", "pitch=48", "scale=1", "steps=12", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps"));
                      r.add("arp").pluck(c.p("pitch"), c.evenly(n, 0.0, 0.85), 1.0 - c.p("brightness")).hit_notes(c.scale_walk(n, 0, 2)).decay_scale(1.4).hit_jitter(0.0, 0.15).place(0, 0.8).fade(0.2, 25);
                  });

                d("Arpeggio Fall", OneShot(), "Plucked notes stepping down the chosen scale: the mirror image of Arpeggio Rise.",
                  {"arpeggio", "falling", "pluck", "scale"}, {"duration=2", "pitch=84", "scale=1", "steps=12", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps"));
                      r.add("arp").pluck(c.p("pitch"), c.evenly(n, 0.0, 0.85), 1.0 - c.p("brightness")).hit_notes(c.scale_walk(n, 0, -2)).decay_scale(1.4).hit_jitter(0.0, 0.15).place(0, 0.8).fade(0.2, 25);
                  });

                d("Pitch Staircase", Evolving(), "A soft triangle tone that climbs the chosen scale one degree at a time, each step re-gated with a short envelope.",
                  {"staircase", "scale", "stepped", "soft"}, {"duration=3", "pitch=48", "scale=4", "steps=10", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const auto w = c.scale_walk(n, 0, 1); Traj t(c.p("pitch") + w[0]);
                      for (int i = 1; i < n; ++i) t.to(static_cast<double>(i) / n, c.p("pitch") + w[static_cast<size_t>(i)], Interp::Step);
                      r.add("tone").osc(Wave::Triangle, t).lp(c.lg(c.p("brightness"), 1500, 7000)).pulses(n / c.duration, 800.0 * c.duration / n, 0.85).shape(Traj(0.5).to(0.9, 1.0).to(1.0, 0.003, Interp::Exp)).place(0, 0.4).fade(0.5, 15);
                  });

                d("Octave Sweep", Evolving(), "A saw glides up two octaves while a second saw an octave above fades in: the sweep seems to double.",
                  {"sweep", "octave", "saw", "rising"}, {"duration=3", "pitch=36", "range=24", "brightness=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("low").osc(Wave::Saw, Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range"))).unison(3, 10).lp(Traj::exp(400, c.lg(c.p("brightness"), 3000, 10000)), 1.5, 4).shape(Traj::exp(0.1, 1.0)).place(-0.2, c.p("width")).fade(1, 10);
                      r.add("high").osc(Wave::Saw, Traj::linear(c.p("pitch") + 12, c.p("pitch") + c.p("range") + 12)).unison(3, 10).lp(Traj::exp(600, 9000), 1.5, 4).shape(Traj(0.001).to(0.4, 0.05, Interp::Exp).to(1.0, 0.9, Interp::Exp)).gain(0.7).place(0.2, c.p("width")).fade(1, 10);
                  });

                d("Harmonic Glissando", Evolving(), "An additive voice sliding up a fifth-and-octave interval while its upper partials grow stronger.",
                  {"glissando", "harmonic", "additive", "rising"}, {"duration=3", "pitch=45", "range=19", "brightness=0.6", "width=0.7"},
                  [](Rc r, Cc c) {
                      r.add("gliss").additive(Traj::smooth(c.p("pitch"), c.p("pitch") + c.p("range")), { 0.0, 12.0 }, 16, Traj::linear(2.2, 0.5 + 0.8 * (1.0 - c.p("brightness")))).spread(Traj(6)).shape(env_swell(0.8, 0.05, 0.003)).place(0, c.p("width")).fade(2, 15);
                  });

                d("Dissonance-to-Consonance Sweep", Evolving(), "Chord tones start crowded within a few semitones of each other and glide apart onto the chosen chord: tension resolving.",
                  {"chord", "resolution", "dissonance", "sweep"}, {"duration=4", "pitch=48", "chord=0", "spread=0.5", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      const auto ch = c.chord_notes();
                      for (size_t i = 0; i < ch.size(); ++i) {
                          const double target = c.p("pitch") + ch[i]; const double start = c.p("pitch") + 0.6 * static_cast<double>(i) * (0.5 + c.p("spread")) + 0.3;
                          r.add("tone" + std::to_string(i)).osc(Wave::Saw, Traj(start).to(0.45, start, Interp::Linear).to(0.85, target, Interp::Smooth)).unison(2, 8).lp(c.lg(c.p("brightness"), 1500, 6000), 1.0, 4)
                              .shape(Traj(0.001).to(0.12, 1.0, Interp::Smooth).to(0.9, 1.0).to(1.0, 0.003, Interp::Exp)).gain(0.8).place(-0.5 + 1.0 * static_cast<double>(i) / std::max<size_t>(1, ch.size() - 1), 0.5).fade(5, 15);
                      }
                  });

                d("Tonal Reverse Swell", Evolving(), "A chord stab with a long reverb, reversed: the chord appears to be sucked in and lands on its attack.",
                  {"chord", "reverse", "swell", "tonal"}, {"duration=3", "pitch=55", "chord=5", "brightness=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 10, 1.1).lp(c.lg(c.p("brightness"), 2500, 9000)).shape(env_hit(0.02, 0.003)).len(0.7).ring(std::max(0.0, c.duration - 0.7)).reverb(3.0, 0.4, 1.4).reverse().place(0, c.p("width")).fade(2, 3);
                  }, true);

                d("Tension Cluster", Sustained(), "A static semitone cluster of saws with slow beating and a gentle tremolo: sustained tension that never moves.",
                  {"tension", "cluster", "sustained", "dissonant"}, {"duration=6", "pitch=50", "spread=0.5", "intensity=0.5", "width=0.8"},
                  [](Rc r, Cc c) {
                      const double s = 0.5 + c.p("spread");
                      r.add("cluster").additive(c.p("pitch"), { 0.0, s, 2 * s, 3 * s }, 8, 1.1).spread(Traj(12)).trem(Traj(0.4), 0.15 + 0.3 * c.p("intensity")).lp(3000, 0.9, 4).shape(Traj(0.001).to(0.15, 1.0, Interp::Smooth).to(0.85, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0, c.p("width")).fade(5, 12);
                  });

                d("Resolved Shimmer", Evolving(), "High shimmering partials begin as a dissonant cluster and cross-fade into the chosen chord, then ring out: shimmer that resolves.",
                  {"shimmer", "resolution", "chord", "bright"}, {"duration=3.5", "tail=1.5", "pitch=72", "chord=4", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("tense").additive(c.p("pitch"), { 0.0, 1.0, 6.0, 7.0 }, 6, 1.0).spread(Traj(12)).hp(600).trem(6.0, 0.3).lp(c.lg(c.p("brightness"), 5000, 15000)).shape(Traj(0.001).to(0.15, 1.0, Interp::Smooth).to(0.45, 0.8).to(0.65, 0.001, Interp::Smooth)).place(-0.3, 1);
                      r.add("resolved").additive(c.p("pitch"), c.chord_notes(), 6, 1.0).spread(Traj(8)).hp(600).trem(6.0, 0.3).lp(c.lg(c.p("brightness"), 5000, 15000)).shape(Traj(0.001).to(0.4, 0.001).to(0.65, 1.0, Interp::Smooth).to(1.0, 0.002, Interp::Exp)).len(c.duration).ring(c.p("tail"))
                          .reverb(1.5 + c.p("tail"), 0.4, 0.6).place(0.3, 1).fade(5, 20);
                  });

                d("Detuned Chord Bloom", Evolving(), "Detuned saw voices on every chord tone: the detune starts wide and settles while the filter opens, so the chord blooms into focus.",
                  {"chord", "bloom", "detuned", "supersaw"}, {"duration=3.5", "pitch=48", "chord=1", "brightness=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      const auto ch = c.chord_notes();
                      for (size_t i = 0; i < ch.size(); ++i)
                          r.add("tone" + std::to_string(i)).osc(Wave::Saw, c.p("pitch") + ch[i]).unison(5, Traj::linear(60, 10)).lp(Traj::exp(300, c.lg(c.p("brightness"), 2500, 9000)), 1.0, 4).shape(env_swell(0.7, 0.02, 0.003)).gain(0.8)
                              .place(-0.7 + 1.4 * static_cast<double>(i) / std::max<size_t>(1, ch.size() - 1), c.p("width")).fade(5, 15);
                  });

                d("Pitch Spiral", Evolving(), "A sine voice swirling up and down around a centre pitch with a growing swing and a rising centre: a spiralling motion.",
                  {"spiral", "pitch", "sine", "motion"}, {"duration=3", "pitch=60", "range=12", "depth=0.7", "width=0.6"},
                  [](Rc r, Cc c) {
                      Traj t(c.p("pitch")); const int K = 48;
                      for (int i = 1; i <= K; ++i) { const double u = static_cast<double>(i) / K; const double swing = c.p("range") * u * c.p("depth"); t.to(u, c.p("pitch") + 0.3 * c.p("range") * u + swing * std::sin(TWO_PI * (1.5 * u + 3.0 * u * u))); }
                      r.add("spiral").osc(Wave::Sine, t).fm(0.8, 2.0).shape(Traj(0.001).to(0.1, 1.0, Interp::Smooth).to(0.85, 1.0).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(2, 15);
                  });

                d("Harmonic Drop", OneShot(), "A chord's pitch collapses two octaves while its upper partials die away first, leaving a low thud: a harmonic drop-off.",
                  {"drop", "harmonic", "chord", "fall"}, {"duration=2.2", "pitch=60", "chord=0", "range=24", "sub=0.7"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(Traj::linear(c.p("pitch"), c.p("pitch") - c.p("range")), c.chord_notes(), 14, Traj::linear(0.6, 3.0)).shape(Traj(1.0).to(0.2, 0.8).to(1.0, 0.002, Interp::Exp)).place(0, 0.7).fade(0.3, 20);
                      r.add("sub").osc(Wave::Sine, Traj::linear(c.p("pitch") - 12, c.p("pitch") - c.p("range") - 12)).shape(Traj(1.0).to(0.2, 0.9).to(1.0, 0.002, Interp::Exp)).gain(0.7 * c.p("sub")).fade(0.3, 20);
                  });
            }
        }
    }
}
