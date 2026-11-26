#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_machines(Registry& R) {
                Def d{R, Category::Machines};

                d("Engine Idle", Sustained(), "A petrol engine ticking over: a low, lumpy idle built from firing pulses and an exhaust rumble.",
                  {"engine", "idle", "car", "motor"}, {"duration=8", "rate=14", "pitch=34", "brightness=0.4", "width=0.5"},
                  [](Rc r, Cc c) {
                      r.add("fire").osc(Wave::Saw, Traj(c.p("pitch"))).unison(2, 10).drive(2.0, DriveP::Asym).lp(Traj(c.lg(c.p("brightness"), 300, 1200)), 1.0, 4).pulses(Traj(c.p("rate")), 38, 0.65, 0.12).gain(0.9).place(0, c.p("width")).fade(300, 500);
                      r.add("rumble").noise(NoiseColor::Brown).lp(180).trem(Traj(c.p("rate")), Traj(0.3)).gain(0.35).fade(300, 500);
                      r.add("mech").noise(NoiseColor::Pink).bp(1400, 1.0).trem(Traj(c.p("rate") * 2.0), Traj(0.5)).gain(0.05).fade(300, 500);
                  });

                d("Engine Rev", OneShot(), "An engine being revved: the firing rate and pitch climb quickly, hold at the top and fall back.",
                  {"engine", "rev", "car", "motor"}, {"duration=3", "pitch=34", "range=19", "intensity=0.7", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("fire").osc(Wave::Saw, Traj(c.p("pitch")).to(0.3, c.p("pitch") + c.p("range"), Interp::Smooth).to(0.55, c.p("pitch") + c.p("range"), Interp::Linear).to(1.0, c.p("pitch"), Interp::Smooth)).unison(2, 10).drive(2.0 + 2.0 * c.p("intensity"), DriveP::Asym)
                          .lp(Traj(c.lg(c.p("brightness"), 600, 2500)), 1.0, 4).pulses(Traj(14.0).to(0.3, 45.0, Interp::Smooth).to(0.55, 45.0).to(1.0, 14.0, Interp::Smooth), 25, 0.5, 0.1).shape(Traj(0.7).to(0.3, 1.0).to(0.55, 1.0).to(1.0, 0.7)).gain(0.9).place(0, 0.5).fade(30, 120);
                      r.add("exhaust").noise(NoiseColor::Brown).lp(Traj(250).to(0.3, 900, Interp::Smooth).to(1.0, 250, Interp::Smooth)).gain(0.4);
                  });

                d("Car Pass-By", OneShot(), "A car driving past: the engine note falls in pitch as it goes by, with the level and tyre noise swelling and fading.",
                  {"car", "pass", "doppler", "vehicle"}, {"duration=4", "pitch=48", "range=6", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("engine").osc(Wave::Saw, Traj(c.p("pitch") + c.p("range")).to(0.45, c.p("pitch") + c.p("range")).to(0.55, c.p("pitch") - c.p("range"), Interp::Smooth).to(1.0, c.p("pitch") - c.p("range"))).unison(2, 8).drive(2.0).lp(Traj(c.lg(c.p("brightness"), 800, 2500)), 1.0, 4).pulses(Traj(30.0), 20, 0.4, 0.1)
                          .shape(env_bell(0.5, 0.2)).gain(0.8).place(-0.8, 0.0).fade(30, 60);
                      r.add("tyres").noise(NoiseColor::Pink).bp(Traj(2000).to(0.5, 1700, Interp::Smooth).to(1.0, 1200, Interp::Smooth), 0.8).shape(env_bell(0.5, 0.2)).gain(0.45).place(-0.8, 0.0);
                  });

                d("Jet Flyby", OneShot(), "A jet passing overhead: a rising scream of noise and a heavy rumble, dropping away behind you.",
                  {"jet", "flyby", "aircraft", "plane"}, {"duration=6", "tail=1.5", "brightness=0.6", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("scream").noise(NoiseColor::White).bp(Traj(1200).to(0.45, c.lg(c.p("brightness"), 4000, 8000), Interp::Smooth).to(1.0, 900, Interp::Smooth), 0.9, 4).shape(env_bell(0.45, 0.2)).ring(c.p("tail")).reverb(1.5, 0.5, 0.25).gain(0.8).place(-0.9, 0.5).fade(100, 200);
                      r.add("rumble").noise(NoiseColor::Brown).lp(Traj(150).to(0.45, 500, Interp::Smooth).to(1.0, 120, Interp::Smooth)).shape(env_bell(0.5, 0.22)).gain(0.8 * c.p("intensity") + 0.1).place(-0.5, 0.4);
                      r.add("whine").osc(Wave::Saw, Traj(88.0).to(0.45, 94.0, Interp::Smooth).to(1.0, 80.0, Interp::Smooth)).bp(2800, 2.0).shape(env_bell(0.45, 0.2)).gain(0.15);
                  });

                d("Helicopter", Sustained(), "A helicopter hovering: a fast, thudding rotor beat over a turbine whine and rushing air.",
                  {"helicopter", "rotor", "aircraft", "chopper"}, {"duration=8", "rate=11", "intensity=0.7", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("rotor").noise(NoiseColor::Brown).lp(Traj(500)).pulses(Traj(c.p("rate")), 60, 0.9, 0.03).drive(1.5).gain(0.9).place(0, c.p("width")).fade(500, 700);
                      r.add("blade").noise(NoiseColor::Pink).bp(900, 1.0).pulses(Traj(c.p("rate")), 40, 0.7, 0.03).gain(0.35).place(0, c.p("width")).fade(500, 700);
                      r.add("turbine").osc(Wave::Saw, Traj(hz_to_midi(1700.0))).bp(1700, 4.0).gain(0.1 + 0.1 * c.p("intensity")).fade(500, 700);
                      r.add("air").noise(NoiseColor::White).hp(2500).lp(8000).gain(0.05).fade(500, 700);
                  });

                d("Police Siren", Sustained(), "A rising and falling wail: a siren sweeping smoothly between two pitches, with a slightly harsh edge.",
                  {"siren", "police", "emergency", "wail"}, {"duration=6", "pitch=76", "range=10", "rate=0.4"},
                  [](Rc r, Cc c) {
                      Traj sweep(c.p("pitch")); const int cycles = std::max(1, static_cast<int>(std::round(c.duration * c.p("rate")))); const int keys = cycles * 2;
                      for (int i = 1; i <= keys; ++i) sweep.to(static_cast<double>(i) / keys, i % 2 ? c.p("pitch") + c.p("range") : c.p("pitch"), Interp::Smooth);
                      r.add("wail").osc(Wave::Saw, sweep).drive(2.0, DriveP::Hard, 0.5).bp(1800, 0.7, 4).gain(0.8).fade(20, 100);
                      r.add("body").osc(Wave::Square, sweep.shifted(-12.0)).lp(1500).gain(0.25).fade(20, 100);
                  });

                d("Ambulance Siren", Sustained(), "A two-tone emergency horn: two alternating, hard-edged notes that never glide.",
                  {"siren", "ambulance", "emergency", "two-tone"}, {"duration=5", "pitch=79", "rate=1.5"},
                  [](Rc r, Cc c) {
                      const int swaps = std::max(2, static_cast<int>(std::round(c.duration * c.p("rate") * 2.0)));
                      std::vector<double> n; for (int i = 0; i < swaps; ++i) n.push_back(i % 2 ? c.p("pitch") - 5 : c.p("pitch"));
                      r.add("horn").osc(Wave::Square, stair(n)).unison(2, 8).drive(2.0, DriveP::Hard, 0.5).bp(1600, 0.8, 4).gain(0.8).fade(20, 100);
                      r.add("body").osc(Wave::Saw, stair(n).shifted(-12.0)).lp(1800).gain(0.2).fade(20, 100);
                  });

                d("Train Pass", OneShot(), "A train going by: a rolling rumble with a steady clickety-clack that swells and fades, and a distant horn.",
                  {"train", "pass", "rail", "vehicle"}, {"duration=10", "rate=7", "brightness=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("rumble").noise(NoiseColor::Brown).lp(Traj(c.lg(c.p("brightness"), 300, 700))).shape(env_bell(0.5, 0.25)).gain(0.9).place(0, c.p("width")).fade(50, 200);
                      r.add("clack").noise(NoiseColor::Pink).bp(1400, 1.2).pulses(Traj(c.p("rate")), 40, 1.0, 0.04).shape(env_bell(0.5, 0.25)).gain(0.5).place(0, c.p("width")).fade(50, 200);
                      r.add("clack2").modal(modes_metal(), Traj(66.0), {0.0}).decay_scale(0.08).strike(0.4).pulses(Traj(c.p("rate")), 30, 0.7, 0.04).shape(env_bell(0.5, 0.25)).gain(0.25).place(0, 0.5);
                      r.add("horn").osc(Wave::Saw, Traj(46.0)).unison(2, 14).bp(500, 1.2, 4).shape(Traj(0.0).to(0.05, 1.0).to(0.2, 1.0).to(0.24, 0.0).to(1.0, 0.0)).len(c.duration).gain(0.25).reverb(2.0, 0.5, 0.3).place(-0.3, 0.5);
                  });

                d("Factory Hum", Sustained(), "An industrial hall: a steady low hum of machinery, a regular thump and distant clatter.",
                  {"factory", "industrial", "machinery", "hum"}, {"duration=12", "rate=1.5", "brightness=0.4", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("hum").additive(Traj(hz_to_midi(50.0)), {0.0}, 14, 0.9).lp(Traj(c.lg(c.p("brightness"), 600, 2400))).gain(0.35).place(0, c.p("width")).fade(1000, 1200);
                      r.add("thump").osc(Wave::Sine, Traj::linear(60, 38)).pulses(Traj(c.p("rate")), 90, 0.85).drive(1.5).gain(0.35).fade(1000, 1200);
                      r.add("clatter").noise(NoiseColor::Dust, Traj(18.0)).bp(2500, 2.0).gain(0.5).place(0, c.p("width")).reverb(1.5, 0.5, 0.3).fade(1000, 1200);
                      r.add("air").noise(NoiseColor::Pink).lp(1200).gain(0.15).fade(1000, 1200);
                  });

                d("Electric Fan", Sustained(), "A desk fan: a smooth airy whoosh with a gentle, regular flutter from the blades.",
                  {"fan", "air", "appliance", "household"}, {"duration=10", "rate=7", "brightness=0.5", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("air").noise(NoiseColor::Pink).bp(Traj(c.lg(c.p("brightness"), 700, 1800)), 0.7).trem(Traj(c.p("rate")), Traj(0.18)).gain(0.8).place(0, c.p("width")).fade(500, 700);
                      r.add("motor").osc(Wave::Saw, Traj(hz_to_midi(120.0))).lp(600).gain(0.06).fade(500, 700);
                  });

                d("Servo Motor", OneShot(), "A small electric motor: a quick whine that rises, holds a moment and settles, with a faint gear chatter.",
                  {"servo", "motor", "robot", "mechanical"}, {"duration=1", "pitch=78", "range=12", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("whine").osc(Wave::Saw, Traj(c.p("pitch")).to(0.25, c.p("pitch") + c.p("range"), Interp::Smooth).to(0.7, c.p("pitch") + c.p("range"), Interp::Linear).to(1.0, c.p("pitch") - 4, Interp::Smooth)).bp(Traj(2500), 2.0, 4).shape(Traj(0.0).to(0.1, 1.0).to(0.85, 1.0).to(1.0, 0.0)).gain(0.7).fade(5, 30);
                      r.add("chatter").noise(NoiseColor::Pink).bp(1800, 2.0).pulses(Traj(70.0), 6, 0.8, 0.1).shape(Traj(0.0).to(0.1, 1.0).to(0.85, 1.0).to(1.0, 0.0)).gain(0.25 * c.p("intensity") + 0.05);
                  });

                d("Gear Grind", OneShot(), "A gearbox protesting: a harsh, juddering grind of clashing metal teeth and a falling, strained groan.",
                  {"gear", "grind", "mechanical", "metal"}, {"duration=1.5", "pitch=52", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("grind").osc(Wave::Saw, Traj(c.p("pitch")).to(1.0, c.p("pitch") - 10, Interp::Smooth)).unison(3, 30).ringmod(Traj(55.0).to(1.0, 38.0), 0.6).bp(1200, 0.9, 4).drive(2.0 + 2.0 * c.p("intensity"), DriveP::Hard, 0.6).shape(Traj(0.0).to(0.08, 1.0).to(0.8, 0.8).to(1.0, 0.0)).gain(0.8).fade(5, 60);
                      r.add("teeth").noise(NoiseColor::White).bp(2800, 1.5).pulses(Traj(32.0).to(1.0, 18.0), 12, 0.9, 0.1).shape(Traj(0.0).to(0.08, 1.0).to(0.8, 0.8).to(1.0, 0.0)).gain(0.35);
                  });

                d("Steam Hiss", OneShot(), "A release of steam: a long, hissing burst that starts hard and thins out with a low rushing body.",
                  {"steam", "hiss", "vent", "pressure"}, {"duration=2", "tail=0.6", "brightness=0.7", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("hiss").noise(NoiseColor::White).hp(Traj(c.lg(c.p("brightness"), 2000, 4500))).shape(Traj(0.0).to(0.03, 1.0, Interp::Smooth).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).gain(0.8).place(0, 0.7).fade(1, 40);
                      r.add("rush").noise(NoiseColor::Pink).bp(1100, 0.6).shape(Traj(0.0).to(0.03, 1.0, Interp::Smooth).to(1.0, 0.001, Interp::Exp)).len(c.duration).gain(0.35 * c.p("intensity") + 0.1);
                  });

                d("Elevator Ding", OneShot(), "An elevator arriving: a clean, soft two-tone bell with a rounded attack.",
                  {"elevator", "ding", "bell", "arrival"}, {"duration=1.4", "tail=0.8", "pitch=79"},
                  [](Rc r, Cc c) {
                      r.add("a").modal(modes_bell(), Traj(c.p("pitch")), {0.0}).decay_scale(0.35).strike(2.0).len(0.7).ring(c.p("tail")).gain(0.8).reverb(0.8, 0.5, 0.2);
                      r.add("b").modal(modes_bell(), Traj(c.p("pitch") - 5), {0.0}).decay_scale(0.35).strike(2.0).at(0.55).len(c.duration - 0.55).ring(c.p("tail")).gain(0.8).reverb(0.8, 0.5, 0.2);
                  });

                d("Phone Ring", Sustained(), "An old landline: a dual-tone electric bell ringing in two bursts followed by a pause.",
                  {"phone", "ring", "telephone", "landline"}, {"duration=6", "rate=20", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      const double step = 0.25;
                      std::vector<double> pat = {1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
                      r.add("bell1").osc(Wave::Square, Traj(hz_to_midi(1000.0))).trem(Traj(c.p("rate")), Traj(0.6)).lp(3500).gate_sec(pat, step * 0.5, 1.0, 3.0).gain(0.6).place(-0.1, 0.0).fade(5, 20);
                      r.add("bell2").osc(Wave::Square, Traj(hz_to_midi(1400.0))).trem(Traj(c.p("rate")), Traj(0.6)).lp(4500).gate_sec(pat, step * 0.5, 1.0, 3.0).gain(0.4 + 0.2 * c.p("intensity")).place(0.1, 0.0).fade(5, 20);
                      r.add("clack").modal(modes_metal(), Traj(96.0), {0.0}).decay_scale(0.05).strike(0.3).pulses(Traj(c.p("rate")), 12, 0.8).gate_sec(pat, step * 0.5, 1.0, 3.0).gain(0.15);
                  });

                d("Alarm Clock", Sustained(), "A bedside alarm: rapid, harsh electronic beeps in fast groups of four with a short gap between.",
                  {"alarm", "clock", "beep", "wake"}, {"duration=5", "pitch=96", "rate=8"},
                  [](Rc r, Cc c) {
                      std::vector<double> pat = {1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0};
                      r.add("beep").osc(Wave::Square, Traj(c.p("pitch"))).lp(7000).gate_sec(pat, 1.0 / (c.p("rate") * 2.0), 1.0, 1.5).gain(0.7).fade(5, 30);
                      r.add("buzz").osc(Wave::Square, Traj(c.p("pitch") + 7)).lp(6000).gate_sec(pat, 1.0 / (c.p("rate") * 2.0), 1.0, 1.5).gain(0.25).fade(5, 30);
                  });

                d("Doorbell", OneShot(), "A two-note chime: a rising pair of bell tones, the second ringing out into the room.",
                  {"doorbell", "chime", "bell", "household"}, {"duration=2", "tail=1.5", "pitch=76", "decay=0.5"},
                  [](Rc r, Cc c) {
                      r.add("ding").modal(modes_bell(), Traj(c.p("pitch")), {0.0}).decay_scale(0.3 + 0.5 * c.p("decay")).strike(1.0).len(0.7).ring(c.p("tail")).gain(0.8).reverb(0.9, 0.5, 0.2);
                      r.add("dong").modal(modes_bell(), Traj(c.p("pitch") - 4), {0.0}).decay_scale(0.35 + 0.6 * c.p("decay")).strike(1.0).at(0.55).len(c.duration - 0.55).ring(c.p("tail")).gain(0.9).reverb(0.9, 0.5, 0.2);
                  });

                d("Ship Horn", OneShot(), "A large vessel's horn: a huge, low, slowly swelling blast that rings across open water.",
                  {"ship", "horn", "foghorn", "vessel"}, {"duration=3", "tail=2.5", "pitch=36", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("horn").osc(Wave::Saw, Traj(c.p("pitch"))).unison(3, 7).drive(1.5 + 2.0 * c.p("intensity")).lp(1000, 1.2, 4).shape(Traj(0.0).to(0.1, 1.0, Interp::Smooth).to(0.8, 0.9).to(1.0, 0.0, Interp::Smooth)).len(c.duration).ring(c.p("tail")).reverb(3.0 + c.p("tail"), 0.5, 0.5).gain(0.9).place(0, 0.8).fade(30, 80);
                      r.add("fifth").osc(Wave::Saw, Traj(c.p("pitch") + 7)).lp(900, 1.0, 4).shape(Traj(0.0).to(0.12, 0.5, Interp::Smooth).to(0.8, 0.45).to(1.0, 0.0, Interp::Smooth)).len(c.duration).ring(c.p("tail")).gain(0.4);
                  });
            }
        }
    }
}
