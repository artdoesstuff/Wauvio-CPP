#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline Traj bed(double fade = 0.12) { return Traj(0.001).to(fade, 1.0, Interp::Smooth).to(1.0 - fade, 1.0).to(1.0, 0.001, Interp::Smooth); }

            inline void register_textures(Registry& R) {
                Def d{R, Category::Textures};

                d("Noise Bed", Sustained(), "Pink noise through a low-pass that drifts slowly; a plain, steady bed of sound.",
                  {"texture", "noise", "bed", "steady"}, {"duration=8", "brightness=0.5", "intensity=0.4", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::Pink).lp(c.lg(c.p("brightness"), 800, 6000), 0.7, 4).filter_lfo(lfo(Mod::Shape::Smooth, 0.15, 0.3 + 0.6 * c.p("intensity"))).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });

                d("Air Texture", Sustained(), "High-passed white noise with a faint breathing movement: just the top of the spectrum.",
                  {"texture", "air", "hiss", "high"}, {"duration=8", "brightness=0.7", "depth=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("air").noise(NoiseColor::White).hp(c.lg(c.p("brightness"), 2500, 7000), 0.707, 4).lp(16000).trem(0.18, c.p("depth") * 0.6).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });

                d("Metallic Drone", Sustained(), "A stack of band-limited squares at inharmonic ratios, slowly sweeping filter: a bell-metal hum.",
                  {"texture", "drone", "metallic", "inharmonic"}, {"duration=8", "pitch=36", "intensity=0.5", "brightness=0.5", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("stack").osc(Wave::Square, c.p("pitch")).stack({ 1.0, 1.47, 1.98, 2.42, 2.89, 3.51, 4.12 }).lp(c.lg(c.p("brightness"), 900, 6000), 1.0 + 3.0 * c.p("intensity"), 4)
                          .filter_lfo(lfo(Mod::Shape::Sine, 0.08, 0.7)).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });

                d("Dissonant Drone", Sustained(), "Semitone and tritone partials with a slow tremolo: a sustained unease.",
                  {"texture", "drone", "dissonant", "tense"}, {"duration=8", "pitch=40", "spread=0.5", "intensity=0.5", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("cluster").additive(c.p("pitch"), { 0.0, 1.0 + c.p("spread"), 6.0, 13.0 }, 6, 1.2).spread(Traj(8)).trem(0.27, 0.25 + 0.4 * c.p("intensity")).lp(2500, 0.8, 4).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });

                d("Harmonic Shimmer", Sustained(), "High partials of a fifth-and-octave chord with a gentle tremolo and echo: glassy and bright.",
                  {"texture", "shimmer", "harmonic", "bright"}, {"duration=8", "pitch=72", "brightness=0.7", "depth=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("shimmer").additive(c.p("pitch"), { 0.0, 7.0, 12.0 }, 8, 1.1).spread(Traj(10)).hp(500).trem(5.5, 0.15 + 0.4 * c.p("depth")).lp(c.lg(c.p("brightness"), 4000, 14000)).echo(0.33, 0.45, 0.6, 0.4).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });

                d("Swirling Texture", Sustained(), "Noise through a moving band-pass and a drifting comb filter, rendered with decorrelated channels: a slow phasing swirl.",
                  {"texture", "swirl", "comb", "movement"}, {"duration=8", "pitch=48", "intensity=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("swirl").noise(NoiseColor::White).bp(Traj::exp(300, 4000), 2.0 + 4.0 * c.p("intensity"), 4).filter_lfo(lfo(Mod::Shape::Sine, 0.21, 1.2))
                          .comb(Traj::linear(midi_to_hz(c.p("pitch")), midi_to_hz(c.p("pitch") + 7)), 0.8, 0.6).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });

                d("Granular Cloud", Sustained(), "A dense cloud of 80-200 ms bell grains scattered in pitch and position.",
                  {"texture", "granular", "cloud", "bell"}, {"duration=8", "pitch=72", "density=0.5", "spread=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("cloud").grain(GrainSource::Bell, 12 + 70 * c.p("density"), Traj::linear(90, 200), c.p("pitch")).scatter(1.0 + 10.0 * c.p("spread"), 0.9).grain_pos(Traj::linear(0.1, 0.5)).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });

                d("Frozen Texture", Sustained(), "A chord that is spectrally frozen early on: the same spectrum is held with randomised phase for the whole effect.",
                  {"texture", "frozen", "spectral", "pad"}, {"duration=8", "pitch=55", "chord=4", "brightness=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("pad").additive(c.p("pitch"), c.chord_notes(), 10, 1.1).spectral(SpectralP::Freeze, 1.0, 0.1).lp(c.lg(c.p("brightness"), 1500, 8000)).shape(bed(0.08)).place(0, c.p("width")).fade(5, 5);
                  });

                d("Spectral Wash", Sustained(), "A rich chord whose phases are re-randomised in every analysis frame: the pitches survive but turn into a smeared, ghostly wash.",
                  {"texture", "wash", "spectral", "smeared", "chord"}, {"duration=8", "pitch=55", "chord=4", "brightness=0.5", "intensity=0.9", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("wash").additive(c.p("pitch"), c.chord_notes(), 14, 1.0).spread(Traj(10)).spectral(SpectralP::Scramble, c.p("intensity")).lp(c.lg(c.p("brightness"), 1200, 9000)).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });

                d("Inharmonic Texture", Sustained(), "Struck inharmonic modes retriggered every second or so with random velocities: ringing metal, constantly renewed.",
                  {"texture", "inharmonic", "resonant", "ringing"}, {"duration=8", "pitch=60", "density=0.5", "decay=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(3 + 12 * c.p("density"));
                      r.add("modes").modal(modes_inharmonic(c.seed, 9, 4.0), c.p("pitch"), c.scattered(n, 7, 0.0, 0.95)).hit_jitter(0.0, 0.6).decay_scale(0.4 + 1.6 * c.p("decay")).strike(1.0).lp(7000).shape(bed(0.04)).place(0, c.p("width")).fade(5, 5);
                  });

                d("Evolving Noise", Sustained(), "Noise whose band-pass centre wanders on a smooth random modulator while a slow tremolo moves its level: it never settles.",
                  {"texture", "evolving", "noise", "random-walk"}, {"duration=10", "brightness=0.5", "intensity=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).bp(c.lg(c.p("brightness"), 500, 3000), 1.5 + 3.0 * c.p("intensity"), 4).filter_lfo(lfo(Mod::Shape::Smooth, 0.35, 1.6))
                          .trem(Traj(0.4), 0.5).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });

                d("Detuned Atmosphere", Sustained(), "Two groups of detuned saws a fifth apart through a dark low-pass: a wide, warm pad.",
                  {"texture", "pad", "detuned", "warm"}, {"duration=8", "pitch=45", "brightness=0.4", "spread=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("root").osc(Wave::Saw, c.p("pitch")).unison(7, 10 + 40 * c.p("spread")).lp(c.lg(c.p("brightness"), 600, 3500), 0.9, 4).filter_lfo(lfo(Mod::Shape::Sine, 0.1, 0.3)).shape(bed()).place(-0.3, c.p("width")).fade(5, 5);
                      r.add("fifth").osc(Wave::Saw, c.p("pitch") + 7).unison(5, 12 + 30 * c.p("spread")).lp(c.lg(c.p("brightness"), 500, 3000), 0.9, 4).shape(bed()).gain(0.6).place(0.3, c.p("width")).fade(5, 5);
                  });

                d("Resonant Ambience", Sustained(), "Noise rings through parallel comb resonators tuned to a chord, then a long reverb: the hum of a resonant room.",
                  {"texture", "resonant", "room", "comb"}, {"duration=8", "pitch=48", "chord=5", "brightness=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      const auto ch = c.chord_notes();
                      for (size_t i = 0; i < ch.size(); ++i)
                          r.add("room" + std::to_string(i)).noise(NoiseColor::Pink).comb(midi_to_hz(c.p("pitch") + ch[i]), 0.93, 0.95).lp(c.lg(c.p("brightness"), 1500, 8000)).reverb(2.5, 0.5, 0.5).shape(bed()).gain(0.8).place(-0.6 + 1.2 * static_cast<double>(i) / ch.size(), c.p("width")).fade(5, 5).variation(i);
                  });

                d("Pulsating Drone", Sustained(), "A low saw-and-sine drone amplitude-pulsed in time with the tempo (quarter notes by default).",
                  {"texture", "drone", "pulse", "tempo-sync"}, {"duration=8", "pitch=36", "subdivision=1", "depth=0.7", "intensity=0.4"},
                  [](Rc r, Cc c) {
                      r.add("drone").osc(Wave::Saw, c.p("pitch")).sub(0.8).unison(2, 8).drive(1.0 + 2.0 * c.p("intensity")).lp(900, 1.0, 4).trem(c.beat_hz(c.p("subdivision")), c.p("depth")).shape(bed()).place(0, 0.4).fade(5, 5);
                  });

                d("Static Wash", Sustained(), "Dense crackle (dust) layered over blue-noise hiss: a radio-static carpet.",
                  {"texture", "static", "crackle", "hiss"}, {"duration=8", "density=0.6", "brightness=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("crackle").noise(NoiseColor::Dust, 400 + 5000 * c.p("density")).hp(1200).lp(c.lg(c.p("brightness"), 4000, 16000)).shape(bed()).place(0, c.p("width")).fade(5, 5);
                      r.add("hiss").noise(NoiseColor::Blue).lp(9000).shape(bed()).gain(0.35).place(0, c.p("width")).fade(5, 5);
                  });

                d("Harmonic Haze", Sustained(), "Low, many-partialled additive tones softened by a dark filter and echo: warm haze with no sharp edges.",
                  {"texture", "haze", "warm", "harmonic"}, {"duration=8", "pitch=38", "brightness=0.4", "depth=0.4", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("haze").additive(Traj(c.p("pitch")).to(0.5, c.p("pitch") + 0.2).to(1.0, c.p("pitch")), { 0.0, 7.0, 12.0, 19.0 }, 20, 1.6).spread(Traj(14)).lp(c.lg(c.p("brightness"), 500, 3500), 0.7, 4)
                          .trem(0.12, c.p("depth") * 0.4).echo(0.47, 0.4, 0.5, 0.6).shape(bed()).place(0, c.p("width")).fade(5, 5);
                  });
            }
        }
    }
}
