#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_magic(Registry& R) {
                Def d{R, Category::Magic};

                d("Spell Cast", OneShot(), "A spell leaving the hands: a bright rising sweep that bursts into a shimmering chord.",
                  {"spell", "cast", "magic", "fantasy"}, {"duration=1.6", "tail=1.2", "pitch=64", "chord=0", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      const double t = c.duration * 0.45;
                      r.add("sweep").osc(Wave::Saw, Traj::exp(c.p("pitch") - 12, c.p("pitch") + 24)).lp(Traj::exp(800, c.lg(c.p("brightness"), 5000, 11000))).shape(env_swell(0.95, 0.05)).len(t).gain(0.5).place(0, 0.6);
                      r.add("burst").additive(Traj(c.p("pitch") + 12), c.chord_notes(), 6, 1.1).spread(Traj(15.0)).shape(env_hit(0.004, 0.002)).at(t).len(c.duration - t).ring(c.p("tail")).reverb(1.8, 0.4, 0.4).gain(0.8).place(0, 0.9).fade(1, 30);
                      r.add("glint").modal(modes_glass(), Traj(c.p("pitch") + 36), c.scattered(8, 3, 0.0, 0.9)).decay_scale(0.2).strike(0.4).at(t).len(c.duration - t).ring(c.p("tail")).gain(0.25).place(0, 1.0);
                  });

                d("Magic Sparkle", OneShot(), "A glitter of tiny, high bells: a rapid scatter of bright pings that thins out.",
                  {"sparkle", "glitter", "magic", "shimmer"}, {"duration=1.4", "tail=1", "pitch=96", "density=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("pings").modal(modes_glass(), Traj(c.p("pitch")), c.scattered(10 + static_cast<int>(30.0 * c.p("density")), 4, 0.0, 0.85)).hit_notes({0.0, 4.0, 7.0, 12.0, -5.0, 9.0}).hit_jitter(0.0, 0.6).decay_scale(0.2).strike(0.3).shape(Traj(1.0).to(1.0, 0.05)).len(c.duration).ring(c.p("tail")).gain(0.8).place(0, c.p("width")).reverb(1.2, 0.4, 0.3);
                  });

                d("Magic Chime", OneShot(), "A single magical chime: a pure bell strike with a soft, glassy shimmer in the ring.",
                  {"chime", "bell", "magic", "tone"}, {"duration=2", "tail=2", "pitch=88", "decay=0.6"},
                  [](Rc r, Cc c) {
                      r.add("bell").modal(modes_bell(), Traj(c.p("pitch")), {0.0}).decay_scale(0.4 + 1.0 * c.p("decay")).strike(0.8).len(c.duration).ring(c.p("tail")).gain(0.9).reverb(1.8, 0.4, 0.4).place(0, 0.5);
                      r.add("shimmer").additive(Traj(c.p("pitch") + 12), {0.0, 7.0}, 4, 1.0).spread(Traj(20.0)).shape(Traj(0.0).to(0.2, 1.0).to(1.0, 0.0, Interp::Exp)).len(c.duration).ring(c.p("tail")).gain(0.15).place(0, 1.0);
                  });

                d("Healing Spell", OneShot(), "A warm restoring wash: a soft, rising swell of sustained notes with a golden shimmer on top.",
                  {"heal", "spell", "restore", "magic"}, {"duration=2.5", "tail=1.5", "pitch=60", "chord=0", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("pad").additive(Traj(c.p("pitch")), c.chord_notes(), 8, 1.3).spread(Traj(10.0)).lp(Traj(c.lg(c.p("brightness"), 2000, 7000))).shape(env_swell(0.55, 0.02)).len(c.duration).ring(c.p("tail")).reverb(2.0 + c.p("tail"), 0.4, 0.4).gain(0.8).place(0, 0.9).fade(100, 100);
                      r.add("sparkle").modal(modes_glass(), Traj(c.p("pitch") + 36), c.scattered(10, 5, 0.1, 0.8)).decay_scale(0.25).strike(0.4).len(c.duration).ring(c.p("tail")).gain(0.12).place(0, 1.0);
                  });

                d("Fireball", OneShot(), "A flaming projectile: a rising roar that streaks past and bursts into a crackling blast.",
                  {"fireball", "fire", "spell", "projectile"}, {"duration=2.2", "tail=1.2", "intensity=0.7", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      const double t = c.duration * 0.55;
                      r.add("whoosh").noise(NoiseColor::Pink).bp(Traj::exp(300, c.lg(c.p("brightness"), 2500, 6000)), 0.8, 4).shape(env_swell(0.9, 0.05)).len(t).gain(0.7).place(-0.4, 0.8);
                      r.add("roar").noise(NoiseColor::Brown).lp(Traj::exp(1500, 200)).shape(env_hit(0.01, 0.001)).at(t).len(c.duration - t).ring(c.p("tail")).reverb(1.5, 0.5, 0.35).gain(0.8 * c.p("intensity") + 0.1).place(0, 1.0).fade(2, 40);
                      r.add("boom").osc(Wave::Sine, Traj::linear(70, 30)).drive(2.0).shape(env_hit(0.003, 0.002)).at(t).len(0.6).gain(0.8).fade(0.3, 30);
                      r.add("crackle").noise(NoiseColor::Dust, Traj(250.0).to(1.0, 20.0)).hp(2000).drive(2.0, DriveP::Hard, 0.5).shape(Traj(1.0).to(1.0, 0.001, Interp::Exp)).at(t).len(c.duration - t).ring(c.p("tail")).gain(0.45).place(0, 1.0);
                  });

                d("Ice Crystal", OneShot(), "Ice forming: a sharp glassy crackle that rises into a cold, ringing crystalline chord.",
                  {"ice", "crystal", "frost", "spell"}, {"duration=1.8", "tail=1.5", "pitch=88", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("crack").noise(NoiseColor::Dust, Traj(150.0).to(0.5, 600.0).to(1.0, 40.0)).hp(Traj(c.lg(c.p("brightness"), 3000, 6000))).shape(env_bell(0.3, 0.35)).len(c.duration).gain(0.5).place(0, 1.0);
                      r.add("ring").modal(modes_glass(), Traj(c.p("pitch")), {0.0, 0.1, 0.22}).hit_notes({0.0, 7.0, 12.0}).decay_scale(0.7).strike(0.3).len(c.duration).ring(c.p("tail")).gain(0.8).place(0, 0.8).reverb(1.6, 0.3, 0.4);
                      r.add("air").noise(NoiseColor::White).hp(6000).shape(env_bell(0.3, 0.35)).len(c.duration).gain(0.08);
                  });

                d("Dark Curse", OneShot(), "A sinister hex: a low, groaning cluster with a rasping whisper and a descending, dissonant slide.",
                  {"curse", "dark", "hex", "spell"}, {"duration=3", "tail=2", "pitch=46", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("groan").osc(Wave::Saw, Traj::linear(c.p("pitch") + 3, c.p("pitch") - 5)).unison(5, 28).lp(Traj(900).to(1.0, 200, Interp::Exp), 1.2, 4).drive(2.0 + 2.0 * c.p("intensity")).shape(env_swell(0.3, 0.1)).len(c.duration).ring(c.p("tail")).reverb(2.5, 0.5, 0.4).gain(0.8).place(0, 0.8);
                      r.add("tritone").osc(Wave::Saw, Traj::linear(c.p("pitch") + 9, c.p("pitch") + 1)).unison(3, 24).lp(700, 1.0, 4).shape(env_swell(0.3, 0.1)).len(c.duration).ring(c.p("tail")).gain(0.4);
                      r.add("whisper").formant(Traj(3.0).to(0.5, 1.0, Interp::Smooth).to(1.0, 2.0, Interp::Smooth), Traj(c.p("pitch") + 12), 1.0).gate_sec({1, 0, 1, 1, 0, 1, 0, 0}, 0.2, 0.8, 50.0).reverb(2.0, 0.5, 0.5).gain(0.2).place(0.5, 1.0);
                  });

                d("Summoning", Evolving(), "A ritual reaching its peak: a rising choir-like swell, a rumbling drone and a final deep impact.",
                  {"summon", "ritual", "magic", "epic"}, {"duration=5", "tail=2.5", "pitch=48", "chord=0", "intensity=0.8"},
                  [](Rc r, Cc c) {
                      const double t = c.duration * 0.85;
                      r.add("drone").osc(Wave::Saw, Traj(c.p("pitch") - 12)).unison(5, 15).lp(Traj::exp(250, 1500), 1.2, 4).shape(env_swell(0.97, 0.05)).len(t).gain(0.7).place(0, 0.8);
                      r.add("choir").additive(Traj(c.p("pitch") + 12), c.chord_notes(), 6, 1.2).spread(Traj(20.0)).shape(env_swell(0.97, 0.02)).len(t).gain(0.5).place(0, 1.0).reverb(2.5, 0.5, 0.4);
                      r.add("impact").osc(Wave::Sine, Traj::linear(70, 28)).drive(2.0).shape(env_hit(0.003, 0.001)).at(t).len(c.duration - t + 0.8).gain(0.9 * c.p("intensity") + 0.1).fade(0.3, 40);
                      r.add("wash").noise(NoiseColor::Pink).lp(Traj::exp(5000, 300)).shape(env_hit(0.003, 0.001)).at(t).len(c.duration - t + 0.2).ring(c.p("tail")).reverb(2.5 + c.p("tail"), 0.5, 0.5).gain(0.5).place(0, 1.0);
                  });

                d("Portal", Evolving(), "A dimensional gateway: a swirling, ring-modulated tone that spirals upward through a rush of wind.",
                  {"portal", "gateway", "magic", "swirl"}, {"duration=4", "pitch=57", "rate=6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("swirl").osc(Wave::Sine, Traj(c.p("pitch")).to(1.0, c.p("pitch") + 19, Interp::Exp)).fm(Traj(3.0), 2.0).ringmod(Traj(c.p("rate") * 12.0), 0.6).trem(Traj(c.p("rate")), Traj(0.4)).shape(env_swell(0.8, 0.1)).gain(0.7).place(0, c.p("width")).reverb(1.5, 0.4, 0.3);
                      r.add("wind").noise(NoiseColor::Pink).bp(Traj(600).to(0.5, 2200, Interp::Smooth).to(1.0, 900, Interp::Smooth), 1.5, 4).filter_lfo(lfo(Mod::Shape::Sine, c.p("rate") * 0.5, 0.5)).shape(env_swell(0.8, 0.1)).gain(0.35).place(0, c.p("width"));
                      r.add("sub").osc(Wave::Sine, Traj(c.p("pitch") - 24)).shape(env_swell(0.8, 0.1)).gain(0.35);
                  });

                d("Fairy Dust", Evolving(), "A drifting glitter: soft, high, bell-like grains scattered randomly across the stereo field.",
                  {"fairy", "dust", "sparkle", "magic"}, {"duration=4", "density=0.6", "pitch=90", "width=1", "tail=1"},
                  [](Rc r, Cc c) {
                      r.add("grains").grain(GrainSource::Bell, Traj(8.0 + 40.0 * c.p("density")), Traj(90.0), Traj(c.p("pitch"))).scatter(7.0, 0.0).shape(env_bell(0.45, 0.35)).len(c.duration).ring(c.p("tail")).reverb(1.4, 0.4, 0.35).gain(0.7).place(0, c.p("width"));
                      r.add("air").noise(NoiseColor::White).hp(7000).shape(env_bell(0.45, 0.35)).gain(0.04).place(0, c.p("width"));
                  });

                d("Enchant", OneShot(), "An item being imbued: a quick upward arpeggio of glassy plucks ending in a held, glowing note.",
                  {"enchant", "imbue", "arpeggio", "magic"}, {"duration=1.6", "tail=1", "pitch=72", "steps=6", "scale=1"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const auto walk = c.scale_walk(n, 0, 2);
                      for (int i = 0; i < n; ++i) r.add("p" + std::to_string(i)).pluck(Traj(c.p("pitch") + walk[static_cast<size_t>(i)]), {0.0}, 0.3).at(i * 0.08).len(0.5).gain(0.6).place(-0.6 + 1.2 * i / std::max(1, n - 1), 0.2).reverb(1.0, 0.4, 0.3);
                      r.add("glow").additive(Traj(c.p("pitch") + walk.back()), {0.0, 12.0}, 6, 1.0).spread(Traj(10.0)).shape(Traj(0.0).to(0.25, 1.0, Interp::Smooth).to(1.0, 0.0, Interp::Exp)).at(n * 0.08).len(c.duration - n * 0.08).ring(c.p("tail")).reverb(1.8, 0.4, 0.4).gain(0.5).place(0, 0.9);
                  });

                d("Lightning Bolt", OneShot(), "A magical lightning strike: a hard electric crack with crackling arcs and a thunderous low boom.",
                  {"lightning", "bolt", "electric", "spell"}, {"duration=1.8", "tail=1.2", "intensity=0.8", "brightness=0.8"},
                  [](Rc r, Cc c) {
                      r.add("crack").noise(NoiseColor::White).hp(1500).drive(3.0, DriveP::Hard).shape(env_hit(0.0008, 0.001)).len(0.2).gain(0.9).fade(0.05, 20);
                      r.add("arcs").noise(NoiseColor::Dust, Traj(500.0).to(1.0, 40.0)).bp(Traj(c.lg(c.p("brightness"), 3000, 7000)), 1.2).crush(Traj(6.0), Traj(2.0)).shape(Traj(1.0).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).gain(0.5 * c.p("intensity") + 0.1).place(0, 1.0);
                      r.add("boom").osc(Wave::Sine, Traj::linear(90, 30)).drive(2.0).shape(env_hit(0.004, 0.001)).len(c.duration * 0.7).gain(0.8).fade(0.3, 40);
                      r.add("zap").osc(Wave::Saw, Traj::linear(100, 55)).lp(8000).shape(env_hit(0.003, 0.003)).len(0.15).gain(0.4).fade(0.1, 10);
                  });

                d("Wand Flick", OneShot(), "A quick flourish with a wand: a thin whoosh with a bright little ping at the end.",
                  {"wand", "flick", "whoosh", "magic"}, {"duration=0.5", "pitch=92", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("whoosh").noise(NoiseColor::White).bp(Traj::exp(1500, c.lg(c.p("brightness"), 5000, 9000)), 2.0, 4).shape(env_bell(0.5, 0.22)).len(c.duration * 0.7).gain(0.6).place(-0.4, 0.5).fade(2, 20);
                      r.add("ping").modal(modes_glass(), Traj(c.p("pitch")), {0.0}).decay_scale(0.4).strike(0.4).at(c.duration * 0.55).len(c.duration * 0.6).gain(0.6).reverb(0.8, 0.4, 0.25);
                  });

                d("Holy Swell", Evolving(), "A radiant, angelic rise: a choir-like chord building slowly with a high, glowing shimmer.",
                  {"holy", "angelic", "swell", "choir"}, {"duration=5", "tail=3", "pitch=60", "chord=0", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("choir").formant(Traj(1.0), Traj(c.p("pitch")), 0.2).spread(Traj(14.0)).shape(env_swell(0.8, 0.02)).len(c.duration).ring(c.p("tail")).reverb(3.0 + c.p("tail"), 0.4, 0.5).gain(0.5).place(-0.5, 0.8);
                      r.add("choir2").formant(Traj(1.0), Traj(c.p("pitch") + 7), 0.2).spread(Traj(14.0)).shape(env_swell(0.8, 0.02)).len(c.duration).ring(c.p("tail")).reverb(3.0 + c.p("tail"), 0.4, 0.5).gain(0.45).place(0.5, 0.8).variation(2);
                      r.add("glow").additive(Traj(c.p("pitch") + 24), c.chord_notes(), 6, 1.2).lp(Traj(c.lg(c.p("brightness"), 4000, 12000))).shape(env_swell(0.8, 0.02)).len(c.duration).ring(c.p("tail")).reverb(3.0, 0.4, 0.5).gain(0.3).place(0, 1.0);
                  });

                d("Treasure Chest", OneShot(), "A chest being opened: a creaking lid, a burst of jingling coins and a bright, rewarding sparkle.",
                  {"treasure", "chest", "loot", "reward"}, {"duration=2.2", "tail=1.2", "pitch=88", "density=0.6"},
                  [](Rc r, Cc c) {
                      r.add("creak").osc(Wave::Saw, Traj(55.0).to(1.0, 66.0, Interp::Smooth)).fm(Traj(2.0), 1.7).bp(Traj(700).to(1.0, 1100, Interp::Smooth), 5.0, 4).shape(env_bell(0.5, 0.35)).len(0.7).gain(0.3).fade(10, 60);
                      r.add("coins").modal(modes_metal(), Traj(c.p("pitch")), c.scattered(8 + static_cast<int>(20.0 * c.p("density")), 6, 0.0, 0.9)).hit_notes({0.0, 4.0, -3.0, 7.0, -7.0}).hit_jitter(0.0, 0.7).decay_scale(0.12).strike(0.4).at(0.6).len(c.duration - 0.6).gain(0.7).place(0, 0.8);
                      r.add("sparkle").modal(modes_glass(), Traj(c.p("pitch") + 12), {0.0, 0.1, 0.2, 0.32}).hit_notes({0.0, 4.0, 7.0, 12.0}).decay_scale(0.5).strike(0.3).at(0.7).len(c.duration - 0.7).ring(c.p("tail")).gain(0.5).reverb(1.4, 0.4, 0.35);
                  });

                d("Level Shimmer", OneShot(), "A short, bright upward glissando of tiny harps and bells with a satisfying shimmer at the top.",
                  {"shimmer", "glissando", "harp", "reward"}, {"duration=1.4", "tail=1", "pitch=72", "steps=10", "scale=1"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const auto walk = c.scale_walk(n);
                      for (int i = 0; i < n; ++i) r.add("h" + std::to_string(i)).pluck(Traj(c.p("pitch") + walk[static_cast<size_t>(i)]), {0.0}, 0.25).at(i * 0.05).len(0.45).gain(0.55).place(-0.5 + i * 1.0 / std::max(1, n - 1), 0.2).reverb(1.0, 0.4, 0.3);
                      r.add("top").modal(modes_bell(), Traj(c.p("pitch") + walk.back() + 12), {0.0}).decay_scale(0.5).strike(0.5).at(n * 0.05).len(c.duration - n * 0.05).ring(c.p("tail")).gain(0.6).reverb(1.4, 0.4, 0.3);
                  });
            }
        }
    }
}
