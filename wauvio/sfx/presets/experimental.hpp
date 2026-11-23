#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_experimental(Registry& R) {
                Def d{R, Category::Experimental};

                d("Chaotic FM Burst", OneShot(), "A carrier whose pitch is driven by a Lorenz attractor running at audio-ish speed: a burst of unpredictable, bounded FM.",
                  {"chaos", "fm", "burst", "lorenz"}, {"duration=1.2", "pitch=60", "intensity=0.7", "rate=60"},
                  [](Rc r, Cc c) {
                      r.add("lorenz").chaos(ChaosKind::Lorenz, Traj::exp(c.p("rate"), c.p("rate") * 5.0), c.p("pitch")).chaos_depth(Traj::linear(8.0 + 30.0 * c.p("intensity"), 4.0), 0.3).lp(9000).shape(env_hit(0.004, 0.003)).place(0, 0.5).fade(0.2, 12);
                  });

                d("Unstable Oscillator Texture", Sustained(), "A saw whose pitch is steered by a logistic map in its chaotic regime and smoothed: a wandering, never-repeating drone.",
                  {"chaos", "oscillator", "logistic", "drone"}, {"duration=6", "pitch=48", "intensity=0.6", "rate=12"},
                  [](Rc r, Cc c) {
                      r.add("logistic").chaos(ChaosKind::Logistic, c.p("rate"), c.p("pitch")).chaos_depth(5.0 + 12.0 * c.p("intensity")).lp(3500, 1.0, 4).shape(Traj(0.001).to(0.15, 1.0, Interp::Smooth).to(0.85, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0, 0.7).fade(5, 12);
                  });

                d("Inharmonic Cluster", OneShot(), "A dense cluster of strongly stretched partials (inharmonicity 0.05) on closely spaced notes: bell-like but unpitched.",
                  {"inharmonic", "cluster", "additive", "dissonant"}, {"duration=3", "pitch=55", "spread=0.6", "decay=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("cluster").additive(c.p("pitch"), { 0.0, 1.0 + c.p("spread"), 3.0, 8.0 + c.p("spread") }, 10, 0.8).inharmonic(0.02 + 0.06 * c.p("spread")).spread(Traj(25)).hp(150).lp(9000)
                          .shape(Traj(0.001).to(0.01, 1.0).to(1.0, 0.002 + 0.01 * (1.0 - c.p("decay")), Interp::Exp)).place(0, c.p("width")).fade(0.2, 25);
                  });

                d("Nonlinear Pitch Sweep", Evolving(), "A sine-FM voice whose pitch follows an irregular multi-segment curve: slow, a lurch upward, then a fast final climb.",
                  {"sweep", "nonlinear", "pitch", "fm"}, {"duration=3", "pitch=36", "range=60", "intensity=0.5"},
                  [](Rc r, Cc c) {
                      const double p = c.p("pitch"), rg = c.p("range");
                      r.add("voice").osc(Wave::Sine, Traj(p).to(0.3, p + 0.1 * rg, Interp::Exp).to(0.55, p + 0.35 * rg, Interp::Smooth).to(0.9, p + 0.55 * rg, Interp::Linear).to(1.0, p + rg, Interp::Exp)).fm(Traj::linear(0.5, 1.0 + 6.0 * c.p("intensity")), 1.5)
                          .lp(12000).shape(Traj(0.3).to(0.8, 1.0, Interp::Smooth).to(1.0, 0.003, Interp::Exp)).place(0, 0.4).fade(1, 10);
                  });

                d("Fractal-Like Noise Texture", Sustained(), "Six octave-spaced noise bands. Each band's centre wanders on smooth random movement and its level swells sinusoidally, both at rates that double per octave: self-similar, 1/f-ish motion.",
                  {"fractal", "noise", "texture", "self-similar"}, {"duration=8", "brightness=0.6", "intensity=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      for (int k = 0; k < 6; ++k) {
                          const double f = 150.0 * std::pow(2.0, k);
                          r.add("band" + std::to_string(k)).noise(NoiseColor::White).bp(f, 2.5 + 2.0 * c.p("intensity"), 4).filter_lfo(lfo(Mod::Shape::Smooth, 0.1 * std::pow(2.0, k), 0.5)).trem(Traj(0.15 * std::pow(2.0, k)), 0.9).gain(std::pow(0.75, k) * std::pow(2.0, k * (c.p("brightness") - 0.5) * 0.9))
                              .shape(Traj(0.001).to(0.12, 1.0, Interp::Smooth).to(0.88, 1.0).to(1.0, 0.001, Interp::Smooth)).place(-0.8 + 1.6 * k / 5.0, c.p("width")).fade(5, 12).variation(static_cast<uint64_t>(k));
                      }
                  });

                d("Chaotic Modulation Swell", Evolving(), "A saw swells while a Lorenz attractor bends its pitch and level more and more strongly.",
                  {"chaos", "swell", "modulation", "lorenz"}, {"duration=4", "pitch=45", "intensity=0.7", "rate=25"},
                  [](Rc r, Cc c) {
                      r.add("lorenz").chaos(ChaosKind::Lorenz, c.p("rate"), c.p("pitch")).chaos_depth(Traj::linear(0.5, 4.0 + 18.0 * c.p("intensity")), 0.6).lp(Traj::exp(600, 8000), 1.5, 4).shape(env_swell(0.85, 0.03)).place(0, 0.6).fade(1, 12);
                  });

                d("Frequency-Folded Burst", OneShot(), "A sine swept upward and wavefolded with an increasing fold amount: the harmonics fold back on themselves into a harsh, bright burst.",
                  {"fold", "burst", "harsh", "sine"}, {"duration=1", "pitch=48", "range=36", "intensity=0.8"},
                  [](Rc r, Cc c) {
                      r.add("sine").osc(Wave::Sine, Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range"))).drive(Traj::linear(1.0, 4.0 + 20.0 * c.p("intensity")), DriveP::Fold).shape(Traj(1.0).to(0.2, 1.0).to(1.0, 0.003, Interp::Exp)).place(0, 0.4).fade(0.1, 10);
                  });

                d("Spectral Disintegration", Evolving(), "A rich saw chord whose spectrum is erased bin by bin while high frequencies fade first: the sound crumbles to nothing.",
                  {"spectral", "disintegrate", "crumble", "chord"}, {"duration=4", "pitch=48", "chord=1", "intensity=0.8", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 20, 0.8).spectral(SpectralP::TiltFade, Traj::linear(0.0, 0.7)).spectral(SpectralP::Disintegrate, Traj::linear(0.0, 0.6 + 0.39 * c.p("intensity")))
                          .shape(Traj(1.0).to(0.7, 0.9).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(1, 20);
                  });

                d("Unstable Resonator", Sustained(), "Noise driving a feedback comb (gain 0.80 to 0.94, set by intensity) whose tuning wanders on random steps: a ringing resonance that keeps re-tuning itself.",
                  {"resonator", "comb", "unstable", "ringing"}, {"duration=5", "pitch=48", "range=19", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(10); Traj f(midi_to_hz(c.p("pitch"))); for (int i = 1; i <= 10; ++i) f.to(i / 10.0, midi_to_hz(c.p("pitch") + g.range(0, c.p("range"))), Interp::Smooth);
                      r.add("res").noise(NoiseColor::White).comb(f, 0.80 + 0.14 * c.p("intensity"), 0.9).drive(1.5).hp(80).shape(Traj(0.001).to(0.1, 1.0, Interp::Smooth).to(0.85, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0, 0.6).fade(5, 15);
                  });

                d("Attractor-Like Modulation Texture", Sustained(), "The output of a Henon map, smoothed, is the sound itself: a rough, quasi-periodic texture that sits between noise and tone.",
                  {"chaos", "henon", "texture", "attractor"}, {"duration=6", "rate=300", "brightness=0.5", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("henon").chaos(ChaosKind::Henon, Traj::linear(c.p("rate"), c.p("rate") * 2.5), 48).chaos_out(true).lp(c.lg(c.p("brightness"), 1500, 9000), 1.0, 4).shape(Traj(0.001).to(0.12, 1.0, Interp::Smooth).to(0.88, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0, c.p("width")).fade(5, 12);
                  });

                d("Glissando Cluster", Evolving(), "Eight sine voices glide in different directions from a tight cluster to a wide spread: a sheet of crossing glissandi.",
                  {"glissando", "cluster", "sine", "spread"}, {"duration=4", "pitch=60", "range=24", "spread=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(11);
                      for (int i = 0; i < 8; ++i) {
                          const double a = c.p("pitch") + g.range(-1.5, 1.5), b = c.p("pitch") + g.range(-1.0, 1.0) * c.p("range") * (0.4 + 0.6 * c.p("spread"));
                          r.add("voice" + std::to_string(i)).osc(Wave::Sine, Traj::linear(a, b)).shape(Traj(0.001).to(0.1, 1.0, Interp::Smooth).to(0.8, 0.8).to(1.0, 0.001, Interp::Smooth)).gain(0.5).place(-0.9 + 1.8 * i / 7.0).fade(5, 12);
                      }
                  });

                d("Deterministic Noise Sculpture", Sustained(), "Noise cut by a 64-step hashed gate and rectified, then filtered through stepped band-pass frequencies: a hard-edged, repeatable noise object.",
                  {"noise", "sculpture", "deterministic", "harsh"}, {"duration=3", "density=0.5", "intensity=0.7", "width=0.8"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(12); Traj centre(300.0);
                      for (int i = 1; i <= 12; ++i) centre.to(i / 12.0, 150.0 * std::pow(2.0, g.range(0.0, 5.0)), Interp::Step);       // stepped band-pass frequencies
                      r.add("sculpt").noise(NoiseColor::White).bp(centre, 3.0).drive(2.0 + 6.0 * c.p("intensity"), DriveP::Rectify)
                          .gate(hashed_pattern(c.seed, 64, 0.3 + 0.5 * c.p("density")), 0.0625, 1.0, 0.8).shape(Traj(1.0).to(0.9, 1.0).to(1.0, 0.003, Interp::Exp)).place(0, c.p("width")).fade(0.2, 12);
                  });

                d("Extreme Formant Distortion", OneShot(), "A voiced formant source with its formants shifted by up to an octave, jumping between vowels and wavefolded: a mangled, speech-like scream.",
                  {"formant", "extreme", "distortion", "vocal-like"}, {"duration=1.5", "pitch=48", "intensity=0.9", "range=12"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(13);
                      r.add("voice").formant(random_stairs(g, 8, 0, 4, false), Traj::linear(c.p("pitch") + c.p("range"), c.p("pitch"))).shift(Traj::linear(-3.0, 4.0 + 8.0 * c.p("intensity"))).drive(4.0 + 12.0 * c.p("intensity"), DriveP::Fold)
                          .shape(Traj(1.0).to(0.8, 0.9).to(1.0, 0.003, Interp::Exp)).place(0, 0.5).fade(0.2, 12);
                  });

                d("Tonal Fragmentation", OneShot(), "A chord broken into pieces by random dropouts, short repeated slices and light bit reduction: a tonal sound in shards.",
                  {"tonal", "fragmentation", "shards", "glitch"}, {"duration=2", "pitch=55", "chord=1", "bits=9", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 10, 1.0).dropout(10.0 + 20.0 * c.p("intensity"), 8, 90, 1.0).stutter(0.035, 3, 0.4).crush(c.p("bits"), 1.0 + 3.0 * c.p("intensity"))
                          .shape(Traj(1.0).to(0.75, 0.9).to(1.0, 0.004, Interp::Exp)).place(0, 0.7).fade(0.3, 15);
                  });

                d("Microtonal Cluster", Sustained(), "Five sines a quarter-tone apart beat against each other slowly; the cluster shimmers without resolving to a pitch.",
                  {"microtonal", "cluster", "beating", "sine"}, {"duration=6", "pitch=60", "spread=0.5", "intensity=0.5", "width=0.8"},
                  [](Rc r, Cc c) {
                      const double step = 0.25 + 0.5 * c.p("spread");
                      r.add("cluster").additive(c.p("pitch"), { 0.0, step, 2 * step, 3 * step, 4 * step }, 3, 1.5).trem(Traj(0.3), 0.15 + 0.3 * c.p("intensity")).lp(5000).shape(Traj(0.001).to(0.15, 1.0, Interp::Smooth).to(0.85, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0, c.p("width")).fade(5, 12);
                  });

                d("Feedback-Like Synthetic Swell", Evolving(), "Noise through a high-feedback comb whose tuning rises while a drive stage keeps the level bounded: a howl that swells without ever running away.",
                  {"feedback", "swell", "howl", "bounded"}, {"duration=4", "pitch=48", "range=24", "intensity=0.8"},
                  [](Rc r, Cc c) {
                      r.add("howl").noise(NoiseColor::Pink).comb(Traj::exp(midi_to_hz(c.p("pitch")), midi_to_hz(c.p("pitch") + c.p("range"))), 0.88 + 0.07 * c.p("intensity"), 1.0).drive(2.0 + 4.0 * c.p("intensity"), DriveP::Tanh).hp(100)
                          .shape(env_swell(0.9, 0.02)).place(0, 0.6).fade(1, 12);
                  });
            }
        }
    }
}