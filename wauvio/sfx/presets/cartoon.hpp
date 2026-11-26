#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_cartoon(Registry& R) {
                Def d{R, Category::Cartoon};

                d("Boing", OneShot(), "A springy bounce: a note that bobs up and down in pitch with shrinking swings.",
                  {"boing", "spring", "bounce", "cartoon"}, {"duration=0.9", "pitch=60", "range=18"},
                  [](Rc r, Cc c) {
                      const double p = c.p("pitch"), g = c.p("range");
                      r.add("boing").osc(Wave::Sine, Traj(p).to(0.06, p + g, Interp::Smooth).to(0.18, p - g * 0.5, Interp::Smooth).to(0.32, p + g * 0.6, Interp::Smooth).to(0.48, p - g * 0.3, Interp::Smooth).to(0.66, p + g * 0.25, Interp::Smooth).to(1.0, p, Interp::Smooth))
                          .fm(Traj(1.0), 2.0).shape(Traj(1.0).to(1.0, 0.0, Interp::Exp)).drive(1.5).gain(0.9).fade(0.5, 30);
                  });

                d("Slide Whistle Up", OneShot(), "A rising slide whistle: a pure, slightly breathy tone that swoops upward.",
                  {"slide whistle", "rise", "cartoon", "whistle"}, {"duration=0.8", "pitch=70", "range=30"},
                  [](Rc r, Cc c) {
                      r.add("whistle").osc(Wave::Sine, Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range"))).shape(Traj(0.0).to(0.1, 1.0).to(0.9, 0.9).to(1.0, 0.0)).trem(Traj(6.0), Traj(0.1)).gain(0.9);
                      r.add("breath").noise(NoiseColor::White).bp(Traj::exp(c.lg(0.0, 1800, 1800), 4500), 4.0).shape(Traj(0.0).to(0.1, 1.0).to(0.9, 0.9).to(1.0, 0.0)).gain(0.08);
                  });

                d("Slide Whistle Down", OneShot(), "A falling slide whistle: the same pure tone swooping downward, as if something fell.",
                  {"slide whistle", "fall", "cartoon", "whistle"}, {"duration=0.9", "pitch=96", "range=30"},
                  [](Rc r, Cc c) {
                      r.add("whistle").osc(Wave::Sine, Traj::linear(c.p("pitch"), c.p("pitch") - c.p("range"))).shape(Traj(0.0).to(0.08, 1.0).to(0.9, 0.9).to(1.0, 0.0)).trem(Traj(6.0), Traj(0.1)).gain(0.9);
                      r.add("breath").noise(NoiseColor::White).bp(Traj::exp(4500, 1800), 4.0).shape(Traj(0.0).to(0.08, 1.0).to(0.9, 0.9).to(1.0, 0.0)).gain(0.08);
                  });

                d("Bonk", OneShot(), "A hit on the head: a hollow wooden knock with a quick, downward-bending boom.",
                  {"bonk", "hit", "head", "cartoon"}, {"duration=0.5", "pitch=55", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("knock").modal(modes_wood(), Traj(c.p("pitch") + 12), {0.0}).decay_scale(0.5).strike(1.5).len(c.duration).gain(0.9);
                      r.add("boom").osc(Wave::Sine, Traj::linear(c.p("pitch") + 14, c.p("pitch") - 4)).drive(1.0 + 2.0 * c.p("intensity")).shape(env_hit(0.01, 0.002)).len(0.25).gain(0.8).fade(0.3, 15);
                  });

                d("Splat", OneShot(), "A wet impact: a squelchy burst of low noise and a dropping blob of tone.",
                  {"splat", "wet", "squish", "cartoon"}, {"duration=0.6", "intensity=0.7", "brightness=0.4"},
                  [](Rc r, Cc c) {
                      r.add("squelch").noise(NoiseColor::Pink).bp(Traj::exp(c.lg(c.p("brightness"), 1500, 3000), 300), 2.0).shape(env_hit(0.01, 0.002)).len(c.duration).gain(0.8).fade(0.2, 40).place(0, 0.5);
                      r.add("blob").osc(Wave::Sine, Traj::linear(70, 38)).drive(1.5 + 2.0 * c.p("intensity")).shape(env_hit(0.01, 0.002)).len(0.3).gain(0.7).fade(0.3, 20);
                  });

                d("Pop", OneShot(), "A cartoon pop: a sharp little burst with a quick upward chirp, light and cheeky.",
                  {"pop", "cartoon", "bubble", "burst"}, {"duration=0.15", "pitch=84"},
                  [](Rc r, Cc c) {
                      r.add("pop").osc(Wave::Sine, Traj(c.p("pitch") - 12).to(0.4, c.p("pitch") + 5, Interp::Exp).to(1.0, c.p("pitch"), Interp::Exp)).shape(env_hit(0.02, 0.003)).gain(0.9).fade(0.3, 8);
                      r.add("snap").noise(NoiseColor::White).hp(3500).shape(env_hit(0.001, 0.001)).len(0.015).gain(0.3).fade(0.05, 3);
                  });

                d("Squeak", OneShot(), "A rubber squeak: a short, high pitch that bends up then down with a plasticky edge.",
                  {"squeak", "rubber", "toy", "cartoon"}, {"duration=0.25", "pitch=90", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("squeak").osc(Wave::Saw, Traj(c.p("pitch") - 6).to(0.4, c.p("pitch") + 4, Interp::Smooth).to(1.0, c.p("pitch") - 2, Interp::Smooth)).fm(Traj(1.0 + 2.0 * c.p("intensity")), 2.0).bp(3000, 2.5).shape(env_bell(0.45, 0.3)).gain(0.8).fade(1, 20);
                  });

                d("Spring", OneShot(), "A coiled spring let go: a metallic, buzzing wobble that shakes itself out.",
                  {"spring", "sproing", "wobble", "cartoon"}, {"duration=1.2", "pitch=62", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("wobble").osc(Wave::Saw, Traj(c.p("pitch") + 8).to(0.15, c.p("pitch") - 5, Interp::Smooth).to(0.4, c.p("pitch") + 3, Interp::Smooth).to(1.0, c.p("pitch"), Interp::Smooth)).ringmod(Traj(34.0).to(1.0, 15.0), 0.7).bp(1600, 1.5).shape(Traj(1.0).to(1.0, 0.0, Interp::Exp)).gain(0.8).fade(0.5, 30);
                      r.add("ring").modal(modes_metal(), Traj(c.p("pitch") + 12), {0.0}).decay_scale(0.7).strike(0.5).len(c.duration).gain(0.25).place(0, 0.5);
                  });

                d("Wobble", Sustained(), "A woozy, wobbling tone: a pitch that sways slowly, like a dizzy character.",
                  {"wobble", "dizzy", "woozy", "cartoon"}, {"duration=2", "pitch=64", "rate=4", "depth=0.7"},
                  [](Rc r, Cc c) {
                      r.add("wob").osc(Wave::Triangle, Traj(c.p("pitch"))).fm(Traj(6.0 * c.p("depth")), 1.0).trem(Traj(c.p("rate")), Traj(0.3)).gain(0.8).fade(40, 80);
                  });

                d("Zip", OneShot(), "A fast zip across the screen: a quick, rising whoosh with a pitched sweep inside it.",
                  {"zip", "swoosh", "fast", "cartoon"}, {"duration=0.35", "pitch=60", "range=36"},
                  [](Rc r, Cc c) {
                      r.add("sweep").osc(Wave::Saw, Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range"))).lp(6000).shape(Traj(0.0).to(0.3, 1.0).to(1.0, 0.0)).gain(0.5).fade(1, 20);
                      r.add("air").noise(NoiseColor::White).bp(Traj::exp(800, 6000), 1.5).shape(Traj(0.0).to(0.3, 1.0).to(1.0, 0.0)).gain(0.5).place(0, 0.6);
                  });

                d("Scurry", OneShot(), "Scampering feet: a very fast run of soft, light taps that rises in pitch as it goes.",
                  {"scurry", "run", "feet", "cartoon"}, {"duration=0.9", "steps=12", "pitch=76"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const double gap = c.duration / n;
                      for (int i = 0; i < n; ++i)
                          tone_at(r, "t" + std::to_string(i), Wave::Triangle, c.p("pitch") + (i % 2 ? 3 : 0) + i * 0.4, gap * i, 0.05, 0.6, 0.02).place(i % 2 ? 0.3 : -0.3, 0.0);
                  });

                d("Sad Trombone", OneShot(), "The failure sting: four falling, wah-wah trombone notes, the last one sagging in pitch.",
                  {"trombone", "fail", "sad", "wah"}, {"duration=3", "pitch=58", "tail=0.5"},
                  [](Rc r, Cc c) {
                      const double ns[] = {0.0, -1.0, -2.0, -3.0}; const double starts[] = {0.0, 0.6, 1.2, 1.8}; const double lens[] = {0.55, 0.55, 0.55, 1.2};
                      for (int i = 0; i < 4; ++i) {
                          const double p = c.p("pitch") + ns[i];
                          r.add("note" + std::to_string(i)).osc(Wave::Saw, i == 3 ? Traj(p).to(0.4, p).to(1.0, p - 2.0, Interp::Smooth) : Traj(p)).lp(Traj(700).to(0.3, 1900, Interp::Smooth).to(1.0, 800, Interp::Smooth), 2.0, 4).trem(Traj(5.0), Traj(0.08))
                              .shape(Traj(0.0).to(0.1, 1.0).to(0.85, 0.85).to(1.0, 0.0)).at(starts[i]).len(lens[i]).gain(0.7).fade(2, 30).ring(i == 3 ? c.p("tail") : 0.0);
                      }
                  });

                d("Duck Quack", OneShot(), "A single quack: a nasal, buzzing burst that bends downward as it fades.",
                  {"duck", "quack", "animal", "cartoon"}, {"duration=0.4", "pitch=58", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("quack").formant(Traj(0.0).to(1.0, 3.0), Traj(c.p("pitch") + 6).to(1.0, c.p("pitch") - 4), 0.0).shift(Traj(2.0)).drive(1.0 + 2.0 * c.p("intensity"), DriveP::Hard).shape(Traj(0.0).to(0.08, 1.0).to(1.0, 0.0)).gain(0.9).fade(1, 30);
                      r.add("buzz").osc(Wave::Saw, Traj(c.p("pitch") + 6).to(1.0, c.p("pitch") - 4)).bp(1200, 3.0).shape(Traj(0.0).to(0.08, 1.0).to(1.0, 0.0)).gain(0.3);
                  });

                d("Cuckoo", OneShot(), "A cuckoo clock call: two soft, hollow whistled notes, the second lower.",
                  {"cuckoo", "clock", "bird", "whistle"}, {"duration=1", "pitch=84"},
                  [](Rc r, Cc c) {
                      tone_at(r, "cu", Wave::Sine, c.p("pitch"), 0.0, 0.3, 0.8, 0.1).fm(Traj(0.6), 1.0);
                      tone_at(r, "koo", Wave::Sine, c.p("pitch") - 4, 0.38, 0.45, 0.8, 0.1).fm(Traj(0.6), 1.0).reverb(0.6, 0.5, 0.15);
                  });
            }
        }
    }
}
