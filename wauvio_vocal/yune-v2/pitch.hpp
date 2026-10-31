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
            float  vel = 0.75f;               // for pitch_loud coupling
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
                                            const VoiceModel& voice, const Curve& phrase_curve, uint32_t seed = 0)
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
                const double P0 = std::max(0.005, std::min<double>(nt.style.portamento,
                                  0.45 * std::max(0.02, nt.end - nt.start)));
                double P = P0;
                double prev_pitch = nt.midi;
                if (nt.glide_from >= 0.0) prev_pitch = nt.glide_from;
                else if (active > 0 && nt.connected) prev_pitch = notes[active - 1].midi;
                else prev_pitch = nt.midi - nt.style.scoop;
                if (nt.style.porta_interval > 0.f && (nt.connected || nt.glide_from >= 0.0))   // wide leaps glide longer
                    P = std::min(P0 * (1.0 + static_cast<double>(nt.style.porta_interval) * std::fabs(nt.midi - prev_pitch) / 12.0),
                                 0.45 * std::max(0.02, nt.end - nt.start));
                double dest = nt.midi;
                if (nt.style.undershoot > 0.f && (nt.connected || nt.glide_from >= 0.0)) {    // fast notes fall short
                    const double k = std::min(1.0, std::max(0.0, (0.2 - (nt.end - nt.start)) / 0.2));
                    dest = nt.midi - (nt.midi - prev_pitch) * static_cast<double>(nt.style.undershoot) * 0.3 * k;
                }

                double r;                               // ramp progress 0..1
                if (nt.connected || nt.glide_from >= 0.0) {
                    r = (t - (nt.start - 0.25 * P)) / P;
                } else {
                    const double len = std::max(0.04, P);
                    r = (t - nt.start) / len;
                }
                r = r < 0 ? 0 : (r > 1 ? 1 : r);
                double target = prev_pitch + (dest - prev_pitch) * r;
                if (nt.style.pitch_loud != 0.f) target += static_cast<double>(nt.style.pitch_loud) * (static_cast<double>(nt.vel) - 0.5);

                // exit sag on phrase-final / pre-rest notes
                if (nt.followed_by_gap && nt.end - nt.start > static_cast<double>(nt.style.exit_fall_min) && nt.style.exit_fall != 0.f) {
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
                if (nt.style.pitch_snap > 0.f) out = x + static_cast<double>(nt.style.pitch_snap) * (std::floor(x + 0.5) - x);   // robotic stepping

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
                        const double nat = vn.style.natural;
                        const double m = std::max<double>(vn.style.micro, nat);
                        const double ph = 0.7 * vn.index;
                        double d2 = depth * (1.0 + 0.10 * m * std::sin(TWO_PI * 0.53 * tn + 2.0 * ph));
                        if (nat > 0.0) d2 *= 1.0 + 0.40 * nat * static_cast<double>(vnoise(seed + 11u + static_cast<uint32_t>(vn.index) * 7u, tn, 0.9));   // depth wanders
                        if (m == 0.0) {
                            out += d2 * env * std::sin(TWO_PI * rate * (tn - dly));
                        } else {
                            // integrate the slowly modulated rate so the phase never jumps
                            const double wm = TWO_PI * 0.31;
                            const double phase = rate * ((tn - dly) + 0.05 * m * (std::cos(wm * dly + ph) - std::cos(wm * tn + ph)) / wm);
                            out += d2 * env * std::sin(TWO_PI * phase);
                        }
                    }
                }
                // tremor (shaky voice): pitch part; the amplitude part lives in the renderer
                {
                    size_t vi = active;
                    if (active > 0 && t < notes[active - 1].end && t < nt.start) vi = active - 1;
                    const PitchNote& vn = notes[vi];
                    if (vn.style.tremor > 0.f && t >= vn.start && t < vn.end) {
                        const double fall = smoothstep((vn.end - t) / 0.06);
                        out += static_cast<double>(vn.style.tremor) * 0.30 * fall *
                               std::sin(TWO_PI * static_cast<double>(vn.style.tremor_rate) * (t - vn.start));
                    }
                }
                // living-voice pitch wander (slow, a few cents) and tiny fast flutter
                if (nt.style.natural > 0.f) {
                    const double nat = nt.style.natural;
                    out += nat * (0.28 * vnoise(seed + 3u, t, 0.6) + 0.14 * vnoise(seed + 5u, t, 2.3) + 0.04 * vnoise(seed + 9u, t, 9.0));
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
