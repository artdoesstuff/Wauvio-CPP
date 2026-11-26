#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_foley(Registry& R) {
                Def d{R, Category::Foley};

                d("Footstep Wood", OneShot(), "A single step on a wooden floor: a dull thud with a short, hollow knock on top.",
                  {"footstep", "wood", "floor", "step"}, {"duration=0.3", "pitch=48", "intensity=0.6", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("thud").osc(Wave::Sine, Traj::linear(c.p("pitch") + 14, c.p("pitch"))).shape(env_hit(0.01, 0.002)).len(0.12).gain(0.8).fade(0.3, 12);
                      r.add("knock").modal(modes_wood(), Traj(c.p("pitch") + 12), {0.0}).decay_scale(0.4).strike(2.0).gain(0.5 * c.p("intensity") + 0.2).len(0.25);
                      r.add("scuff").noise(NoiseColor::Pink).bp(Traj(c.lg(c.p("brightness"), 600, 2500)), 0.8).shape(env_hit(0.05, 0.002)).len(0.1).gain(0.25).fade(1, 20);
                  });

                d("Footsteps Walking", Evolving(), "A short walk: steady steps alternating left and right on a hard floor, slightly uneven.",
                  {"footsteps", "walk", "floor", "steps"}, {"duration=4", "steps=6", "pitch=46", "brightness=0.5", "width=0.6"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); Rng g = c.rng(1); const double gap = c.duration / n;
                      for (int i = 0; i < n; ++i) {
                          const double t = gap * i + g.range(-0.02, 0.02), side = (i % 2 ? 1.0 : -1.0) * 0.3 * c.p("width"), p = c.p("pitch") + (i % 2 ? 1.5 : 0.0) + g.range(-0.5, 0.5);
                          r.add("t" + std::to_string(i)).osc(Wave::Sine, Traj::linear(p + 14, p)).shape(env_hit(0.01, 0.002)).at(std::max(0.0, t)).len(0.12).gain(g.range(0.7, 1.0)).place(side, 0.0).fade(0.3, 12);
                          r.add("k" + std::to_string(i)).noise(NoiseColor::Pink).bp(Traj(c.lg(c.p("brightness"), 700, 2400)), 1.2).shape(env_hit(0.04, 0.002)).at(std::max(0.0, t)).len(0.09).gain(g.range(0.3, 0.5)).place(side, 0.0).fade(0.5, 10);
                      }
                  });

                d("Footstep Gravel", OneShot(), "One step on loose stones: a crunch of many tiny grains that settles quickly.",
                  {"footstep", "gravel", "crunch", "step"}, {"duration=0.4", "density=0.6", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("grains").noise(NoiseColor::Dust, Traj(1500.0 + 2500.0 * c.p("density")).to(1.0, 200.0)).bp(Traj(c.lg(c.p("brightness"), 1800, 4200)), 0.8).shape(Traj(0.0).to(0.05, 1.0).to(1.0, 0.002, Interp::Exp)).len(c.duration).gain(0.9).place(0, 0.6).fade(0.5, 20);
                      r.add("thud").osc(Wave::Sine, Traj::linear(70, 50)).shape(env_hit(0.01, 0.002)).len(0.1).gain(0.6).fade(0.3, 10);
                  });

                d("Door Knock", OneShot(), "Three knocks on a wooden door: solid, slightly uneven raps with a hollow ring.",
                  {"knock", "door", "wood", "rap"}, {"duration=1", "pitch=52", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("rap").modal(modes_wood(), Traj(c.p("pitch")), {0.0, 0.2, 0.4}).hit_jitter(0.03, 0.2).decay_scale(0.5).strike(1.5).len(c.duration).gain(0.9);
                      r.add("thump").osc(Wave::Sine, Traj(c.p("pitch") - 6)).pulses_at({0.0, 0.2, 0.4}, 60.0, 1.0, 1.0).len(c.duration).gain(0.4 * c.p("intensity") + 0.1).lp(300);
                  });

                d("Door Creak", OneShot(), "An old hinge: a slow, groaning sweep with a nasal, rasping edge.",
                  {"creak", "door", "hinge", "horror"}, {"duration=2.5", "pitch=58", "intensity=0.6", "tail=0.5"},
                  [](Rc r, Cc c) {
                      r.add("groan").osc(Wave::Saw, Traj(c.p("pitch")).to(0.3, c.p("pitch") + 5, Interp::Smooth).to(0.65, c.p("pitch") - 2, Interp::Smooth).to(1.0, c.p("pitch") + 3, Interp::Smooth)).fm(Traj(1.0 + 3.0 * c.p("intensity")), 1.7)
                          .bp(Traj(900).to(0.5, 1400, Interp::Smooth).to(1.0, 800, Interp::Smooth), 5.0, 4).shape(env_bell(0.5, 0.3)).ring(c.p("tail")).reverb(0.8, 0.5, 0.2).gain(0.8).fade(20, 80);
                      r.add("rasp").noise(NoiseColor::Pink).bp(Traj(1500).to(1.0, 1800, Interp::Smooth), 6.0).trem(Traj(24.0), Traj(0.8)).shape(env_bell(0.5, 0.3)).gain(0.15);
                  });

                d("Glass Clink", OneShot(), "Two glasses touching: a bright, short, high ring that fades quickly.",
                  {"glass", "clink", "toast", "drink"}, {"duration=0.8", "tail=0.5", "pitch=92", "decay=0.5"},
                  [](Rc r, Cc c) {
                      r.add("clink").modal(modes_glass(), Traj(c.p("pitch")), {0.0}).decay_scale(0.15 + 0.5 * c.p("decay")).strike(0.6).len(c.duration).ring(c.p("tail")).gain(0.9).reverb(0.5, 0.5, 0.1);
                      r.add("clink2").modal(modes_glass(), Traj(c.p("pitch") + 3), {0.0}).decay_scale(0.12 + 0.4 * c.p("decay")).strike(0.6).at(0.004).len(c.duration).ring(c.p("tail")).gain(0.5).place(0.2, 0.0);
                  });

                d("Glass Break", OneShot(), "A pane shattering: a sharp crash of noise followed by a scatter of small falling glass tinkles.",
                  {"glass", "break", "shatter", "crash"}, {"duration=1.4", "tail=0.6", "intensity=0.7", "pitch=92"},
                  [](Rc r, Cc c) {
                      r.add("crash").noise(NoiseColor::White).hp(2000).shape(env_hit(0.002, 0.002)).len(0.35).gain(0.9).fade(0.1, 40);
                      r.add("shards").modal(modes_glass(), Traj(c.p("pitch")), c.scattered(14 + static_cast<int>(10.0 * c.p("intensity")), 3, 0.02, 0.95)).hit_notes({0.0, 7.0, -5.0, 12.0, 4.0, -9.0}).hit_jitter(0.0, 0.7).decay_scale(0.18).strike(0.5).len(c.duration).ring(c.p("tail")).gain(0.7).place(0, 0.9).reverb(0.6, 0.5, 0.15);
                      r.add("low").osc(Wave::Sine, Traj::linear(120, 60)).shape(env_hit(0.003, 0.002)).len(0.15).gain(0.4).fade(0.3, 10);
                  });

                d("Metal Clang", OneShot(), "A heavy metal object struck once: a loud inharmonic clang with a long, slowly beating ring.",
                  {"metal", "clang", "hit", "impact"}, {"duration=1.6", "tail=1", "pitch=60", "decay=0.5"},
                  [](Rc r, Cc c) {
                      r.add("clang").modal(modes_metal(), Traj(c.p("pitch")), {0.0}).decay_scale(0.3 + 1.2 * c.p("decay")).strike(1.0).len(c.duration).ring(c.p("tail")).gain(0.9).reverb(0.8, 0.5, 0.15);
                      r.add("click").noise(NoiseColor::White).bp(3500, 1.0).shape(env_hit(0.001, 0.001)).len(0.02).gain(0.4).fade(0.05, 3);
                  });

                d("Wood Hit", OneShot(), "A single strike on wood: a short, hollow, dry knock with very little ring.",
                  {"wood", "hit", "knock", "block"}, {"duration=0.4", "pitch=64", "decay=0.4"},
                  [](Rc r, Cc c) {
                      r.add("hit").modal(modes_wood(), Traj(c.p("pitch")), {0.0}).decay_scale(0.3 + 0.8 * c.p("decay")).strike(1.0).len(c.duration).gain(0.9);
                  });

                d("Paper Rustle", OneShot(), "Paper being crumpled or turned: a soft, dry burst of irregular crinkling noise.",
                  {"paper", "rustle", "crumple", "page"}, {"duration=1.2", "density=0.6", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("crinkle").noise(NoiseColor::Dust, Traj(300.0 + 900.0 * c.p("density"))).bp(Traj(c.lg(c.p("brightness"), 3500, 7000)), 0.9).shape(env_bell(0.4, 0.3)).gain(0.9).place(0, 0.6).fade(5, 30);
                      r.add("body").noise(NoiseColor::White).hp(3000).pulses(Traj(14.0), 40, 0.8, 0.8).shape(env_bell(0.4, 0.3)).gain(0.3).place(0, 0.6);
                  });

                d("Clap", OneShot(), "One hand-clap: a few tight bursts of noise a fraction of a second apart, with a short room.",
                  {"clap", "hands", "hit", "applause"}, {"duration=0.4", "brightness=0.6", "width=0.5"},
                  [](Rc r, Cc c) {
                      r.add("burst").noise(NoiseColor::White).bp(Traj(c.lg(c.p("brightness"), 1500, 3500)), 0.9).pulses_at({0.0, 0.03, 0.07, 0.11}, 18.0, 0.3, 1.0).shape(Traj(1.0).to(1.0, 0.002, Interp::Exp)).gain(0.9).place(0, c.p("width")).reverb(0.4, 0.5, 0.15).fade(0.2, 15);
                      r.add("tail").noise(NoiseColor::White).bp(1500, 0.8).shape(env_hit(0.02, 0.001)).at(0.04).len(0.25).gain(0.2).place(0, c.p("width"));
                  });

                d("Finger Snap", OneShot(), "A crisp snap of the fingers: a very short, bright pop with a tiny hollow resonance.",
                  {"snap", "finger", "click", "hit"}, {"duration=0.25", "pitch=84", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("snap").noise(NoiseColor::White).bp(Traj(c.lg(c.p("brightness"), 2500, 5000)), 1.5).shape(env_hit(0.002, 0.002)).len(0.05).gain(0.8).fade(0.05, 6);
                      r.add("ring").modal(modes_wood(), Traj(c.p("pitch") - 24), {0.0}).decay_scale(0.15).strike(0.5).len(0.2).gain(0.4);
                  });

                d("Camera Shutter", OneShot(), "An SLR shutter: two quick, dry mechanical clicks close together.",
                  {"camera", "shutter", "click", "photo"}, {"duration=0.3", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("c1").noise(NoiseColor::White).bp(Traj(c.lg(c.p("brightness"), 1800, 3500)), 1.5).shape(env_hit(0.001, 0.001)).len(0.03).gain(0.8).fade(0.05, 4);
                      r.add("c2").noise(NoiseColor::White).bp(Traj(c.lg(c.p("brightness"), 1400, 2800)), 1.5).shape(env_hit(0.001, 0.001)).at(0.07).len(0.04).gain(0.7).fade(0.05, 5);
                      r.add("body").modal(modes_wood(), Traj(40.0), {0.0, 0.3}).decay_scale(0.12).strike(0.5).len(0.2).gain(0.4);
                  });

                d("Typing Keys", Evolving(), "A keyboard being typed on: many short, hard clicks at an uneven rhythm with the odd bigger key.",
                  {"typing", "keyboard", "keys", "office"}, {"duration=3", "density=0.6", "brightness=0.6", "width=0.5"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(2); const int n = 8 + static_cast<int>(28.0 * c.p("density")); double t = 0.0;
                      for (int i = 0; i < n && t < c.duration - 0.05; ++i) {
                          const bool big = g.uniform() < 0.1;
                          r.add("k" + std::to_string(i)).noise(NoiseColor::White).bp(Traj(c.lg(c.p("brightness"), 1500, 3500) * (big ? 0.6 : g.range(0.8, 1.2))), 1.5).shape(env_hit(0.001, 0.001)).at(t).len(big ? 0.05 : 0.03).gain(big ? 0.9 : g.range(0.4, 0.7)).place(g.range(-0.3, 0.3) * c.p("width"), 0.0).fade(0.05, 5);
                          t += g.range(0.04, 0.25);
                      }
                  });

                d("Clock Tick", Sustained(), "A wall clock: a steady tick-tock, with a short dry click and a slightly lower second tick.",
                  {"clock", "tick", "tock", "time"}, {"duration=6", "rate=2", "pitch=80", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("tick").modal(modes_wood(), Traj(c.p("pitch")), {0.0}).decay_scale(0.1).strike(0.5).pulses(Traj(c.p("rate") * 0.5), 40, 1.0).len(c.duration).gain(0.9).fade(1, 20);
                      r.add("tock").modal(modes_wood(), Traj(c.p("pitch") - 4), {0.0}).decay_scale(0.1).strike(0.5).pulses(Traj(c.p("rate") * 0.5), 40, 1.0).at(0.5 / c.p("rate")).len(c.duration - 0.5 / c.p("rate")).gain(0.7).fade(1, 20);
                  });

                d("Water Drop", OneShot(), "A single drop landing in water: a quick rising bloop with a small ripple of ring behind it.",
                  {"water", "drop", "plink", "bloop"}, {"duration=0.4", "tail=0.5", "pitch=80", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("bloop").osc(Wave::Sine, Traj(c.p("pitch") - 6).to(1.0, c.p("pitch") + 6, Interp::Exp)).shape(env_hit(0.05, 0.002)).len(0.18).ring(c.p("tail")).reverb(0.8, 0.5, 0.2).gain(0.9).fade(0.5, 20);
                      r.add("splash").noise(NoiseColor::White).bp(Traj(c.lg(c.p("brightness"), 3000, 6000)), 1.0).shape(env_hit(0.01, 0.002)).len(0.04).gain(0.15).fade(0.1, 8);
                  });

                d("Bubble Pop", OneShot(), "A bubble bursting: a short rising blip that ends with a tiny snap.",
                  {"bubble", "pop", "burst", "water"}, {"duration=0.2", "pitch=76"},
                  [](Rc r, Cc c) {
                      r.add("blip").osc(Wave::Sine, Traj(c.p("pitch") - 4).to(1.0, c.p("pitch") + 10, Interp::Exp)).shape(Traj(0.0).to(0.2, 1.0).to(1.0, 0.0)).len(c.duration).gain(0.8).fade(1, 10);
                      r.add("snap").noise(NoiseColor::White).hp(3000).shape(env_hit(0.001, 0.001)).at(c.duration * 0.75).len(0.02).gain(0.25).fade(0.05, 3);
                  });

                d("Cork Pop", OneShot(), "A bottle being opened: a hollow, low 'pop' with a quick resonance and a small hiss.",
                  {"cork", "pop", "bottle", "champagne"}, {"duration=0.5", "pitch=52", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("pop").osc(Wave::Sine, Traj(c.p("pitch") + 12).to(1.0, c.p("pitch") - 4, Interp::Exp)).drive(1.0 + 2.0 * c.p("intensity")).shape(env_hit(0.01, 0.002)).len(0.12).gain(0.9).fade(0.3, 12);
                      r.add("hollow").modal(modes_wood(), Traj(c.p("pitch") + 7), {0.0}).decay_scale(0.35).strike(1.0).len(0.3).gain(0.4);
                      r.add("fizz").noise(NoiseColor::White).hp(4000).shape(Traj(0.0).to(0.05, 1.0).to(1.0, 0.001, Interp::Exp)).at(0.03).len(0.3).gain(0.05).place(0, 0.7);
                  });

                d("Switch Click", OneShot(), "A light switch or button: a small, crisp mechanical click with a hint of body.",
                  {"switch", "click", "button", "light"}, {"duration=0.15", "brightness=0.6", "pitch=72"},
                  [](Rc r, Cc c) {
                      r.add("click").noise(NoiseColor::White).bp(Traj(c.lg(c.p("brightness"), 2000, 4500)), 1.8).shape(env_hit(0.001, 0.001)).len(0.03).gain(0.9).fade(0.05, 4);
                      r.add("body").modal(modes_wood(), Traj(c.p("pitch")), {0.0}).decay_scale(0.1).strike(0.5).len(0.1).gain(0.5);
                  });

                d("Match Strike", OneShot(), "A match being struck: a short scratchy rasp that bursts into a soft flare and settles to a flicker.",
                  {"match", "strike", "fire", "lighter"}, {"duration=1.4", "intensity=0.6", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("scratch").noise(NoiseColor::White).hp(Traj(c.lg(c.p("brightness"), 2000, 4000))).shape(Traj(0.0).to(0.05, 1.0).to(0.12, 0.0)).len(c.duration).gain(0.8).fade(0.5, 5);
                      r.add("flare").noise(NoiseColor::Pink).bp(1800, 0.7).shape(Traj(0.0).to(0.1, 0.0).to(0.14, 1.0, Interp::Smooth).to(1.0, 0.001, Interp::Exp)).len(c.duration).gain(0.4 * c.p("intensity") + 0.1).fade(0.5, 40);
                      r.add("flicker").noise(NoiseColor::Dust, Traj(60.0)).hp(2000).shape(Traj(0.0).to(0.15, 0.0).to(0.2, 1.0).to(1.0, 0.0)).len(c.duration).gain(0.3).place(0, 0.5);
                  });

                d("Coin Drop", OneShot(), "A coin dropped on a hard surface: a ringing clink that bounces and rattles to a stop.",
                  {"coin", "drop", "metal", "bounce"}, {"duration=1.2", "tail=0.5", "pitch=86", "decay=0.5"},
                  [](Rc r, Cc c) {
                      r.add("coin").modal(modes_metal(), Traj(c.p("pitch")), {0.0, 0.16, 0.29, 0.39, 0.47, 0.53, 0.57, 0.6}).hit_jitter(0.0, 0.3).decay_scale(0.12 + 0.3 * c.p("decay")).strike(0.5).len(c.duration).ring(c.p("tail")).gain(0.9).place(0, 0.3).reverb(0.4, 0.5, 0.1);
                  });

                d("Keys Jingle", OneShot(), "A bunch of keys shaken: many small metallic clinks at once, bright and busy.",
                  {"keys", "jingle", "metal", "rattle"}, {"duration=1.2", "density=0.6", "pitch=90", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("keys").modal(modes_metal(), Traj(c.p("pitch")), c.scattered(10 + static_cast<int>(30.0 * c.p("density")), 9, 0.0, 0.9)).hit_notes({0.0, 4.0, -3.0, 7.0, -7.0, 2.0}).hit_jitter(0.0, 0.8).decay_scale(0.1).strike(0.4).shape(env_bell(0.4, 0.3)).len(c.duration).gain(0.8).place(0, 0.7).lp(Traj(c.lg(c.p("brightness"), 8000, 16000)));
                  });

                d("Zipper", OneShot(), "A zip pulled quickly: a rising, ratcheting buzz of tiny teeth that speeds up to the end.",
                  {"zipper", "zip", "pull", "cloth"}, {"duration=0.7", "brightness=0.6", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("teeth").noise(NoiseColor::White).bp(Traj(c.lg(c.p("brightness"), 2500, 4500)), 1.5).pulses(Traj(60.0).to(1.0, 180.0), 8.0, 0.9, 0.1).shape(Traj(0.4).to(0.9, 1.0).to(1.0, 0.0)).gain(0.8).place(0, 0.4).fade(2, 20);
                      r.add("buzz").osc(Wave::Saw, Traj::linear(60, 84)).bp(2500, 2.0).gain(0.12 * c.p("intensity")).shape(Traj(0.3).to(0.9, 1.0).to(1.0, 0.0));
                  });

                d("Applause", Sustained(), "A crowd clapping: a dense, rolling roar of overlapping claps with the odd louder one on top.",
                  {"applause", "crowd", "clapping", "audience"}, {"duration=6", "density=0.7", "brightness=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("claps").noise(NoiseColor::Dust, Traj(300.0 + 1500.0 * c.p("density"))).bp(Traj(c.lg(c.p("brightness"), 1500, 3200)), 0.7).gain(0.9).place(0, c.p("width")).reverb(0.6, 0.5, 0.2).fade(400, 700);
                      r.add("roar").noise(NoiseColor::Pink).bp(1500, 0.6).trem(Traj(2.5), Traj(0.3)).gain(0.3).place(0, c.p("width")).fade(400, 700);
                      r.add("loud").noise(NoiseColor::Dust, Traj(12.0 + 20.0 * c.p("density"))).bp(2200, 1.0).drive(2.0, DriveP::Hard, 0.5).gain(0.6).place(0, 1.0).fade(400, 700).variation(3);
                  });
            }
        }
    }
}
