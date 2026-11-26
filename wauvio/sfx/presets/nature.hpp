#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_nature(Registry& R) {
                Def d{R, Category::Nature};

                d("Light Rain", Sustained(), "Soft, sparse drops over a faint hiss of fine rain: steady and calm.",
                  {"rain", "weather", "ambience", "calm"}, {"duration=10", "density=0.4", "brightness=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("drops").noise(NoiseColor::Dust, Traj(60.0 + 340.0 * c.p("density"))).bp(Traj(c.lg(c.p("brightness"), 2500, 6500)), 0.8).gain(0.9).place(0, c.p("width")).fade(400, 800);
                      r.add("hiss").noise(NoiseColor::White).hp(5000).lp(Traj(c.lg(c.p("brightness"), 7000, 14000))).gain(0.1).place(0, c.p("width")).fade(400, 800);
                  });

                d("Heavy Rain", Sustained(), "Dense downpour: thousands of drops, a body of pink noise and a low rumble beneath.",
                  {"rain", "storm", "weather", "downpour"}, {"duration=10", "density=0.7", "brightness=0.5", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("drops").noise(NoiseColor::Dust, Traj(800.0 + 1700.0 * c.p("density"))).bp(Traj(c.lg(c.p("brightness"), 2000, 5000)), 0.5).gain(0.9).place(0, c.p("width")).fade(500, 900);
                      r.add("body").noise(NoiseColor::Pink).hp(400).lp(Traj(c.lg(c.p("brightness"), 3500, 8000))).gain(0.5).place(0, c.p("width")).fade(500, 900);
                      r.add("rumble").noise(NoiseColor::Brown).lp(220).gain(0.15).fade(500, 900);
                  });

                d("Rain on Roof", Sustained(), "Rain heard from inside: the drops turn into small hollow taps over a dull wash.",
                  {"rain", "roof", "indoors", "weather"}, {"duration=10", "density=0.5", "brightness=0.4", "width=0.5"},
                  [](Rc r, Cc c) {
                      r.add("taps").noise(NoiseColor::Dust, Traj(120.0 + 500.0 * c.p("density"))).bp(Traj(c.lg(c.p("brightness"), 700, 1800)), 2.5).gain(1.0).place(0, c.p("width")).fade(400, 800);
                      r.add("wash").noise(NoiseColor::Pink).lp(Traj(c.lg(c.p("brightness"), 900, 2500))).hp(150).gain(0.35).place(0, c.p("width")).fade(400, 800);
                      r.add("sizzle").noise(NoiseColor::White).hp(4500).gain(0.06).fade(400, 800);
                  });

                d("Distant Thunder", OneShot(), "A long, low roll of thunder far away: no crack, just a swelling rumble and a big room.",
                  {"thunder", "storm", "distant", "rumble"}, {"duration=5", "tail=3", "intensity=0.6", "brightness=0.3"},
                  [](Rc r, Cc c) {
                      r.add("rumble").noise(NoiseColor::Brown).lp(Traj::exp(c.lg(c.p("brightness"), 250, 700), 50.0), 0.9, 4).shape(Traj(0.02).to(0.12, 1.0, Interp::Smooth).to(0.5, 0.5, Interp::Linear).to(1.0, 0.001, Interp::Exp))
                          .len(c.duration).ring(c.p("tail")).reverb(3.5 + c.p("tail"), 0.5, 0.6).gain(0.9).place(0, 0.7).fade(20, 50);
                      r.add("sub").osc(Wave::Sine, Traj::linear(34, 26)).shape(Traj(0.0).to(0.15, 1.0, Interp::Smooth).to(1.0, 0.001, Interp::Exp)).len(c.duration).gain(0.3 * c.p("intensity")).fade(50, 60);
                  });

                d("Thunderclap", OneShot(), "A close strike: a sharp crack, a heavy body of rumble and the long rolling decay behind it.",
                  {"thunder", "storm", "crack", "close"}, {"duration=4", "tail=3", "intensity=0.8", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("crack").noise(NoiseColor::White).hp(Traj::exp(c.lg(c.p("brightness"), 800, 2500), 400)).shape(env_hit(0.0008, 0.0008)).len(0.35).gain(0.9).fade(0.05, 10);
                      r.add("body").noise(NoiseColor::Brown).lp(Traj::exp(1200, 80), 1.0, 4).shape(env_hit(0.01, 0.0008)).len(c.duration).ring(c.p("tail")).reverb(3.0 + c.p("tail"), 0.5, 0.7).gain(0.9).place(0, 0.8).fade(0.2, 30);
                      r.add("sub").osc(Wave::Sine, Traj::linear(40, 28)).shape(env_hit(0.004, 0.001)).len(c.duration * 0.6).gain(0.6 * c.p("intensity")).fade(0.5, 40);
                      r.add("rolls").noise(NoiseColor::Pink).lp(500).pulses(Traj(2.5), 400, 0.8, 0.7).shape(Traj(0.0).to(0.1, 1.0).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).reverb(3.0, 0.5, 0.5).gain(0.35).place(0, 1.0);
                  });

                d("Gentle Wind", Sustained(), "A slow breath of wind: filtered noise whose pitch drifts up and down by itself.",
                  {"wind", "breeze", "weather", "ambience"}, {"duration=12", "brightness=0.5", "depth=0.5", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("air").noise(NoiseColor::Pink).bp(Traj(c.lg(c.p("brightness"), 350, 900)), 1.4, 4).filter_lfo(lfo(Mod::Shape::Smooth, 0.18, 0.9 * c.p("depth"))).gain(0.9).place(0, c.p("width")).fade(1200, 1500);
                      r.add("hiss").noise(NoiseColor::White).hp(3000).lp(7000).trem(Traj(0.2), Traj(0.7 * c.p("depth"))).gain(0.08).place(0, c.p("width")).fade(1200, 1500);
                  });

                d("Howling Wind", Sustained(), "A thin, wailing wind: narrow resonances sliding up and down over a rushing bed.",
                  {"wind", "howl", "storm", "eerie"}, {"duration=12", "intensity=0.6", "brightness=0.5", "width=0.9"},
                  [](Rc r, Cc c) {
                      const double q = 4.0 + 8.0 * c.p("intensity");
                      r.add("howl1").noise(NoiseColor::Pink).bp(Traj(500).to(0.3, 820, Interp::Smooth).to(0.6, 610, Interp::Smooth).to(1.0, 740, Interp::Smooth), q, 4).filter_lfo(lfo(Mod::Shape::Smooth, 0.3, 0.35)).gain(0.8).place(-0.5, c.p("width")).fade(1500, 1500);
                      r.add("howl2").noise(NoiseColor::Pink).bp(Traj(900).to(0.4, 1250, Interp::Smooth).to(0.7, 980, Interp::Smooth).to(1.0, 1150, Interp::Smooth), q, 4).filter_lfo(lfo(Mod::Shape::Smooth, 0.22, 0.3)).gain(0.6).place(0.5, c.p("width")).fade(1500, 1500).variation(3);
                      r.add("rush").noise(NoiseColor::Pink).lp(Traj(c.lg(c.p("brightness"), 1200, 4000))).hp(200).gain(0.35).place(0, c.p("width")).fade(1500, 1500);
                  });

                d("Wind Gust", OneShot(), "A single gust rising out of calm and dying away: the filter opens and closes with the level.",
                  {"wind", "gust", "whoosh", "weather"}, {"duration=3.5", "tail=1", "brightness=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("gust").noise(NoiseColor::Pink).bp(Traj(300).to(0.45, c.lg(c.p("brightness"), 900, 2200), Interp::Smooth).to(1.0, 350, Interp::Smooth), 1.2, 4).shape(env_bell(0.45, 0.2)).ring(c.p("tail")).reverb(1.0, 0.5, 0.2).place(0, c.p("width")).gain(1.0).fade(10, 30);
                  });

                d("Ocean Waves", Evolving(), "Slow surf: two out-of-step swells of filtered noise, with foam hissing on each crest.",
                  {"ocean", "waves", "sea", "beach", "ambience"}, {"duration=16", "brightness=0.5", "depth=0.9", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("swell1").noise(NoiseColor::Pink).lp(Traj(c.lg(c.p("brightness"), 900, 2600)), 0.8, 4).trem(Traj(0.11), Traj(c.p("depth"))).gain(0.9).place(-0.4, c.p("width")).fade(1500, 1800);
                      r.add("swell2").noise(NoiseColor::Pink).lp(Traj(c.lg(c.p("brightness"), 800, 2200)), 0.8, 4).trem(Traj(0.07), Traj(c.p("depth"))).gain(0.8).place(0.4, c.p("width")).fade(1500, 1800).variation(2);
                      r.add("foam").noise(NoiseColor::White).hp(3500).lp(11000).trem(Traj(0.11), Traj(0.95)).gain(0.12).place(0, c.p("width")).fade(1500, 1800);
                      r.add("deep").noise(NoiseColor::Brown).lp(150).trem(Traj(0.09), Traj(0.5)).gain(0.25).fade(1500, 1800);
                  });

                d("Stream", Sustained(), "A shallow brook: a stream of tiny bubbling resonances over a smooth, bright flow.",
                  {"water", "stream", "brook", "nature"}, {"duration=10", "density=0.6", "brightness=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("burble").noise(NoiseColor::Dust, Traj(500.0 + 900.0 * c.p("density"))).bp(Traj(c.lg(c.p("brightness"), 1200, 3200)), 4.0).gain(1.0).place(0, c.p("width")).fade(500, 800);
                      r.add("flow").noise(NoiseColor::Pink).bp(Traj(c.lg(c.p("brightness"), 1200, 2800)), 1.0).gain(0.45).place(0, c.p("width")).fade(500, 800);
                      r.add("sparkle").noise(NoiseColor::White).hp(6000).trem(Traj(6.0), Traj(0.4)).gain(0.06).fade(500, 800);
                  });

                d("Waterfall", Sustained(), "A broad, constant roar of falling water with a heavy low end and a bright mist on top.",
                  {"water", "waterfall", "roar", "nature"}, {"duration=12", "brightness=0.6", "intensity=0.7", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("roar").noise(NoiseColor::White).hp(150).lp(Traj(c.lg(c.p("brightness"), 3500, 10000))).gain(0.6).place(0, c.p("width")).fade(800, 1000);
                      r.add("body").noise(NoiseColor::Pink).lp(1200).gain(0.5 * c.p("intensity") + 0.2).place(0, c.p("width")).fade(800, 1000).variation(4);
                      r.add("thud").noise(NoiseColor::Brown).lp(180).gain(0.3 * c.p("intensity")).trem(Traj(0.4), Traj(0.3)).fade(800, 1000);
                  });

                d("Fire Crackle", Sustained(), "A campfire: irregular pops and snaps over a soft, low roar of flame.",
                  {"fire", "crackle", "campfire", "fireplace"}, {"duration=10", "density=0.5", "intensity=0.6", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("crackle").noise(NoiseColor::Dust, Traj(15.0 + 60.0 * c.p("density"))).hp(1200).lp(Traj(c.lg(c.p("brightness"), 5000, 11000))).drive(2.0 + 3.0 * c.p("intensity"), DriveP::Hard, 0.6).gain(0.9).place(0, 0.6).fade(300, 500);
                      r.add("pops").noise(NoiseColor::Dust, Traj(3.0 + 7.0 * c.p("density"))).bp(2200, 1.2).drive(3.0, DriveP::Hard).gain(1.0).place(0, 0.9).fade(300, 500).variation(5);
                      r.add("roar").noise(NoiseColor::Brown).lp(400).hp(60).trem(Traj(0.7), Traj(0.35)).gain(0.3).fade(300, 500);
                  });

                d("Bird Chirps", Evolving(), "A handful of small birds: quick rising and falling chirps scattered through time at different pitches.",
                  {"bird", "chirp", "nature", "morning"}, {"duration=8", "density=0.4", "pitch=92", "width=0.9", "tail=0.5"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(1); const int n = 4 + static_cast<int>(12.0 * c.p("density"));
                      for (int i = 0; i < n; ++i) {
                          const double start = g.range(0.0, std::max(0.2, c.duration - 0.6)), p = c.p("pitch") + g.range(-8.0, 8.0), len = g.range(0.1, 0.28);
                          const bool up = g.uniform() < 0.6;
                          r.add("chirp" + std::to_string(i)).osc(Wave::Sine, up ? Traj(p).to(0.5, p + 5.0, Interp::Smooth).to(1.0, p + 2.0, Interp::Smooth) : Traj(p + 3.0).to(1.0, p - 4.0, Interp::Smooth))
                              .fm(Traj(1.0), 2.0).shape(env_bell(0.4, 0.28)).at(start).len(len).gain(g.range(0.4, 1.0)).place(g.range(-0.9, 0.9), 0.0).reverb(0.8, 0.5, 0.15).ring(0.2).fade(1, 15);
                      }
                  });

                d("Dawn Chorus", Sustained(), "A thick morning chorus: many birds at once, near and far, in a light outdoor reverb.",
                  {"bird", "chorus", "dawn", "forest"}, {"duration=12", "density=0.8", "pitch=90", "width=1", "tail=0.5"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(2); const int n = 18 + static_cast<int>(30.0 * c.p("density"));
                      for (int i = 0; i < n; ++i) {
                          const double start = g.range(0.0, std::max(0.2, c.duration - 0.5)), p = c.p("pitch") + g.range(-12.0, 10.0), len = g.range(0.08, 0.3);
                          r.add("bird" + std::to_string(i)).osc(Wave::Sine, g.uniform() < 0.5 ? Traj(p).to(0.5, p + g.range(2.0, 7.0), Interp::Smooth).to(1.0, p) : Traj(p + 4.0).to(1.0, p - 3.0))
                              .fm(Traj(g.range(0.5, 1.5)), g.range(1.5, 3.0)).shape(env_bell(0.4, 0.3)).at(start).len(len).gain(g.range(0.15, 0.6)).place(g.range(-1.0, 1.0), 0.0).reverb(1.5, 0.5, 0.35).ring(0.4).fade(1, 15);
                      }
                      r.add("air").noise(NoiseColor::Pink).bp(1500, 0.6).gain(0.05).fade(600, 900);
                  });

                d("Crickets", Sustained(), "Night crickets: short trains of very high-pitched pulses, repeating at slightly different speeds.",
                  {"cricket", "night", "insect", "ambience"}, {"duration=10", "rate=3", "density=0.5", "width=0.8"},
                  [](Rc r, Cc c) {
                      const double step = 0.012;
                      for (int i = 0; i < 3; ++i) {
                          const double pitch = hz_to_midi(3900.0 + 380.0 * i);
                          std::vector<double> pat = {1, 0, 1, 0, 1, 0};
                          const int rest = static_cast<int>(1.0 / (c.p("rate") * (1.0 + 0.13 * i) * step)) - 6;
                          for (int k = 0; k < std::max(4, rest); ++k) pat.push_back(0.0);
                          r.add("cricket" + std::to_string(i)).osc(Wave::Sine, pitch).gate_sec(pat, step, 1.0, 1.5).gain(0.35 + 0.25 * c.p("density")).place(-0.7 + 0.7 * i, c.p("width") * 0.3).fade(400, 600).variation(static_cast<uint64_t>(i));
                      }
                  });

                d("Frogs", Sustained(), "A small pond at night: a few frogs croaking in short, throaty trains at different pitches.",
                  {"frog", "pond", "night", "ambience"}, {"duration=10", "pitch=50", "rate=5", "width=0.9"},
                  [](Rc r, Cc c) {
                      const double step = 1.0 / (c.p("rate") * 3.0);
                      for (int i = 0; i < 3; ++i) {
                          std::vector<double> pat = {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                          for (int k = 0; k < 6 + 5 * i; ++k) pat.push_back(0.0);
                          r.add("frog" + std::to_string(i)).osc(Wave::Saw, c.p("pitch") + 3.0 * i).fm(Traj(2.0), 1.5).lp(900, 2.0).gate_sec(pat, step * (1.0 + 0.2 * i), 1.0, 4.0)
                              .reverb(0.8, 0.5, 0.2).gain(0.7).place(-0.8 + 0.8 * i, c.p("width") * 0.3).fade(500, 700).variation(static_cast<uint64_t>(i));
                      }
                  });

                d("Rustling Leaves", Sustained(), "A canopy moving in the breeze: soft bursts of dry, high noise that come and go.",
                  {"leaves", "rustle", "forest", "wind"}, {"duration=10", "density=0.5", "brightness=0.6", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("rustle").noise(NoiseColor::White).hp(Traj(c.lg(c.p("brightness"), 2500, 5000))).lp(11000).pulses(Traj(4.0 + 8.0 * c.p("density")), 120, 0.85, 0.7).gain(0.5).place(0, c.p("width")).fade(500, 800);
                      r.add("crinkle").noise(NoiseColor::Dust, Traj(150.0 + 450.0 * c.p("density"))).bp(5500, 1.0).gain(0.6).place(0, c.p("width")).fade(500, 800);
                      r.add("air").noise(NoiseColor::Pink).bp(600, 0.8).filter_lfo(lfo(Mod::Shape::Smooth, 0.2, 0.5)).gain(0.12).fade(500, 800);
                  });
            }
        }
    }
}
