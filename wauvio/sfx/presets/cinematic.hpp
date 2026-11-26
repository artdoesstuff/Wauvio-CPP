#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_cinematic(Registry& R) {
                Def d{R, Category::Cinematic};

                d("Braam", OneShot(), "The big trailer horn blast: a wall of detuned, distorted low brass that swells in and rings out in a huge room.",
                  {"braam", "trailer", "brass", "horn", "cinematic"}, {"duration=3", "tail=3", "pitch=34", "intensity=0.8", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("brass").osc(Wave::Saw, Traj(c.p("pitch"))).unison(8, 18).drive(2.0 + 3.0 * c.p("intensity")).lp(Traj(300).to(0.12, c.lg(c.p("brightness"), 1800, 5000), Interp::Smooth).to(1.0, 400, Interp::Exp), 1.2, 4)
                          .shape(Traj(0.0).to(0.06, 1.0, Interp::Smooth).to(0.4, 0.8).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).reverb(2.5 + c.p("tail"), 0.5, 0.45).gain(0.9).place(0, 0.8).fade(5, 30);
                      r.add("fifth").osc(Wave::Saw, Traj(c.p("pitch") + 7)).unison(5, 16).lp(1200, 1.0, 4).shape(Traj(0.0).to(0.08, 0.6, Interp::Smooth).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).gain(0.4).place(0, 0.8);
                      r.add("sub").osc(Wave::Sine, Traj(c.p("pitch") - 12)).shape(Traj(0.0).to(0.05, 1.0, Interp::Smooth).to(1.0, 0.001, Interp::Exp)).len(c.duration).gain(0.6);
                  });

                d("Horror Stinger", OneShot(), "A sudden shock chord: harsh, screeching strings in a dissonant cluster that cuts off into silence.",
                  {"stinger", "horror", "shock", "scare"}, {"duration=1.4", "tail=1.2", "pitch=82", "intensity=0.8"},
                  [](Rc r, Cc c) {
                      const double ns[] = {0.0, 1.0, 6.0, 7.0, 13.0};
                      for (int i = 0; i < 5; ++i)
                          r.add("s" + std::to_string(i)).osc(Wave::Saw, Traj::linear(c.p("pitch") + ns[i] - 1.0, c.p("pitch") + ns[i])).unison(3, 25).bp(2800, 0.8, 4).drive(2.0 + 2.0 * c.p("intensity"), DriveP::Hard, 0.5).shape(Traj(1.0).to(0.8, 0.8).to(1.0, 0.0)).len(c.duration * 0.75).ring(c.p("tail")).gain(0.35).place(-0.8 + 0.4 * i, 0.3).fade(1, 20);
                      r.add("hit").noise(NoiseColor::White).hp(1500).shape(env_hit(0.002, 0.002)).len(0.3).gain(0.6).fade(0.1, 20).reverb(1.5, 0.5, 0.3);
                      r.add("boom").osc(Wave::Sine, Traj::linear(60, 32)).shape(env_hit(0.003, 0.002)).len(0.9).gain(0.7).fade(0.3, 40);
                  });

                d("Heartbeat", Sustained(), "A heart thumping in the chest: a soft, deep double beat repeating at a steady pulse.",
                  {"heartbeat", "pulse", "tension", "horror"}, {"duration=8", "rate=1.2", "intensity=0.6", "pitch=40"},
                  [](Rc r, Cc c) {
                      const double period = std::max(1.0 / c.p("rate"), c.duration / 30.0); const int beats = std::max(1, static_cast<int>(c.duration / period));
                      for (int i = 0; i < beats; ++i) {
                          r.add("lub" + std::to_string(i)).osc(Wave::Sine, Traj::linear(c.p("pitch") + 12, c.p("pitch"))).lp(250).shape(env_hit(0.02, 0.002)).at(period * i).len(0.16).gain(0.9).fade(2, 25);
                          r.add("dub" + std::to_string(i)).osc(Wave::Sine, Traj::linear(c.p("pitch") + 9, c.p("pitch") - 2)).lp(220).shape(env_hit(0.02, 0.002)).at(period * i + 0.2).len(0.14).gain(0.6 * (0.5 + 0.5 * c.p("intensity"))).fade(2, 25);
                      }
                  });

                d("Dark Drone", Sustained(), "A low, oppressive drone: layers of detuned saws and a sub, slowly breathing as the filter moves.",
                  {"drone", "dark", "tension", "cinematic"}, {"duration=14", "pitch=31", "brightness=0.4", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("saws").osc(Wave::Saw, Traj(c.p("pitch"))).unison(7, 16).lp(Traj(c.lg(c.p("brightness"), 200, 900)), 1.3, 4).filter_lfo(lfo(Mod::Shape::Sine, 0.07, 0.5)).gain(0.8).place(0, c.p("width")).fade(2500, 3000);
                      r.add("fifth").osc(Wave::Saw, Traj(c.p("pitch") + 7)).unison(4, 12).lp(500, 1.0, 4).filter_lfo(lfo(Mod::Shape::Sine, 0.05, 0.4)).gain(0.3).place(0, c.p("width")).fade(2500, 3000);
                      r.add("sub").osc(Wave::Sine, Traj(c.p("pitch") - 12)).gain(0.5).fade(2500, 3000);
                  });

                d("Creepy Whisper", Evolving(), "Barely audible whispering voices: breathy vowel sounds that drift in and out of focus.",
                  {"whisper", "creepy", "voices", "horror"}, {"duration=6", "density=0.5", "width=1", "tail=1"},
                  [](Rc r, Cc c) {
                      const int voices = 2 + static_cast<int>(4.0 * c.p("density"));
                      for (int i = 0; i < voices; ++i) {
                          Rng g = c.rng(20 + static_cast<uint64_t>(i));
                          r.add("w" + std::to_string(i)).formant(Traj(g.range(0.0, 4.0)).to(0.3, g.range(0.0, 4.0), Interp::Smooth).to(0.6, g.range(0.0, 4.0), Interp::Smooth).to(1.0, g.range(0.0, 4.0), Interp::Smooth), Traj(g.range(48.0, 60.0)), 1.0)
                              .gate_sec({1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0}, g.range(0.15, 0.3), 0.9, 60.0).reverb(2.0, 0.5, 0.45).ring(c.p("tail")).gain(0.3).place(g.range(-1.0, 1.0), 0.0).fade(800, 1200).variation(static_cast<uint64_t>(i));
                      }
                  });

                d("Ticking Tension", Sustained(), "A nervous, steady tick with a low pad building underneath: a countdown feeling.",
                  {"ticking", "tension", "clock", "suspense"}, {"duration=8", "rate=2", "pitch=70", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("tick").modal(modes_wood(), Traj(c.p("pitch")), {0.0}).decay_scale(0.1).strike(0.5).pulses(Traj(c.p("rate")), 30, 1.0).len(c.duration).gain(0.9).fade(1, 20).place(0, 0.3);
                      r.add("pad").osc(Wave::Saw, Traj(c.p("pitch") - 34)).unison(4, 14).lp(Traj(300).to(1.0, 1400, Interp::Exp), 1.2, 4).shape(Traj(0.1).to(1.0, 1.0, Interp::Exp)).gain(0.5 * c.p("intensity") + 0.1).place(0, 0.8);
                  });

                d("Deep Rumble", Sustained(), "A sub-bass rumble felt more than heard: low-passed brown noise with a slow, uneven wobble.",
                  {"rumble", "sub", "low", "earthquake"}, {"duration=8", "intensity=0.6", "brightness=0.3", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("rumble").noise(NoiseColor::Brown).lp(Traj(c.lg(c.p("brightness"), 80, 300)), 0.9, 4).trem(Traj(0.7), Traj(0.35 * c.p("intensity"))).gain(1.0).place(0, c.p("width")).fade(800, 1500);
                      r.add("sub").osc(Wave::Sine, Traj(26.0)).trem(Traj(0.4), Traj(0.3)).gain(0.5 * c.p("intensity")).fade(800, 1500);
                  });

                d("Metal Scrape", OneShot(), "Metal dragged over metal: a long, shuddering screech with harsh overtones.",
                  {"scrape", "metal", "screech", "horror"}, {"duration=2.5", "pitch=78", "intensity=0.7", "tail=0.5"},
                  [](Rc r, Cc c) {
                      r.add("screech").osc(Wave::Saw, Traj(c.p("pitch")).to(0.3, c.p("pitch") + 4, Interp::Smooth).to(1.0, c.p("pitch") - 2, Interp::Smooth)).fm(Traj(4.0 * c.p("intensity")), 2.3).ringmod(Traj(180.0).to(1.0, 130.0), 0.5).bp(2600, 1.5, 4).trem(Traj(22.0), Traj(0.5)).shape(env_bell(0.45, 0.3)).ring(c.p("tail")).reverb(1.2, 0.4, 0.25).gain(0.8).fade(10, 80);
                      r.add("grit").noise(NoiseColor::White).bp(3500, 1.0).trem(Traj(28.0), Traj(0.8)).shape(env_bell(0.45, 0.3)).gain(0.15);
                  });

                d("Distant Explosion", OneShot(), "A far-off blast: a muffled thump, a low rolling rumble and a long, thin echo in the sky.",
                  {"explosion", "distant", "blast", "war"}, {"duration=3.5", "tail=3", "intensity=0.7", "brightness=0.3"},
                  [](Rc r, Cc c) {
                      r.add("thump").osc(Wave::Sine, Traj::linear(55, 28)).shape(env_hit(0.01, 0.001)).len(1.2).gain(0.9).fade(2, 40);
                      r.add("rumble").noise(NoiseColor::Brown).lp(Traj::exp(c.lg(c.p("brightness"), 500, 1200), 70), 1.0, 4).shape(env_hit(0.02, 0.001)).len(c.duration).ring(c.p("tail")).reverb(3.5 + c.p("tail"), 0.6, 0.6).gain(0.8 * c.p("intensity") + 0.2).place(0, 0.9).fade(5, 40);
                      r.add("crack").noise(NoiseColor::White).bp(700, 0.8).shape(env_hit(0.005, 0.001)).len(0.2).gain(0.3).reverb(2.5, 0.6, 0.5);
                  });

                d("Choir Swell", Evolving(), "A wordless choir building from nothing: layered, breathy 'ah' voices rising to a big, ringing chord.",
                  {"choir", "swell", "voices", "epic"}, {"duration=5", "tail=2", "pitch=57", "chord=0", "width=1"},
                  [](Rc r, Cc c) {
                      const auto notes = c.chord_notes(); int i = 0;
                      for (double n : notes) {
                          r.add("v" + std::to_string(i)).formant(Traj(1.0), Traj(c.p("pitch") + n), 0.25).spread(Traj(18.0)).shape(env_swell(0.8, 0.02)).len(c.duration).ring(c.p("tail")).reverb(3.0 + c.p("tail"), 0.5, 0.5).gain(0.5).place(-0.8 + 1.6 * i / std::max<size_t>(1, notes.size() - 1), c.p("width")).fade(100, 200).variation(static_cast<uint64_t>(i));
                          ++i;
                      }
                      r.add("air").noise(NoiseColor::Pink).bp(2500, 0.7).shape(env_swell(0.8, 0.02)).len(c.duration).ring(c.p("tail")).gain(0.1).reverb(3.0, 0.5, 0.5);
                  });

                d("Brass Blast", OneShot(), "A single huge, low brass note with a slow, rising swell and a violent edge, in the style of epic trailers.",
                  {"brass", "horn", "blast", "trailer"}, {"duration=2.2", "tail=2", "pitch=38", "intensity=0.8"},
                  [](Rc r, Cc c) {
                      r.add("horn").osc(Wave::Saw, Traj(c.p("pitch"))).unison(6, 12).drive(2.0 + 3.0 * c.p("intensity"), DriveP::Hard, 0.7).bp(Traj(250).to(0.35, 1200, Interp::Smooth).to(1.0, 500, Interp::Smooth), 0.9, 4)
                          .shape(Traj(0.0).to(0.3, 1.0, Interp::Smooth).to(0.8, 0.9).to(1.0, 0.0, Interp::Exp)).len(c.duration).ring(c.p("tail")).reverb(2.5 + c.p("tail"), 0.5, 0.4).gain(0.9).place(0, 0.7).fade(40, 40);
                      r.add("sub").osc(Wave::Sine, Traj(c.p("pitch") - 12)).shape(Traj(0.0).to(0.3, 1.0, Interp::Smooth).to(1.0, 0.0)).len(c.duration).gain(0.5);
                  });

                d("Creepy Music Box", Evolving(), "A music box playing a slow, detuned minor tune that winds down and slows to a stop.",
                  {"music box", "creepy", "lullaby", "horror"}, {"duration=8", "pitch=79", "tail=2", "steps=8"},
                  [](Rc r, Cc c) {
                      const double ns[] = {0, 3, 7, 3, 0, -2, 0, 3};
                      const int n = std::min(8, static_cast<int>(c.p("steps"))); double t = 0.0, gap = 0.45;
                      for (int i = 0; i < n; ++i) {
                          r.add("n" + std::to_string(i)).modal(modes_bell(), Traj(c.p("pitch") + ns[i] - 0.3 * i), {0.0}).decay_scale(0.18).strike(0.4).at(t).len(0.9).gain(0.7).place(0.2, 0.3).reverb(1.8, 0.5, 0.3).ring(i == n - 1 ? c.p("tail") : 0.2);
                          t += gap; gap *= 1.16;
                      }
                  });

                d("Ghost Wail", Evolving(), "A thin, mournful voice-like wail that glides up and falls away with a wide, cold echo.",
                  {"ghost", "wail", "eerie", "voice"}, {"duration=4", "pitch=66", "tail=2.5", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("wail").formant(Traj(2.0).to(0.5, 1.0, Interp::Smooth).to(1.0, 3.0, Interp::Smooth), Traj(c.p("pitch")).to(0.4, c.p("pitch") + 7, Interp::Smooth).to(1.0, c.p("pitch") - 3, Interp::Smooth), 0.35).spread(Traj(14.0)).trem(Traj(5.0), Traj(0.15))
                          .shape(env_bell(0.45, 0.28)).ring(c.p("tail")).echo(0.45, 0.45, 0.4, 0.5).reverb(3.5, 0.6, 0.5).gain(0.8).place(0, c.p("width")).fade(100, 300);
                  });

                d("Sub Hit", OneShot(), "A pure, heavy low hit: a sub-bass thump with a very short click and no tail, for punctuating a cut.",
                  {"sub", "hit", "boom", "punch"}, {"duration=1.2", "pitch=36", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("sub").osc(Wave::Sine, Traj::linear(c.p("pitch") + 10, c.p("pitch") - 4)).drive(1.0 + 2.0 * c.p("intensity")).shape(Traj(1.0).to(0.2, 0.9).to(1.0, 0.001, Interp::Exp)).gain(1.0).fade(0.4, 30);
                      r.add("click").noise(NoiseColor::White).bp(900, 1.0).shape(env_hit(0.001, 0.001)).len(0.03).gain(0.35).fade(0.05, 4);
                  });

                d("String Screech", OneShot(), "A violent high string stab: a harsh, slightly out-of-tune swoop with a rasping bow noise.",
                  {"strings", "screech", "stab", "horror"}, {"duration=1.2", "pitch=88", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("bow").osc(Wave::Saw, Traj::linear(c.p("pitch") - 5, c.p("pitch"))).unison(3, 30).bp(3200, 1.0, 4).drive(2.0 + 2.0 * c.p("intensity")).shape(Traj(1.0).to(0.7, 0.8).to(1.0, 0.0)).gain(0.8).place(0, 0.6).fade(1, 60);
                      r.add("rasp").noise(NoiseColor::White).bp(4500, 1.2).shape(Traj(1.0).to(1.0, 0.0)).gain(0.2).place(0, 0.6);
                  });

                d("Tension Strings", Evolving(), "A cluster of high strings rising slowly with a trembling bow: unease building without a release.",
                  {"strings", "tension", "tremolo", "suspense"}, {"duration=8", "pitch=76", "range=7", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      const double ns[] = {0.0, 1.0, 2.0};
                      for (int i = 0; i < 3; ++i)
                          r.add("s" + std::to_string(i)).osc(Wave::Saw, Traj::linear(c.p("pitch") + ns[i], c.p("pitch") + ns[i] + c.p("range"))).unison(3, 14).lp(4000, 1.0, 4).trem(Traj(9.0 + 3.0 * i), Traj(0.4 + 0.3 * c.p("intensity")))
                              .shape(Traj(0.05).to(1.0, 1.0, Interp::Exp)).gain(0.35).place(-0.6 + 0.6 * i, 0.4).reverb(2.0, 0.5, 0.3).fade(1500, 400).variation(static_cast<uint64_t>(i));
                  });

                d("Impact Whoosh Hit", OneShot(), "A short cinematic accent: a sweeping whoosh that lands on a tight, deep impact.",
                  {"whoosh", "hit", "accent", "transition"}, {"duration=1.2", "tail=1.2", "intensity=0.7", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      const double t = c.duration * 0.45;
                      r.add("whoosh").noise(NoiseColor::White).bp(Traj::exp(300, c.lg(c.p("brightness"), 4000, 9000)), 1.0, 4).shape(env_swell(0.9, 0.03)).len(t).gain(0.7).place(0, 0.9);
                      r.add("hit").osc(Wave::Sine, Traj::linear(80, 34)).drive(2.0).shape(env_hit(0.003, 0.002)).at(t).len(c.duration - t).ring(c.p("tail")).reverb(1.5, 0.5, 0.35).gain(0.9 * c.p("intensity") + 0.1).fade(0.3, 30);
                      r.add("crack").noise(NoiseColor::White).hp(1500).shape(env_hit(0.001, 0.001)).at(t).len(0.1).gain(0.45).fade(0.05, 8);
                  });
            }
        }
    }
}
