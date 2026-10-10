#pragma once

#include "voice.hpp"

namespace wauvio {
    namespace vocal {
        struct PitchNote {
            double start = 0.0, end = 0.0;
            double midi  = 60.0;
            bool   connected = false;         // previous note ends right where this one starts
            bool   followed_by_gap = true;    // no note immediately follows
            Style  style;
            Curve  bend;                      // semitones vs seconds-from-note-start
            double glide_from = -1.0;
            int    index = 0;
        };

        constexpr double PITCH_RATE = 1000.0; // control-rate samples per second

        struct PitchTrack {
            std::vector<float> midi;          // fractional MIDI note per 1 ms
            float at(double t) const {
                if (midi.empty()) return 60.f;
                double x = t * PITCH_RATE;
                if (x <= 0) return midi.front();
                size_t i = static_cast<size_t>(x);
                if (i + 1 >= midi.size()) return midi.back();
                float f = static_cast<float>(x - static_cast<double>(i));
                return midi[i] + (midi[i + 1] - midi[i]) * f;
            }
        };

        inline double smoothstep(double x) { x = x < 0 ? 0 : (x > 1 ? 1 : x); return x * x * (3 - 2 * x); }

        inline PitchTrack build_pitch_track(const std::vector<PitchNote>& notes, double total_seconds,
                                            const VoiceModel& voice, const Curve& phrase_curve)
        {
            PitchTrack tr;
            const size_t n = static_cast<size_t>(total_seconds * PITCH_RATE) + 2;
            tr.midi.assign(n, notes.empty() ? 60.f : static_cast<float>(notes.front().midi));
            if (notes.empty()) return tr;

            const double dt = 1.0 / PITCH_RATE;
            size_t active = 0;
            double x = notes.front().midi - notes.front().style.scoop, v = 0.0;
            bool first = true;

            for (size_t k = 0; k < n; ++k) {
                const double t = static_cast<double>(k) * dt;

                // active note = latest note whose entry has begun
                while (active + 1 < notes.size()) {
                    const PitchNote& nx = notes[active + 1];
                    const double P = std::max(0.005, static_cast<double>(nx.style.portamento));
                    const double switch_t = nx.connected ? nx.start - 0.25 * P
                                                         : std::max(notes[active].end, nx.start - 0.10);
                    if (t >= switch_t) {
                        ++active;
                        if (!nx.connected && nx.glide_from < 0.0) {   // silent gap: reset the tracker
                            x = nx.midi - nx.style.scoop; v = 0.0;
                        }
                    } else break;
                }
                const PitchNote& nt = notes[active];
                const double P = std::max(0.005, std::min<double>(nt.style.portamento,
                                  0.45 * std::max(0.02, nt.end - nt.start)));
                double prev_pitch = nt.midi;
                if (nt.glide_from >= 0.0) prev_pitch = nt.glide_from;
                else if (active > 0 && nt.connected) prev_pitch = notes[active - 1].midi;
                else prev_pitch = nt.midi - nt.style.scoop;

                double r;                               // ramp progress 0..1
                if (nt.connected || nt.glide_from >= 0.0) {
                    r = (t - (nt.start - 0.25 * P)) / P;
                } else {
                    const double len = std::max(0.04, P);
                    r = (t - nt.start) / len;
                }
                r = r < 0 ? 0 : (r > 1 ? 1 : r);
                double target = prev_pitch + (nt.midi - prev_pitch) * r;

                // exit sag on phrase-final / pre-rest notes
                if (nt.followed_by_gap && nt.end - nt.start > 0.2 && nt.style.exit_fall != 0.f) {
                    const double fall_t = 0.14;
                    const double u = (t - (nt.end - fall_t)) / fall_t;
                    target -= nt.style.exit_fall * smoothstep(u);
                }
                // per-note bend (smoothed by the tracker, so note changes stay continuous)
                if (!nt.bend.empty() && t >= nt.start) target += curve_eval(nt.bend, t - nt.start);

                if (first) { x = target; first = false; }

                // 2nd-order tracker
                const double resp = nt.style.pitch_response;
                const double wn   = TWO_PI * (4.0 + 26.0 * std::min(1.0, std::max(0.0, resp)));
                const double zeta = std::max(0.28, 1.0 - 1.2 * std::min(1.0, std::max(0.0, (double)nt.style.overshoot)));
                const double acc  = wn * wn * (target - x) - 2.0 * zeta * wn * v;
                v += acc * dt;
                x += v * dt;

                double out = x;

                // vibrato (belongs to the note whose time span contains t)
                {
                    size_t vi = active;
                    // while the previous note is still sounding (consonant lead of next), keep its vibrato
                    if (active > 0 && t < notes[active - 1].end && t < nt.start) vi = active - 1;
                    const PitchNote& vn = notes[vi];
                    double depth = vn.style.vibrato_depth >= 0 ? vn.style.vibrato_depth : voice.vib_depth_st;
                    double rate  = vn.style.vibrato_rate  >= 0 ? vn.style.vibrato_rate  : voice.vib_rate_hz;
                    double dly   = vn.style.vibrato_delay >= 0 ? vn.style.vibrato_delay : voice.vib_delay;
                    double fade  = vn.style.vibrato_fade  >= 0 ? vn.style.vibrato_fade  : voice.vib_fade;
                    const double tn = t - vn.start;
                    if (depth > 0.0 && tn > dly && t < vn.end) {
                        double env = smoothstep((tn - dly) / std::max(0.02, fade));
                        env *= smoothstep((vn.end - t) / 0.06);
                        const double m = vn.style.micro;
                        const double ph = 0.7 * vn.index;
                        const double r2 = rate * (1.0 + 0.05 * m * std::sin(TWO_PI * 0.31 * tn + ph));
                        const double d2 = depth * (1.0 + 0.10 * m * std::sin(TWO_PI * 0.53 * tn + 2.0 * ph));
                        out += d2 * env * std::sin(TWO_PI * r2 * (tn - dly));
                    }
                }
                // slow drift
                if (nt.style.drift != 0.f)
                    out += nt.style.drift * (0.6 * std::sin(TWO_PI * 0.23 * t + 1.3) + 0.4 * std::sin(TWO_PI * 0.61 * t + 0.4));
                // phrase-level shaping
                if (!phrase_curve.empty()) out += curve_eval(phrase_curve, t);

                tr.midi[k] = static_cast<float>(out);
            }
            return tr;
        }

    }
}
