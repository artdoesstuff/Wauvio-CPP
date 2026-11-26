#pragma once
#include "extra_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_game(Registry& R) {
                Def d{R, Category::Game};

                d("Coin", OneShot(), "The classic pickup: two bright square notes, a fifth apart, the second one ringing a little longer.",
                  {"coin", "pickup", "retro", "8-bit"}, {"duration=0.4", "pitch=83", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      tone_at(r, "a", Wave::Pulse, c.p("pitch"), 0.0, 0.08, 0.7, 0.01).pw(0.25).lp(Traj(c.lg(c.p("brightness"), 4000, 12000)));
                      tone_at(r, "b", Wave::Pulse, c.p("pitch") + 5, 0.07, c.duration - 0.07, 0.8, 0.01).pw(0.25).lp(Traj(c.lg(c.p("brightness"), 4000, 12000)));
                  });

                d("Arcade Power-Up", OneShot(), "A rising run of notes that climbs a scale and lands on the top note: reward and energy.",
                  {"powerup", "arpeggio", "retro", "reward"}, {"duration=0.9", "pitch=60", "steps=6", "scale=1"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const auto walk = c.scale_walk(n);
                      const double step = std::max(0.02, (c.duration - 0.25) / n);
                      for (int i = 0; i < n; ++i) tone_at(r, "n" + std::to_string(i), Wave::Pulse, c.p("pitch") + walk[static_cast<size_t>(i)], step * i, i == n - 1 ? 0.3 : step * 1.2, 0.7).pw(0.5).lp(8000);
                  });

                d("Jump", OneShot(), "A springy hop: a square wave that bends quickly upward and fades, like a character leaving the ground.",
                  {"jump", "hop", "retro", "platformer"}, {"duration=0.3", "pitch=60", "range=14"},
                  [](Rc r, Cc c) {
                      r.add("hop").osc(Wave::Pulse, Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range"))).pw(0.25).lp(6000).shape(Traj(1.0).to(0.7, 0.8).to(1.0, 0.0)).fade(0.3, 15).gain(0.8);
                  });

                d("Hurt", OneShot(), "Taking damage: a harsh falling buzz with a burst of noise on the front.",
                  {"hurt", "damage", "hit", "retro"}, {"duration=0.4", "pitch=64", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("buzz").osc(Wave::Square, Traj::linear(c.p("pitch"), c.p("pitch") - 20)).drive(1.0 + 2.0 * c.p("intensity"), DriveP::Hard).crush(Traj(6.0), Traj(2.0)).shape(Traj(1.0).to(1.0, 0.0)).gain(0.8).fade(0.3, 10);
                      r.add("burst").noise(NoiseColor::White).lp(5000).shape(env_hit(0.01, 0.002)).len(0.1).gain(0.5).fade(0.1, 5);
                  });

                d("8-Bit Explosion", OneShot(), "A crunchy retro blast: low-bit noise sweeping from bright to dull with a dropping thud.",
                  {"explosion", "8-bit", "retro", "blast"}, {"duration=0.9", "intensity=0.7", "bits=4"},
                  [](Rc r, Cc c) {
                      r.add("noise").noise(NoiseColor::White).crush(Traj(c.p("bits")), Traj(3.0)).lp(Traj::exp(8000, 300), 1.0, 4).shape(env_hit(0.005, 0.002)).gain(0.9).fade(0.1, 30);
                      r.add("thud").osc(Wave::Square, Traj::linear(46, 20)).lp(500).shape(env_hit(0.004, 0.002)).len(c.duration * 0.6).gain(0.6 * c.p("intensity") + 0.2).fade(0.2, 20);
                  });

                d("Pickup", OneShot(), "A small item grab: a quick, cheerful two-note blip with a light, rounded tone.",
                  {"pickup", "item", "collect", "retro"}, {"duration=0.3", "pitch=79"},
                  [](Rc r, Cc c) {
                      tone_at(r, "a", Wave::Triangle, c.p("pitch"), 0.0, 0.07, 0.8, 0.02);
                      tone_at(r, "b", Wave::Triangle, c.p("pitch") + 7, 0.06, c.duration - 0.06, 0.9, 0.02);
                  });

                d("Menu Click", OneShot(), "A tiny, tight UI tick: a very short blip with a hard edge, for moving a cursor.",
                  {"menu", "click", "ui", "cursor"}, {"duration=0.08", "pitch=88", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("tick").osc(Wave::Pulse, Traj(c.p("pitch"))).pw(0.25).lp(Traj(c.lg(c.p("brightness"), 3000, 10000))).shape(env_hit(0.02, 0.01)).gain(0.8).fade(0.1, 6);
                  });

                d("Menu Select", OneShot(), "Choosing an option: a short two-note rise with a clean, slightly hollow tone.",
                  {"menu", "select", "ui", "choose"}, {"duration=0.22", "pitch=76"},
                  [](Rc r, Cc c) {
                      tone_at(r, "a", Wave::Pulse, c.p("pitch"), 0.0, 0.07, 0.7, 0.02).pw(0.5).lp(8000);
                      tone_at(r, "b", Wave::Pulse, c.p("pitch") + 7, 0.06, c.duration - 0.06, 0.8, 0.02).pw(0.5).lp(8000);
                  });

                d("Confirm", OneShot(), "Yes, accepted: a bright rising third with a short sparkle, clearly positive.",
                  {"confirm", "accept", "ui", "yes"}, {"duration=0.4", "pitch=72"},
                  [](Rc r, Cc c) {
                      tone_at(r, "a", Wave::Pulse, c.p("pitch"), 0.0, 0.1, 0.7, 0.01).pw(0.25).lp(9000);
                      tone_at(r, "b", Wave::Pulse, c.p("pitch") + 4, 0.09, 0.1, 0.7, 0.01).pw(0.25).lp(9000);
                      tone_at(r, "c", Wave::Pulse, c.p("pitch") + 12, 0.18, c.duration - 0.18, 0.8, 0.01).pw(0.25).lp(9000);
                  });

                d("Cancel", OneShot(), "No, back out: a short falling pair of notes, softer and lower than the confirm.",
                  {"cancel", "back", "ui", "no"}, {"duration=0.3", "pitch=72"},
                  [](Rc r, Cc c) {
                      tone_at(r, "a", Wave::Pulse, c.p("pitch"), 0.0, 0.09, 0.7, 0.01).pw(0.5).lp(6000);
                      tone_at(r, "b", Wave::Pulse, c.p("pitch") - 5, 0.08, c.duration - 0.08, 0.7, 0.01).pw(0.5).lp(6000);
                  });

                d("Error Buzz", OneShot(), "Not allowed: two short, low, buzzy square pulses in a row.",
                  {"error", "buzz", "ui", "denied"}, {"duration=0.4", "pitch=46", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("buzz").osc(Wave::Square, Traj(c.p("pitch"))).unison(2, 15).drive(1.0 + 2.0 * c.p("intensity"), DriveP::Hard).lp(2500).gate_sec({1, 0, 1, 0, 0}, c.duration / 5.0, 1.0, 2.0).gain(0.8).fade(1, 15);
                  });

                d("Notification", OneShot(), "A soft, friendly alert: two rounded bell-like tones with a gentle ring.",
                  {"notification", "alert", "ui", "message"}, {"duration=0.8", "tail=0.6", "pitch=84", "decay=0.5"},
                  [](Rc r, Cc c) {
                      r.add("a").modal(modes_bell(), Traj(c.p("pitch")), {0.0}).decay_scale(0.2 + 0.4 * c.p("decay")).strike(1.0).gain(0.8).ring(c.p("tail")).len(0.3).reverb(0.8, 0.5, 0.2);
                      r.add("b").modal(modes_bell(), Traj(c.p("pitch") + 7), {0.0}).decay_scale(0.2 + 0.4 * c.p("decay")).strike(1.0).gain(0.8).at(0.14).ring(c.p("tail")).len(c.duration - 0.14).reverb(0.8, 0.5, 0.2);
                  });

                d("Level Up", OneShot(), "A short triumphant fanfare: a quick run up a major arpeggio ending on a held, shimmering top note.",
                  {"levelup", "fanfare", "victory", "reward"}, {"duration=1.4", "tail=0.6", "pitch=60"},
                  [](Rc r, Cc c) {
                      note_seq(r, "arp", Wave::Pulse, {c.p("pitch"), c.p("pitch") + 4, c.p("pitch") + 7, c.p("pitch") + 12, c.p("pitch") + 16}, 0.0, 0.09, 0.14, 0.6, 0.25);
                      tone_at(r, "top", Wave::Pulse, c.p("pitch") + 19, 0.45, c.duration - 0.45, 0.8, 0.01).pw(0.25).lp(9000).ring(c.p("tail")).reverb(0.8, 0.5, 0.2);
                      tone_at(r, "low", Wave::Triangle, c.p("pitch") - 12, 0.45, c.duration - 0.45, 0.5, 0.02);
                  });

                d("Game Over", OneShot(), "Defeat: a slow, falling run of notes that droops and finally lands low and flat.",
                  {"gameover", "defeat", "lose", "retro"}, {"duration=1.8", "pitch=67", "steps=5"},
                  [](Rc r, Cc c) {
                      const double ns[] = {0, -2, -4, -7, -12};
                      const int n = std::min(5, static_cast<int>(c.p("steps")));
                      for (int i = 0; i < n; ++i) tone_at(r, "n" + std::to_string(i), Wave::Pulse, c.p("pitch") + ns[i], i * 0.28, i == n - 1 ? c.duration - 0.28 * i : 0.3, 0.7, 0.02).pw(0.5).lp(5000);
                  });

                d("Countdown Beep", OneShot(), "A single timer beep: a clean sine tone of fixed length; use a higher pitch for the final one.",
                  {"countdown", "beep", "timer", "ui"}, {"duration=0.25", "pitch=81"},
                  [](Rc r, Cc c) {
                      r.add("beep").osc(Wave::Sine, Traj(c.p("pitch"))).shape(Traj(1.0).to(0.8, 1.0).to(1.0, 0.0)).gain(0.9).fade(2, 8);
                  });

                d("Combo", OneShot(), "A rising count of hits: each step goes up a scale note, so a longer combo climbs higher.",
                  {"combo", "chain", "score", "retro"}, {"duration=1", "pitch=64", "steps=6", "scale=1"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const auto walk = c.scale_walk(n);
                      const double step = c.duration / n;
                      for (int i = 0; i < n; ++i) tone_at(r, "n" + std::to_string(i), Wave::Pulse, c.p("pitch") + walk[static_cast<size_t>(i)], step * i, step * 0.9, 0.5 + 0.1 * i, 0.005).pw(0.25).lp(9000);
                  });

                d("Health Restore", OneShot(), "A soft, warm heal: gentle sine arpeggio rising with a little shimmer on top.",
                  {"health", "heal", "restore", "recovery"}, {"duration=1.2", "tail=0.6", "pitch=67"},
                  [](Rc r, Cc c) {
                      note_seq(r, "n", Wave::Sine, {c.p("pitch"), c.p("pitch") + 4, c.p("pitch") + 7, c.p("pitch") + 12, c.p("pitch") + 16}, 0.0, 0.11, 0.5, 0.6);
                      r.add("sparkle").additive(Traj(c.p("pitch") + 24), {0.0, 4.0, 7.0}, 4, 1.0).shape(env_bell(0.5, 0.3)).len(c.duration).ring(c.p("tail")).reverb(1.2, 0.4, 0.4).gain(0.25).place(0, 0.8);
                  });

                d("Door Unlock", OneShot(), "A lock opening: a small click, a mechanical clunk and a bright confirming chime.",
                  {"unlock", "door", "key", "puzzle"}, {"duration=0.9", "pitch=84", "tail=0.5"},
                  [](Rc r, Cc c) {
                      r.add("click").noise(NoiseColor::White).bp(2500, 2.0).shape(env_hit(0.001, 0.001)).len(0.03).gain(0.5).fade(0.05, 3);
                      r.add("clunk").modal(modes_metal(), Traj(60.0), {0.0}).decay_scale(0.12).strike(1.0).at(0.09).len(0.2).gain(0.6);
                      r.add("chime").modal(modes_bell(), Traj(c.p("pitch")), {0.0}).decay_scale(0.35).strike(1.0).at(0.3).len(c.duration - 0.3).ring(c.p("tail")).gain(0.6).reverb(0.8, 0.5, 0.2);
                  });

                d("Retro Laser", OneShot(), "A chip-style zap: a pulse wave swooping down very quickly, as heard in a 1980s arcade.",
                  {"laser", "zap", "retro", "arcade"}, {"duration=0.3", "pitch=96", "range=40"},
                  [](Rc r, Cc c) {
                      r.add("zap").osc(Wave::Pulse, Traj::linear(c.p("pitch"), c.p("pitch") - c.p("range"))).pw(0.25).lp(10000).shape(Traj(1.0).to(0.8, 0.8).to(1.0, 0.0)).fade(0.2, 12).gain(0.8);
                  });

                d("Shop Bell", OneShot(), "A counter bell: a bright metallic ding with a short, clear ring.",
                  {"shop", "bell", "ding", "store"}, {"duration=1.2", "tail=1", "pitch=88", "decay=0.5"},
                  [](Rc r, Cc c) {
                      r.add("ding").modal(modes_bell(), Traj(c.p("pitch")), {0.0}).decay_scale(0.3 + 0.7 * c.p("decay")).strike(0.8).len(c.duration).ring(c.p("tail")).gain(0.9).reverb(0.6, 0.5, 0.15);
                      r.add("click").noise(NoiseColor::White).bp(4500, 1.5).shape(env_hit(0.001, 0.001)).len(0.01).gain(0.2).fade(0.05, 2);
                  });

                d("Achievement", OneShot(), "A trophy sound: a rising shimmer followed by a rich, ringing chord that sustains briefly.",
                  {"achievement", "trophy", "reward", "unlock"}, {"duration=1.8", "tail=1.2", "pitch=67", "chord=0"},
                  [](Rc r, Cc c) {
                      note_seq(r, "run", Wave::Triangle, {c.p("pitch"), c.p("pitch") + 5, c.p("pitch") + 9, c.p("pitch") + 12}, 0.0, 0.08, 0.2, 0.5);
                      r.add("chord").additive(Traj(c.p("pitch") + 12), c.chord_notes(), 6, 1.2).shape(env_hit(0.02, 0.002)).at(0.35).len(c.duration - 0.35).ring(c.p("tail")).reverb(1.8, 0.4, 0.4).gain(0.7).place(0, 0.8).fade(2, 20);
                      r.add("bell").modal(modes_bell(), Traj(c.p("pitch") + 24), {0.0}).decay_scale(0.4).strike(0.8).at(0.35).len(c.duration - 0.35).ring(c.p("tail")).gain(0.25);
                  });

                d("One-Up", OneShot(), "An extra life: a bouncing run of seven fast notes up a major scale, short and joyful.",
                  {"1up", "extra life", "bonus", "retro"}, {"duration=0.8", "pitch=64"},
                  [](Rc r, Cc c) {
                      const double ns[] = {0, 4, 7, 12, 11, 14, 19};
                      for (int i = 0; i < 7; ++i) tone_at(r, "n" + std::to_string(i), Wave::Pulse, c.p("pitch") + ns[i], i * (c.duration - 0.12) / 7.0, 0.1 + (i == 6 ? 0.1 : 0.0), 0.7, 0.01).pw(0.25).lp(9000);
                  });

                d("Fall", OneShot(), "A long drop: a tone sliding down and down, wobbling slightly, with the volume fading out.",
                  {"fall", "drop", "plummet", "retro"}, {"duration=1.4", "pitch=88", "range=48"},
                  [](Rc r, Cc c) {
                      r.add("slide").osc(Wave::Pulse, Traj::linear(c.p("pitch"), c.p("pitch") - c.p("range"))).pw(0.5).lp(6000).shape(Traj(1.0).to(0.7, 0.7).to(1.0, 0.0)).gain(0.8).fade(1, 20).trem(Traj(10.0), Traj(0.15));
                  });

                d("Bounce", OneShot(), "A ball bouncing: a few quick, soft thuds that get closer together and quieter.",
                  {"bounce", "ball", "boing", "retro"}, {"duration=1.2", "pitch=50", "steps=5"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); double t = 0.0, gap = c.duration * 0.28;
                      for (int i = 0; i < n; ++i) {
                          r.add("b" + std::to_string(i)).osc(Wave::Sine, Traj::linear(c.p("pitch") + 12, c.p("pitch"))).shape(env_hit(0.01, 0.002)).at(t).len(0.1).gain(std::pow(0.7, i)).fade(0.3, 8);
                          t += gap; gap *= 0.72;
                      }
                  });
            }
        }
    }
}
