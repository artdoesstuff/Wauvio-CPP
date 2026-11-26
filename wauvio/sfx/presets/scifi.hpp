#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_scifi(Registry& R) {
                Def d{R, Category::SciFi};

                d("Laser Shot", OneShot(), "A classic blaster: a bright saw that dives down in pitch, with a sharp click at the start.",
                  {"laser", "shot", "blaster", "scifi"}, {"duration=0.45", "pitch=96", "range=48", "intensity=0.6", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("zap").osc(Wave::Saw, Traj::linear(c.p("pitch"), c.p("pitch") - c.p("range"))).drive(1.5 + 3.0 * c.p("intensity")).lp(Traj(c.lg(c.p("brightness"), 4000, 14000))).shape(env_hit(0.004, 0.01)).fade(0.2, 10).gain(0.9);
                      r.add("body").osc(Wave::Square, Traj::linear(c.p("pitch") - 12, c.p("pitch") - c.p("range") - 12)).lp(3000).shape(env_hit(0.004, 0.01)).fade(0.2, 10).gain(0.4);
                      r.add("click").noise(NoiseColor::White).hp(3000).shape(env_hit(0.002, 0.001)).len(0.02).gain(0.5).fade(0.05, 3);
                  });

                d("Laser Burst", OneShot(), "Rapid-fire blasts: a quick run of short laser shots at slightly different pitches.",
                  {"laser", "burst", "rapid", "scifi"}, {"duration=0.9", "steps=4", "pitch=94", "range=36", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const double gap = c.duration / (n + 1);
                      for (int i = 0; i < n; ++i)
                          r.add("shot" + std::to_string(i)).osc(Wave::Saw, Traj::linear(c.p("pitch") - i, c.p("pitch") - c.p("range") - i)).drive(2.0 + 3.0 * c.p("intensity")).lp(9000).shape(env_hit(0.01, 0.01)).at(gap * i).len(gap * 0.9 + 0.05).gain(0.8).fade(0.2, 6);
                  });

                d("Plasma Blast", OneShot(), "A heavy energy bolt: a buzzing, ring-modulated saw falls into a thick sub boom with a crackling tail.",
                  {"plasma", "blast", "energy", "heavy"}, {"duration=1.2", "tail=0.6", "pitch=70", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("bolt").osc(Wave::Saw, Traj::linear(c.p("pitch"), c.p("pitch") - 30)).unison(3, 12).ringmod(Traj(140), 0.6).drive(2.0 + 4.0 * c.p("intensity")).lp(Traj::exp(8000, 800)).shape(env_hit(0.004, 0.004)).len(c.duration).ring(c.p("tail")).gain(0.9).fade(0.2, 25);
                      r.add("boom").osc(Wave::Sine, Traj::linear(44, 28)).shape(env_hit(0.003, 0.002)).len(c.duration * 0.8).gain(0.8).fade(0.3, 30);
                      r.add("crackle").noise(NoiseColor::Dust, Traj(80.0)).hp(2500).shape(Traj(1.0).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).gain(0.4).place(0, 0.8);
                  });

                d("Force Field", Sustained(), "A shield humming around you: layered detuned tones with a slow pulse and a faint electric shimmer.",
                  {"force field", "shield", "hum", "scifi"}, {"duration=6", "pitch=55", "rate=3", "brightness=0.5", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("hum").osc(Wave::Saw, Traj(c.p("pitch"))).unison(6, 14).lp(Traj(c.lg(c.p("brightness"), 500, 2600)), 1.0, 4).trem(Traj(c.p("rate")), Traj(0.35)).gain(0.8).place(0, c.p("width")).fade(250, 400);
                      r.add("shimmer").additive(Traj(c.p("pitch") + 24), {0.0, 7.0}, 6, 1.0).spread(Traj(25.0)).trem(Traj(c.p("rate") * 1.5), Traj(0.4)).gain(0.25).place(0, c.p("width")).fade(250, 400);
                      r.add("fizz").noise(NoiseColor::White).hp(5000).trem(Traj(c.p("rate") * 2.0), Traj(0.7)).gain(0.05).fade(250, 400);
                  });

                d("Teleport", OneShot(), "A rising chirp-and-sparkle that cuts out, followed by a short whoosh as you arrive.",
                  {"teleport", "beam", "transport", "scifi"}, {"duration=1.6", "tail=0.6", "pitch=60", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("rise").osc(Wave::Sine, Traj::exp(c.p("pitch"), c.p("pitch") + 36)).fm(Traj(1.0).to(1.0, 8.0), 2.0).shape(Traj(0.0).to(0.8, 1.0, Interp::Smooth).to(0.82, 0.0, Interp::Linear)).len(c.duration * 0.7).gain(0.7).fade(2, 4);
                      r.add("sparkle").noise(NoiseColor::Dust, Traj(150.0).to(1.0, 900.0)).hp(4000).shape(Traj(0.0).to(0.8, 1.0, Interp::Smooth).to(0.85, 0.0)).len(c.duration * 0.7).gain(0.35).place(0, 0.9);
                      r.add("arrive").noise(NoiseColor::White).bp(Traj::exp(c.lg(c.p("brightness"), 5000, 9000), 600), 1.2, 4).shape(env_hit(0.02, 0.002)).at(c.duration * 0.7).len(c.duration * 0.3).ring(c.p("tail")).reverb(0.9, 0.5, 0.3).gain(0.6).place(0, 0.7).fade(2, 10);
                  });

                d("Warp Drive", Evolving(), "Spooling up to light speed: a climbing whine, a widening noise roar and a comb-filtered rush.",
                  {"warp", "drive", "engine", "riser", "scifi"}, {"duration=4", "tail=1", "pitch=44", "range=36", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("whine").osc(Wave::Saw, Traj::exp(c.p("pitch"), c.p("pitch") + c.p("range"))).unison(3, 9).drive(2.0).lp(Traj::exp(500, 9000), 1.2, 4).shape(env_swell(0.9, 0.05)).len(c.duration).ring(c.p("tail")).gain(0.8).place(0, 0.7);
                      r.add("roar").noise(NoiseColor::Pink).bp(Traj::exp(300, 5000), 0.8, 4).comb(Traj(120.0).to(1.0, 600.0), 0.8, 0.5).shape(env_swell(0.9, 0.03)).len(c.duration).ring(c.p("tail")).reverb(1.5, 0.5, 0.3).gain(0.7 * c.p("intensity") + 0.2).place(0, 1.0);
                      r.add("sub").osc(Wave::Sine, Traj::linear(c.p("pitch") - 12, c.p("pitch"))).shape(env_swell(0.9, 0.1)).len(c.duration).gain(0.35);
                  });

                d("Reactor Hum", Sustained(), "A power core at idle: a deep, throbbing drone with slowly shifting overtones.",
                  {"reactor", "engine", "drone", "hum", "scifi"}, {"duration=10", "pitch=36", "rate=2", "brightness=0.4", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("core").osc(Wave::Saw, Traj(c.p("pitch"))).unison(5, 7).lp(Traj(c.lg(c.p("brightness"), 250, 1200)), 1.4, 4).filter_lfo(lfo(Mod::Shape::Sine, c.p("rate") * 0.1, 0.4)).trem(Traj(c.p("rate")), Traj(0.25)).gain(0.9).place(0, c.p("width")).fade(500, 700);
                      r.add("over").additive(Traj(c.p("pitch") + 12), {0.0, 7.0, 12.0}, 5, 1.1).lp(1800).trem(Traj(c.p("rate") * 0.5), Traj(0.3)).gain(0.25).place(0, c.p("width")).fade(500, 700);
                      r.add("sub").osc(Wave::Sine, Traj(c.p("pitch") - 12)).gain(0.4).fade(500, 700);
                  });

                d("Hologram Flicker", Evolving(), "An unstable projection: a glassy tone that stutters in and out with digital dropouts and static.",
                  {"hologram", "flicker", "glitch", "scifi"}, {"duration=3", "pitch=76", "intensity=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("tone").additive(Traj(c.p("pitch")), {0.0, 7.0}, 6, 1.0).dropout(3.0 + 8.0 * c.p("intensity"), 20.0, 120.0, 1.0).trem(Traj(18.0), Traj(0.3)).shape(env_bell(0.5, 0.35)).reverb(0.8, 0.4, 0.2).gain(0.7).place(0, c.p("width"));
                      r.add("static").noise(NoiseColor::White).hp(3500).crush(Traj(5.0), Traj(3.0)).dropout(6.0 + 8.0 * c.p("intensity"), 10.0, 70.0, 1.0).gain(0.18).place(0, 1.0);
                  });

                d("Sci-Fi Door", OneShot(), "A pressurised door: a hiss of air, a short rising servo whine and a solid latch clunk.",
                  {"door", "airlock", "servo", "scifi"}, {"duration=1.2", "pitch=64", "intensity=0.6", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("hiss").noise(NoiseColor::White).hp(Traj(c.lg(c.p("brightness"), 2000, 5000))).shape(Traj(0.0).to(0.08, 1.0, Interp::Smooth).to(1.0, 0.001, Interp::Exp)).len(c.duration * 0.8).gain(0.5).place(0, 0.7);
                      r.add("servo").osc(Wave::Saw, Traj(c.p("pitch") - 12).to(0.4, c.p("pitch") + 4, Interp::Smooth).to(1.0, c.p("pitch") - 6, Interp::Smooth)).lp(2200, 1.5).shape(Traj(0.0).to(0.1, 1.0).to(0.85, 0.8).to(1.0, 0.0)).at(0.05).len(c.duration * 0.7).gain(0.35).fade(5, 30);
                      r.add("clunk").osc(Wave::Sine, Traj::linear(60, 38)).drive(2.0).shape(env_hit(0.003, 0.002)).at(c.duration * 0.75).len(0.2).gain(0.7).fade(0.3, 10);
                      r.add("latch").noise(NoiseColor::White).bp(1800, 2.0).shape(env_hit(0.001, 0.001)).at(c.duration * 0.75).len(0.04).gain(0.4).fade(0.05, 4);
                  });

                d("Alarm Klaxon", Sustained(), "A harsh emergency alarm: a falling two-tone blare repeated, with a hard edge to the sound.",
                  {"alarm", "klaxon", "warning", "siren"}, {"duration=4", "pitch=69", "rate=2", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      const int swaps = std::max(2, static_cast<int>(std::round(c.duration * c.p("rate") * 2.0)));
                      std::vector<double> hi, lo;
                      for (int i = 0; i < swaps; ++i) { hi.push_back(i % 2 ? c.p("pitch") - 3 : c.p("pitch") + 4); lo.push_back(i % 2 ? c.p("pitch") - 15 : c.p("pitch") - 8); }
                      r.add("blare").osc(Wave::Saw, stair(hi)).unison(2, 8).drive(2.0 + 3.0 * c.p("intensity"), DriveP::Hard).lp(3500).gain(0.8).fade(5, 20).trem(Traj(c.p("rate") * 2.0), Traj(0.15));
                      r.add("under").osc(Wave::Square, stair(lo)).lp(1200).gain(0.3).fade(5, 20);
                  });

                d("Radar Ping", OneShot(), "A sonar-style ping: a pure tone with a sharp attack that rings out and repeats in a long echo.",
                  {"radar", "sonar", "ping", "scifi"}, {"duration=1.2", "tail=2.5", "pitch=84", "decay=0.6"},
                  [](Rc r, Cc c) {
                      r.add("ping").osc(Wave::Sine, Traj(c.p("pitch"))).shape(env_hit(0.01, 0.002)).len(0.3 + 0.7 * c.p("decay")).ring(c.p("tail")).echo(0.55, 0.5, 0.4, 0.5).reverb(1.5, 0.5, 0.3).gain(0.9).fade(0.5, 15).place(0, 0.4);
                      r.add("click").noise(NoiseColor::White).bp(4000, 1.5).shape(env_hit(0.001, 0.001)).len(0.02).gain(0.2).fade(0.05, 3);
                  });

                d("Data Stream", Evolving(), "Information streaming past: dozens of tiny high beeps at random pitches in a fast, jittery run.",
                  {"data", "stream", "computer", "beeps"}, {"duration=3", "density=0.7", "pitch=84", "width=0.8"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(6); const int n = 12 + static_cast<int>(40.0 * c.p("density"));
                      for (int i = 0; i < n; ++i)
                          tone_at(r, "bit" + std::to_string(i), g.uniform() < 0.5 ? Wave::Square : Wave::Pulse, c.p("pitch") + std::round(g.range(-12.0, 12.0)), g.range(0.0, std::max(0.0, c.duration - 0.05)), g.range(0.015, 0.05), g.range(0.3, 0.8), 0.003).lp(7000).place(g.range(-0.8, 0.8), 0.0);
                  });

                d("Scanner Sweep", OneShot(), "A scanning beam passing over a surface: a tone that sweeps up then back down with an echoing trail.",
                  {"scanner", "sweep", "beam", "scifi"}, {"duration=1.6", "tail=1", "pitch=72", "range=24"},
                  [](Rc r, Cc c) {
                      r.add("beam").osc(Wave::Sine, Traj(c.p("pitch")).to(0.5, c.p("pitch") + c.p("range"), Interp::Smooth).to(1.0, c.p("pitch"), Interp::Smooth)).fm(Traj(1.5), 3.0).shape(env_bell(0.5, 0.4)).ring(c.p("tail")).echo(0.2, 0.4, 0.35, 0.4).gain(0.8).fade(5, 20).place(0, 0.5);
                      r.add("air").noise(NoiseColor::Pink).bp(Traj(1000.0).to(0.5, 5000, Interp::Smooth).to(1.0, 1000, Interp::Smooth), 2.0).shape(env_bell(0.5, 0.4)).gain(0.2);
                  });

                d("Tractor Beam", Sustained(), "A pulling energy beam: a throbbing low tone wrapped in a ring-modulated, wavering shimmer.",
                  {"tractor", "beam", "energy", "scifi"}, {"duration=6", "pitch=40", "rate=6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("throb").osc(Wave::Saw, Traj(c.p("pitch"))).unison(3, 10).lp(1000, 1.3, 4).trem(Traj(c.p("rate")), Traj(0.6)).gain(0.8).place(0, c.p("width")).fade(300, 500);
                      r.add("shimmer").osc(Wave::Sine, Traj(c.p("pitch") + 31).to(0.5, c.p("pitch") + 34, Interp::Smooth).to(1.0, c.p("pitch") + 31, Interp::Smooth)).ringmod(Traj(c.p("rate") * 20.0), 0.8).trem(Traj(c.p("rate")), Traj(0.5)).gain(0.3).place(0, c.p("width")).fade(300, 500);
                  });

                d("Power Down", OneShot(), "A machine losing power: pitch and brightness sag together while the hum dies away.",
                  {"power", "down", "shutdown", "scifi"}, {"duration=2.2", "pitch=60", "range=36", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("tone").osc(Wave::Saw, Traj::exp(c.p("pitch"), c.p("pitch") - c.p("range"))).unison(3, 10).lp(Traj::exp(c.lg(c.p("brightness"), 3000, 9000), 150), 1.1, 4).shape(Traj(1.0).to(0.6, 0.7).to(1.0, 0.0)).gain(0.9).fade(2, 40);
                      r.add("whirr").noise(NoiseColor::Pink).bp(Traj::exp(3000, 200), 1.0).shape(Traj(0.6).to(1.0, 0.0)).gain(0.2);
                  });

                d("Power Up", OneShot(), "A machine coming alive: a whine climbing in pitch and brightness to a steady, bright tone.",
                  {"power", "up", "startup", "scifi"}, {"duration=2.2", "pitch=48", "range=36", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("tone").osc(Wave::Saw, Traj::exp(c.p("pitch"), c.p("pitch") + c.p("range"))).unison(3, 10).lp(Traj::exp(150, c.lg(c.p("brightness"), 3000, 9000)), 1.1, 4).shape(Traj(0.0).to(0.85, 1.0, Interp::Smooth).to(1.0, 0.0, Interp::Exp)).gain(0.9).fade(10, 25);
                      r.add("whirr").noise(NoiseColor::Pink).bp(Traj::exp(200, 3500), 1.0).shape(Traj(0.0).to(0.85, 1.0, Interp::Smooth).to(1.0, 0.0)).gain(0.2);
                  });

                d("EMP Pulse", OneShot(), "An electromagnetic burst: a sharp electrical crack, a dropping thump and a fizzing, dying tail.",
                  {"emp", "pulse", "electric", "shock"}, {"duration=1.4", "tail=0.8", "intensity=0.7", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("crack").noise(NoiseColor::White).hp(2000).drive(3.0, DriveP::Hard).shape(env_hit(0.001, 0.001)).len(0.12).gain(0.9).fade(0.05, 6);
                      r.add("thump").osc(Wave::Sine, Traj::linear(90, 30)).drive(2.0).shape(env_hit(0.003, 0.002)).len(c.duration * 0.6).gain(0.8).fade(0.3, 25);
                      r.add("fizz").noise(NoiseColor::Dust, Traj(300.0).to(1.0, 20.0)).hp(2500).crush(Traj(6.0), Traj(2.0)).shape(Traj(1.0).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).gain(0.4 * c.p("intensity") + 0.1).place(0, 0.9);
                      r.add("whine").osc(Wave::Sine, Traj::linear(96, 60)).shape(env_hit(0.01, 0.001)).len(c.duration * 0.7).gain(0.2).reverb(1.0, 0.5, 0.3);
                  });

                d("Hyperspace Jump", OneShot(), "A leap through space: a fast rising rush that snaps into a hard, boomy impact with a stretching tail.",
                  {"hyperspace", "jump", "warp", "transition"}, {"duration=2.4", "tail=2", "pitch=40", "intensity=0.8"},
                  [](Rc r, Cc c) {
                      const double t = c.duration * 0.8;
                      r.add("rush").noise(NoiseColor::White).bp(Traj::exp(200, 9000), 0.8, 4).shape(env_swell(0.97, 0.03)).len(t).gain(0.7).place(0, 1.0);
                      r.add("whine").osc(Wave::Saw, Traj::exp(c.p("pitch") + 12, c.p("pitch") + 60)).lp(Traj::exp(400, 9000)).shape(env_swell(0.97, 0.05)).len(t).gain(0.5).place(0, 0.5);
                      r.add("boom").osc(Wave::Sine, Traj::linear(60, 30)).drive(2.0).shape(env_hit(0.003, 0.001)).at(t).len(c.duration - t + 0.4).gain(0.9 * c.p("intensity") + 0.1).fade(0.3, 40);
                      r.add("tail").noise(NoiseColor::Pink).lp(Traj::exp(5000, 200)).shape(env_hit(0.003, 0.001)).at(t).len(c.duration - t + 0.2).ring(c.p("tail")).reverb(2.0 + c.p("tail"), 0.5, 0.7).gain(0.5).place(0, 1.0);
                  });

                d("Computer Beeps", Evolving(), "An old computer thinking: short beeps at random pitches, a little irregular and a little too busy.",
                  {"computer", "beeps", "retro", "terminal"}, {"duration=3", "density=0.5", "pitch=79", "width=0.5"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(8); const int n = 6 + static_cast<int>(18.0 * c.p("density"));
                      double t = 0.0;
                      for (int i = 0; i < n && t < c.duration - 0.1; ++i) {
                          const double len = g.range(0.03, 0.12);
                          tone_at(r, "beep" + std::to_string(i), Wave::Square, c.p("pitch") + std::round(g.range(-7.0, 12.0)), t, len, 0.5, 0.01).lp(6000).place(g.range(-0.3, 0.3), 0.0);
                          t += len + g.range(0.0, 0.18);
                      }
                  });

                d("Transmission Static", Sustained(), "A bad radio link: crackling static that fades in and out, with bursts of garbled tones.",
                  {"static", "radio", "transmission", "noise"}, {"duration=5", "intensity=0.6", "brightness=0.6", "width=0.7"},
                  [](Rc r, Cc c) {
                      r.add("static").noise(NoiseColor::White).bp(Traj(c.lg(c.p("brightness"), 1500, 4500)), 0.8).crush(Traj(6.0), Traj(2.0)).dropout(2.0 + 6.0 * c.p("intensity"), 20.0, 250.0, 1.0).gain(0.8).place(0, c.p("width")).fade(100, 300);
                      r.add("tones").osc(Wave::Square, stair({70, 82, 66, 90, 75, 80})).bp(1600, 3.0).dropout(5.0, 20.0, 120.0, 1.0).gain(0.12 + 0.2 * c.p("intensity")).place(0, 0.3).fade(100, 300);
                      r.add("crackle").noise(NoiseColor::Dust, Traj(40.0)).hp(2000).drive(3.0, DriveP::Hard).gain(0.5).fade(100, 300);
                  });
            }
        }
    }
}
