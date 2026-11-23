#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_vocal(Registry& R) {
                Def d{R, Category::Vocal};

                d("Synthetic Breath", OneShot(), "Whispered noise shaped by three formants: a short rise and fall like a breath out of a mouth that is nearly closed.",
                  {"breath", "whisper", "formant", "short"}, {"duration=1.2", "pitch=50", "brightness=0.6", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("breath").formant(Traj(0).to(1.0, 1.0), c.p("pitch"), 1.0).shift(Traj(c.p("brightness") * 6.0 - 2.0)).shape(Traj(0.02).to(0.35, 1.0, Interp::Smooth).to(1.0, 0.003, Interp::Exp)).place(0, c.p("width")).fade(5, 15);
                  });

                d("Breathy Noise Swell", Evolving(), "A whispered vowel that opens from 'u' to 'a' while its level swells, then cuts: a breath that builds.",
                  {"breath", "swell", "whisper", "vowel"}, {"duration=2.5", "pitch=50", "brightness=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      r.add("swell").formant(Traj::linear(4, 0), c.p("pitch"), 0.9).shift(Traj::linear(-2, 2.0 + 6.0 * c.p("brightness"))).shape(Traj::exp(0.03, 1.0)).place(0, c.p("width")).fade(5, 10);
                  });

                d("Vocal-Like Riser", Evolving(), "A voiced formant tone rising two octaves with the vowel opening from 'u' to 'a' and a growing breath component.",
                  {"vocal-like", "riser", "formant", "pitch"}, {"duration=3", "pitch=48", "range=24", "intensity=0.5", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("voice").formant(Traj::linear(4, 0), Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range")), 0.0).shift(Traj::linear(0, 5.0 * c.p("brightness"))).drive(1.0 + 3.0 * c.p("intensity")).shape(Traj::exp(0.1, 1.0)).place(0, 0.5).fade(2, 10);
                      r.add("air").formant(Traj::linear(4, 0), Traj::linear(c.p("pitch"), c.p("pitch") + c.p("range")), 1.0).shape(Traj::exp(0.05, 1.0)).gain(0.4).place(0, 1).fade(2, 10);
                  });

                d("Formant Transition", Sustained(), "A steady voiced tone whose vowel glides a -> i -> u: movement without any change of pitch.",
                  {"vocal-like", "formant", "vowel", "glide"}, {"duration=2.5", "pitch=55", "brightness=0.5", "intensity=0.4"},
                  [](Rc r, Cc c) {
                      r.add("voice").formant(Traj(0).to(0.5, 2, Interp::Smooth).to(1.0, 4, Interp::Smooth), c.p("pitch")).shift(Traj(c.p("brightness") * 6.0 - 3.0)).drive(1.0 + 2.0 * c.p("intensity")).shape(Traj(0.001).to(0.1, 1.0, Interp::Smooth).to(0.9, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0, 0.4).fade(5, 10);
                  });

                d("Whispery Texture", Sustained(), "A long whispered texture whose formants wander between vowels at random: breath without words.",
                  {"vocal-like", "whisper", "texture", "evolving"}, {"duration=6", "pitch=50", "brightness=0.6", "width=1"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(2); Traj v(g.range(0, 4)); for (int i = 1; i <= 8; ++i) v.to(i / 8.0, g.range(0, 4), Interp::Smooth);
                      r.add("whisper").formant(v, c.p("pitch"), 1.0).shift(Traj(c.p("brightness") * 6.0 - 3.0)).trem(0.4, 0.35).shape(Traj(0.001).to(0.15, 1.0, Interp::Smooth).to(0.85, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0, c.p("width")).fade(5, 10);
                  });

                d("Vocal Chop Burst", OneShot(), "Short voiced vowel bursts in a rhythmic pattern, each with a different vowel: a synthetic chopped-vocal figure.",
                  {"vocal-like", "chop", "rhythmic", "burst"}, {"duration=1.5", "pitch=55", "subdivision=0.25", "steps=6", "intensity=0.5"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(4);
                      r.add("chops").formant(random_stairs(g, static_cast<int>(c.p("steps")), 0, 4, true), random_stairs(g, 4, c.p("pitch") - 3, c.p("pitch") + 7)).drive(1.5 + 2.0 * c.p("intensity"))
                          .gate({ 1, 0, 1, 1, 0, 1, 0, 0 }, c.p("subdivision"), 1.0, 2.0).place(0, 0.5).fade(0.3, 12);
                  });

                d("Stuttered Vocal-Like Texture", OneShot(), "A sustained vowel tone whose buffer is repeated in 50-90 ms slices: a stuck, stammering voice.",
                  {"vocal-like", "stutter", "formant", "repeat"}, {"duration=2", "pitch=52", "intensity=0.7", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("voice").formant(Traj(0).to(1.0, 3.0, Interp::Smooth), Traj::linear(c.p("pitch"), c.p("pitch") + 3)).shift(Traj(c.p("brightness") * 6.0 - 3.0)).stutter(0.05 + 0.04 * (1.0 - c.p("intensity")), 3, 0.5 + 0.4 * c.p("intensity"), 0.1, 1.0)
                          .shape(Traj(1.0).to(0.85, 0.9).to(1.0, 0.01, Interp::Exp)).place(0, 0.5).fade(1, 12);
                  });

                d("Synthetic Inhale", OneShot(), "A whisper with a slow rise and quick cut, its formants lifting as it goes: an inhaled breath.",
                  {"breath", "inhale", "whisper", "formant"}, {"duration=1.1", "pitch=52", "brightness=0.6", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("inhale").formant(Traj::linear(2, 0), c.p("pitch"), 1.0).shift(Traj::linear(-2, 4.0 + 4.0 * c.p("brightness"))).shape(Traj(0.02).to(0.78, 1.0, Interp::Smooth).to(1.0, 0.003, Interp::Exp)).place(0, c.p("width")).fade(5, 10);
                  });

                d("Synthetic Exhale", OneShot(), "A whisper with a quick onset and long fade, its formants sinking: an exhaled breath.",
                  {"breath", "exhale", "whisper", "formant"}, {"duration=1.6", "pitch=48", "brightness=0.5", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("exhale").formant(Traj::linear(0, 3), c.p("pitch"), 1.0).shift(Traj::linear(3.0 + 3.0 * c.p("brightness"), -4)).shape(Traj(0.02).to(0.12, 1.0, Interp::Smooth).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(5, 20);
                  });

                d("Formant Glitch", OneShot(), "A voiced formant tone with random vowel jumps, bit reduction and dropouts: a broken speech-synthesis fragment.",
                  {"vocal-like", "glitch", "formant", "crushed"}, {"duration=1.4", "pitch=57", "bits=6", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      Rng g = c.rng(6);
                      r.add("voice").formant(random_stairs(g, 10, 0, 4, false), random_stairs(g, 6, c.p("pitch") - 5, c.p("pitch") + 5)).crush(c.p("bits"), 2.0 + 4.0 * c.p("intensity")).dropout(8.0 + 12.0 * c.p("intensity"), 8, 40, 1.0)
                          .shape(Traj(1.0).to(0.8, 0.9).to(1.0, 0.005, Interp::Exp)).place(0, 0.5).fade(0.3, 12);
                  });

                d("Vowel Sweep", Evolving(), "A voiced tone that glides up through all five vowels and back, with a slight pitch lift in the middle.",
                  {"vocal-like", "vowel", "sweep", "formant"}, {"duration=3", "pitch=52", "range=5", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("voice").formant(Traj(0).to(0.5, 4, Interp::Smooth).to(1.0, 0, Interp::Smooth), Traj(c.p("pitch")).to(0.5, c.p("pitch") + c.p("range"), Interp::Smooth).to(1.0, c.p("pitch"), Interp::Smooth))
                          .shift(Traj(c.p("brightness") * 5.0 - 2.0)).shape(Traj(0.001).to(0.1, 1.0, Interp::Smooth).to(0.9, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0, 0.4).fade(5, 12);
                  });

                d("Vocal-Like Impact", OneShot(), "A low 'oh' that drops in pitch with a noise breath on the attack and a reverb tail: a hit with a human-ish body.",
                  {"vocal-like", "impact", "formant", "low"}, {"duration=1.4", "tail=1.5", "pitch=45", "intensity=0.6", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("voice").formant(Traj(3).to(1.0, 4.0), Traj::linear(c.p("pitch") + 7, c.p("pitch") - 3)).shift(Traj(c.p("brightness") * 4.0 - 2.0)).drive(1.5 + 3.0 * c.p("intensity")).shape(env_hit(0.01, 0.002)).len(c.duration).ring(c.p("tail"))
                          .reverb(1.5 + c.p("tail"), 0.5, 0.5).place(0, 0.6).fade(1, 20);
                      r.add("breath").formant(Traj(1), c.p("pitch") + 12, 1.0).shape(env_hit(0.005, 0.002)).len(0.25).gain(0.5).place(0, 1).fade(0.5, 10);
                  });

                d("Breath Rhythm", Sustained(), "Whispered breaths on every beat division, each with its own rise and fall: a rhythmic panting texture.",
                  {"breath", "rhythmic", "whisper", "tempo-sync"}, {"duration=4", "pitch=50", "subdivision=0.5", "brightness=0.6", "width=0.8"},
                  [](Rc r, Cc c) {
                      const double step = c.beats(c.p("subdivision")); std::vector<double> t; for (double x = 0.0; x + step * 0.5 < c.duration; x += step) t.push_back(x / c.duration);
                      r.add("breaths").formant(Traj::linear(1, 0), c.p("pitch"), 1.0).shift(Traj(c.p("brightness") * 5.0 - 2.0)).pulses_at(t, std::max(60.0, step * 800.0), std::max(8.0, step * 220.0)).place(0, c.p("width")).fade(2, 12);
                  });

                d("Pitch-Granulated Vocal-Like Texture", Sustained(), "Tiny grains of a sung vowel tone scattered across pitch and position: a shimmering, choir-like granular cloud.",
                  {"vocal-like", "granular", "choir-like", "texture"}, {"duration=6", "pitch=57", "density=0.6", "spread=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("cloud").grain(GrainSource::Vowel, 25 + 110 * c.p("density"), Traj::linear(25, 70), c.p("pitch")).scatter(0.3 + 7.0 * c.p("spread"), 0.9).grain_pos(Traj::linear(0.0, 0.6))
                          .shape(Traj(0.001).to(0.15, 1.0, Interp::Smooth).to(0.85, 1.0).to(1.0, 0.001, Interp::Smooth)).place(0, c.p("width")).fade(5, 12);
                  });

                d("Synthetic Gasp", OneShot(), "A very fast, bright whispered inhale with raised formants: a sharp intake of breath.",
                  {"breath", "gasp", "whisper", "sharp"}, {"duration=0.55", "pitch=60", "brightness=0.7", "width=0.5"},
                  [](Rc r, Cc c) {
                      r.add("gasp").formant(Traj::linear(1, 0), c.p("pitch"), 1.0).shift(Traj::linear(4.0, 6.0 + 4.0 * c.p("brightness"))).hp(900).shape(Traj(0.05).to(0.35, 1.0, Interp::Smooth).to(1.0, 0.002, Interp::Exp)).place(0, c.p("width")).fade(2, 10);
                  });

                d("Vocal Texture Tail", OneShot(), "A short voiced 'ah' that dissolves into a long whispered, reverberant tail.",
                  {"vocal-like", "tail", "reverb", "whisper"}, {"duration=1", "tail=4", "pitch=55", "brightness=0.5", "width=1"},
                  [](Rc r, Cc c) {
                      r.add("voice").formant(Traj(0).to(1.0, 3.0), c.p("pitch"), 0.0).shape(Traj(0.001).to(0.1, 1.0, Interp::Smooth).to(1.0, 0.002, Interp::Exp)).len(c.duration).ring(c.p("tail")).reverb(1.5 + c.p("tail"), 0.6, 1.2)
                          .place(0, c.p("width")).fade(2, 25);
                      r.add("whisper").formant(Traj(0).to(1.0, 3.0), c.p("pitch"), 1.0).shape(Traj(0.01).to(0.3, 0.8, Interp::Smooth).to(1.0, 0.001, Interp::Exp)).len(c.duration).ring(c.p("tail")).reverb(1.5 + c.p("tail"), 0.6, 1.5)
                          .gain(0.4).place(0, 1).fade(2, 25);
                  });
            }
        }
    }
}
