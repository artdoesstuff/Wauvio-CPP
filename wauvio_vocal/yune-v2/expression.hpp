#pragma once

#include "events.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace wauvio {
    namespace vocal {

        // ---- Pitch ornaments: named bend curves (semitones vs seconds from note start) -------------------
        //   fall, doit, shakuri, scoop_in, kobushi, turn, trill, mordent
        // `depth` scales the size (1 = typical). Unknown names return an empty curve.
        inline Curve make_ornament(const std::string& name, double dur, float depth = 1.f) {
            const double d = static_cast<double>(depth);
            Curve c;
            auto P = [&](double t, double v) { c.push_back({ std::max(0.0, std::min(dur, t)), v }); };
            if (name == "fall") { P(0, 0); P(std::max(0.0, dur - 0.22), 0); P(dur, -2.2 * d); }
            else if (name == "doit") { P(0, 0); P(std::max(0.0, dur - 0.18), 0); P(dur, 2.5 * d); }
            else if (name == "shakuri") { P(0, -2.6 * d); P(std::min(0.14, 0.4 * dur), 0); P(dur, 0); }
            else if (name == "scoop_in") { P(0, 1.5 * d); P(std::min(0.10, 0.4 * dur), 0); P(dur, 0); }
            else if (name == "kobushi") {                                   // enka-style quick turn at the start
                const double s = std::min(1.0, 0.8 * dur / 0.24);
                P(0, 0); P(0.05 * s, 1.6 * d); P(0.11 * s, -0.5 * d); P(0.17 * s, 0.9 * d); P(0.24 * s, 0); P(dur, 0);
            } else if (name == "turn") {                                    // main-upper-main-lower-main
                const double t0 = 0.25 * dur, s = std::min(1.0, 0.5 * dur / 0.24);
                P(0, 0); P(t0, 0); P(t0 + 0.06 * s, 2.0 * d); P(t0 + 0.12 * s, 0); P(t0 + 0.18 * s, -1.0 * d); P(t0 + 0.24 * s, 0); P(dur, 0);
            } else if (name == "mordent") { P(0, 0); P(0.03, 0); P(0.07, -1.5 * d); P(0.12, 0); P(dur, 0); }
            else if (name == "trill") {                                     // alternate with the upper neighbour at ~7 Hz
                const double t0 = 0.35 * dur, half = 1.0 / 14.0; bool up = true; P(0, 0); P(t0, 0);
                for (double t = t0 + half; t < dur - 0.02; t += half, up = !up) P(t, up ? 1.6 * d : 0.0);
                P(dur, 0);
            }
            return c;
        }

        // ---- Humanisation: small deterministic imperfections --------------------------------------------
        struct Humanize {
            bool   on = false;
            double timing = 0.0;    // s, spread of note-start offsets
            double bias = 0.0;      // s, constant push (+ late / lazy, - ahead)
            double length = 0.0;    // fraction, spread of note-length changes
            double pitch = 0.0;     // semitones, spread of per-note tuning offsets
            double tone = 0.0;      // 0..1: per-note random brightness / breathiness / mouth-opening (no two notes sound identical)
            double entry = 0.0;     // semitones, spread of the pitch error a note starts with (corrected within ~90 ms)
            double pressure = 0.0;  // 0..1: breath pressure sags over a phrase (slightly quieter and flatter toward its end)
            double phrase_gap = 0.30; // s of silence that starts a new phrase
            uint32_t seed = 1;      // different seed = a different "take"
        };

        namespace expr_detail {
            inline float hash01(uint32_t x) { x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16; return static_cast<float>(x & 0xFFFFFF) / 16777216.f; }
            inline float sym3(uint32_t seed, uint32_t i, uint32_t k) {      // ~triangular in [-1,1]
                return (hash01(seed * 7919u + i * 104729u + k * 31u) + hash01(seed * 1299709u + i * 15485863u + k * 7u + 1u) - 1.f);
            }
        }

        inline void apply_humanize(std::vector<SynthNote>& ns, const Humanize& h) {
            if (!h.on) return;
            using expr_detail::sym3;
            for (size_t i = 0; i < ns.size(); ++i) {
                SynthNote& n = ns[i];
                if (n.kind != NoteKind::Normal) continue;
                const uint32_t ii = static_cast<uint32_t>(i);
                const bool conn = i > 0 && std::fabs(n.start - (ns[i - 1].start + ns[i - 1].dur)) < 0.012;
                double sh = h.bias + h.timing * static_cast<double>(sym3(h.seed, ii, 1));
                sh = std::max(-0.4 * n.dur, std::min(0.4 * n.dur, sh));
                if (conn) { SynthNote& p = ns[i - 1]; sh = std::max(-0.4 * p.dur, std::min(0.4 * p.dur, sh)); p.dur += sh; }
                n.start += sh; n.dur -= sh;
                if (h.length > 0.0) n.dur = std::max(0.05, n.dur * (1.0 + h.length * static_cast<double>(sym3(h.seed, ii, 2))));
                if (n.event.empty() && h.tone > 0.0) {
                    auto hold = [&](Ch c, double amt, uint32_t k) { const double v = h.tone * amt * static_cast<double>(sym3(h.seed, ii, k)); n.lanes.push_back({ c, Curve{ { 0.0, v }, { n.dur, v } } }); };
                    hold(Ch::Brightness, 0.20, 5); hold(Ch::Breath, 0.12, 6); hold(Ch::MouthOpen, 0.14, 7);
                }
                if (n.event.empty() && h.entry > 0.0) {
                    const double e = h.entry * static_cast<double>(sym3(h.seed, ii, 4)) * (conn ? 0.5 : 1.0);
                    Curve o{ { 0.0, e }, { std::min(0.09, 0.5 * n.dur), 0.0 }, { n.dur, 0.0 } };
                    n.bend = curve_add(n.bend, o);
                }
                if (h.pitch > 0.0) {
                    const double off = h.pitch * static_cast<double>(sym3(h.seed, ii, 3));
                    Curve o{ { 0.0, off }, { n.dur, off } };
                    n.bend = curve_add(n.bend, o);
                }
            }
        }

        // Breath pressure over phrases: a gentle level and pitch sag that resets at each new phrase.
        inline void apply_pressure(std::vector<SynthNote>& ns, const Humanize& h) {
            if (!h.on || h.pressure <= 0.0 || ns.empty()) return;
            size_t s0 = 0; double last_end = ns[0].start + ns[0].dur;
            auto run = [&](size_t a, size_t b) {
                const double t0 = ns[a].start, t1 = last_end, len = t1 - t0, k = std::min(1.0, len / 4.0) * h.pressure;
                if (len <= 0.0) return;
                for (size_t i = a; i < b; ++i) {
                    SynthNote& n = ns[i]; if (n.kind != NoteKind::Normal || !n.event.empty()) continue;
                    const double u0 = (n.start - t0) / len, u1 = (n.start + n.dur - t0) / len;
                    n.lanes.push_back({ Ch::Volume, Curve{ { 0.0, -0.10 * k * u0 }, { n.dur, -0.10 * k * u1 } } });
                    n.bend = curve_add(n.bend, Curve{ { 0.0, -0.05 * k * u0 * u0 }, { n.dur, -0.05 * k * u1 * u1 } });
                }
            };
            for (size_t i = 1; i < ns.size(); ++i) {
                if (ns[i].start - last_end >= h.phrase_gap) { run(s0, i); s0 = i; }
                last_end = std::max(last_end, ns[i].start + ns[i].dur);
            }
            run(s0, ns.size());
        }

        // ---- Auto-expression: rule-based defaults. Touches only fields still at their Style defaults. ---
        struct AutoExpr {
            bool on = false;
            bool vibrato = true;      // onset/fade/depth follow note length
            bool dynamics = true;     // louder toward the phrase's high point
            bool phrase_end = true;   // longer fall + release on a phrase's last note
            bool articulation = true; // crisper consonants on fast notes
            double phrase_gap = 0.30; // s of silence that starts a new phrase
        };

        inline void apply_auto_expression(std::vector<SynthNote>& ns, const AutoExpr& a, const VoiceModel& voice) {
            if (!a.on || ns.empty()) return;
            const Style D;
            // phrases = runs of normal notes separated by gaps >= phrase_gap
            std::vector<std::pair<size_t, size_t>> ph; size_t s0 = 0; double last_end = ns[0].start + ns[0].dur;
            for (size_t i = 1; i < ns.size(); ++i) {
                if (ns[i].start - last_end >= a.phrase_gap) { ph.push_back({ s0, i }); s0 = i; }
                last_end = std::max(last_end, ns[i].start + ns[i].dur);
            }
            ph.push_back({ s0, ns.size() });
            for (auto& pr : ph) {
                double mn = 1e9, mx = -1e9, sum = 0; int cnt = 0;
                for (size_t i = pr.first; i < pr.second; ++i) if (ns[i].kind == NoteKind::Normal && ns[i].event.empty()) { mn = std::min(mn, double(ns[i].midi)); mx = std::max(mx, double(ns[i].midi)); sum += ns[i].midi; ++cnt; }
                const double mean = cnt ? sum / cnt : 0.0, half = std::max(1.0, 0.5 * (mx - mn));
                for (size_t i = pr.first; i < pr.second; ++i) {
                    SynthNote& n = ns[i]; Style& s = n.style;
                    if (n.kind != NoteKind::Normal || !n.event.empty()) continue;
                    if (a.vibrato && n.dur >= 0.4) {
                        if (s.vibrato_delay < 0.f) s.vibrato_delay = static_cast<float>(std::max(0.12, std::min(0.40, 0.30 * n.dur)));
                        if (s.vibrato_fade < 0.f)  s.vibrato_fade  = static_cast<float>(std::max(0.15, std::min(0.50, 0.45 * (n.dur - s.vibrato_delay))));
                        if (s.vibrato_depth < 0.f) s.vibrato_depth = voice.vib_depth_st * static_cast<float>(smoothstep((n.dur - 0.3) / 0.7));
                    }
                    if (a.dynamics && cnt > 2) n.vel = std::min(1.f, std::max(0.05f, n.vel * (1.f + 0.12f * static_cast<float>(std::max(-1.0, std::min(1.0, (n.midi - mean) / half))))));
                    if (a.phrase_end && i + 1 == pr.second && n.dur >= 0.25) {
                        if (s.exit_fall == D.exit_fall) s.exit_fall = 0.35f;
                        if (s.release == D.release)     s.release = 0.75f;
                    }
                    if (a.articulation && n.dur < 0.18 && s.consonant_strength == D.consonant_strength) s.consonant_strength = std::min(1.f, D.consonant_strength + 0.15f);
                }
            }
        }

        // ---- Phrase arcs: volume shape across a group of notes -------------------------------------------
        inline double phrase_arc_shape(const std::string& kind, double u) {
            u = std::max(0.0, std::min(1.0, u));
            if (kind == "crescendo")   return -0.30 + 0.45 * u;
            if (kind == "decrescendo") return 0.15 - 0.45 * u;
            if (kind == "swell")       return -0.25 + 0.45 * std::sin(PI * u);
            return -0.20 + 0.40 * std::sin(PI * u);                       // "arch" (default)
        }

        // ---- Speech: intonation contours (semitones vs 0..1 across the utterance) ----------------------
        inline double speech_contour(const std::string& kind, double u) {
            u = std::max(0.0, std::min(1.0, u));
            if (kind == "question") return 1.0 * (1.0 - u) - 0.5 + 4.5 * smoothstep((u - 0.7) / 0.3);
            if (kind == "flat")     return 0.0;
            if (kind == "trail")    return -3.0 * u;
            if (kind == "emphatic") return 2.2 * std::sin(PI * u) + 0.5 * (1.0 - u);
            if (kind == "ne")       return 3.5 * smoothstep((u - 0.35) / 0.65);       // ね↗
            if (kind == "yo")       return 1.5 - 3.5 * smoothstep(u);                 // よ↘
            if (kind == "ka")       return 4.0 * smoothstep((u - 0.4) / 0.6);         // か↗
            if (kind == "no")       return 2.0 * smoothstep((u - 0.5) / 0.5);         // の (soft rise)
            return 1.5 * (1.0 - u) - 0.8 - 1.5 * smoothstep((u - 0.8) / 0.2);        // "statement"
        }

    }
}
