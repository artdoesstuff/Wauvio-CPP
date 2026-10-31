#pragma once

#include "pitch.hpp"
#include "../wauvio_ext/core/core.hpp"   // audio::Articulation

#include <array>
#include <memory>

namespace wauvio {
    namespace vocal {
        struct Tune {
            float out_scale     = 0.65f;   // overall level of voiced signal
            float asp_gain      = 3.00f;   // aspiration noise level into the tract
            float fric_gain     = 0.30f;   // frication level scale
            float upper_gain    = 2.00f;   // parallel upper-formant bank
            float sing_gain     = 2.00f;   // singer's formant
            float air_gain      = 0.10f;   // un-filtered breath "air"
            float pre_gain      = 1.0f;    // source pre-emphasis amount (x brightness)
            float f23_gain      = 30.0f;    // parallel boost of the moving F2/F3 (x brightness)
        };
        inline Tune& tune() { static Tune t; return t; }

        //  LF glottal flow-derivative tables, parameterised by Rd (Fant 1995)
        //  Rd ~0.4 pressed/tense, ~1.0 modal, ~2 breathy.
        class GlottalTables {
        public:
            static constexpr int N  = 1024;
            static constexpr int NR = 12;
            static constexpr float RD_MIN = 0.35f, RD_MAX = 2.55f;

            GlottalTables() {
                for (int k = 0; k < NR; ++k) {
                    const double rd = RD_MIN + (RD_MAX - RD_MIN) * k / (NR - 1);
                    make(rd, tab_[k]);
                }
            }
            // phase in [0,1)
            float read(float phase, float rd) const {
                float x = (rd - RD_MIN) / (RD_MAX - RD_MIN) * (NR - 1);
                x = x < 0 ? 0 : (x > NR - 1 ? NR - 1 : x);
                int k = static_cast<int>(x); if (k >= NR - 1) k = NR - 2;
                const float f = x - k;
                const float p = phase * N;
                int i = static_cast<int>(p); const float pf = p - i;
                i &= (N - 1); const int j = (i + 1) & (N - 1);
                const float a0 = tab_[k][i]     + (tab_[k][j]     - tab_[k][i])     * pf;
                const float a1 = tab_[k + 1][i] + (tab_[k + 1][j] - tab_[k + 1][i]) * pf;
                return a0 + (a1 - a0) * f;
            }
        private:
            std::array<std::array<float, N>, NR> tab_;

            static void make(double Rd, std::array<float, N>& out) {
                const double Rap = (-1.0 + 4.8 * Rd) / 100.0;
                const double Rkp = (22.4 + 11.8 * Rd) / 100.0;
                const double Rgp = Rkp / (4.0 * (0.11 * Rd / (0.5 + 1.2 * Rkp) - Rap));
                const double T0 = 1.0, Tp = T0 / (2.0 * Rgp), Te = Tp * (Rkp + 1.0), Ta = Rap * T0;
                const double w = PI / Tp;
                // epsilon: eps*Ta = 1 - exp(-eps*(T0-Te))
                double eps = 1.0 / Ta;
                for (int it = 0; it < 60; ++it) {
                    const double e = std::exp(-eps * (T0 - Te));
                    const double f = eps * Ta - 1.0 + e;
                    const double df = Ta - (T0 - Te) * e;
                    eps -= f / df;
                    if (eps < 1.0) eps = 1.0;
                }
                auto area = [&](double a) {
                    const double s = std::sin(w * Te), c = std::cos(w * Te), ea = std::exp(a * Te);
                    const double open = (ea * (a * s - w * c) + w) / (a * a + w * w);
                    const double Ee = -ea * s;
                    const double d = T0 - Te, ee = std::exp(-eps * d);
                    const double ret = -(Ee / (eps * Ta)) * ((1.0 - ee) / eps - d * ee);
                    return open + ret;
                };
                double lo = -60.0, hi = 60.0, flo = area(lo);
                // scan for a sign change, then bisect
                double a_prev = lo, f_prev = flo; bool found = false;
                for (double a = lo + 0.25; a <= hi; a += 0.25) {
                    const double f = area(a);
                    if ((f_prev < 0) != (f < 0)) { lo = a_prev; hi = a; found = true; break; }
                    a_prev = a; f_prev = f;
                }
                double alpha = 0.0;
                if (found) {
                    double flo2 = area(lo);
                    for (int it = 0; it < 80; ++it) {
                        const double mid = 0.5 * (lo + hi), fm = area(mid);
                        if ((fm < 0) == (flo2 < 0)) { lo = mid; flo2 = fm; } else hi = mid;
                    }
                    alpha = 0.5 * (lo + hi);
                }
                const double Ee = -std::exp(alpha * Te) * std::sin(w * Te);
                double mean = 0.0;
                for (int i = 0; i < N; ++i) {
                    const double t = static_cast<double>(i) / N;
                    double e;
                    if (t < Te) e = std::exp(alpha * t) * std::sin(w * t);
                    else        e = -(Ee / (eps * Ta)) * (std::exp(-eps * (t - Te)) - std::exp(-eps * (T0 - Te)));
                    out[i] = static_cast<float>(e / Ee);
                    mean += out[i];
                }
                mean /= N;
                for (int i = 0; i < N; ++i) out[i] -= static_cast<float>(mean);
                // Normalise RMS so loudness doesn't swing wildly with Rd; the peak ratio
                // (spectral tilt) is what changes the timbre.
                double rms = 0.0; for (int i = 0; i < N; ++i) rms += out[i] * out[i];
                rms = std::sqrt(rms / N);
                const float g = static_cast<float>(0.30 / std::max(1e-6, rms));
                for (int i = 0; i < N; ++i) out[i] *= g;
            }
        };
        inline const GlottalTables& glottal_tables() { static const GlottalTables t; return t; }

        //  Parameter vector + key-frames
        enum PIdx { F1, F2, F3, B1, B2, B3, TRK, AMP, VOICE, ASP, FRIC, FRIC_F, FRIC_BW,
                    NASAL, NZ, RD, TILT, HI, SING,
                    RF1, RF2, RF3, RTRK,      // formants of the vowel this state belongs to (loudness reference)
                    N_P };

        struct PV {
            std::array<float, N_P> v{};
            float& operator[](int i) { return v[static_cast<size_t>(i)]; }
            float  operator[](int i) const { return v[static_cast<size_t>(i)]; }
        };
        inline PV lerp_pv(const PV& a, const PV& b, float t) {
            PV o; for (int i = 0; i < N_P; ++i) o.v[static_cast<size_t>(i)] = a.v[static_cast<size_t>(i)] + (b.v[static_cast<size_t>(i)] - a.v[static_cast<size_t>(i)]) * t;
            return o;
        }
        struct Key { double t; PV p; };

        class KeyTrack {
        public:
            std::vector<Key> keys;
            void push(double t, const PV& p) {
                if (!keys.empty() && t < keys.back().t) t = keys.back().t;
                keys.push_back({ t, p });
            }
            PV at(double t) const {
                if (keys.empty()) return PV{};
                if (t >= keys.back().t) return keys.back().p;
                if (t < keys.front().t) return keys.front().p;
                size_t lo = 0, hi = keys.size() - 1;
                while (hi - lo > 1) { size_t m = (lo + hi) / 2; (keys[m].t <= t ? lo : hi) = m; }
                const double span = keys[hi].t - keys[lo].t;
                if (span <= 1e-9) return keys[hi].p;
                const double u = smoothstep((t - keys[lo].t) / span);
                return lerp_pv(keys[lo].p, keys[hi].p, static_cast<float>(u));
            }
            double last_t() const { return keys.empty() ? 0.0 : keys.back().t; }
        };

        //  Timed morae
        struct SynthNote {
            double start = 0.0, dur = 0.5;
            int    midi = 60;
            float  vel = 0.7f;
            Style  style;
            std::vector<Mora> morae;
            audio::Articulation art = audio::Articulation::Sustain;
            Curve  bend;
            double glide_from = -1.0;
        };

        struct Ev {
            Mora m; SurfaceMora sm;
            double t0 = 0, t1 = 0;          // vowel onset .. end of mora
            Style st; float vel = 0.7f;
            audio::Articulation art = audio::Articulation::Sustain;
            Vowel v = Vowel::A;
            bool devoiced = false;
            bool first_in_note = false;
            Surf next_surf = Surf::None;    // for ん place assimilation
        };

        inline std::vector<Ev> build_events(const std::vector<SynthNote>& notes) {
            std::vector<Ev> evs;
            Vowel prev_v = Vowel::A;
            for (const SynthNote& n : notes) {
                std::vector<Mora> ms = n.morae;
                if (ms.empty()) { Mora m; m.longv = true; m.v = prev_v; ms.push_back(m); }
                double gate = 1.0;
                using A = audio::Articulation;
                if (n.art == A::Staccato || n.art == A::Spiccato || n.art == A::ShortStab) gate = 0.45;
                else if (n.art == A::Marcato) gate = 0.88;
                const double end = n.start + n.dur * gate;
                const double step = n.dur / static_cast<double>(ms.size());
                for (size_t i = 0; i < ms.size(); ++i) {
                    Ev e; e.m = ms[i];
                    if (n.art == A::Humming) { e.m = Mora(); e.m.nasal = true; }
                    if (e.m.longv) e.m.v = prev_v;
                    e.sm = surface(e.m);
                    e.v = e.m.v; if (!e.m.nasal && !e.m.geminate) prev_v = e.v;
                    e.t0 = n.start + step * static_cast<double>(i);
                    e.t1 = (i + 1 == ms.size()) ? end : n.start + step * static_cast<double>(i + 1);
                    e.st = n.style; e.vel = n.vel; e.art = n.art; e.first_in_note = (i == 0);
                    evs.push_back(e);
                }
            }
            // neighbours
            for (size_t i = 0; i < evs.size(); ++i) {
                Surf ns = Surf::None;
                for (size_t j = i + 1; j < evs.size(); ++j) {
                    if (evs[j].sm.surf == Surf::GEM) { continue; }
                    ns = evs[j].sm.surf; break;
                }
                evs[i].next_surf = ns;
            }
            // devoicing of short i/u between voiceless consonants
            for (size_t i = 0; i < evs.size(); ++i) {
                Ev& e = evs[i];
                if (!e.st.devoicing || e.m.longv || e.m.nasal || e.m.geminate) continue;
                if (e.v != Vowel::I && e.v != Vowel::U) continue;
                if (!is_obstruent_unvoiced(e.sm.surf)) continue;
                if (e.t1 - e.t0 > 0.11) continue;
                const Surf ns = e.next_surf;
                bool next_ok = (ns == Surf::None) || is_obstruent_unvoiced(ns) || ns == Surf::GEM;
                // only if the next sounding mora actually follows closely (or phrase ends)
                if (next_ok) e.devoiced = true;
            }
            return evs;
        }

        //  Articulation: events -> key-frames
        class Articulator {
        public:
            Articulator(const VoiceModel& vm, int sr) : vm_(vm), sr_(sr) {}

            KeyTrack build(const std::vector<Ev>& evs, double& end_time) {
                KeyTrack kt;
                end_time = 0.0;
                if (evs.empty()) return kt;

                // silence at the start
                PV first = vowel_state(evs.front(), evs.front().v);
                kt.push(0.0, silent_like(first));

                // index of previous sounding event
                int prev_s = -1;
                for (size_t i = 0; i < evs.size(); ++i) {
                    const Ev& e = evs[i];
                    if (e.sm.surf == Surf::GEM) { continue; }   // handled via its neighbours
                    const Ev* prev = prev_s >= 0 ? &evs[static_cast<size_t>(prev_s)] : nullptr;
                    const Ev* gem  = (i > 0 && evs[i - 1].sm.surf == Surf::GEM) ? &evs[i - 1] : nullptr;
                    const Ev* next = nullptr;
                    for (size_t j = i + 1; j < evs.size(); ++j) if (evs[j].sm.surf != Surf::GEM) { next = &evs[j]; break; }
                    const bool next_contig = next && (next->t0 - e.t1 < 0.006) &&
                                             !(i + 1 < evs.size() && evs[i + 1].sm.surf == Surf::GEM);
                    emit(kt, e, prev, gem, next, next_contig);
                    prev_s = static_cast<int>(i);
                    end_time = std::max(end_time, e.t1);
                }
                // final fade to silence
                PV last = kt.at(kt.last_t());
                kt.push(end_time + 0.35, silent_like(last));
                return kt;
            }

        private:
            const VoiceModel& vm_;
            int sr_;

            static PV silent_like(PV p) {
                p[AMP] = 0; p[VOICE] = 0; p[ASP] = 0; p[FRIC] = 0; p[NASAL] = 0; return p;
            }

            float vel_amp(const Ev& e) const {
                const float vr = e.st.velocity_response;
                float a = std::pow(std::max(0.02f, e.vel), 1.6f);
                a = (1.f - vr) + vr * a;
                a = std::pow(a, 1.f - 0.6f * e.st.compression);
                return e.st.volume * (0.18f + 0.82f * a);
            }

            // Steady vowel state for an event (target for vowel v).
            PV vowel_state(const Ev& e, Vowel v) const {
                const VowelTarget& vt = vm_.vowel[static_cast<int>(v)];
                const Style& s = e.st;
                PV p;
                const float sf = 1.f + 0.12f * (s.femininity - 0.5f);
                const float clar = (s.vowel_clarity - 0.5f) * 2.f;
                float f2 = vt.f2;
                if (v == Vowel::I || v == Vowel::E) f2 *= 1.f + 0.045f * clar;
                if (v == Vowel::O)                   f2 *= 1.f - 0.07f * clar;
                const float bws = (1.25f - 0.55f * s.resonance) * (1.f - 0.25f * vm_.synthetic);
                p[F1] = vt.f1 * sf; p[F2] = f2 * sf; p[F3] = vt.f3 * sf;
                p[B1] = vt.b1 * bws; p[B2] = vt.b2 * bws; p[B3] = vt.b3 * bws;
                p[TRK] = vt.f1_track * (0.6f + 0.4f * s.vowel_clarity);
                p[RF1] = p[F1]; p[RF2] = p[F2]; p[RF3] = p[F3]; p[RTRK] = p[TRK];
                const float loud = (e.vel - 0.5f) * s.dynamic_timbre;
                p[AMP] = vel_amp(e) * vt.gain;
                p[VOICE] = 1.f;
                p[ASP] = std::max(0.f, vm_.aspiration + 0.30f * s.breathiness + 0.06f * s.softness - 0.04f * std::max(0.f, loud));
                p[FRIC] = 0; p[FRIC_F] = 5000; p[FRIC_BW] = 2000;
                p[NASAL] = 0; p[NZ] = 1500;
                p[RD] = std::min(2.5f, std::max(0.4f,
                         vm_.rd_base + 0.8f * (0.5f - s.tension) + 0.9f * s.softness + 0.55f * s.breathiness
                         - 0.4f * (s.power - 0.5f) - 0.35f * loud));
                const float bright = clamp01(s.brightness + 0.25f * loud + 0.15f * (s.power - 0.5f));
                p[TILT] = vm_.tilt_hz_min + (vm_.tilt_hz_max - vm_.tilt_hz_min) * std::pow(bright, 1.2f);
                p[HI]   = (0.10f + 0.90f * bright) * (0.7f + 0.6f * vm_.synthetic);
                p[SING] = (0.15f + 0.85f * s.power) * (0.4f + 0.6f * bright);

                using A = audio::Articulation;
                if (e.art == A::Whisper) { p[VOICE] = 0.f; p[ASP] = 1.1f; p[AMP] *= 1.3f; }
                else if (e.art == A::Breath) { p[VOICE] = 0.55f; p[ASP] = 0.65f; }
                if (e.devoiced) { p[VOICE] = 0.f; p[ASP] = 0.9f; p[AMP] *= 0.75f; }
                return p;
            }

            struct Locus { float f1, f2, f3, b1, b2, b3; };
            enum Place { LABIAL, ALVEOLAR, PALATAL, VELAR };

            Locus locus(Place pl, const PV& V, float f1_target) const {
                float f2, f3;
                switch (pl) {
                    case LABIAL:   f2 = 0.5f * 900.f  + 0.5f * V[F2]; f3 = 0.5f * 2400.f + 0.5f * V[F3]; break;
                    case ALVEOLAR: f2 = 0.55f * 1750.f + 0.45f * V[F2]; f3 = 0.6f * 2900.f + 0.4f * V[F3]; break;
                    case PALATAL:  f2 = 0.7f * 2450.f + 0.3f * V[F2]; f3 = 0.6f * 3200.f + 0.4f * V[F3]; break;
                    default:       f2 = 0.6f * V[F2] + 0.4f * 2150.f; f3 = 0.7f * V[F3] + 0.3f * 2900.f; break;
                }
                return { f1_target, f2, f3, 90.f, 140.f, 200.f };
            }

            static void apply_locus(PV& p, const Locus& l) {
                p[F1] = l.f1; p[F2] = l.f2; p[F3] = l.f3; p[B1] = l.b1; p[B2] = l.b2; p[B3] = l.b3; p[TRK] = 0.f;
            }

            PV blend_formants(const PV& a, const PV& b, float t) const {
                PV o = a;
                for (int i : { (int)F1, (int)F2, (int)F3, (int)B1, (int)B2, (int)B3, (int)TRK })
                    o[i] = a[i] + (b[i] - a[i]) * t;
                return o;
            }

            float cons_scale(const Ev& e) const { return 0.45f + 1.1f * e.st.consonant_strength; }

            void emit(KeyTrack& kt, const Ev& e, const Ev* prev, const Ev* gem, const Ev* next, bool next_contig) const {
                const Style& st = e.st;
                const Surf sf = e.sm.surf;
                PV V = vowel_state(e, e.v);
                const double share = e.t1 - e.t0;
                const bool prev_contig = prev && (e.t0 - prev->t1 < 0.006 + (gem ? (gem->t1 - gem->t0) : 0.0) + 0.0);

                if (sf == Surf::MORAIC_N) { emit_nasal_mora(kt, e, prev, next, next_contig, V); return; }

                double T = e.t0;                   // vowel onset
                bool have_cons = (sf != Surf::None);

                // How much lead room is there?
                double room = 1e9;
                if (prev) {
                    const double gap = std::max(0.0, e.t0 - prev->t1);
                    room = gap + 0.45 * (prev->t1 - prev->t0);
                    if (gem) room = 1e9;
                }

                if (have_cons) {
                    // The previous mora's tail is already keyed; evaluate the state at the
                    // beginning of the consonant lead below.
                    double gem_extra = gem ? (gem->t1 - gem->t0) : 0.0;
                    emit_consonant(kt, e, V, room, gem_extra, prev);
                } else {
                    // vowel-only mora
                    const double atk = (0.05 - 0.042 * st.attack_sharpness) + 0.04 * st.softness;
                    if (!prev || e.t0 - prev->t1 > 0.006 || !prev_contig) {
                        // onset from silence
                        PV s = silent_like(V);
                        if (kt.last_t() < T - atk) kt.push(T - atk - 0.002, silent_like(V));
                        kt.push(T - atk, s);
                        if (e.first_in_note && e.m.longv == false) { PV o = V; o[ASP] = std::min(1.f, V[ASP] + 0.25f * st.softness); kt.push(T, o); }
                        else kt.push(T, V);
                    } else {
                        // vowel -> vowel (or long-vowel continuation)
                        const bool same = prev->v == e.v;
                        const PV pv = vowel_state(*prev, prev->v);
                        double trvv = same ? 0.025 : std::min(0.095, 0.45 * std::min(prev->t1 - prev->t0, share));
                        trvv = std::max(0.012, trvv);
                        kt.push(std::max(kt.last_t(), T - 0.25 * trvv), kt.at(std::max(kt.last_t(), T - 0.25 * trvv)));
                        // re-articulation dip when notes change on the same vowel (not for ー)
                        if (!e.m.longv && e.first_in_note) {
                            const float dip = 0.55f * (1.f - st.legato);
                            PV d = lerp_pv(pv, V, 0.5f); d[AMP] *= (1.f - dip);
                            kt.push(T + 0.25 * trvv, d);
                        }
                        kt.push(T + 0.75 * trvv, V);
                    }
                }

                // When another mora follows immediately, it starts its own transition from
                // the current state, so nothing is emitted here.
                if (!(next && next_contig)) {
                    const double rel = std::min(0.5 * share, 0.015 + 0.14 * st.release);
                    kt.push(std::max(kt.last_t(), e.t1 - rel), kt.at(std::max(kt.last_t(), e.t1 - rel)));
                    kt.push(e.t1, silent_like(V));
                }
            }

            void emit_nasal_mora(KeyTrack& kt, const Ev& e, const Ev* prev, const Ev* next, bool next_contig, const PV& Vown) const {
                const Style& st = e.st;
                PV N = nasal_state(e, e.next_surf, prev ? vowel_state(*prev, prev->v) : Vown);
                const bool prev_contig = prev && (e.t0 - prev->t1 < 0.006);
                const double tr = std::min(0.05, 0.35 * (e.t1 - e.t0));
                if (!prev_contig) {
                    PV s = silent_like(N);
                    kt.push(e.t0 - 0.03, s);
                    kt.push(e.t0 + 0.01, N);
                } else {
                    kt.push(std::max(kt.last_t(), e.t0 - 0.5 * tr), kt.at(std::max(kt.last_t(), e.t0 - 0.5 * tr)));
                    kt.push(e.t0 + 0.5 * tr, N);
                }
                if (!(next && next_contig)) {
                    const double rel = std::min(0.5 * (e.t1 - e.t0), 0.02 + 0.14 * st.release);
                    kt.push(std::max(kt.last_t(), e.t1 - rel), N);
                    kt.push(e.t1, silent_like(N));
                }
            }

            PV nasal_state(const Ev& e, Surf next, const PV& ref) const {
                PV p = ref;
                float f2, nz;
                switch (next) {
                    case Surf::P: case Surf::B: case Surf::M: case Surf::FU: f2 = 1150; nz = 1000; break;
                    case Surf::K: case Surf::G:                               f2 = 2000; nz = 1500; break;
                    case Surf::T: case Surf::D: case Surf::N: case Surf::S: case Surf::Z: case Surf::R:
                    case Surf::TS: case Surf::CH: case Surf::J: case Surf::SH: case Surf::NY: f2 = 1700; nz = 1750; break;
                    default:                                                  f2 = 1300; nz = 1250; break;   // uvular ɴ
                }
                if (e.art == audio::Articulation::Humming) { f2 = 1150; nz = 1000; }
                p[F1] = 270; p[B1] = 100; p[F2] = f2; p[B2] = 160; p[F3] = 2600; p[B3] = 260; p[TRK] = 1.f;
                p[AMP] = vel_amp(e) * 0.9f; p[VOICE] = 1; p[ASP] = 0.01f; p[FRIC] = 0;
                p[NASAL] = 1.f; p[NZ] = nz; p[RD] = std::min(2.2f, ref[RD] + 0.1f);
                p[TILT] = 1400.f + 1800.f * clamp01(e.st.brightness); p[HI] = 0.08f; p[SING] = 0.f;
                return p;
            }

            // Returns tr_in (time after vowel onset to reach the vowel target).
            double emit_consonant(KeyTrack& kt, const Ev& e, const PV& V, double room, double gem_extra,
                                  const Ev* prev) const
            {
                const Style& st = e.st;
                const Surf sf = e.sm.surf;
                const double T = e.t0;
                const double share = e.t1 - e.t0;
                const float cs = cons_scale(e);
                const bool palv = e.sm.palatal || e.v == Vowel::I;
                const float AV = V[AMP];

                struct Spec { double clo, bur, rel; bool voiced_clo; };
                Spec sp{ 0, 0, 0, false };
                float fric_f = 5000, fric_bw = 2000, fric_a = 0, burst_a = 0, asp_a = 0;
                Place pl = ALVEOLAR; float loc_f1 = 280;
                enum Kind { STOP, FRICATIVE, AFFR, NASAL_, TAP, GLIDE, ASPIR } kind = STOP;
                bool voiced = false; Vowel glide_v = Vowel::I;

                switch (sf) {
                    case Surf::K: kind = STOP; sp = { 0.040, 0.008, 0.030, false }; pl = VELAR;
                        fric_f = palv ? 3400.f : (e.v == Vowel::E ? 2600.f : (e.v == Vowel::A ? 1900.f : 1300.f));
                        fric_bw = 1500; burst_a = 0.85f; asp_a = 0.80f; break;
                    case Surf::G: kind = STOP; sp = { 0.030, 0.006, 0.014, true }; pl = VELAR; voiced = true;
                        fric_f = palv ? 3000.f : (e.v == Vowel::E ? 2400.f : (e.v == Vowel::A ? 1800.f : 1250.f));
                        fric_bw = 1500; burst_a = 0.35f; break;
                    case Surf::T: kind = STOP; sp = { 0.034, 0.007, 0.024, false }; pl = ALVEOLAR;
                        fric_f = 4800; fric_bw = 3200; burst_a = 1.0f; asp_a = 0.75f; break;
                    case Surf::D: kind = STOP; sp = { 0.028, 0.006, 0.012, true }; pl = ALVEOLAR; voiced = true;
                        fric_f = 4300; fric_bw = 3000; burst_a = 0.40f; break;
                    case Surf::P: kind = STOP; sp = { 0.040, 0.006, 0.020, false }; pl = LABIAL;
                        fric_f = 1100; fric_bw = 1800; burst_a = 0.55f; asp_a = 0.70f; break;
                    case Surf::B: kind = STOP; sp = { 0.030, 0.005, 0.012, true }; pl = LABIAL; voiced = true;
                        fric_f = 1000; fric_bw = 1800; burst_a = 0.28f; break;
                    case Surf::TS: kind = AFFR; sp = { 0.030, 0.005, 0.055, false }; pl = ALVEOLAR;
                        fric_f = 6800; fric_bw = 3600; fric_a = 0.80f; burst_a = 0.9f; break;
                    case Surf::CH: kind = AFFR; sp = { 0.030, 0.005, 0.060, false }; pl = PALATAL;
                        fric_f = 3900; fric_bw = 2300; fric_a = 0.85f; burst_a = 0.8f; break;
                    case Surf::J:  kind = AFFR; sp = { 0.026, 0.005, 0.045, true }; pl = PALATAL; voiced = true;
                        fric_f = 3600; fric_bw = 2200; fric_a = 0.50f; burst_a = 0.35f; break;
                    case Surf::S:  kind = FRICATIVE; sp = { 0, 0, 0.090, false }; pl = ALVEOLAR;
                        fric_f = 7600; fric_bw = 3800; fric_a = 0.85f; break;
                    case Surf::SH: kind = FRICATIVE; sp = { 0, 0, 0.095, false }; pl = PALATAL;
                        fric_f = 3800; fric_bw = 2100; fric_a = 0.95f; break;
                    case Surf::Z:  kind = FRICATIVE; sp = { 0, 0, 0.065, true }; pl = ALVEOLAR; voiced = true;
                        fric_f = 7000; fric_bw = 3600; fric_a = 0.45f; break;
                    case Surf::H:  kind = ASPIR; sp = { 0, 0, 0.065, false }; asp_a = 1.0f; break;
                    case Surf::HY: kind = ASPIR; sp = { 0, 0, 0.065, false }; asp_a = 0.9f; pl = PALATAL;
                        fric_f = 4200; fric_bw = 1900; fric_a = 0.18f; break;
                    case Surf::FU: kind = ASPIR; sp = { 0, 0, 0.070, false }; asp_a = 0.75f; pl = LABIAL;
                        fric_f = 1500; fric_bw = 2600; fric_a = 0.25f; break;
                    case Surf::N:  kind = NASAL_; sp = { 0, 0, 0.050, true }; pl = ALVEOLAR; voiced = true; break;
                    case Surf::NY: kind = NASAL_; sp = { 0, 0, 0.055, true }; pl = PALATAL; voiced = true; break;
                    case Surf::M:  kind = NASAL_; sp = { 0, 0, 0.055, true }; pl = LABIAL; voiced = true; break;
                    case Surf::R:  kind = TAP; sp = { 0, 0, 0.030, true }; pl = ALVEOLAR; voiced = true; break;
                    case Surf::Y:  kind = GLIDE; glide_v = Vowel::I; voiced = true; break;
                    case Surf::W:  kind = GLIDE; glide_v = Vowel::U; voiced = true; break;
                    default: break;
                }
                if (e.sm.palatal && (pl == ALVEOLAR || pl == VELAR || pl == LABIAL) && kind != GLIDE && kind != NASAL_)
                { if (pl == VELAR) fric_f = std::max(fric_f, 3200.f); }

                const double dscale = st.consonant_duration;
                double total = (sp.clo + sp.bur + sp.rel) * dscale;
                if (kind == GLIDE) total = 0.030;
                if (kind == NASAL_ || kind == TAP) total = sp.rel * dscale;

                // fit into the room available (consonant compression for fast notes)
                double s = 1.0;
                if (total > room) s = std::max(0.30, room / total);
                double clo = sp.clo * dscale * s, bur = std::max(0.003, sp.bur * dscale * std::sqrt(s)), rel = sp.rel * dscale * s;
                if (kind == NASAL_ || kind == TAP) rel = total * s;
                if (kind == GLIDE) rel = total * s;
                if (gem_extra > 0.0) {
                    // geminate: closure (or frication) lasts through the sokuon
                    if (kind == STOP || kind == AFFR) clo += gem_extra;
                    else rel += gem_extra;
                }
                double lead = (kind == STOP || kind == AFFR) ? clo + bur + rel : rel;
                if (kind == STOP || kind == AFFR) lead = clo + bur + rel; // rel = VOT / frication
                if (kind == GLIDE) lead = rel;
                double ts = T - lead;
                if (prev && !gem_extra) ts = std::max(ts, prev->t0 + 0.55 * (prev->t1 - prev->t0));
                if (ts > T - 0.004) ts = T - 0.004;

                // starting key: the state we are leaving
                PV from = kt.at(std::max(kt.last_t(), ts));
                const bool prev_gap = !prev || (ts - prev->t1 > 0.004);
                if (prev_gap) {
                    PV s0 = silent_like(V);
                    apply_locus(s0, locus(pl, V, loc_f1));
                    if (kt.last_t() < ts - 0.002) kt.push(ts - 0.002, s0);
                    kt.push(ts, s0);
                } else {
                    kt.push(ts, from);
                }

                const float vs = cs;
                PV L = V; apply_locus(L, locus(pl, V, (kind == STOP || kind == NASAL_) ? 250.f : 330.f));
                L[TRK] = 0;
                PV closed = silent_like(L);
                closed[FRIC_F] = fric_f; closed[FRIC_BW] = fric_bw; closed[NZ] = 1500;
                // voiced closure ("voice bar") : low, muffled, quiet voicing
                PV bar = L; bar[AMP] = AV * 0.42f; bar[VOICE] = 1; bar[ASP] = 0; bar[FRIC] = 0;
                bar[TILT] = 600; bar[HI] = 0; bar[SING] = 0; bar[F1] = 230; bar[B1] = 120; bar[TRK] = 1.f;
                bar[FRIC_F] = fric_f; bar[FRIC_BW] = fric_bw;

                double tr_in = std::min(0.040 * (1.15 - 0.3 * st.vowel_clarity), 0.45 * share);
                if (e.sm.palatal) tr_in = std::min(0.060, 0.5 * share);
                tr_in = std::max(0.014, tr_in);

                auto fricvec = [&](PV p, float a, float f, float bw) { p[FRIC] = a * AV * 1.0f * vs / 1.1f; p[FRIC_F] = f; p[FRIC_BW] = bw; return p; };

                switch (kind) {
                    case STOP: case AFFR: {
                        const double tc1 = ts + clo, tb1 = tc1 + bur;
                        PV c = voiced ? bar : closed;
                        kt.push(ts + std::min(0.010, 0.4 * clo), c);
                        kt.push(tc1, c);
                        PV b = c; b = fricvec(b, burst_a * (kind == AFFR ? 1.0f : 1.0f), fric_f, fric_bw);
                        if (voiced) { b[AMP] = AV * 0.30f; b[TILT] = 3000; b[HI] = 0.2f; }
                        kt.push(tc1 + 0.0007, b);
                        if (kind == AFFR) {
                            PV fr = fricvec(c, fric_a, fric_f, fric_bw);
                            if (voiced) { fr[AMP] = AV * 0.35f; fr[VOICE] = 0.8f; fr[TILT] = 4000; fr[HI] = 0.3f; }
                            kt.push(tb1, fr);
                            PV fr2 = fr; kt.push(T - 0.010, fr2);
                            PV tail = blend_formants(L, V, 0.6f); tail = fricvec(tail, 0.f, fric_f, fric_bw);
                            tail[AMP] = AV * 0.6f; tail[VOICE] = 1; tail[ASP] = V[ASP]; tail[TILT] = V[TILT]; tail[HI] = V[HI]; tail[SING] = V[SING];
                            kt.push(T + 0.006, tail);
                        } else {
                            PV d = blend_formants(L, V, 0.35f);
                            d = fricvec(d, 0.0f, fric_f, fric_bw);
                            d[FRIC] = b[FRIC] * 0.10f;
                            d[AMP] = voiced ? AV * 0.40f : AV; d[VOICE] = voiced ? 1.f : 0.f;
                            d[ASP] = voiced ? V[ASP] : std::min(1.f, asp_a * vs); d[TILT] = V[TILT]; d[HI] = V[HI]; d[SING] = V[SING]; d[RD] = V[RD];
                            kt.push(tb1, d);
                            PV d2 = blend_formants(L, V, 0.75f);
                            d2[AMP] = voiced ? AV * 0.8f : AV; d2[VOICE] = voiced ? 1.f : 0.f;
                            d2[ASP] = voiced ? V[ASP] : std::min(1.f, asp_a * vs * 0.85f);
                            d2[TILT] = V[TILT]; d2[HI] = V[HI]; d2[SING] = V[SING]; d2[RD] = V[RD];
                            kt.push(std::max(tb1 + 0.002, T - 0.004), d2);
                            PV v1 = blend_formants(L, V, 0.9f); v1[AMP] = AV; v1[VOICE] = 1; v1[ASP] = V[ASP];
                            v1[TILT] = V[TILT]; v1[HI] = V[HI]; v1[SING] = V[SING]; v1[RD] = V[RD];
                            kt.push(T + 0.008, v1);
                        }
                        break;
                    }
                    case FRICATIVE: {
                        PV fr = L; fr = fricvec(fr, fric_a, fric_f, fric_bw);
                        fr[AMP] = voiced ? AV * 0.35f : 0.f; fr[VOICE] = voiced ? 0.9f : 0.f; fr[ASP] = 0.f;
                        if (voiced) { fr[TILT] = 3500; fr[HI] = 0.25f; fr[SING] = 0.f; }
                        PV pre = silent_like(fr); pre[FRIC] = 0; pre[FRIC_F] = fric_f; pre[FRIC_BW] = fric_bw;
                        if (!prev_gap) pre = kt.at(kt.last_t()), pre[FRIC_F] = fric_f, pre[FRIC_BW] = fric_bw;
                        kt.push(ts, pre);
                        kt.push(ts + std::min(0.016, 0.3 * rel), fr);
                        kt.push(std::max(ts + 0.017, T - 0.014), fr);
                        PV tail = blend_formants(L, V, 0.5f); tail = fricvec(tail, 0.f, fric_f, fric_bw);
                        tail[FRIC] = fr[FRIC] * 0.15f;
                        tail[AMP] = AV * 0.7f; tail[VOICE] = 1; tail[ASP] = V[ASP]; tail[TILT] = V[TILT]; tail[HI] = V[HI]; tail[SING] = V[SING]; tail[RD] = V[RD];
                        kt.push(T + 0.004, tail);
                        break;
                    }
                    case ASPIR: {
                        // h / ç / ɸ : breath noise shaped by the *following* vowel's formants
                        PV h = V; h[VOICE] = 0; h[ASP] = std::min(1.2f, asp_a * vs); h[AMP] = AV * 0.85f;
                        if (kind == ASPIR && sf == Surf::HY) { h[F2] = 0.5f * (V[F2] + 2600.f); h[B2] = 110.f; }
                        if (sf == Surf::FU) { h[F1] = 330; h[F2] = 1100; h[F3] = 2600; h[TRK] = 0; }
                        h[TILT] = V[TILT]; h[HI] = V[HI] * 0.5f;
                        if (fric_a > 0) { h = fricvec(h, fric_a, fric_f, fric_bw); }
                        PV pre = h; pre[AMP] = 0; pre[ASP] = 0; pre[FRIC] = 0;
                        if (!prev_gap) { pre = kt.at(kt.last_t()); pre[VOICE] = std::min(pre[VOICE], 0.f); }
                        kt.push(ts, prev_gap ? pre : kt.at(kt.last_t()));
                        kt.push(ts + 0.018, h);
                        kt.push(std::max(ts + 0.019, T - 0.012), h);
                        PV v1 = V; v1[ASP] = V[ASP] + 0.15f; v1[VOICE] = 1; kt.push(T + 0.005, v1);
                        break;
                    }
                    case NASAL_: {
                        PV nm = L; nm[F1] = 270; nm[B1] = 100; nm[TRK] = 1.f; nm[AMP] = AV * 0.6f; nm[VOICE] = 1; nm[ASP] = 0.01f; nm[FRIC] = 0;
                        nm[NASAL] = 1; nm[NZ] = (sf == Surf::M) ? 1000.f : (sf == Surf::N ? 1750.f : 2300.f);
                        nm[TILT] = 1400.f + 1600.f * clamp01(st.brightness); nm[HI] = 0.08f; nm[SING] = 0; nm[RD] = V[RD];
                        kt.push(ts + 0.012, nm);
                        kt.push(std::max(ts + 0.013, T - 0.004), nm);
                        PV r = blend_formants(L, V, 0.45f); r[AMP] = AV * 0.9f; r[NASAL] = 0.25f; r[VOICE] = 1; r[ASP] = V[ASP];
                        r[TILT] = V[TILT]; r[HI] = V[HI]; r[SING] = V[SING]; r[RD] = V[RD]; r[NZ] = nm[NZ];
                        kt.push(T + 0.014, r);
                        break;
                    }
                    case TAP: {
                        PV d = L; d[F1] = 340; d[AMP] = AV * 0.28f; d[VOICE] = 1; d[ASP] = V[ASP]; d[FRIC] = 0;
                        d[TILT] = V[TILT] * 0.6f; d[HI] = V[HI] * 0.4f; d[SING] = 0; d[RD] = V[RD];
                        const double tm = ts + 0.45 * lead;
                        kt.push(tm, d);
                        PV b = d; b = fricvec(b, 0.20f, 3800, 3000); b[AMP] = AV * 0.30f;
                        kt.push(tm + 0.006, b);
                        PV r = blend_formants(L, V, 0.55f); r[AMP] = AV * 0.9f; r[VOICE] = 1; r[ASP] = V[ASP];
                        r[TILT] = V[TILT]; r[HI] = V[HI]; r[SING] = V[SING]; r[RD] = V[RD];
                        kt.push(T + 0.004, r);
                        break;
                    }
                    case GLIDE: {
                        // y / w : start from an i-like (or u-like) tract, then move into the vowel
                        PV g = V;
                        const VowelTarget& gt = vm_.vowel[static_cast<int>(glide_v)];
                        const float sfm = 1.f + 0.12f * (st.femininity - 0.5f);
                        g[F1] = gt.f1 * sfm; g[F2] = (glide_v == Vowel::I ? 2600.f : 1000.f) * sfm; g[F3] = gt.f3 * sfm;
                        g[B1] = 70; g[B2] = 120; g[B3] = 170; g[TRK] = 0;
                        g[AMP] = AV * 0.85f; g[VOICE] = 1; g[HI] = V[HI] * 0.7f;
                        if (prev_gap) { PV a = silent_like(g); kt.push(ts, a); }
                        kt.push(T, g);
                        tr_in = std::min(0.075, 0.5 * share);
                        break;
                    }
                    default: break;
                }

                // palatalised mora (きゃ etc.): brief j-like tract between the release and the vowel
                if (e.sm.palatal && kind != GLIDE) {
                    const VowelTarget& it = vm_.vowel[static_cast<int>(Vowel::I)];
                    const float sfm = 1.f + 0.12f * (st.femininity - 0.5f);
                    PV j = V; j[F1] = it.f1 * sfm; j[F2] = 2500.f * sfm; j[F3] = it.f3 * sfm; j[B1] = 70; j[B2] = 120; j[B3] = 170; j[TRK] = 0;
                    j[AMP] = AV * 0.9f; j[VOICE] = 1;
                    kt.push(T + 0.012, j);
                    tr_in = std::min(0.070, 0.5 * share);
                    kt.push(T + 0.012 + 0.4 * tr_in, j);
                }

                // arrive at the vowel target
                const double vt = std::min(T + std::max(tr_in, 0.02), e.t0 + 0.5 * share);
                kt.push(std::max(kt.last_t() + 0.001, vt), V);
                return tr_in;
            }
        };

        // The vocal-tract filter chain. Two instances run in parallel: the real one, and
        // a reference fed with a constant fully-voiced source whose RMS drives a slow
        // loudness normaliser, so loudness does not depend on where harmonics happen to
        // land relative to F1 (vibrato, high notes, vowel changes).
        struct TractChain {
            SVFilter r1, r2, r3, q2, q3, u4, u5, u6, sg;
            float pre_lp = 0.f, tilt_state = 0.f;
            float q2v = 1, q3v = 1, pre_a = 0, tilt_a = 0, k4 = 1, k5 = 1, k6 = 1, ksg = 1;

            TractChain() {
                for (SVFilter* f : { &r1, &r2, &r3 }) f->mode = SVFilter::Mode::LowPass;
                for (SVFilter* f : { &q2, &q3, &u4, &u5, &u6, &sg }) f->mode = SVFilter::Mode::BandPass;
            }
            void set(const VoiceModel& vm, const PV& p, float f1, float f2, float f3, int sr) {
                const float nyq = 0.46f * static_cast<float>(sr);
                const float q1v = std::max(0.7f, f1 / std::max(30.f, p[B1]));
                q2v = std::max(0.7f, f2 / std::max(30.f, p[B2]));
                q3v = std::max(0.7f, f3 / std::max(30.f, p[B3]));
                r1.set_params(f1, q1v, sr); r2.set_params(f2, q2v, sr); r3.set_params(f3, q3v, sr);
                q2.set_params(f2, q2v, sr); q3.set_params(f3, q3v, sr);
                u4.set_params(std::min(vm.f4, nyq), vm.f4 / vm.b4, sr);
                u5.set_params(std::min(vm.f5, nyq), vm.f5 / vm.b5, sr);
                u6.set_params(std::min(vm.f6, nyq), vm.f6 / vm.b6, sr);
                sg.set_params(vm.singer_formant_hz, vm.singer_formant_hz / vm.singer_formant_bw, sr);
                k4 = vm.b4 / vm.f4; k5 = 0.70f * vm.b5 / vm.f5; k6 = 0.45f * vm.b6 / vm.f6;
                ksg = vm.singer_formant_bw / vm.singer_formant_hz;
                const float fs = static_cast<float>(sr);
                pre_a  = 1.f - std::exp(-6.2831853f * 1200.f / fs);
                tilt_a = 1.f - std::exp(-6.2831853f * std::min(nyq, std::max(200.f, p[TILT])) / fs);
            }
            // src: excitation entering the tract; hsrc: voiced-only excitation for the parallel parts
            float tick(float src, float hsrc, float hi, float sing, int sr, const Tune& tn) {
                pre_lp += pre_a * (src - pre_lp);
                const float emph = src + tn.pre_gain * hi * (src - pre_lp);
                tilt_state += tilt_a * (emph - tilt_state);
                float y = r3.tick(r2.tick(r1.tick(tilt_state, sr), sr), sr);
                y += tn.f23_gain * hi * (q2.tick(hsrc, sr) / q2v + 0.8f * q3.tick(hsrc, sr) / q3v);
                const float up = u4.tick(hsrc, sr) * k4 + u5.tick(hsrc, sr) * k5 + u6.tick(hsrc, sr) * k6;
                y += hi * tn.upper_gain * up * 2.2f + sing * tn.sing_gain * sg.tick(hsrc, sr) * ksg * 1.2f;
                return y;
            }
        };

        class VoiceRenderer {
        public:
            VoiceRenderer(const VoiceModel& vm, int sr) : vm_(vm), sr_(sr) {}

            Buffer render(const KeyTrack& kt, const PitchTrack& pt, double total_sec, uint32_t seed = 0xA17E5u) const {
                const size_t n = static_cast<size_t>(total_sec * sr_) + 1;
                Buffer out(n, 0.0f);
                const GlottalTables& gt = glottal_tables();
                const float sr = static_cast<float>(sr_);
                const float nyq = 0.46f * sr;
                const Tune& tn = tune();

                NoiseGenerator nz_asp(seed), nz_fric(seed ^ 0x5bd1e995u), nz_irr(seed ^ 0x9e3779b9u);
                TractChain main_c, ref_c;
                SVFilter fz1, fz2, nzf;
                fz1.mode = fz2.mode = SVFilter::Mode::BandPass; nzf.mode = SVFilter::Mode::BandPass;

                constexpr int B = 16;
                double ph = 0.0;
                float air_lp = 0.f, dc_x = 0.f, dc_y = 0.f, jit = 0.f, shim = 1.f, ms_ref = 7.0f;
                const float ms_a = 1.f - std::exp(-1.f / (0.030f * sr));
                PV prev = kt.at(0.0);
                double f0_prev = 440.0 * std::pow(2.0, (pt.at(0.0) - 69.0) / 12.0);
                const float irr = 0.0008f * (1.f - 0.7f * vm_.synthetic);

                for (size_t s0 = 0; s0 < n; s0 += B) {
                    const size_t blk = std::min<size_t>(B, n - s0);
                    const double t_mid = (static_cast<double>(s0) + 0.5 * static_cast<double>(blk)) / sr;
                    const double t_end = static_cast<double>(s0 + blk) / sr;
                    const PV p = kt.at(t_mid);
                    double f0_next = 440.0 * std::pow(2.0, (pt.at(t_end) - 69.0) / 12.0);
                    f0_next = std::min(1800.0, std::max(60.0, f0_next));
                    const double f0_mid = 0.5 * (f0_prev + f0_next);

                    float f1 = p[F1], f2 = p[F2], f3 = p[F3];
                    if (p[TRK] > 0.f) {                       // keep vowel identity at high pitch
                        const float want = 1.04f * static_cast<float>(f0_mid);
                        if (want > f1) f1 += p[TRK] * (want - f1);
                    }
                    f2 = std::max(f2, f1 * 1.30f + 120.f);
                    f3 = std::max(f3, f2 + 350.f);
                    f1 = std::min(f1, nyq); f2 = std::min(f2, nyq); f3 = std::min(f3, nyq);
                    main_c.set(vm_, p, f1, f2, f3, sr_);
                    {   // reference chain: the vowel's own formants, fixed brightness
                        float g1 = p[RF1], g2 = p[RF2], g3 = p[RF3];
                        if (p[RTRK] > 0.f) { const float want = 1.04f * static_cast<float>(f0_mid); if (want > g1) g1 += p[RTRK] * (want - g1); }
                        g2 = std::max(g2, g1 * 1.30f + 120.f); g3 = std::max(g3, g2 + 350.f);
                        PV rp = p; rp[TILT] = 7000.f; rp[B1] = 110.f; rp[B2] = 130.f; rp[B3] = 190.f;
                        ref_c.set(vm_, rp, std::min(g1, nyq), std::min(g2, nyq), std::min(g3, nyq), sr_);
                    }
                    const float fc = std::min(nyq, std::max(300.f, p[FRIC_F]));
                    const float fq = std::max(0.6f, fc / std::max(300.f, p[FRIC_BW]));
                    fz1.set_params(fc, fq, sr_); fz2.set_params(fc, fq, sr_);
                    nzf.set_params(std::min(nyq, std::max(300.f, p[NZ])), 3.0f, sr_);
                    const float fric_norm = 1.f / fq;

                    for (size_t k = 0; k < blk; ++k) {
                        const float u = (static_cast<float>(k) + 0.5f) / static_cast<float>(blk);
                        const float amp   = prev[AMP]   + (p[AMP]   - prev[AMP])   * u;
                        const float voice = prev[VOICE] + (p[VOICE] - prev[VOICE]) * u;
                        const float asp   = prev[ASP]   + (p[ASP]   - prev[ASP])   * u;
                        const float fric  = prev[FRIC]  + (p[FRIC]  - prev[FRIC])  * u;
                        const float hi    = prev[HI]    + (p[HI]    - prev[HI])    * u;
                        const float sing  = prev[SING]  + (p[SING]  - prev[SING])  * u;
                        const float nas   = prev[NASAL] + (p[NASAL] - prev[NASAL]) * u;
                        const double f0 = f0_prev + (f0_next - f0_prev) * u;

                        ph += f0 * (1.0 + jit) / sr;
                        if (ph >= 1.0) {
                            ph -= 1.0;
                            jit  = irr * nz_irr.tick_white();
                            shim = 1.f + 2.f * irr * nz_irr.tick_white();
                        }
                        const float g = gt.read(static_cast<float>(ph), p[RD]);
                        const float pm = 0.55f + 0.45f * std::cos(static_cast<float>(TWO_PI * (ph - 0.35)));
                        const float wn = nz_asp.tick_white();

                        // reference chain: constant, fully voiced -> loudness estimate
                        const float yr = ref_c.tick(g, g, 0.7f, 0.5f, sr_, tn);
                        ms_ref += ms_a * (yr * yr - ms_ref);
                        const float norm = 0.22f / std::sqrt(std::max(1e-4f, ms_ref));

                        const float hsrc = g * voice * shim;
                        const float src = hsrc + wn * asp * tn.asp_gain * pm * 0.9f;
                        float y = main_c.tick(src, hsrc, hi, sing, sr_, tn);
                        if (nas > 0.001f) y -= 0.85f * nas * (nzf.tick(y, sr_) / 3.0f);
                        air_lp += 0.35f * (wn - air_lp);
                        y += (wn - air_lp) * asp * hi * tn.air_gain * pm / std::max(0.05f, norm);

                        float o = y * norm * amp * tn.out_scale;

                        if (fric > 0.0005f) {
                            float fn = nz_fric.tick_white();
                            fn = fz2.tick(fz1.tick(fn, sr_), sr_) * fric_norm;
                            o += fn * fric * tn.fric_gain * 1.6f * (1.f - 0.45f * voice * (1.f - pm));
                        }
                        const float yo = o - dc_x + 0.995f * dc_y; dc_x = o; dc_y = yo;
                        out[s0 + k] = yo;
                    }
                    prev = p; f0_prev = f0_next;
                }
                return out;
            }

        private:
            const VoiceModel& vm_;
            int sr_;
        };

    }
}
