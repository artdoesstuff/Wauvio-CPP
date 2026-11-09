#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_risers(Registry& R) {
                Def d{R, Category::Risers};

                d("Noise Riser", Evolving(), "White noise through a low-pass whose cutoff sweeps up exponentially while the level swells; the classic build-up.",
                  {"riser", "noise", "build", "transition"}, {"duration=3", "intensity", "brightness=0.7", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).lp(Traj::exp(180, c.lg(c.p("brightness"), 4000, 18000)), 0.7 + 3.0 * c.p("intensity"), 4)
                          .shape(Traj::exp(0.04, 1.0)).place(0, c.p("width")).gain(1.0).fade(1, 8);
                  });

                d("Tonal Riser", Evolving(), "A supersaw-style stack plus its fifth glides upward through an opening low-pass; a pitched build.",
                  {"riser", "tonal", "saw", "build"}, {"duration=4", "pitch=48", "range=12", "brightness", "intensity", "width=0.7"},
                  [](Rc r, Cc c) {
                      const double p = c.p("pitch"), rg = c.p("range");
                      r.add("saws").osc(Wave::Saw, Traj::linear(p - rg, p)).unison(5, Traj::linear(25, 8)).lp(Traj::exp(300, c.lg(c.p("brightness"), 2500, 12000)), 1.0 + 4.0 * c.p("intensity"), 4)
                          .shape(Traj::exp(0.1, 1.0)).place(0, c.p("width")).gain(1.0).fade(1, 8);
                      r.add("fifth").osc(Wave::Saw, Traj::linear(p - rg + 7, p + 7)).unison(3, 12).lp(Traj::exp(300, 6000), 1.5, 4).shape(Traj::exp(0.05, 0.7)).gain(0.5).place(0.2, 0.5).fade(1, 8);
                  });

                d("Pitch Riser", Evolving(), "A single triangle voice with a growing FM shimmer (a sine modulator) whose pitch rises exponentially across a wide range.",
                  {"riser", "pitch", "sweep", "fm"}, {"duration=2.5", "pitch=36", "range=48", "intensity", "brightness"},
                  [](Rc r, Cc c) {
                      r.add("voice").osc(Wave::Triangle, Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range"))).fm(Traj::linear(0.2, 1.0 + 7.0 * c.p("intensity")), 2.0)
                          .sub(0.2).lp(c.lg(c.p("brightness"), 3000, 16000)).shape(Traj::exp(0.15, 1.0)).gain(1.0).fade(1, 8);
                  });

                d("Reverse Riser", Evolving(), "A noise burst with a long reverb tail, reversed: the tail swells into a sudden stop.",
                  {"riser", "reverse", "noise", "swell"}, {"duration=3", "brightness=0.6", "decay=0.7", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("burst").noise(NoiseColor::White).lp(c.lg(c.p("brightness"), 2500, 14000)).shape(Traj::decay(0.002)).len(0.35).ring(std::max(0.0, c.duration - 0.35))
                          .reverb(0.8 + 3.5 * c.p("decay"), 0.35, 1.2).reverse().place(0, c.p("width")).gain(1.0).fade(2, 3);
                  }, true);

                d("Filter Sweep", Evolving(), "A detuned saw stack through a resonant low-pass that opens and then closes again.",
                  {"sweep", "filter", "saw", "resonant"}, {"duration=4", "pitch=36", "brightness", "intensity=0.7", "width=0.7"},
                  [](Rc r, Cc c) {
                      const double hi = c.lg(c.p("brightness"), 2500, 9000);
                      r.add("saws").osc(Wave::Saw, c.p("pitch")).unison(5, 18).sub(0.4).lp(Traj(150).to(0.5, hi, Interp::Exp).to(1.0, 220, Interp::Exp), 2.0 + 8.0 * c.p("intensity"), 4)
                          .shape(Traj(0.9).to(0.85, 1.0).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(2, 10);
                  });

                d("Resonance Sweep", Evolving(), "Noise through a very narrow band-pass that sweeps upward; a whistling, vocal resonance.",
                  {"sweep", "resonance", "noise", "whistle"}, {"duration=3", "intensity=0.8", "brightness=0.7", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("whistle").noise(NoiseColor::White).bp(Traj::exp(220, c.lg(c.p("brightness"), 3000, 9000)), 8.0 + 20.0 * c.p("intensity"), 4)
                          .shape(Traj(0.3).to(0.6, 1.0, Interp::Smooth).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(3, 8);
                  });

                d("Pitch Dive", OneShot(), "A sine and saw voice that falls exponentially from a high note to a low one with a little drive.",
                  {"dive", "pitch", "fall", "sweep"}, {"duration=1.6", "pitch=36", "range=60", "intensity", "brightness"},
                  [](Rc r, Cc c) {
                      r.add("voice").osc(Wave::Saw, Traj::linear(c.p("pitch") + c.p("range"), c.p("pitch"))).sub(0.5).drive(1.0 + 5.0 * c.p("intensity"), DriveP::Tanh)
                          .lp(c.lg(c.p("brightness"), 1200, 9000)).shape(Traj(1.0).to(0.6, 0.7).to(1.0, 0.002, Interp::Exp)).fade(1, 10);
                  });

                d("Downlifter", Evolving(), "Noise and a saw voice fall together: both the cutoff and the pitch sink while the level fades.",
                  {"downlifter", "fall", "noise", "transition"}, {"duration=3", "pitch=60", "range=36", "brightness=0.7", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).lp(Traj::exp(c.lg(c.p("brightness"), 4000, 16000), 140), 1.5, 4).shape(Traj::decay(0.002)).place(0, c.p("width")).gain(0.9).fade(1, 12);
                      r.add("saw").osc(Wave::Saw, Traj::linear(c.p("pitch"), c.p("pitch") - c.p("range"))).unison(3, 15).lp(Traj::exp(5000, 200), 1.5, 4).shape(Traj::decay(0.002)).gain(0.7).fade(1, 12);
                  });

                d("Noise Fall", OneShot(), "A short fall of pink noise: the cutoff drops quickly through the spectrum.",
                  {"fall", "noise", "short", "whoosh"}, {"duration=1.4", "brightness=0.7", "intensity=0.5", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::Pink).lp(Traj::exp(c.lg(c.p("brightness"), 5000, 18000), 90), 0.7 + 4.0 * c.p("intensity"), 4)
                          .shape(Traj(0.8).to(0.08, 1.0).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(1, 10);
                  });

                d("Transition Sweep", Evolving(), "A band-passed noise sweep and a rising sine glide meet in a reverberant tail: a whoosh for scene changes.",
                  {"transition", "sweep", "whoosh", "tail"}, {"duration=2.5", "tail=1.2", "brightness=0.7", "intensity", "width=0.9"},
                  [](Rc r, Cc c) {
                      const double len = c.duration;
                      r.add("whoosh").noise(NoiseColor::White).bp(Traj::exp(250, c.lg(c.p("brightness"), 2500, 9000)), 1.2 + 4.0 * c.p("intensity"), 4).shape(env_swell(0.8, 0.03)).len(len).ring(c.p("tail"))
                          .reverb(1.2, 0.5, 0.6).place(0, c.p("width")).fade(2, 20);
                      r.add("glide").osc(Wave::Sine, Traj::linear(48, 84)).shape(env_swell(0.8, 0.05)).len(len).ring(c.p("tail") * 0.5).reverb(1.0, 0.5, 0.5).gain(0.35).fade(2, 20);
                  });

                d("Harmonic Riser", Evolving(), "A fixed-pitch harmonic series whose upper partials fade in over time: the spectrum brightens without the pitch moving.",
                  {"riser", "harmonic", "additive", "brightening"}, {"duration=4", "pitch=45", "brightness", "intensity", "width=0.5"},
                  [](Rc r, Cc c) {
                      r.add("partials").additive(c.p("pitch"), { 0.0, 7.0 }, 28, Traj::linear(3.0, 0.2 + 0.8 * (1.0 - c.p("brightness")))).spread(Traj::linear(0, 12))
                          .shape(Traj::exp(0.1, 1.0)).place(0, c.p("width")).fade(2, 10);
                  });

                d("Detuned Riser", Evolving(), "Seven saws that start in tune and spread apart by tens of cents as the pitch climbs: a thickening, unstable rise.",
                  {"riser", "detuned", "supersaw", "unstable"}, {"duration=4", "pitch=48", "range=19", "intensity=0.7", "brightness", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("saws").osc(Wave::Saw, Traj::linear(c.p("pitch") - c.p("range"), c.p("pitch"))).unison(7, Traj::linear(3, 20 + 70 * c.p("intensity")))
                          .lp(Traj::exp(500, c.lg(c.p("brightness"), 3000, 14000)), 1.0, 4).shape(Traj::exp(0.12, 1.0)).place(0, c.p("width")).fade(1, 10);
                  });

                d("Formant Sweep", Evolving(), "A voiced formant source rises in pitch while its vowel travels from 'a' towards 'u' and the formants lift.",
                  {"riser", "formant", "vowel", "vocal-like"}, {"duration=3", "pitch=45", "range=19", "brightness=0.6", "intensity=0.5"},
                  [](Rc r, Cc c) {
                      r.add("formant").formant(Traj::linear(0, 4), Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range")), 0.05).shift(Traj::linear(0, 4.0 + 8.0 * c.p("brightness")))
                          .drive(1.0 + 3.0 * c.p("intensity")).shape(Traj::exp(0.2, 1.0)).place(0, 0.4).fade(2, 10);
                  });

                d("Bandpass Sweep", Evolving(), "A low saw drone through a medium-Q band-pass sweeping up: the harmonics pass through the band one by one.",
                  {"sweep", "bandpass", "saw", "tonal"}, {"duration=3.5", "pitch=33", "brightness=0.7", "intensity=0.6", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("saw").osc(Wave::Saw, c.p("pitch")).unison(2, 6).bp(Traj::exp(120, c.lg(c.p("brightness"), 2500, 8000)), 3.0 + 9.0 * c.p("intensity"), 4)
                          .shape(Traj(0.4).to(0.7, 1.0, Interp::Smooth).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(2, 10);
                  });

                d("Staircase Riser", Evolving(), "A saw that climbs in equal pitch steps under an opening filter, each step re-gated for a stepped feel.",
                  {"riser", "stairs", "stepped", "saw"}, {"duration=3", "pitch=40", "range=24", "steps=8", "brightness"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps"));
                      r.add("saw").osc(Wave::Saw, Traj::stairs(c.p("pitch"), c.p("pitch") + c.p("range"), n)).unison(2, 8).lp(Traj::exp(500, c.lg(c.p("brightness"), 3000, 10000)), 2.0, 4)
                          .gate_sec({ 1.0, 0.45 }, c.duration / n / 2.0, 0.6, 4.0).shape(Traj::exp(0.15, 1.0)).fade(1, 10);
                  });

                d("Tension Swell", Evolving(), "A dissonant cluster of additive partials with slow beating swells to a peak and drops away.",
                  {"tension", "swell", "dissonant", "cluster"}, {"duration=5", "pitch=50", "spread=0.5", "intensity=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      const double s = c.p("spread");
                      r.add("cluster").additive(Traj::linear(c.p("pitch") - 2, c.p("pitch")), { 0.0, 1.0 + s, 6.0, 6.0 + 1.0 + s, 11.0 }, 5, 1.1).spread(Traj::linear(5, 30))
                          .trem(Traj::linear(1.5, 5.0 + 5.0 * c.p("intensity")), Traj::linear(0.1, 0.45)).lp(Traj::exp(600, 9000), 1.0, 4).shape(env_swell(0.88, 0.04)).place(0, c.p("width")).fade(2, 15);
                  });
            }
        }
    }
}
