#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_ambience(Registry& R) {
                Def d{R, Category::Ambience};

                d("Room Tone", Sustained(), "The sound of a quiet room: a faint wash of air and a barely-there low hum.",
                  {"room", "tone", "quiet", "background"}, {"duration=12", "brightness=0.4", "width=0.5"},
                  [](Rc r, Cc c) {
                      r.add("air").noise(NoiseColor::Pink).lp(Traj(c.lg(c.p("brightness"), 500, 2500))).hp(60).gain(0.5).place(0, c.p("width")).fade(1000, 1000);
                      r.add("hum").additive(Traj(31.0), {0.0}, 3, 1.5).gain(0.1).fade(1000, 1000);
                  });

                d("Office Hum", Sustained(), "Fluorescent lights and ventilation: a mains hum with its harmonics, over a steady air-conditioning wash.",
                  {"office", "hum", "mains", "interior"}, {"duration=12", "brightness=0.5", "intensity=0.4"},
                  [](Rc r, Cc c) {
                      r.add("mains").additive(Traj(hz_to_midi(100.0)), {0.0}, 12, 0.9).lp(Traj(c.lg(c.p("brightness"), 1200, 4500))).gain(0.1 + 0.15 * c.p("intensity")).fade(1000, 1000);
                      r.add("vent").noise(NoiseColor::Pink).bp(700, 0.7).gain(0.35).place(0, 0.6).fade(1000, 1000);
                  });

                d("Cafe Chatter", Sustained(), "A busy cafe: a crowd of overlapping, whispered voices, with the odd clink of crockery.",
                  {"cafe", "crowd", "murmur", "voices"}, {"duration=14", "density=0.6", "brightness=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      const int voices = 4 + static_cast<int>(8.0 * c.p("density"));
                      for (int i = 0; i < voices; ++i) {
                          Rng g = c.rng(10 + static_cast<uint64_t>(i));
                          r.add("voice" + std::to_string(i)).formant(Traj(g.range(0.0, 4.0)).to(0.25, g.range(0.0, 4.0), Interp::Smooth).to(0.5, g.range(0.0, 4.0), Interp::Smooth).to(0.75, g.range(0.0, 4.0), Interp::Smooth).to(1.0, g.range(0.0, 4.0), Interp::Smooth), Traj(g.range(46.0, 62.0)), 0.9)
                              .gate_sec({1, 1, 0, 1, 1, 1, 0, 0, 1, 0}, g.range(0.14, 0.26), 0.9, 40.0).lp(Traj(c.lg(c.p("brightness"), 2500, 5500))).gain(0.25).place(g.range(-1.0, 1.0), 0.0).fade(800, 1000).variation(static_cast<uint64_t>(i));
                      }
                      r.add("clinks").modal(modes_glass(), Traj(96.0), {0.1, 0.34, 0.6, 0.83}).hit_jitter(0.1, 0.5).decay_scale(0.25).gain(0.08).place(0.3, 0.8).reverb(0.8, 0.5, 0.2);
                      r.add("room").noise(NoiseColor::Pink).lp(900).gain(0.15).fade(800, 1000);
                  });

                d("City Traffic", Sustained(), "Distant city traffic: a low, rolling wash of engines with the occasional pass-by rising and falling.",
                  {"city", "traffic", "street", "urban"}, {"duration=14", "density=0.5", "brightness=0.4", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("bed").noise(NoiseColor::Brown).lp(Traj(c.lg(c.p("brightness"), 350, 900))).gain(0.7).place(0, c.p("width")).fade(1000, 1200);
                      r.add("mid").noise(NoiseColor::Pink).bp(600, 0.5).trem(Traj(0.13), Traj(0.4)).gain(0.25).place(0, c.p("width")).fade(1000, 1200).variation(3);
                      Rng g = c.rng(7); const int n = 2 + static_cast<int>(5.0 * c.p("density"));
                      for (int i = 0; i < n; ++i)
                          r.add("car" + std::to_string(i)).noise(NoiseColor::Pink).bp(Traj(500).to(0.5, 900, Interp::Smooth).to(1.0, 450, Interp::Smooth), 1.0).shape(env_bell(0.5, 0.2)).at(g.range(0.0, std::max(0.0, c.duration - 2.5))).len(2.5)
                              .gain(0.35).place(g.range(-0.9, 0.9), 0.3);
                  });

                d("Space Station", Sustained(), "The interior of a spacecraft: a deep machine drone, slow breathing air and distant electrical ticks.",
                  {"space", "station", "drone", "scifi", "interior"}, {"duration=14", "pitch=33", "brightness=0.4", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("drone").osc(Wave::Saw, Traj(c.p("pitch"))).unison(5, 9).lp(Traj(c.lg(c.p("brightness"), 250, 900)), 1.2, 4).filter_lfo(lfo(Mod::Shape::Sine, 0.08, 0.3)).gain(0.6).place(0, c.p("width")).fade(1500, 1500);
                      r.add("air").noise(NoiseColor::Pink).bp(Traj(450), 1.0).trem(Traj(0.2), Traj(0.5)).gain(0.25).place(0, c.p("width")).fade(1500, 1500);
                      r.add("ticks").noise(NoiseColor::Dust, Traj(1.2)).bp(3500, 3.0).gain(0.35).place(0.6, 0.6).reverb(1.2, 0.5, 0.3).fade(1500, 1500);
                  });

                d("Cave Drips", Sustained(), "Water dripping in a large cave: single drops, each ringing out in a long, dark echo.",
                  {"cave", "drip", "water", "echo"}, {"duration=12", "density=0.4", "pitch=84", "width=0.9", "tail=2"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(4); const int n = 3 + static_cast<int>(10.0 * c.p("density"));
                      for (int i = 0; i < n; ++i) {
                          const double p = c.p("pitch") + g.range(-9.0, 9.0);
                          r.add("drip" + std::to_string(i)).osc(Wave::Sine, Traj(p).to(1.0, p + 9.0, Interp::Exp)).shape(env_hit(0.02, 0.002)).at(g.range(0.0, std::max(0.0, c.duration - 0.4))).len(0.12).gain(g.range(0.4, 1.0))
                              .place(g.range(-0.8, 0.8), 0.0).echo(g.range(0.2, 0.4), 0.45, 0.3, 0.5).reverb(3.5, 0.6, 0.5).ring(c.p("tail")).fade(0.5, 20);
                      }
                      r.add("air").noise(NoiseColor::Brown).lp(250).gain(0.12).fade(1000, 1200);
                  });

                d("Underwater", Sustained(), "Submerged: a muffled, pressurised hush with soft bubbles drifting upward.",
                  {"underwater", "submerged", "bubbles", "muffled"}, {"duration=12", "density=0.4", "brightness=0.3", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("hush").noise(NoiseColor::Brown).lp(Traj(c.lg(c.p("brightness"), 200, 600)), 0.9, 4).trem(Traj(0.15), Traj(0.3)).gain(0.9).place(0, c.p("width")).fade(1000, 1200);
                      r.add("hiss").noise(NoiseColor::Pink).lp(900).gain(0.2).fade(1000, 1200);
                      Rng g = c.rng(5); const int n = 4 + static_cast<int>(12.0 * c.p("density"));
                      for (int i = 0; i < n; ++i) {
                          const double p = g.range(64.0, 80.0);
                          r.add("bubble" + std::to_string(i)).osc(Wave::Sine, Traj(p).to(1.0, p + 10.0, Interp::Exp)).shape(env_hit(0.02, 0.002)).at(g.range(0.0, std::max(0.0, c.duration - 0.3))).len(0.1).gain(g.range(0.2, 0.6))
                              .place(g.range(-0.8, 0.8), 0.0).lp(1800).fade(0.5, 15);
                      }
                  });

                d("Refrigerator Hum", Sustained(), "A kitchen appliance: a steady low hum with a slight flutter and a faint rattle in its housing.",
                  {"hum", "appliance", "kitchen", "household"}, {"duration=12", "intensity=0.4", "brightness=0.4"},
                  [](Rc r, Cc c) {
                      r.add("hum").additive(Traj(hz_to_midi(60.0)), {0.0}, 8, 1.2).lp(Traj(c.lg(c.p("brightness"), 500, 1800))).trem(Traj(0.9), Traj(0.06)).gain(0.5).fade(800, 900);
                      r.add("rattle").noise(NoiseColor::Pink).bp(250, 3.0).trem(Traj(11.0), Traj(0.5)).gain(0.1 + 0.15 * c.p("intensity")).fade(800, 900);
                  });

                d("Vinyl Crackle", Sustained(), "A record's surface noise: random clicks and pops over a faint, steady hiss, ready to lay under music.",
                  {"vinyl", "crackle", "lofi", "noise"}, {"duration=12", "density=0.5", "brightness=0.6", "intensity=0.5"},
                  [](Rc r, Cc c) {
                      r.add("clicks").noise(NoiseColor::Dust, Traj(8.0 + 50.0 * c.p("density"))).hp(900).lp(Traj(c.lg(c.p("brightness"), 4000, 12000))).gain(0.7 + 0.3 * c.p("intensity")).place(0, 0.4).fade(300, 500);
                      r.add("pops").noise(NoiseColor::Dust, Traj(0.6 + 2.0 * c.p("density"))).bp(1800, 0.9).drive(2.0, DriveP::Hard, 0.6).gain(0.9).place(0, 0.6).fade(300, 500).variation(2);
                      r.add("hiss").noise(NoiseColor::White).hp(2500).lp(9000).gain(0.05 + 0.05 * c.p("intensity")).fade(300, 500);
                      r.add("rumble").noise(NoiseColor::Brown).lp(90).gain(0.1).fade(300, 500);
                  });

                d("Tape Hiss", Sustained(), "The noise floor of an analogue tape: a smooth high hiss with a slow, uneven wobble.",
                  {"tape", "hiss", "noise", "analog"}, {"duration=12", "brightness=0.6", "depth=0.3"},
                  [](Rc r, Cc c) {
                      r.add("hiss").noise(NoiseColor::White).hp(1800).lp(Traj(c.lg(c.p("brightness"), 5000, 13000))).trem(Traj(0.35), Traj(0.15 * c.p("depth"))).gain(0.6).place(0, 0.5).fade(300, 500);
                      r.add("floor").noise(NoiseColor::Pink).lp(600).gain(0.05).fade(300, 500);
                  });

                d("Haunted House", Evolving(), "An empty, creaking old house: a deep drone, settling groans and a distant, formless whisper.",
                  {"haunted", "creepy", "horror", "ambience"}, {"duration=14", "intensity=0.6", "brightness=0.4", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("drone").osc(Wave::Saw, Traj(30.0)).unison(4, 22).lp(Traj(c.lg(c.p("brightness"), 180, 600)), 1.2, 4).gain(0.5).place(0, c.p("width")).fade(2000, 2000);
                      r.add("groan").osc(Wave::Saw, Traj(40.0).to(0.3, 42.0, Interp::Smooth).to(0.6, 38.0, Interp::Smooth).to(1.0, 41.0, Interp::Smooth)).bp(Traj(500).to(0.5, 380, Interp::Smooth).to(1.0, 520, Interp::Smooth), 5.0, 4)
                          .gain(0.25 * c.p("intensity") + 0.1).trem(Traj(0.25), Traj(0.7)).reverb(2.0, 0.5, 0.4).place(-0.5, c.p("width")).fade(2000, 2000);
                      r.add("whisper").formant(Traj(1.0).to(0.5, 3.0, Interp::Smooth).to(1.0, 0.0, Interp::Smooth), Traj(52.0), 1.0).lp(3500).gate_sec({0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0}, 0.6, 0.9, 150.0).reverb(3.0, 0.6, 0.6).gain(0.18).place(0.6, 1.0).fade(2000, 2000);
                  });

                d("Night Forest", Sustained(), "A forest after dark: a soft breeze, slow crickets and the odd far-off creak of wood.",
                  {"forest", "night", "ambience", "outdoor"}, {"duration=14", "density=0.4", "brightness=0.4", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("breeze").noise(NoiseColor::Pink).bp(Traj(c.lg(c.p("brightness"), 300, 800)), 1.2, 4).filter_lfo(lfo(Mod::Shape::Smooth, 0.12, 0.8)).gain(0.45).place(0, c.p("width")).fade(1500, 1500);
                      std::vector<double> pat = {1, 0, 1, 0, 1, 0};
                      for (int k = 0; k < 36; ++k) pat.push_back(0.0);
                      r.add("cricket").osc(Wave::Sine, Traj(hz_to_midi(4200.0))).gate_sec(pat, 0.012, 1.0, 1.5).gain(0.18 + 0.2 * c.p("density")).place(-0.6, 0.3).fade(1500, 1500);
                      r.add("cricket2").osc(Wave::Sine, Traj(hz_to_midi(3700.0))).gate_sec(pat, 0.0135, 1.0, 1.5).gain(0.14 + 0.15 * c.p("density")).place(0.6, 0.3).fade(1500, 1500);
                      r.add("creak").noise(NoiseColor::Pink).bp(Traj(300).to(0.5, 520, Interp::Smooth).to(1.0, 280, Interp::Smooth), 7.0, 4).shape(env_bell(0.5, 0.18)).at(c.duration * 0.55).len(1.2).reverb(2.0, 0.5, 0.5).gain(0.15).place(0.4, 0.5);
                  });
            }
        }
    }
}
