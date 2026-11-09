#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_candy(Registry& R) {
                Def d{R, Category::EarCandy};

                d("Sparkle", OneShot(), "A scatter of very high, short glass-mode pings at random times and velocities.",
                  {"sparkle", "glass", "high", "scatter"}, {"duration=1.4", "pitch=96", "density=0.6", "brightness=0.7", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("pings").modal(modes_glass(), c.p("pitch"), c.scattered(static_cast<int>(6 + 24 * c.p("density")), 1, 0.0, 0.85)).hit_notes({ 0, 2, 4, 7, 9, 12, -3, -5 }).hit_jitter(0.0, 0.7).decay_scale(0.25).strike(0.5)
                          .hp(1500).lp(c.lg(c.p("brightness"), 6000, 18000)).place(0, c.p("width")).fade(0.1, 15);
                  });

                d("Shimmer Burst", OneShot(), "A short bloom of high additive partials with fast tremolo and a reverb tail: a brief wash of glitter.",
                  {"shimmer", "burst", "bloom", "high"}, {"duration=1", "tail=1.5", "pitch=84", "chord=0", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("shimmer").additive(c.p("pitch"), c.chord_notes(), 5, 1.0).spread(Traj(15)).trem(9.0, 0.5).hp(800).lp(c.lg(c.p("brightness"), 5000, 16000)).shape(Traj::ar(0.08, 1.0, 0.002)).len(c.duration).ring(c.p("tail"))
                          .reverb(1.5 + c.p("tail"), 0.4, 0.7).place(0, 1).fade(1, 20);
                  });

                d("Bell Cascade", OneShot(), "A fast, dense run of struck bells that alternates left and right while stepping down the chosen scale; the notes pile into one long shared ring.",
                  {"bells", "cascade", "scale", "descending", "dense"}, {"duration=2.5", "pitch=84", "scale=1", "steps=16", "decay=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const auto deg = scale_degrees(static_cast<int>(c.p("scale"))); const auto all = c.evenly(n, 0.0, 0.25);
                      for (int side = 0; side < 2; ++side) {
                          std::vector<double> t, notes; for (int i = side; i < n; i += 2) { t.push_back(all[static_cast<size_t>(i)]); notes.push_back(scale_step(deg, -i)); }
                          r.add(side ? "right" : "left").modal(modes_bell(), c.p("pitch"), t).hit_notes(notes).hit_jitter(0.0, 0.3).decay_scale(0.8 + 2.2 * c.p("decay")).place(side ? 0.7 : -0.7, c.p("width") * 0.3).fade(0.2, 25);
                      }
                  });

                d("Pitch-Bent Chime", OneShot(), "A chime built from sine partials at bell ratios whose pitch bends down after the strike.",
                  {"chime", "bend", "pitch", "sine"}, {"duration=1.6", "pitch=84", "range=2", "decay=0.5", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("chime").osc(Wave::Sine, Traj(c.p("pitch")).to(0.25, c.p("pitch") - c.p("range"), Interp::Exp)).stack({ 1.0, 2.4, 3.0, 4.5 }).lp(c.lg(c.p("brightness"), 5000, 16000))
                          .shape(Traj(1.0).to(0.003, 1.0).to(1.0, 0.002 + 0.01 * (1.0 - c.p("decay")), Interp::Exp)).place(0, 0.3).fade(0.1, 20);
                  });

                d("Reverse Sparkle", Evolving(), "Glass pings with a long reverb tail, reversed: the glitter builds up and cuts off.",
                  {"sparkle", "reverse", "glass", "swell"}, {"duration=2", "pitch=96", "density=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("pings").modal(modes_glass(), c.p("pitch"), c.scattered(static_cast<int>(5 + 14 * c.p("density")), 2, 0.0, 0.3)).hit_notes({ 0, 4, 7, 12, 2, 9 }).hit_jitter(0.0, 0.6).decay_scale(0.3).hp(1500)
                          .len(c.duration).reverb(2.2, 0.35, 1.3).reverse().place(0, c.p("width")).fade(1, 3);
                  }, true);

                d("Metallic Ping", OneShot(), "A single high, bright strike of inharmonic metal modes with a very short decay.",
                  {"ping", "metallic", "short", "bright"}, {"duration=0.6", "pitch=88", "decay=0.4", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("ping").modal(modes_metal(), c.p("pitch"), { 0.0 }).decay_scale(0.12 + 0.5 * c.p("decay")).strike(1.0).hp(900).lp(c.lg(c.p("brightness"), 6000, 18000)).place(0, 0.3).fade(0.05, 15);
                  });

                d("Glissando Flourish", OneShot(), "A fast harp-like run of plucked strings sweeping up the chosen scale.",
                  {"flourish", "glissando", "pluck", "scale"}, {"duration=1.4", "pitch=60", "scale=1", "steps=14", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("harp").pluck(c.p("pitch"), c.evenly(static_cast<int>(c.p("steps")), 0.0, 0.5), 1.0 - c.p("brightness")).hit_notes(c.scale_walk(static_cast<int>(c.p("steps")))).decay_scale(1.5).hit_jitter(0.0, 0.2).place(0, 0.8).fade(0.2, 20);
                  });

                d("Microtonal Twinkle", OneShot(), "Quarter-tone-spaced glass pings in random order: the pitches sit between the keys.",
                  {"twinkle", "microtonal", "glass", "random"}, {"duration=1.6", "pitch=90", "spread=0.5", "density=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      std::vector<double> n; for (int i = -4; i <= 4; ++i) n.push_back(0.5 * i * (0.5 + c.p("spread")) * 2.0);
                      Rng g = c.rng(5); for (size_t i = n.size(); i > 1; --i) std::swap(n[i - 1], n[static_cast<size_t>(g.index(static_cast<int>(i)))]);
                      r.add("twinkle").modal(modes_glass(), c.p("pitch"), c.scattered(static_cast<int>(6 + 18 * c.p("density")), 3, 0.0, 0.85)).hit_notes(n).hit_jitter(0.0, 0.6).decay_scale(0.3).hp(1500).place(0, c.p("width")).fade(0.1, 15);
                  });

                d("Digital Chime", OneShot(), "Two-operator FM bell (modulator at ratio 3.5, index falling towards zero) with an echo: a clean, electric chime.",
                  {"chime", "fm", "digital", "echo"}, {"duration=1.4", "pitch=79", "intensity=0.6", "brightness=0.7"},
                  [](Rc r, Cc c) {
                      r.add("fm").osc(Wave::Sine, c.p("pitch")).fm(Traj::exp(1.0 + 7.0 * c.p("intensity"), 0.05), 3.5).lp(c.lg(c.p("brightness"), 5000, 16000)).shape(Traj::ar(0.003, 1.0, 0.002)).len(c.duration).ring(0.8)
                          .echo(0.19, 0.4, 0.7, 0.3).place(0, 0.4).fade(0.1, 20);
                  });

                d("Glass Shimmer", Evolving(), "A slowly swelling stack of slightly stretched (inharmonic) high partials with tremolo and reverb: a glassy bloom.",
                  {"shimmer", "glass", "inharmonic", "swell"}, {"duration=3", "tail=1.5", "pitch=80", "brightness=0.7", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("glass").additive(c.p("pitch"), { 0.0, 7.0 }, 9, 0.9).inharmonic(0.002).spread(Traj(9)).trem(6.0, 0.35).hp(900).lp(c.lg(c.p("brightness"), 6000, 17000)).shape(env_swell(0.6, 0.02, 0.003)).len(c.duration).ring(c.p("tail"))
                          .reverb(1.5 + c.p("tail"), 0.4, 0.6).place(0, c.p("width")).fade(5, 20);
                  });

                d("Arpeggiated Flourish", OneShot(), "A quick arpeggio of the chosen chord over several octaves, plucked and slightly detuned.",
                  {"flourish", "arpeggio", "chord", "pluck"}, {"duration=1.2", "pitch=60", "chord=4", "steps=8", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const auto ch = c.chord_notes(); std::vector<double> notes; for (int i = 0; i < n; ++i) notes.push_back(ch[static_cast<size_t>(i) % ch.size()] + 12.0 * (i / static_cast<int>(ch.size())));
                      r.add("arp").pluck(c.p("pitch"), c.evenly(n, 0.0, 0.55), 1.0 - c.p("brightness")).hit_notes(notes).decay_scale(1.2).hit_jitter(0.0, 0.25).place(0, 0.6).fade(0.2, 20);
                  });

                d("Falling Chimes", OneShot(), "Glass-like chimes stepping down the scale with widening gaps and a soft echo: a slow, sparse fall.",
                  {"chimes", "falling", "glass", "descending"}, {"duration=3", "pitch=86", "scale=4", "steps=8", "decay=0.7"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const auto deg = scale_degrees(static_cast<int>(c.p("scale"))); std::vector<double> notes, times; double t = 0.0, gap = 0.05;
                      for (int i = 0; i < n; ++i) { notes.push_back(scale_step(deg, -i)); times.push_back(t); t += gap; gap *= 1.35; }
                      for (auto& x : times) x = x / (t) * 0.85;
                      r.add("chimes").modal(modes_glass(), c.p("pitch"), times).hit_notes(notes).decay_scale(0.5 + 1.5 * c.p("decay")).hit_jitter(0.0, 0.25).echo(0.31, 0.4, 0.5, 0.5).place(0, 0.7).fade(0.2, 25);
                  });

                d("Rising Chimes", OneShot(), "Bell hits climbing the scale with shrinking gaps: the mirror image of Falling Chimes.",
                  {"chimes", "rising", "bells", "ascending"}, {"duration=3", "pitch=66", "scale=4", "steps=8", "decay=0.7"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(c.p("steps")); const auto deg = scale_degrees(static_cast<int>(c.p("scale"))); std::vector<double> notes, times; double t = 0.0, gap = 0.4;
                      for (int i = 0; i < n; ++i) { notes.push_back(scale_step(deg, i)); times.push_back(t); t += gap; gap *= 0.74; }
                      for (auto& x : times) x = x / (t) * 0.8;
                      r.add("chimes").modal(modes_bell(), c.p("pitch"), times).hit_notes(notes).decay_scale(0.5 + 1.5 * c.p("decay")).hit_jitter(0.0, 0.25).place(0, 0.7).fade(0.2, 25);
                  });

                d("Randomized Pluck Cloud", OneShot(), "Many plucked strings at random times and random notes of the chosen scale, panned across the field.",
                  {"pluck", "cloud", "random", "scale"}, {"duration=2.5", "pitch=64", "scale=5", "density=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      const int n = static_cast<int>(10 + 30 * c.p("density")); const auto deg = scale_degrees(static_cast<int>(c.p("scale"))); Rng g = c.rng(9); std::vector<double> notes;
                      for (int i = 0; i < n; ++i) notes.push_back(scale_step(deg, g.index(14) - 3));
                      r.add("cloud").pluck(c.p("pitch"), c.scattered(n, 4, 0.0, 0.9), 0.35).hit_notes(notes).hit_jitter(0.0, 0.6).decay_scale(0.8).place(0, c.p("width")).fade(0.2, 20);
                  });

                d("Harmonic Droplets", OneShot(), "Short, round plinks whose pitches follow the harmonic series, like water drops of different sizes.",
                  {"droplets", "harmonic", "plink", "water"}, {"duration=1.8", "pitch=48", "density=0.5", "decay=0.4", "width=1"},
                  [](Rc r, Cc c) {
                      std::vector<Mode> m = { { 1.0, 0.16, 1.0 }, { 2.0, 0.08, 0.25 } };
                      r.add("drops").modal(m, c.p("pitch"), c.scattered(static_cast<int>(5 + 14 * c.p("density")), 6, 0.0, 0.9)).hit_notes({ 0, 12, 19, 24, 28, 31, 34, 36 }).hit_jitter(0.0, 0.7).decay_scale(0.5 + 2.0 * c.p("decay")).strike(0.6)
                          .place(0, c.p("width")).fade(0.1, 15);
                  });

                d("Tiny Impact Cluster", OneShot(), "A burst of micro-hits: band-passed noise ticks and wood-mode knocks piled into a short accelerating cluster.",
                  {"impact", "cluster", "ticks", "short"}, {"duration=0.9", "pitch=84", "density=0.7", "brightness=0.6", "width=0.9"},
                  [](Rc r, Cc c) {
                      r.add("ticks").noise(NoiseColor::White).bp(c.lg(c.p("brightness"), 2500, 7000), 2.0).pulses(Traj::exp(10 + 10 * c.p("density"), 70), 14.0, 1.0, 0.3).shape(Traj(1.0).to(0.8, 0.9).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(0.1, 10);
                      r.add("knocks").modal(modes_wood(), c.p("pitch") - 12, c.scattered(10, 7, 0.0, 0.8)).hit_notes({ 0, 5, 7, 12 }).hit_jitter(0.0, 0.5).decay_scale(0.5).gain(0.7).place(0, c.p("width")).fade(0.1, 10);
                  });
            }
        }
    }
}
