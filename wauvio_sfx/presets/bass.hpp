#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            inline void register_bass(Registry& R) {
                Def d{R, Category::Bass};

                d("Wobble Bass Stab", OneShot(), "A saw-and-sub stab through a high-Q low-pass modulated by a tempo-synced LFO: the dubstep wobble in one hit.",
                  {"bass", "wobble", "stab", "tempo-sync"}, {"duration=1.6", "pitch=33", "subdivision=0.5", "intensity=0.7", "depth=0.8"},
                  [](Rc r, Cc c) {
                      r.add("saw").osc(Wave::Saw, c.p("pitch")).unison(3, 12).sub(0.6).drive(1.5 + 2.0 * c.p("intensity")).lp(700, 3.0 + 9.0 * c.p("intensity"), 4)
                          .filter_lfo(lfo_beats(Mod::Shape::Sine, c.p("subdivision") * 4.0, 1.0 + 2.0 * c.p("depth"))).shape(Traj(1.0).to(0.75, 0.9).to(1.0, 0.003, Interp::Exp)).fade(0.3, 15);
                  });

                d("FM Bass Stab", OneShot(), "A sine carrier phase-modulated by a sine at its own frequency, with a modulation index that decays from bright to pure: a hard, glassy stab.",
                  {"bass", "fm", "stab", "short"}, {"duration=0.9", "pitch=36", "intensity=0.7", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("fm").osc(Wave::Sine, c.p("pitch")).fm(Traj::exp(2.0 + 8.0 * c.p("intensity"), 0.3), 1.0).sub(0.3).lp(c.lg(c.p("brightness"), 2500, 12000)).shape(env_hit(0.004, 0.002)).fade(0.2, 10);
                  });

                d("Reese-Style Sweep", Evolving(), "Two detuned saws drifting against each other through a slowly sweeping low-pass: the beating, phasing bass of drum and bass.",
                  {"bass", "reese", "detuned", "sweep"}, {"duration=3.5", "pitch=33", "intensity=0.6", "brightness=0.5", "width=0.6"},
                  [](Rc r, Cc c) {
                      r.add("reese").osc(Wave::Saw, c.p("pitch")).unison(2, Traj::linear(12, 45)).sub(0.5).drive(1.5 + 2.0 * c.p("intensity")).lp(Traj(250).to(0.5, c.lg(c.p("brightness"), 900, 3500), Interp::Exp).to(1.0, 300, Interp::Exp), 1.5, 4)
                          .shape(Traj(0.9).to(0.85, 1.0).to(1.0, 0.003, Interp::Exp)).place(0, c.p("width")).fade(1, 15);
                  });

                d("Bass Drop", OneShot(), "A falling sine with a growling saw layer whose filter closes behind it, then driven: the drop after a build.",
                  {"bass", "drop", "fall", "growl"}, {"duration=2.2", "pitch=30", "range=36", "intensity=0.7", "sub=0.8"},
                  [](Rc r, Cc c) {
                      r.add("sine").osc(Wave::Sine, Traj(c.p("pitch") + c.p("range")).to(0.25, c.p("pitch"), Interp::Exp)).drive(1.5 + 3.0 * c.p("intensity")).shape(Traj(1.0).to(0.2, 0.85).to(1.0, 0.002, Interp::Exp)).gain(0.5 + 0.5 * c.p("sub")).fade(0.3, 20);
                      r.add("growl").osc(Wave::Saw, Traj(c.p("pitch") + c.p("range")).to(0.25, c.p("pitch") + 12, Interp::Exp)).unison(3, 20).drive(3.0 + 5.0 * c.p("intensity"), DriveP::Fold).lp(Traj::exp(5000, 250), 3.0, 4)
                          .shape(Traj(1.0).to(0.2, 0.8).to(1.0, 0.002, Interp::Exp)).gain(0.6).fade(0.3, 20);
                  });

                d("Distorted Bass Burst", OneShot(), "A static-pitch saw hard-clipped and high-passed slightly: a short, aggressive, fuzzy blast.",
                  {"bass", "distorted", "burst", "fuzz"}, {"duration=0.7", "pitch=36", "intensity=0.8", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("saw").osc(Wave::Saw, c.p("pitch")).unison(2, 15).drive(6.0 + 20.0 * c.p("intensity"), DriveP::Hard).hp(60).lp(c.lg(c.p("brightness"), 1500, 7000), 1.0, 4).shape(env_hit(0.005, 0.003)).fade(0.2, 10);
                  });

                d("Formant Bass Sweep", Evolving(), "A low voiced formant source whose vowel slowly sweeps a to u: the bass seems to open and close its mouth.",
                  {"bass", "formant", "sweep", "vowel"}, {"duration=3", "pitch=33", "intensity=0.6", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("formant").formant(Traj(0).to(0.5, 4.0, Interp::Smooth).to(1.0, 1.0, Interp::Smooth), c.p("pitch")).shift(Traj(-3.0 + 6.0 * c.p("brightness"))).drive(1.5 + 3.0 * c.p("intensity")).lp(5000)
                          .shape(Traj(0.9).to(0.8, 1.0).to(1.0, 0.003, Interp::Exp)).fade(1, 15);
                      r.add("sub").osc(Wave::Sine, c.p("pitch")).gain(0.45).shape(Traj(0.9).to(0.8, 1.0).to(1.0, 0.003, Interp::Exp)).fade(1, 15);
                  });

                d("Resonant Bass Stab", OneShot(), "A saw through a resonant low-pass with a fast-decaying filter envelope: the squelchy, acid-style stab.",
                  {"bass", "resonant", "acid", "stab"}, {"duration=0.8", "pitch=36", "intensity=0.8", "brightness=0.6"},
                  [](Rc r, Cc c) {
                      r.add("saw").osc(Wave::Saw, c.p("pitch")).sub(0.3).lp(Traj::exp(c.lg(c.p("brightness"), 2500, 8000), 180), 6.0 + 18.0 * c.p("intensity"), 4).drive(1.5).shape(Traj(1.0).to(0.6, 0.8).to(1.0, 0.003, Interp::Exp)).fade(0.2, 12);
                  });

                d("Rhythmic Bass Pulse", Sustained(), "A saw-and-sub bass pulsing in time: each beat division gets a short filter-and-level envelope.",
                  {"bass", "pulse", "rhythmic", "tempo-sync"}, {"duration=4", "pitch=33", "subdivision=0.5", "intensity=0.6", "depth=0.9"},
                  [](Rc r, Cc c) {
                      r.add("bass").osc(Wave::Saw, c.p("pitch")).sub(0.7).unison(2, 8).drive(1.5 + 2.0 * c.p("intensity")).lp(1400, 2.0, 4).pulses(c.beat_hz(c.p("subdivision")), 140.0 * c.p("subdivision") * 2.0 + 40.0, c.p("depth"))
                          .shape(Traj(1.0).to(0.9, 1.0).to(1.0, 0.003, Interp::Exp)).fade(0.3, 15);
                  });

                d("Pitch-Sliding Bass", OneShot(), "An 808-style bass: a saturated sine that holds, glides up an interval and rings out.",
                  {"bass", "808", "glide", "slide"}, {"duration=2", "pitch=31", "range=7", "intensity=0.6", "decay=0.6"},
                  [](Rc r, Cc c) {
                      r.add("sine").osc(Wave::Sine, Traj(c.p("pitch")).to(0.25, c.p("pitch")).to(0.45, c.p("pitch") + c.p("range"), Interp::Smooth)).drive(1.5 + 4.0 * c.p("intensity")).lp(900, 0.707, 4)
                          .shape(Traj(1.0).to(0.1, 0.9).to(1.0, 0.002 + 0.02 * (1.0 - c.p("decay")), Interp::Exp)).fade(0.3, 20);
                  });

                d("Sub-Bass Swell", Evolving(), "A sine and a quiet second harmonic that swell in and out slowly; nothing above the bass.",
                  {"bass", "sub", "swell", "pad"}, {"duration=4", "pitch=29", "intensity=0.4", "depth=0.3"},
                  [](Rc r, Cc c) {
                      r.add("sub").osc(Wave::Sine, c.p("pitch")).drive(1.0 + 1.5 * c.p("intensity")).lp(250, 0.707, 4).trem(0.35, c.p("depth") * 0.5).shape(env_swell(0.55, 0.01, 0.002)).fade(5, 20);
                  });

                d("Talking Bass Burst", OneShot(), "A low voiced formant source that jumps between vowels in quick steps, driven: a bass that seems to speak.",
                  {"bass", "formant", "talking", "vowel"}, {"duration=1", "pitch=36", "steps=6", "intensity=0.7"},
                  [](Rc r, Cc c) {
                      r.add("voice").formant(Traj::stairs(0.0, 4.0, static_cast<int>(c.p("steps"))), Traj::linear(c.p("pitch") + 3, c.p("pitch"))).drive(2.0 + 5.0 * c.p("intensity"), DriveP::Asym).hp(50)
                          .shape(Traj(1.0).to(0.7, 0.9).to(1.0, 0.003, Interp::Exp)).fade(0.2, 12);
                  });

                d("Metallic Bass Hit", OneShot(), "Low-pitched inharmonic metal modes with a sub sine under them: a clanging bass hit.",
                  {"bass", "metallic", "clang", "hit"}, {"duration=1.4", "pitch=34", "decay=0.4", "intensity=0.6"},
                  [](Rc r, Cc c) {
                      r.add("metal").modal(modes_metal(), c.p("pitch") + 12, { 0.0 }).decay_scale(0.3 + 1.2 * c.p("decay")).strike(3.0).drive(1.0 + 3.0 * c.p("intensity")).lp(5000).fade(0.2, 15);
                      r.add("sub").osc(Wave::Sine, Traj::linear(c.p("pitch") + 7, c.p("pitch"))).shape(env_hit(0.003, 0.002)).gain(0.8).fade(0.2, 15);
                  });

                d("Growl Sweep", Evolving(), "A saw phase-modulated by a sine an octave below it, wavefolded, then swept: a moving, vocal growl.",
                  {"bass", "growl", "fm", "fold"}, {"duration=3", "pitch=36", "intensity=0.8", "brightness=0.5"},
                  [](Rc r, Cc c) {
                      r.add("growl").osc(Wave::Saw, c.p("pitch")).fm(Traj::linear(1.0, 4.0 + 8.0 * c.p("intensity")), 0.5).drive(Traj::linear(2.0, 2.0 + 8.0 * c.p("intensity")), DriveP::Fold).lp(Traj(400).to(0.5, c.lg(c.p("brightness"), 1500, 6000), Interp::Exp).to(1.0, 400, Interp::Exp), 3.0, 4)
                          .shape(Traj(0.9).to(0.85, 1.0).to(1.0, 0.003, Interp::Exp)).fade(0.5, 15);
                  });

                d("Bass Stutter", OneShot(), "A sustained bass note chopped by a tempo-locked gate and repeating slices.",
                  {"bass", "stutter", "chopped", "tempo-sync"}, {"duration=2", "pitch=34", "subdivision=0.25", "intensity=0.7", "swing=0"},
                  [](Rc r, Cc c) {
                      r.add("bass").osc(Wave::Saw, c.p("pitch")).unison(2, 10).sub(0.6).drive(1.5 + 2.0 * c.p("intensity")).lp(1200, 1.5, 4)
                          .gate(hashed_pattern(c.seed, 8, 0.7), c.p("subdivision"), 1.0, 2.0, c.p("swing")).stutter(c.beats(c.p("subdivision")) * 2.0, 2, 0.3, 0.2, 1.0).shape(Traj(1.0).to(0.9, 1.0).to(1.0, 0.003, Interp::Exp)).fade(0.2, 15);
                  });

                d("Inharmonic Bass Hit", OneShot(), "A sine carrier modulated at a non-integer ratio (1.414) with a decaying index and a sub under it: dense, inharmonic and low.",
                  {"bass", "inharmonic", "fm", "hit"}, {"duration=1.2", "pitch=36", "intensity=0.7", "decay=0.5"},
                  [](Rc r, Cc c) {
                      r.add("fm").osc(Wave::Sine, c.p("pitch") + 12).fm(Traj::exp(2.0 + 8.0 * c.p("intensity"), 0.2), 1.414).lp(5000).shape(env_hit(0.003, 0.002 + 0.01 * (1.0 - c.p("decay")))).fade(0.2, 15);
                      r.add("sub").osc(Wave::Sine, Traj::linear(c.p("pitch") + 5, c.p("pitch"))).shape(env_hit(0.003, 0.002)).gain(0.8).fade(0.2, 15);
                  });

                d("Bass Tail", OneShot(), "A decaying bass note with a darkening feedback echo and a small reverb tail: a bass note that fades into the room.",
                  {"bass", "tail", "echo", "decay"}, {"duration=1.2", "tail=1.8", "pitch=34", "intensity=0.5", "brightness=0.4"},
                  [](Rc r, Cc c) {
                      r.add("bass").osc(Wave::Sine, c.p("pitch")).drive(1.5 + 3.0 * c.p("intensity")).lp(Traj::exp(c.lg(c.p("brightness"), 1000, 5000), 250), 1.0, 4).shape(env_hit(0.003, 0.003)).len(c.duration).ring(c.p("tail"))
                          .echo(0.21, 0.55, 0.8, 0.7).reverb(1.2 + c.p("tail") * 0.5, 0.7, 0.35).fade(0.2, 25);
                  });
            }
        }
    }
}
