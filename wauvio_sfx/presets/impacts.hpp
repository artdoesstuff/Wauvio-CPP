#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            // Shared structure of a layered hit: sub + body + noise transient (+ tail). Used by several impacts with different settings.
            inline void impact_core(Recipe& r, const BuildCtx& c, double sub_hz0, double sub_hz1, double body_midi, double noise_lp_hz) {
                r.add("sub").osc(Wave::Sine, Traj::pitch_hz(sub_hz0, sub_hz1, Interp::Linear)).shape(Traj(1.0).to(0.15, 0.7).to(1.0, 0.002, Interp::Exp)).gain(0.9 * c.p("sub")).fade(0.5, 20);
                r.add("body").osc(Wave::Saw, Traj::linear(body_midi + 7, body_midi)).unison(3, 14).drive(1.0 + 4.0 * c.p("transient")).lp(Traj::exp(3500, 180), 1.0, 4).shape(env_hit(0.002, 0.002)).len(c.duration * 0.6).gain(0.8 * c.p("body")).fade(0.5, 20);
                r.add("transient").noise(NoiseColor::White).lp(noise_lp_hz).shape(env_hit(0.001, 0.0008)).len(0.09).gain(0.8 * c.p("transient")).fade(0.1, 5);
            }

            inline void register_impacts(Registry& R) {
                Def d{R, Category::Impacts};

                d("Cinematic Impact", OneShot(), "A deep sine drop, a distorted saw body, a noise transient and a long synthetic tail: the standard trailer hit.",
                  {"impact", "cinematic", "boom", "trailer"}, {"duration=2.5", "tail=2.5", "body=0.8", "sub=0.8", "transient=0.7", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      impact_core(r, c, 90, 32, 36, c.lg(c.p("brightness"), 3000, 14000));
                      r.add("wash").noise(NoiseColor::Pink).lp(Traj::exp(c.lg(c.p("brightness"), 2500, 9000), 250), 0.8, 4).shape(env_hit(0.002, 0.002)).len(c.duration).ring(c.p("tail")).reverb(2.5 + c.p("tail"), 0.5, 0.9).gain(0.45).place(0, 1).fade(0.5, 30);
                  });

                d("Sub Drop", OneShot(), "A pure sine falling from a low note to sub-bass with a gentle saturation; weight without any noise.",
                  {"sub", "drop", "bass", "weight"}, {"duration=2.2", "pitch=40", "range=24", "intensity=0.4"},
                  [](Rc r, Cc c) {
                      r.add("sine").osc(Wave::Sine, Traj(c.p("pitch") + c.p("range")).to(0.3, c.p("pitch"), Interp::Exp).to(1.0, c.p("pitch") - 5)).drive(1.0 + 3.0 * c.p("intensity"), DriveP::Tanh)
                          .lp(400).shape(Traj(1.0).to(0.2, 0.8).to(1.0, 0.002, Interp::Exp)).fade(0.5, 25);
                  });

                d("Reverse Impact", OneShot(), "A full cinematic hit rendered forwards and then reversed: the tail swells and the hit lands at the very end.",
                  {"impact", "reverse", "swell", "transition"}, {"duration=2.5", "tail=2", "body=0.8", "sub=0.7", "transient=0.7"},
                  [](Rc r, Cc c) {
                      impact_core(r, c, 80, 34, 38, 9000);
                      r.add("wash").noise(NoiseColor::Pink).lp(4000).shape(env_hit(0.002, 0.002)).len(c.duration).ring(c.p("tail")).reverb(2.0 + c.p("tail"), 0.45, 0.9).gain(0.5).place(0, 1);
                      r.reverse_all = true; r.fade_in_ms = 4.0; r.fade_out_ms = 2.0;
                  }, true);

                d("Reverse Swell", Evolving(), "A chord pad and bright noise built from a decaying reverb tail reversed; slow and tonal rather than a hit.",
                  {"reverse", "swell", "pad", "tonal"}, {"duration=3.5", "pitch=52", "chord=1", "brightness=0.6", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("pad").additive(c.p("pitch"), c.chord_notes(), 8, 1.3).shape(env_hit(0.02, 0.01)).len(0.6).ring(std::max(0.0, c.duration - 0.6)).reverb(3.5, 0.4, 1.5).reverse().place(0, c.p("width")).gain(0.9).fade(2, 3);
                      r.add("air").noise(NoiseColor::White).hp(2500).lp(c.lg(c.p("brightness"), 5000, 16000)).shape(env_hit(0.01, 0.01)).len(0.4).ring(std::max(0.0, c.duration - 0.4)).reverb(3.0, 0.3, 1.2).reverse().gain(0.4).place(0, 1).fade(2, 3);
                  }, true);

                d("Transition Hit", OneShot(), "A short mid-focused whoosh-and-thump: band-passed noise and a falling saw, tuned for scene cuts.",
                  {"transition", "hit", "whoosh", "short"}, {"duration=0.9", "tail=0.5", "intensity=0.6", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).bp(Traj::exp(c.lg(c.p("brightness"), 1800, 6000), 350), 1.0 + 2.0 * c.p("intensity"), 4).shape(env_hit(0.01, 0.002)).ring(c.p("tail")).reverb(0.7, 0.4, 0.5).place(0, 0.9).fade(0.3, 8);
                      r.add("thump").osc(Wave::Saw, Traj::linear(60, 36)).drive(2.0 + 3.0 * c.p("intensity")).lp(600).shape(env_hit(0.004, 0.002)).len(0.5).gain(0.9).fade(0.3, 8);
                  });

                d("Noise Burst", OneShot(), "A very short burst of high-passed white noise with an instant attack: a hit with no body.",
                  {"burst", "noise", "short", "click"}, {"duration=0.25", "brightness=0.7", "decay=0.4", "width=0.7"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).hp(c.lg(c.p("brightness"), 300, 3000)).shape(Traj(1.0).to(1.0, 0.0008, Interp::Exp)).place(0, c.p("width")).fade(0.05, 4)
                          .len(0.04 + 0.2 * c.p("decay"));
                  });

                d("Bass Impact", OneShot(), "A punchy kick-like hit: a fast sine dive with a saturated saw layer and a tight noise click.",
                  {"impact", "bass", "punch", "kick-like"}, {"duration=1.3", "pitch=36", "intensity=0.7", "transient=0.6", "sub=0.8"},
                  [](Rc r, Cc c) {
                      r.add("dive").osc(Wave::Sine, Traj::pitch_hz(170, midi_to_hz(c.p("pitch")), Interp::Exp)).drive(1.5 + 4.0 * c.p("intensity")).shape(Traj(1.0).to(0.1, 0.9).to(1.0, 0.002, Interp::Exp)).gain(0.5 + 0.5 * c.p("sub")).fade(0.2, 15);
                      r.add("saw").osc(Wave::Saw, Traj::linear(c.p("pitch") + 12, c.p("pitch"))).drive(3.0).lp(Traj::exp(1800, 150)).shape(env_hit(0.002, 0.002)).len(0.4).gain(0.6).fade(0.2, 15);
                      r.add("click").noise(NoiseColor::White).bp(2500, 1.0).shape(env_hit(0.001, 0.001)).len(0.03).gain(0.6 * c.p("transient")).fade(0.05, 3);
                  });

                d("Tail-Out", OneShot(), "A tiny click that excites a long, darkening reverb tail: mostly tail, for ending a section.",
                  {"tail", "reverb", "decay", "end"}, {"duration=0.4", "tail=4", "brightness=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("click").noise(NoiseColor::White).lp(c.lg(c.p("brightness"), 2500, 12000)).shape(env_hit(0.002, 0.001)).len(0.12).ring(c.p("tail")).reverb(1.0 + c.p("tail") * 0.9, 0.7, 1.4)
                          .lp(Traj::exp(9000, 400), 0.7).place(0, c.p("width")).fade(0.1, 40);
                  });

                d("Transition Decay", OneShot(), "A filtered saw-chord stab whose cutoff closes while echoes repeat and fade: a decaying hand-off between sections.",
                  {"transition", "stab", "echo", "decay"}, {"duration=1.8", "tail=1.2", "pitch=48", "chord=1", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("chord").additive(c.p("pitch"), c.chord_notes(), 10, 1.0).lp(Traj::exp(c.lg(c.p("brightness"), 2500, 9000), 250), 1.5, 4).shape(env_hit(0.004, 0.002)).len(c.duration).ring(c.p("tail"))
                          .echo(0.28, 0.55, 0.9, 0.5).place(0, 0.7).fade(0.5, 20);
                  });

                d("Layered Impact", OneShot(), "Five explicit layers (click, mid body, low body, sub and noise wash) so each part can be balanced separately.",
                  {"impact", "layered", "heavy", "cinematic"}, {"duration=2.2", "tail=1.8", "transient=0.8", "body=0.9", "sub=0.9", "brightness=0.6", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      impact_core(r, c, 100, 30, 34, 12000);
                      r.add("mid").osc(Wave::Square, Traj::linear(60, 45)).unison(2, 20).drive(2.0 + 4.0 * c.p("intensity"), DriveP::Hard).bp(Traj::exp(900, 200), 1.0, 4).shape(env_hit(0.002, 0.002)).len(c.duration * 0.5).gain(0.5 * c.p("body")).fade(0.3, 20);
                      r.add("wash").noise(NoiseColor::Brown).lp(Traj::exp(c.lg(c.p("brightness"), 1500, 6000), 150), 0.8, 4).shape(env_hit(0.004, 0.002)).len(c.duration).ring(c.p("tail")).reverb(2.0 + c.p("tail"), 0.5, 0.7).gain(0.5).place(0, 1);
                  });

                d("Metallic Impact", OneShot(), "Struck inharmonic metal modes with a noise clang and a low thud; a heavy, gong-like hit.",
                  {"impact", "metallic", "gong", "inharmonic"}, {"duration=2.5", "pitch=40", "decay=0.6", "intensity=0.6", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("metal").modal(modes_metal(), c.p("pitch") + 12, { 0.0, 0.02 }).decay_scale(0.8 + 2.5 * c.p("decay")).hit_jitter(0.01).strike(4.0).len(c.duration).hp(120).drive(1.0 + 2.0 * c.p("intensity")).place(0, 0.8).fade(0.2, 30);
                      r.add("clang").noise(NoiseColor::White).bp(Traj::exp(c.lg(c.p("brightness"), 4000, 9000), 1500), 2.0, 4).shape(env_hit(0.001, 0.001)).len(0.5).gain(0.45).fade(0.1, 10);
                      r.add("thud").osc(Wave::Sine, Traj::linear(c.p("pitch") + 12, c.p("pitch") - 5)).shape(env_hit(0.002, 0.002)).len(0.8).gain(0.6).fade(0.2, 20);
                  });

                d("Orchestral-Style Synthetic Hit", OneShot(), "A brassy additive chord stab, a timpani-like sine drop and a cymbal-ish noise swell: an orchestral hit built from scratch.",
                  {"impact", "orchestral", "stab", "brass-like"}, {"duration=1.6", "pitch=48", "chord=4", "brightness=0.7", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("brass").additive(Traj(c.p("pitch")).to(0.05, c.p("pitch") + 0.25).to(1.0, c.p("pitch")), c.chord_notes(), 16, 1.0).lp(Traj(900).to(0.08, c.lg(c.p("brightness"), 2500, 7000), Interp::Exp).to(1.0, 500, Interp::Exp), 1.2, 4)
                          .drive(1.0 + 2.0 * c.p("intensity")).shape(Traj(0.4).to(0.04, 1.0).to(1.0, 0.002, Interp::Exp)).place(0, 0.5).fade(0.5, 20);
                      r.add("timp").osc(Wave::Sine, Traj::pitch_hz(110, 62, Interp::Linear)).shape(env_hit(0.003, 0.002)).len(1.0).gain(0.7).fade(0.2, 15);
                      r.add("cymbal").noise(NoiseColor::White).hp(5000).shape(Traj(0.1).to(0.08, 0.7).to(1.0, 0.002, Interp::Exp)).gain(0.35).place(0, 1).fade(1, 20);
                  });

                d("Granular Impact", OneShot(), "A cloud of short noise grains whose density collapses from dense to sparse, over a low thump.",
                  {"impact", "granular", "cloud", "debris"}, {"duration=1.8", "density=0.7", "brightness=0.6", "intensity=0.6", "sub=0.6"},
                  [](Rc r, Cc c) {
                      r.add("grains").grain(GrainSource::Noise, Traj::exp(60 + 340 * c.p("density"), 6), Traj::linear(15, 90)).scatter(7.0 * c.p("intensity"), 0.8).lp(c.lg(c.p("brightness"), 3000, 14000))
                          .shape(Traj(1.0).to(0.05, 1.0).to(1.0, 0.002, Interp::Exp)).place(0, 1).fade(0.2, 20);
                      r.add("thump").osc(Wave::Sine, Traj::pitch_hz(110, 38)).shape(env_hit(0.003, 0.002)).len(0.8).gain(0.8 * c.p("sub") + 0.1).fade(0.2, 15);
                  });

                d("Dissonant Impact", OneShot(), "A tritone-and-semitone stack of saws with FM noise, driven hard, then a reverb decay: an unsettling hit.",
                  {"impact", "dissonant", "tritone", "horror"}, {"duration=2", "tail=1.5", "pitch=40", "intensity=0.7", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("stack").additive(c.p("pitch"), { 0.0, 1.0, 6.0, 7.0, 13.0 }, 12, 1.0).spread(Traj(30)).drive(2.0 + 4.0 * c.p("intensity")).lp(Traj::exp(c.lg(c.p("brightness"), 2500, 8000), 300), 1.0, 4)
                          .shape(env_hit(0.002, 0.002)).len(c.duration).ring(c.p("tail")).reverb(1.5 + c.p("tail"), 0.5, 0.6).place(0, 0.6).fade(0.3, 25);
                      r.add("fm").osc(Wave::Sine, c.p("pitch") + 12).fm(Traj(6.0 * c.p("intensity") + 1.0), 1.41).shape(env_hit(0.002, 0.002)).len(0.9).gain(0.5).fade(0.2, 15);
                  });

                d("Low Boom", OneShot(), "A very low, slowly decaying sine with a slight pitch drop and soft clipping, band-limited to the bottom octaves.",
                  {"boom", "low", "sub", "slow"}, {"duration=3.2", "pitch=28", "intensity=0.5", "decay=0.7"},
                  [](Rc r, Cc c) {
                      r.add("boom").osc(Wave::Sine, Traj(c.p("pitch") + 5).to(0.2, c.p("pitch"), Interp::Exp)).drive(1.5 + 3.0 * c.p("intensity")).lp(220, 0.707, 4)
                          .shape(Traj(1.0).to(0.05, 1.0).to(1.0, 0.002 + 0.02 * (1.0 - c.p("decay")), Interp::Exp)).fade(2, 40);
                  });

                d("Upsweep Impact", Evolving(), "A noise-and-tone upsweep that ends exactly where a low hit lands: the build and the impact in one effect.",
                  {"impact", "upsweep", "transition", "build"}, {"duration=3.5", "tail=1.5", "brightness=0.7", "transient=0.7", "sub=0.8", "body=0.7"},
                  [](Rc r, Cc c) {
                      const double hit_t = c.duration * 0.78;
                      r.add("sweep").noise(NoiseColor::White).bp(Traj::exp(250, c.lg(c.p("brightness"), 3500, 11000)), 1.5, 4).shape(Traj::exp(0.04, 1.0)).len(hit_t).place(0, 0.9).fade(1, 3);
                      r.add("tone").osc(Wave::Saw, Traj::linear(40, 76)).unison(3, 15).lp(Traj::exp(300, 7000), 1.5, 4).shape(Traj::exp(0.05, 0.8)).len(hit_t).gain(0.55).fade(1, 3);
                      r.add("boom").osc(Wave::Sine, Traj::pitch_hz(90, 32)).shape(env_hit(0.002, 0.002)).at(hit_t).len(c.duration - hit_t + 0.4).gain(0.9 * c.p("sub")).fade(0.2, 25);
                      r.add("hit").osc(Wave::Saw, Traj::linear(48, 36)).unison(3, 14).drive(3.0).lp(Traj::exp(3000, 160), 1.0, 4).shape(env_hit(0.002, 0.002)).at(hit_t).len(0.8).gain(0.8 * c.p("body")).fade(0.2, 20);
                      r.add("crack").noise(NoiseColor::White).lp(12000).shape(env_hit(0.001, 0.001)).at(hit_t).len(0.07).gain(0.8 * c.p("transient")).fade(0.05, 4)
                          .ring(c.p("tail")).reverb(1.5 + c.p("tail"), 0.5, 0.8);
                  });
            }
        }
    }
}