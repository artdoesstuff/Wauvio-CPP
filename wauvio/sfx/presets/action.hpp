#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            // The shared layers of a gunshot: a sharp crack, a body thump and a room tail. Used by several weapons with different sizes.
            inline void gun_core(Recipe& r, const BuildCtx& c, double crack_hz, double thump_hz, double body_len, double tail_gain) {
                r.add("crack").noise(NoiseColor::White).hp(crack_hz).drive(2.0, DriveP::Hard, 0.5).shape(env_hit(0.0008, 0.001)).len(0.12).gain(0.9).fade(0.05, 12);
                r.add("thump").osc(Wave::Sine, Traj::linear(thump_hz * 2.0, thump_hz)).drive(1.5).shape(env_hit(0.002, 0.002)).len(body_len).gain(0.8 * c.p("body")).fade(0.3, 20);
                r.add("tail").noise(NoiseColor::Pink).lp(Traj::exp(5000, 300), 0.8, 4).shape(env_hit(0.002, 0.001)).len(c.duration).ring(c.p("tail")).reverb(0.8 + c.p("tail"), 0.5, 0.5).gain(tail_gain).place(0, 1.0).fade(0.2, 40);
            }

            inline void register_action(Registry& R) {
                Def d{R, Category::Action};

                d("Pistol Shot", OneShot(), "A handgun: a sharp crack, a light thump and a short bark of room reflections.",
                  {"gun", "pistol", "shot", "weapon"}, {"duration=0.6", "tail=0.6", "body=0.6"},
                  [](Rc r, Cc c) { gun_core(r, c, 2500, 110, 0.12, 0.45); });

                d("Rifle Shot", OneShot(), "A rifle: a harder crack, a deeper punch and a long echo rolling off into the distance.",
                  {"gun", "rifle", "shot", "weapon"}, {"duration=1", "tail=1.5", "body=0.8"},
                  [](Rc r, Cc c) { gun_core(r, c, 1800, 85, 0.2, 0.55); r.add("echo").noise(NoiseColor::Pink).bp(900, 0.7).shape(env_hit(0.01, 0.001)).at(0.18).len(c.duration).ring(c.p("tail")).echo(0.28, 0.5, 0.4, 0.5).gain(0.15).place(0, 1.0); });

                d("Shotgun Blast", OneShot(), "A big, wide boom: a thick burst of noise and a heavy low thump with a blooming tail.",
                  {"gun", "shotgun", "blast", "weapon"}, {"duration=1.2", "tail=1.4", "body=1"},
                  [](Rc r, Cc c) {
                      gun_core(r, c, 900, 65, 0.28, 0.6);
                      r.add("blast").noise(NoiseColor::Pink).lp(Traj::exp(6000, 400), 0.8, 4).drive(2.5, DriveP::Hard, 0.5).shape(env_hit(0.002, 0.002)).len(0.35).gain(0.8).place(0, 1.0).fade(0.1, 40);
                  });

                d("Explosion", OneShot(), "A solid blast: a bright crack, a huge low thump, a rolling roar and debris pattering down.",
                  {"explosion", "blast", "boom", "bomb"}, {"duration=3", "tail=2.5", "intensity=0.8", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("crack").noise(NoiseColor::White).hp(800).shape(env_hit(0.001, 0.001)).len(0.25).gain(0.8).fade(0.05, 20);
                      r.add("boom").osc(Wave::Sine, Traj::linear(70, 26)).drive(1.5 + 2.0 * c.p("intensity")).shape(env_hit(0.003, 0.001)).len(c.duration * 0.7).gain(0.9).fade(0.3, 50);
                      r.add("roar").noise(NoiseColor::Brown).lp(Traj::exp(c.lg(c.p("brightness"), 2500, 6000), 150), 1.0, 4).shape(env_hit(0.01, 0.001)).len(c.duration).ring(c.p("tail")).reverb(2.0 + c.p("tail"), 0.5, 0.5).gain(0.85).place(0, 1.0).fade(3, 50);
                      r.add("debris").noise(NoiseColor::Dust, Traj(500.0).to(1.0, 30.0)).bp(3200, 0.8).shape(Traj(0.0).to(0.2, 1.0).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).gain(0.3).place(0, 1.0);
                  });

                d("Big Explosion", OneShot(), "A massive detonation: a long, deep, shuddering blast that shakes the whole sound field.",
                  {"explosion", "huge", "blast", "nuke"}, {"duration=5", "tail=4", "intensity=0.9", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("crack").noise(NoiseColor::White).hp(600).shape(env_hit(0.001, 0.001)).len(0.3).gain(0.7).fade(0.05, 25);
                      r.add("boom").osc(Wave::Sine, Traj::linear(60, 22)).drive(2.0 + 2.0 * c.p("intensity")).shape(env_hit(0.004, 0.001)).len(c.duration * 0.8).gain(1.0).fade(0.3, 80);
                      r.add("roar").noise(NoiseColor::Brown).lp(Traj::exp(c.lg(c.p("brightness"), 2000, 5000), 100), 1.1, 4).trem(Traj(7.0), Traj(0.15)).shape(env_hit(0.02, 0.001)).len(c.duration).ring(c.p("tail")).reverb(3.5 + c.p("tail"), 0.6, 0.55).gain(0.9).place(0, 1.0).fade(3, 80);
                      r.add("rubble").noise(NoiseColor::Dust, Traj(800.0).to(1.0, 20.0)).bp(2500, 0.7).shape(Traj(0.0).to(0.15, 1.0).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).gain(0.3).place(0, 1.0);
                  });

                d("Sword Swing", OneShot(), "A blade cutting the air: a fast, narrow whoosh that sweeps up and away.",
                  {"sword", "swing", "whoosh", "melee"}, {"duration=0.4", "brightness=0.6", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("whoosh").noise(NoiseColor::White).bp(Traj(800.0).to(0.6, c.lg(c.p("brightness"), 3500, 7000), Interp::Smooth).to(1.0, 1500, Interp::Smooth), 2.0 + 3.0 * c.p("intensity"), 4).shape(env_bell(0.55, 0.22)).gain(0.9).place(-0.3, 0.6).fade(2, 30);
                  });

                d("Sword Clash", OneShot(), "Two blades meeting: a bright, ringing strike of metal on metal with a quick scraping slide.",
                  {"sword", "clash", "metal", "melee"}, {"duration=1.2", "tail=0.8", "pitch=84", "decay=0.5"},
                  [](Rc r, Cc c) {
                      r.add("ring").modal(modes_metal(), Traj(c.p("pitch")), {0.0}).decay_scale(0.25 + 0.8 * c.p("decay")).strike(0.6).len(c.duration).ring(c.p("tail")).gain(0.9).reverb(0.6, 0.5, 0.15);
                      r.add("ring2").modal(modes_metal(), Traj(c.p("pitch") - 5), {0.0}).decay_scale(0.2 + 0.6 * c.p("decay")).strike(0.6).at(0.003).len(c.duration).ring(c.p("tail")).gain(0.5).place(0.3, 0.0);
                      r.add("scrape").noise(NoiseColor::White).bp(Traj(3000).to(1.0, 6000, Interp::Smooth), 2.0).shape(env_hit(0.02, 0.002)).at(0.03).len(0.25).gain(0.25);
                      r.add("click").noise(NoiseColor::White).hp(3000).shape(env_hit(0.001, 0.001)).len(0.02).gain(0.6).fade(0.05, 3);
                  });

                d("Punch", OneShot(), "A fist landing: a dull, meaty thud with a short body slap on top.",
                  {"punch", "hit", "impact", "fight"}, {"duration=0.3", "intensity=0.7", "pitch=48"},
                  [](Rc r, Cc c) {
                      r.add("thud").osc(Wave::Sine, Traj::linear(c.p("pitch") + 12, c.p("pitch") - 4)).drive(1.5 + 2.0 * c.p("intensity")).shape(env_hit(0.01, 0.002)).len(0.14).gain(0.9).fade(0.3, 15);
                      r.add("slap").noise(NoiseColor::Pink).bp(1400, 0.9).shape(env_hit(0.01, 0.002)).len(0.07).gain(0.5).fade(0.2, 8);
                  });

                d("Kick Hit", OneShot(), "A heavy kick or body blow: a deeper thud with a harder slap and a hint of grunt-like weight.",
                  {"kick", "hit", "impact", "fight"}, {"duration=0.4", "intensity=0.8", "pitch=44"},
                  [](Rc r, Cc c) {
                      r.add("thud").osc(Wave::Sine, Traj::linear(c.p("pitch") + 14, c.p("pitch") - 6)).drive(2.0 + 2.0 * c.p("intensity")).shape(env_hit(0.01, 0.002)).len(0.2).gain(0.95).fade(0.3, 20);
                      r.add("slap").noise(NoiseColor::Pink).bp(1000, 0.8).shape(env_hit(0.01, 0.002)).len(0.1).gain(0.55).fade(0.2, 10);
                      r.add("crack").noise(NoiseColor::White).hp(2500).shape(env_hit(0.001, 0.001)).len(0.02).gain(0.3).fade(0.05, 3);
                  });

                d("Arrow Whoosh", OneShot(), "An arrow passing by: a thin, quick whistle of air that sweeps past from one side to the other.",
                  {"arrow", "whoosh", "projectile", "flyby"}, {"duration=0.6", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("air").noise(NoiseColor::White).bp(Traj(2500).to(0.5, c.lg(c.p("brightness"), 5000, 9000), Interp::Smooth).to(1.0, 2200, Interp::Smooth), 5.0, 4).shape(env_bell(0.5, 0.2)).gain(0.8).place(-0.9, 0.0).fade(3, 40);
                  });

                d("Bow Twang", OneShot(), "A bowstring released: a low, springy twang that rings briefly and fades.",
                  {"bow", "twang", "string", "archery"}, {"duration=0.8", "pitch=48", "decay=0.5"},
                  [](Rc r, Cc c) {
                      r.add("twang").pluck(Traj(c.p("pitch")), {0.0}, 0.15 + 0.4 * (1.0 - c.p("decay"))).len(c.duration).gain(0.9);
                      r.add("snap").noise(NoiseColor::White).bp(2500, 1.5).shape(env_hit(0.001, 0.001)).len(0.02).gain(0.3).fade(0.05, 3);
                  });

                d("Whip Crack", OneShot(), "A whip: an extremely sharp, bright crack with a tiny falling tone from the loop breaking the sound barrier.",
                  {"whip", "crack", "snap", "western"}, {"duration=0.5", "tail=0.4", "brightness=0.8"},
                  [](Rc r, Cc c) {
                      r.add("crack").noise(NoiseColor::White).hp(Traj(c.lg(c.p("brightness"), 2000, 5000))).shape(env_hit(0.0005, 0.001)).len(0.06).ring(c.p("tail")).reverb(0.6, 0.4, 0.2).gain(1.0).fade(0.02, 6);
                      r.add("tone").osc(Wave::Sine, Traj::linear(100, 70)).shape(env_hit(0.002, 0.002)).len(0.05).gain(0.3).fade(0.1, 5);
                      r.add("swish").noise(NoiseColor::White).bp(Traj::exp(1000, 4000), 1.5).shape(Traj(0.0).to(1.0, 1.0, Interp::Smooth)).len(0.12).gain(0.2).place(-0.2, 0.4);
                  });

                d("Cannon Fire", OneShot(), "A big gun going off: a flat, enormous thump, a roaring blast and a long boom rolling away.",
                  {"cannon", "artillery", "boom", "weapon"}, {"duration=2", "tail=2.5", "intensity=0.8"},
                  [](Rc r, Cc c) {
                      r.add("boom").osc(Wave::Sine, Traj::linear(75, 28)).drive(2.0 + 2.0 * c.p("intensity")).shape(env_hit(0.003, 0.001)).len(1.4).gain(1.0).fade(0.3, 50);
                      r.add("blast").noise(NoiseColor::Brown).lp(Traj::exp(3500, 200), 1.0, 4).shape(env_hit(0.005, 0.001)).len(c.duration).ring(c.p("tail")).reverb(2.5 + c.p("tail"), 0.5, 0.5).gain(0.85).place(0, 1.0).fade(1, 50);
                      r.add("crack").noise(NoiseColor::White).hp(1000).shape(env_hit(0.001, 0.001)).len(0.15).gain(0.6).fade(0.05, 15);
                  });

                d("Bullet Whiz", OneShot(), "A bullet flying past your head: a very fast, high-pitched zip that drops in pitch as it passes.",
                  {"bullet", "whiz", "flyby", "projectile"}, {"duration=0.35", "pitch=100", "range=16"},
                  [](Rc r, Cc c) {
                      r.add("zip").osc(Wave::Sine, Traj::linear(c.p("pitch"), c.p("pitch") - c.p("range"))).shape(env_bell(0.45, 0.25)).gain(0.5).place(-0.8, 0.0);
                      r.add("air").noise(NoiseColor::White).bp(Traj::exp(6000, 2500), 3.0, 4).shape(env_bell(0.45, 0.25)).gain(0.5).place(-0.6, 0.3);
                  });

                d("Ricochet", OneShot(), "A bullet glancing off metal: a sharp strike followed by a high, whining falling whine.",
                  {"ricochet", "bullet", "metal", "whine"}, {"duration=0.9", "tail=0.4", "pitch=96", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("hit").modal(modes_metal(), Traj(c.p("pitch") - 12), {0.0}).decay_scale(0.2).strike(0.4).len(0.3).gain(0.7);
                      r.add("whine").osc(Wave::Sine, Traj::linear(c.p("pitch") + 6, c.p("pitch") - 30)).fm(Traj(2.0 * c.p("intensity")), 3.0).shape(Traj(1.0).to(1.0, 0.0, Interp::Exp)).at(0.02).len(c.duration).ring(c.p("tail")).gain(0.4).place(0.2, 0.5).reverb(0.8, 0.5, 0.2);
                      r.add("snap").noise(NoiseColor::White).hp(3000).shape(env_hit(0.001, 0.001)).len(0.02).gain(0.5).fade(0.05, 3);
                  });

                d("Shield Block", OneShot(), "Taking a hit on a shield: a heavy clang with a dull, woody thud underneath.",
                  {"shield", "block", "clang", "defence"}, {"duration=0.8", "pitch=58", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("clang").modal(modes_metal(), Traj(c.p("pitch")), {0.0}).decay_scale(0.35).strike(0.8).len(c.duration).gain(0.8);
                      r.add("thud").osc(Wave::Sine, Traj::linear(c.p("pitch") - 4, c.p("pitch") - 20)).drive(1.5 + 2.0 * c.p("intensity")).shape(env_hit(0.005, 0.002)).len(0.2).gain(0.8).fade(0.3, 20);
                      r.add("knock").modal(modes_wood(), Traj(c.p("pitch") - 6), {0.0}).decay_scale(0.3).strike(1.0).len(0.25).gain(0.4);
                  });

                d("Slash", OneShot(), "A fast slashing cut: a quick, bright swipe with a wet-sounding impact at the end.",
                  {"slash", "cut", "blade", "melee"}, {"duration=0.4", "intensity=0.6", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("swipe").noise(NoiseColor::White).bp(Traj::exp(1500, c.lg(c.p("brightness"), 5000, 9000)), 2.5, 4).shape(Traj(0.0).to(0.7, 1.0, Interp::Smooth).to(1.0, 0.0)).len(c.duration * 0.7).gain(0.8).place(-0.3, 0.5).fade(2, 20);
                      r.add("hit").noise(NoiseColor::Pink).bp(900, 1.0).shape(env_hit(0.01, 0.002)).at(c.duration * 0.6).len(0.12).gain(0.5 * c.p("intensity") + 0.1).fade(0.3, 15);
                  });
            }
        }
    }
}
