#pragma once

#include "pitch.hpp"
#include "expr.hpp"
#include "../../ext/core/core.hpp"   // wauvio::music::Articulation

#include <array>
#include <memory>

namespace wauvio {
    namespace vocal {
        namespace v3 {
            //  LF glottal flow-derivative tables, parameterised by Rd (Fant 1995)
            //  Rd ~0.4 pressed/tense, ~1.0 modal, ~2 breathy.
            class GlottalTables {
            public:
                static constexpr int N  = 2048;     // samples per period
                static constexpr int NR = 12;       // Rd steps
                static constexpr int NL = 9;        // band-limit levels: level l keeps (N/2)>>l harmonics (1024, 512, ... 4)
                static constexpr float RD_MIN = 0.35f, RD_MAX = 2.55f;

                GlottalTables() {
                    std::array<float, N> base;
                    for (int k = 0; k < NR; ++k) {
                        const double rd = RD_MIN + (RD_MAX - RD_MIN) * k / (NR - 1);
                        make(rd, base);
                        for (int l = 0; l < NL; ++l) bandlimit(base, l, tab_[k][l]);
                    }
                }
                /// phase in [0,1); inc = f0 / sample_rate (phase increment per sample). V3: the table is read from
                /// the mip level whose harmonics all fit below Nyquist, so the source cannot alias at high pitch.
                float read(float phase, float rd, float inc) const {
                    float x = (rd - RD_MIN) / (RD_MAX - RD_MIN) * (NR - 1);
                    x = x < 0 ? 0 : (x > NR - 1 ? NR - 1 : x);
                    int k = static_cast<int>(x); if (k >= NR - 1) k = NR - 2;
                    const float f = x - k;
                    // highest harmonic that fits below ~0.95 * Nyquist
                    const float hmax = 0.475f / std::max(inc, 1e-6f);
                    float lf = std::log2(static_cast<float>(N / 2) / std::max(hmax, 1.f));
                    lf = lf < 0.f ? 0.f : (lf > NL - 1 ? static_cast<float>(NL - 1) : lf);
                    int l = static_cast<int>(lf); if (l >= NL - 1) l = NL - 2;
                    const float fl = lf - static_cast<float>(l);
                    const float p = phase * N;
                    int i = static_cast<int>(p); const float pf = p - static_cast<float>(i);
                    i &= (N - 1); const int j = (i + 1) & (N - 1);
                    auto rd_level = [&](int lv) {
                        const float a0 = tab_[k][lv][i]     + (tab_[k][lv][j]     - tab_[k][lv][i])     * pf;
                        const float a1 = tab_[k + 1][lv][i] + (tab_[k + 1][lv][j] - tab_[k + 1][lv][i]) * pf;
                        return a0 + (a1 - a0) * f;
                    };
                    const float v0 = rd_level(l);
                    return fl > 1e-3f ? v0 + (rd_level(l + 1) - v0) * fl : v0;
                }
            private:
                std::array<std::array<std::array<float, N>, NL>, NR> tab_;

                // In-place radix-2 FFT (inverse = conjugate trick done by the caller).
                static void fft(std::vector<double>& re, std::vector<double>& im) {
                    const size_t n = re.size();
                    for (size_t i = 1, j = 0; i < n; ++i) {
                        size_t bit = n >> 1;
                        for (; j & bit; bit >>= 1) j ^= bit;
                        j ^= bit;
                        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
                    }
                    for (size_t len = 2; len <= n; len <<= 1) {
                        const double ang = -2.0 * PI / static_cast<double>(len);
                        const double wr = std::cos(ang), wi = std::sin(ang);
                        for (size_t i = 0; i < n; i += len) {
                            double cr = 1.0, ci = 0.0;
                            for (size_t k = 0; k < len / 2; ++k) {
                                const size_t u = i + k, v = i + k + len / 2;
                                const double xr = re[v] * cr - im[v] * ci, xi = re[v] * ci + im[v] * cr;
                                re[v] = re[u] - xr; im[v] = im[u] - xi;
                                re[u] += xr;        im[u] += xi;
                                const double nr = cr * wr - ci * wi; ci = cr * wi + ci * wr; cr = nr;
                            }
                        }
                    }
                }
                // Keep harmonics 1..H (H = (N/2) >> level) with a short raised-cosine taper at the top, drop the rest.
                static void bandlimit(const std::array<float, N>& src, int level, std::array<float, N>& dst) {
                    if (level == 0) { dst = src; return; }
                    std::vector<double> re(N), im(N, 0.0);
                    for (int i = 0; i < N; ++i) re[static_cast<size_t>(i)] = src[static_cast<size_t>(i)];
                    fft(re, im);
                    const int H = (N / 2) >> level;
                    const int Ht = std::max(1, static_cast<int>(H * 0.85));
                    for (int h = 0; h <= N / 2; ++h) {
                        double w = 1.0;
                        if (h == 0) w = 0.0;
                        else if (h > H) w = 0.0;
                        else if (h > Ht) { const double u = static_cast<double>(h - Ht) / static_cast<double>(H - Ht); w = 0.5 + 0.5 * std::cos(PI * u); }
                        re[static_cast<size_t>(h)] *= w; im[static_cast<size_t>(h)] *= w;
                        if (h > 0 && h < N / 2) { re[static_cast<size_t>(N - h)] *= w; im[static_cast<size_t>(N - h)] *= w; }
                    }
                    for (auto& x : im) x = -x;               // inverse FFT via conjugation
                    fft(re, im);
                    for (int i = 0; i < N; ++i) dst[static_cast<size_t>(i)] = static_cast<float>(re[static_cast<size_t>(i)] / N);
                }

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
                        TRK2, RTRK2,              // F2 follows F0 (very high notes)
                        FSC,                      // V3: tract-size scale applied to the fixed upper formants (F4..F6, singer's formant)
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

            // V3: how each parameter moves between two key-frames.
            //   log   : interpolate geometrically (formant/filter frequencies and bandwidths move proportionally in
            //           pitch space rather than linearly in Hz)
            //   speed : >1 = the parameter lags (starts late), <1 = it leads (starts early). F2/F3 are the fast
            //           articulators (tongue body/front), F1 (jaw) is slower.
            struct ParamMotion { bool log; float speed; };
            inline ParamMotion param_motion(int i) {
                switch (i) {
                    case F1:      return { true, 1.18f };
                    case F2:      return { true, 0.80f };
                    case F3:      return { true, 0.85f };
                    case B1: case B2: case B3: return { true, 1.0f };
                    case FRIC_F: case FRIC_BW: case NZ: case TILT: return { true, 1.0f };
                    case RF1: case RF2: case RF3: return { true, 1.0f };
                    default:      return { false, 1.0f };
                }
            }

            inline PV interp_pv(const PV& a, const PV& b, double u_raw) {
                PV o;
                for (int i = 0; i < N_P; ++i) {
                    const ParamMotion pm = param_motion(i);
                    const double u = smoothstep(pm.speed == 1.0f ? u_raw : std::pow(u_raw, static_cast<double>(pm.speed)));
                    const float x = a.v[static_cast<size_t>(i)], y = b.v[static_cast<size_t>(i)];
                    float r;
                    if (pm.log && x > 1e-3f && y > 1e-3f) r = x * std::pow(y / x, static_cast<float>(u));
                    else r = x + (y - x) * static_cast<float>(u);
                    o.v[static_cast<size_t>(i)] = r;
                }
                return o;
            }

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
                    return interp_pv(keys[lo].p, keys[hi].p, (t - keys[lo].t) / span);
                }
                double last_t() const { return keys.empty() ? 0.0 : keys.back().t; }
            };

            // Kind of note. (An ingressive "Inhale" kind existed in early V2 and was removed: it did not sound right.)
            enum class NoteKind : uint8_t { Normal };

            //  Timed morae
            struct SynthNote {
                double start = 0.0, dur = 0.5;
                int    midi = 60;
                float  vel = 0.7f;
                Style  style;
                std::vector<Mora> morae;
                wauvio::music::Articulation art = wauvio::music::Articulation::Sustain;
                Curve  bend;
                double glide_from = -1.0;
                std::vector<Lane> lanes;   // V2 automation lanes (seconds from note start)
                NoteKind kind = NoteKind::Normal;
                std::string event;         // non-empty: an unexpanded event tag, e.g. "laugh:ha,intensity=0.7"
            };

            struct Ev {
                Mora m; SurfaceMora sm;
                double t0 = 0, t1 = 0;          // vowel onset .. end of mora
                Style st; float vel = 0.7f;
                wauvio::music::Articulation art = wauvio::music::Articulation::Sustain;
                Vowel v = Vowel::A;
                bool devoiced = false;
                bool first_in_note = false;
                Surf next_surf = Surf::None;    // for ん place assimilation
                NoteKind kind = NoteKind::Normal;
                float bid = 1.f;                // ガ行 nasalisation amount (Surf::NG from bidakuon)
                bool  next_vowel = false;       // the next sounding mora starts with a vowel/glide/h (for ん nasalisation)
            };

            /// Cumulative mora boundaries (0..1, size = morae + 1) for one note. Uniform split when mora_weighting is 0
            /// and no explicit weights are given; otherwise weighted by mora type (ん/っ shorter, ー longer).
            inline std::vector<double> mora_cum(const std::vector<Mora>& ms, float mora_weighting) {
                std::vector<double> cum; cum.push_back(0.0);
                bool weighted = mora_weighting > 0.f;
                for (const Mora& m : ms) if (m.weight != 1.f) { weighted = true; break; }
                if (!weighted) {
                    for (size_t i = 0; i < ms.size(); ++i) cum.push_back(static_cast<double>(i + 1) / static_cast<double>(ms.size()));
                    return cum;
                }
                double tot = 0.0;
                for (const Mora& m : ms) {
                    const float typ = m.geminate ? 0.7f : (m.nasal ? 0.85f : (m.longv ? 1.5f : 1.0f));
                    tot += static_cast<double>(m.weight) * (1.0 + static_cast<double>(mora_weighting) * (static_cast<double>(typ) - 1.0));
                    cum.push_back(tot);
                }
                for (double& c : cum) c /= tot;
                return cum;
            }

            inline std::vector<Ev> build_events(const std::vector<SynthNote>& notes) {
                std::vector<Ev> evs;
                Vowel prev_v = Vowel::A;
                bool prev_nasal = false;     // previous sounding mora was ん / hum: ー continues the nasal
                for (const SynthNote& n : notes) {
                    std::vector<Mora> ms = n.morae;
                    if (ms.empty()) { Mora m; m.longv = true; m.v = prev_v; ms.push_back(m); }
                    double gate = 1.0;
                    using A = wauvio::music::Articulation;
                    if (n.art == A::Staccato || n.art == A::Spiccato || n.art == A::ShortStab) gate = 0.45;
                    else if (n.art == A::Marcato) gate = 0.88;
                    const double end = n.start + n.dur * gate;
                    const std::vector<double> cum = mora_cum(ms, n.style.mora_weighting);
                    for (size_t i = 0; i < ms.size(); ++i) {
                        Ev e; e.m = ms[i];
                        if (n.art == A::Humming) { e.m = Mora(); e.m.nasal = true; }
                        if (e.m.longv) { e.m.v = prev_v; if (prev_nasal) e.m.nasal = true; }
                        e.sm = surface(e.m);
                        if (e.sm.surf == Surf::G && n.style.bidakuon > 0.f && !evs.empty() && evs.back().sm.surf != Surf::GEM &&
                            (n.start + n.dur * cum[i]) - evs.back().t1 < 0.05) {
                            e.sm.surf = Surf::NG; e.bid = n.style.bidakuon;                  // word-medial ガ行 -> [ŋ]
                        }
                        e.v = e.m.v; if (!e.m.nasal && !e.m.geminate) prev_v = e.v;
                        if (!e.m.geminate) prev_nasal = e.m.nasal;
                        e.t0 = n.start + n.dur * cum[i];
                        e.t1 = (i + 1 == ms.size()) ? end : n.start + n.dur * cum[i + 1];
                        e.st = n.style; e.vel = n.vel; e.art = n.art; e.first_in_note = (i == 0); e.kind = n.kind;
                        evs.push_back(e);
                    }
                }
                // neighbours
                for (size_t i = 0; i < evs.size(); ++i) {
                    Surf ns = Surf::None; bool nv = false;
                    for (size_t j = i + 1; j < evs.size(); ++j) {
                        if (evs[j].sm.surf == Surf::GEM) { continue; }
                        ns = evs[j].sm.surf;
                        nv = (ns == Surf::None && !evs[j].m.nasal) || ns == Surf::Y || ns == Surf::W || ns == Surf::H || ns == Surf::HY || ns == Surf::FU;
                        break;
                    }
                    evs[i].next_surf = ns; evs[i].next_vowel = nv;
                }
                // devoicing of short i/u between voiceless consonants
                for (size_t i = 0; i < evs.size(); ++i) {
                    Ev& e = evs[i];
                    if (!e.st.devoicing || e.m.longv || e.m.nasal || e.m.geminate) continue;
                    if (e.v != Vowel::I && e.v != Vowel::U) continue;
                    if (!is_obstruent_unvoiced(e.sm.surf)) continue;
                    if (e.t1 - e.t0 > static_cast<double>(e.st.devoice_max)) continue;
                    const Surf ns = e.next_surf;
                    bool next_ok = (ns == Surf::None) || is_obstruent_unvoiced(ns) || ns == Surf::GEM;
                    // only if the next sounding mora actually follows closely (or phrase ends)
                    if (next_ok) e.devoiced = true;
                }
                return evs;
            }

            //  Articulation: events -> key-frames
            /// V3: deterministic variation for one event (a mora): same seed and same time give the same number, a different
            /// take (seed) gives a different one. Roughly triangular in [-1, 1], so most values are small.
            inline float vary(uint32_t seed, double t, uint32_t k) {
                uint32_t x = seed * 2654435761u ^ (static_cast<uint32_t>(t * 1000.0) * 40503u) ^ (k * 0x9E3779B1u);
                x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16;
                return static_cast<float>(x & 0xFFFFu) / 65535.f + static_cast<float>((x >> 16) & 0xFFFFu) / 65535.f - 1.f;
            }

            class Articulator {
            public:
                Articulator(const VoiceModel& vm, int sr, uint32_t seed = 0xA17E5u) : vm_(vm), sr_(sr), seed_(seed) {}

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
                VoiceModel vm_;   // copy: no dangling reference if the caller's model goes away
                int sr_;
                uint32_t seed_;

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
                    const float clar = (s.vowel_clarity - 0.5f) * 2.f - 0.5f * s.mumble;
                    float f2 = vt.f2;
                    if (v == Vowel::I || v == Vowel::E) f2 *= 1.f + 0.045f * clar;
                    if (v == Vowel::O)                   f2 *= 1.f - 0.07f * clar;
                    const float bws = (1.25f - 0.55f * s.resonance) * (1.f - 0.25f * vm_.synthetic);
                    p[F1] = vt.f1 * sf; p[F2] = f2 * sf; p[F3] = vt.f3 * sf;
                    if (s.mumble > 0.f) {                                     // lazy articulation: vowels drift toward the neutral centre
                        const float k = 0.35f * s.mumble;
                        p[F1] += (500.f * sf - p[F1]) * k; p[F2] += (1500.f * sf - p[F2]) * k;
                    }
                    p[B1] = vt.b1 * bws; p[B2] = vt.b2 * bws; p[B3] = vt.b3 * bws;
                    p[TRK] = vt.f1_track * (0.6f + 0.4f * s.vowel_clarity);
                    p[RF1] = p[F1]; p[RF2] = p[F2]; p[RF3] = p[F3]; p[RTRK] = p[TRK];
                    if (s.natural > 0.f) {
                        // V3: every vowel is a little different (target undershoot/overshoot, a slightly different jaw and
                        // tongue position). The reference formants above stay nominal on purpose.
                        const uint32_t k = static_cast<uint32_t>(v);
                        p[F1] *= 1.f + 0.022f * s.natural * vary(seed_, e.t0, 31u + k);
                        p[F2] *= 1.f + 0.018f * s.natural * vary(seed_, e.t0, 41u + k);
                        p[F3] *= 1.f + 0.010f * s.natural * vary(seed_, e.t0, 51u + k);
                    }
                    p[TRK2] = vt.f2_track; p[RTRK2] = p[TRK2];
                    p[FSC] = sf;                                              // V3: tract-size scale for F4..F6 / singer's formant
                    const float loud = (e.vel - 0.5f) * s.dynamic_timbre;
                    p[AMP] = vel_amp(e) * vt.gain * (1.f + 0.07f * s.natural * vary(seed_, e.t0, 61u));   // V3: per-syllable level
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

                    using A = wauvio::music::Articulation;
                    if (e.art == A::Whisper) { p[VOICE] = 0.f; p[ASP] = 1.1f; p[AMP] *= 1.3f; }
                    else if (e.art == A::Breath) { p[VOICE] = 0.55f; p[ASP] = 0.65f; }
                    if (e.devoiced) { p[VOICE] = 0.f; p[ASP] = 0.9f; p[AMP] *= 0.75f; }
                    return p;
                }

                struct Locus { float f1, f2, f3, b1, b2, b3; };
                enum Place { LABIAL, ALVEOLAR, PALATAL, VELAR };

                // V3: locus equations  F_locus = slope * F_vowel + intercept  (per place, see VoiceModel::loci).
                // Intercepts scale with the voice's tract size and the current femininity/formant scale.
                Locus locus(Place pl, const PV& V, float f1_target) const {
                    const LocusEq& q = vm_.loci[static_cast<int>(pl)];
                    const float ts = vm_.tract_scale() * (V[FSC] > 0.f ? V[FSC] : 1.f);
                    return { f1_target, q.s2 * V[F2] + q.c2 * ts, q.s3 * V[F3] + q.c3 * ts, 90.f, 140.f, 200.f };
                }

                static void apply_locus(PV& p, const Locus& l) {
                    p[F1] = l.f1; p[F2] = l.f2; p[F3] = l.f3; p[B1] = l.b1; p[B2] = l.b2; p[B3] = l.b3; p[TRK] = 0.f; p[TRK2] = 0.f;
                }

                PV blend_formants(const PV& a, const PV& b, float t) const {
                    PV o = a;
                    for (int i : { (int)F1, (int)F2, (int)F3, (int)B1, (int)B2, (int)B3, (int)TRK, (int)TRK2 })
                        o[i] = a[i] + (b[i] - a[i]) * t;
                    return o;
                }

                float cons_scale(const Ev& e) const {
                    float c = (0.45f + 1.1f * e.st.consonant_strength) * e.m.cons_scale;
                    if (e.st.mumble > 0.f) c *= 1.f - 0.4f * e.st.mumble; else if (e.st.mumble < 0.f) c *= 1.f - 0.25f * e.st.mumble;
                    return c;
                }

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
                            if (!same && st.vv_glide > 0.f)                    // diphthong-aware: longer glide
                                trvv = std::max(trvv, std::min(0.095 + 0.07 * static_cast<double>(st.vv_glide), 0.6 * std::min(prev->t1 - prev->t0, share)));
                            trvv = std::max(0.012, trvv);
                            if (st.transition_speed != 1.f) trvv /= std::max(0.2, static_cast<double>(st.transition_speed));
                            kt.push(std::max(kt.last_t(), T - 0.25 * trvv), kt.at(std::max(kt.last_t(), T - 0.25 * trvv)));
                            // re-articulation dip when notes change on the same vowel (not for ー)
                            if (!e.m.longv && e.first_in_note) {
                                const float dip = 0.55f * (1.f - st.legato);
                                PV d = lerp_pv(pv, V, 0.5f); d[AMP] *= (1.f - dip);
                                kt.push(T + 0.25 * trvv, d);
                            }
                            if (!same && st.vv_glide > 0.f) {                   // F2/F3 lead, F1 and amplitude follow
                                PV a = kt.at(kt.last_t()), mid = a;
                                for (int i = 0; i < N_P; ++i) {
                                    float fr = 0.5f;
                                    if (i == F1) fr = 0.38f; else if (i == F2 || i == F3) fr = 0.62f;
                                    mid[i] = a[i] + (V[i] - a[i]) * fr;
                                }
                                kt.push(std::max(kt.last_t(), T + 0.4 * trvv), mid);
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
                    if (st.n_nasalize > 0.f && e.next_vowel) {              // ん before a vowel/glide: nasalised vowel, not a full closure
                        PV Vn = prev ? vowel_state(*prev, prev->v) : Vown;
                        Vn[NASAL] = 0.7f; Vn[NZ] = 1250.f; Vn[AMP] = N[AMP];
                        N = lerp_pv(N, Vn, st.n_nasalize);
                    }
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
                        case Surf::K: case Surf::G: case Surf::NG:                f2 = 2000; nz = 1500; break;
                        case Surf::T: case Surf::D: case Surf::N: case Surf::S: case Surf::Z: case Surf::R:
                        case Surf::TS: case Surf::CH: case Surf::J: case Surf::SH: case Surf::NY: f2 = 1700; nz = 1750; break;
                        default:                                                  f2 = 1300; nz = 1250; break;   // uvular ɴ
                    }
                    if (e.art == wauvio::music::Articulation::Humming) { f2 = 1150; nz = 1000; }
                    p[F1] = 270; p[B1] = 100; p[F2] = f2; p[B2] = 160; p[F3] = 2600; p[B3] = 260; p[TRK] = 1.f; p[TRK2] = 0.f;
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
                        case Surf::K: kind = STOP; sp = { 0.040, 0.008, 0.036, false }; pl = VELAR;   // V3: word-initial VOT about 46 ms (40 before)
                            fric_f = palv ? 3400.f : (e.v == Vowel::E ? 2600.f : (e.v == Vowel::A ? 1900.f : 1300.f));
                            fric_bw = 1500; burst_a = 0.85f; asp_a = 0.80f; break;
                        case Surf::G: kind = STOP; sp = { 0.030, 0.006, 0.014, true }; pl = VELAR; voiced = true;
                            fric_f = palv ? 3000.f : (e.v == Vowel::E ? 2400.f : (e.v == Vowel::A ? 1800.f : 1250.f));
                            fric_bw = 1500; burst_a = 0.35f; break;
                        case Surf::T: kind = STOP; sp = { 0.034, 0.007, 0.018, false }; pl = ALVEOLAR;   // V3: word-initial VOT about 27 ms
                            fric_f = 4800; fric_bw = 3200; burst_a = 1.0f; asp_a = 0.75f; break;
                        case Surf::D: kind = STOP; sp = { 0.028, 0.006, 0.012, true }; pl = ALVEOLAR; voiced = true;
                            fric_f = 4300; fric_bw = 3000; burst_a = 0.40f; break;
                        case Surf::P: kind = STOP; sp = { 0.040, 0.006, 0.014, false }; pl = LABIAL;   // V3: word-initial VOT about 22 ms
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
                        case Surf::NG: kind = NASAL_; sp = { 0, 0, 0.055, true }; pl = VELAR; voiced = true; break;
                        case Surf::N:  kind = NASAL_; sp = { 0, 0, 0.050, true }; pl = ALVEOLAR; voiced = true; break;
                        case Surf::NY: kind = NASAL_; sp = { 0, 0, 0.055, true }; pl = PALATAL; voiced = true; break;
                        case Surf::M:  kind = NASAL_; sp = { 0, 0, 0.055, true }; pl = LABIAL; voiced = true; break;
                        case Surf::R:  kind = TAP; sp = { 0, 0, 0.030, true }; pl = ALVEOLAR; voiced = true; break;
                        case Surf::V:  kind = FRICATIVE; sp = { 0, 0, 0.060, true }; pl = LABIAL; voiced = true;   // [v]
                            fric_f = 4800; fric_bw = 4200; fric_a = 0.30f; break;
                        case Surf::Y:  kind = GLIDE; glide_v = Vowel::I; voiced = true; break;
                        case Surf::W:  kind = GLIDE; glide_v = Vowel::U; voiced = true; break;
                        default: break;
                    }
                    if (e.sm.palatal && (pl == ALVEOLAR || pl == VELAR || pl == LABIAL) && kind != GLIDE && kind != NASAL_)
                    { if (pl == VELAR) fric_f = std::max(fric_f, 3200.f); }

                    // V3: no two consonants are identical: duration +-12% and burst/frication strength +-20% at natural = 1
                    const double dscale = st.consonant_duration * (1.0 + 0.12 * static_cast<double>(st.natural) * static_cast<double>(vary(seed_, e.t0, 71u)));
                    {
                        const float av = 1.f + 0.20f * st.natural * vary(seed_, e.t0, 72u);
                        burst_a *= av; fric_a *= av;
                    }
                    // V3 (measured on one Japanese speaker): voiceless stops inside a phrase have a much shorter VOT than at
                    // the start of one (about 13 / 17 / 28 ms for p / t / k against 22 / 27 / 46 ms), so the aspiration part
                    // is shortened when the stop follows a vowel directly (not after a rest, not after a geminate).
                    const bool medial_stop = prev && (T - prev->t1) < 0.030 && !prev->m.geminate;
                    if (kind == STOP && !voiced && medial_stop) {
                        const double med = (pl == VELAR) ? 0.018 : (pl == ALVEOLAR ? 0.008 : 0.005);
                        sp.rel += (med - sp.rel) * static_cast<double>(std::min(1.f, std::max(0.f, st.medial_vot)));   // 0 in singing (measured), 1 in say()
                    }
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
                    // Word-initial voiced stops are (almost) never prevoiced in running speech; a short voicing lag follows the release.
                    const bool initial_voiced = kind == STOP && voiced && prev_gap;
                    const float pvi = std::min(1.f, std::max(0.f, st.prevoice_initial));
                    if (prev_gap) {
                        PV s0 = silent_like(V);
                        apply_locus(s0, locus(pl, V, loc_f1));
                        if (kt.last_t() < ts - 0.002) kt.push(ts - 0.002, s0);
                        kt.push(ts, s0);
                    } else {
                        // V3 coarticulation: the vowel we are leaving leans toward the vowel it is heading for,
                        // starting ~120 ms before the consonant, so V1-C-V2 is not a V1 -> C -> V2 sequence.
                        if (st.coarticulation > 0.f && prev && prev->v != e.v && !prev->m.nasal && !prev->m.geminate && kind != GLIDE) {
                            const double ta = std::max(kt.last_t(), ts - 0.12);
                            if (ta < ts - 0.012) kt.push(ta, kt.at(kt.last_t()));
                            const float amt = 0.20f * st.coarticulation;
                            for (int i : { static_cast<int>(F1), static_cast<int>(F2), static_cast<int>(F3) }) from[i] += (V[i] - from[i]) * amt;
                        }
                        // V3 VC transition: the vowel's formants start moving toward the consonant's locus a few ms BEFORE the
                        // closure (measured on real speech: the last ~10 ms of the vowel, mainly after back vowels), so the
                        // movement is audible in the vowel instead of happening inside the closure.
                        if (st.vc_transition > 0.f && prev && !prev->m.nasal && !prev->m.geminate && kind != GLIDE) {
                            const double tl = std::max(kt.last_t(), ts - 0.008 * static_cast<double>(st.vc_transition));
                            if (tl < ts - 0.002) kt.push(tl, kt.at(kt.last_t()));
                            PV Lc = V; apply_locus(Lc, locus(pl, V, loc_f1));      // this consonant's locus (the closure state is built later)
                            // F2 only, and asymmetric: in the speech data F2 rises toward a higher locus (back vowels) but hardly
                            // falls toward a lower one (front vowels), so a falling move gets a quarter of the strength.
                            // F1/F3 at the vowel end were not measured and are left to the closure transition.
                            const float up = Lc[F2] > from[F2] ? 1.0f : 0.25f;
                            from[F2] += (Lc[F2] - from[F2]) * 0.45f * up * st.vc_transition;
                        }
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
                    if (st.transition_speed != 1.f) tr_in /= std::max(0.2, static_cast<double>(st.transition_speed));
                    tr_in = std::max(0.014, tr_in);

                    auto fricvec = [&](PV p, float a, float f, float bw) { p[FRIC] = a * AV * 1.0f * vs / 1.1f; p[FRIC_F] = f; p[FRIC_BW] = bw; return p; };

                    switch (kind) {
                        case STOP: case AFFR: {
                            const double tc1 = ts + clo, tb1 = tc1 + bur;
                            PV c = voiced ? bar : closed;
                            if (initial_voiced && pvi < 1.f) c = lerp_pv(closed, bar, pvi);   // V3: no murmur before a word-initial voiced stop
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
                                const double lag = (initial_voiced && pvi < 1.f) ? 0.016 * static_cast<double>(1.f - pvi) : 0.0;
                                if (lag > 0.0) { d[VOICE] = 0.f; d[AMP] = AV * 0.5f; d[ASP] = std::min(1.f, asp_a * vs * 0.5f); }   // voicing starts a few ms after the release
                                kt.push(tb1, d);
                                if (lag > 0.0) { PV dl = d; dl[VOICE] = 1.f; dl[ASP] = V[ASP]; dl[AMP] = AV * 0.6f; kt.push(tb1 + lag, dl); }
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
                            nm[NASAL] = (sf == Surf::NG) ? e.bid : 1.f; nm[NZ] = (sf == Surf::M) ? 1000.f : (sf == Surf::N ? 1750.f : (sf == Surf::NG ? 1500.f : 2300.f));
                            nm[TILT] = 1400.f + 1600.f * clamp01(st.brightness); nm[HI] = 0.08f; nm[SING] = 0; nm[RD] = V[RD];
                            kt.push(ts + 0.012, nm);
                            kt.push(std::max(ts + 0.013, T - 0.004), nm);
                            PV r = blend_formants(L, V, 0.45f); r[AMP] = AV * 0.9f; r[NASAL] = (sf == Surf::NG) ? 0.25f * e.bid : 0.25f; r[VOICE] = 1; r[ASP] = V[ASP];
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

                    // labialised mora (くぁ ぐぁ ...): brief w-like tract between the release and the vowel
                    if (e.sm.labial && kind != GLIDE) {
                        const VowelTarget& ut = vm_.vowel[static_cast<int>(Vowel::U)];
                        const float sfm = 1.f + 0.12f * (st.femininity - 0.5f);
                        PV w = V; w[F1] = ut.f1 * sfm; w[F2] = 900.f * sfm; w[F3] = ut.f3 * sfm; w[B1] = 80; w[B2] = 130; w[B3] = 180; w[TRK] = 0; w[TRK2] = 0;
                        w[AMP] = AV * 0.9f; w[VOICE] = 1;
                        kt.push(T + 0.012, w);
                        tr_in = std::min(0.070, 0.5 * share);
                        kt.push(T + 0.012 + 0.4 * tr_in, w);
                    }

                    // arrive at the vowel target
                    const double vt = std::min(T + std::max(tr_in, 0.02), e.t0 + 0.5 * share);
                    kt.push(std::max(kt.last_t() + 0.001, vt), V);
                    return tr_in;
                }
            };

            // Portable white noise (V3). std::uniform_real_distribution is not specified bit-for-bit across standard
            // libraries, so the voice uses its own counter-hash generator: the same seed gives the same noise on every compiler.
            class VoiceNoise {
            public:
                explicit VoiceNoise(uint32_t seed = 1u) : s_(seed * 2654435761u + 0x9E3779B9u) {}
                uint32_t next() {
                    uint32_t z = (s_ += 0x9E3779B9u);
                    z = (z ^ (z >> 16)) * 0x85EBCA6Bu;
                    z = (z ^ (z >> 13)) * 0xC2B2AE35u;
                    return z ^ (z >> 16);
                }
                float tick_white() { return static_cast<float>(static_cast<int32_t>(next())) * 4.6566129e-10f; }   // [-1, 1)
            private:
                uint32_t s_;
            };

            // RBJ notch (the nasal anti-formant). Transposed direct form II: safe to retune every block.
            struct Notch {
                float b0 = 1, b1 = 0, b2 = 1, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
                void set(float f, float q, int sr) {
                    const float w0 = 6.2831853f * f / static_cast<float>(sr);
                    const float al = std::sin(w0) / (2.f * std::max(0.3f, q));
                    const float c = std::cos(w0), a0 = 1.f + al;
                    b0 = 1.f / a0; b1 = -2.f * c / a0; b2 = 1.f / a0; a1 = -2.f * c / a0; a2 = (1.f - al) / a0;
                }
                float tick(float x) { const float y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
            };

            // The vocal-tract filter chain. Two instances run in parallel: the real one, and
            // a reference fed with a constant fully-voiced source whose RMS drives a slow
            // loudness normaliser, so loudness does not depend on where harmonics happen to
            // land relative to F1 (vibrato, high notes, vowel changes).
            struct TractChain {
                wauvio::filter::SVFilter r1, r2, r3, q2, q3, u4, u5, u6, sg;
                float pre_lp = 0.f, tilt_state = 0.f;
                float q2v = 1, q3v = 1, pre_a = 0, tilt_a = 0, k4 = 1, k5 = 1, k6 = 1, ksg = 1;

                TractChain() {
                    for (wauvio::filter::SVFilter* f : { &r1, &r2, &r3 }) f->mode = wauvio::filter::SVFilter::Mode::LowPass;
                    for (wauvio::filter::SVFilter* f : { &q2, &q3, &u4, &u5, &u6, &sg }) f->mode = wauvio::filter::SVFilter::Mode::BandPass;
                }
                // fsc: tract-size scale (V3). F4..F6 and the singer's formant move with the voice's formant scale,
                // not only F1..F3, so a formant shift changes the whole tract instead of just its lower half.
                void set(const VoiceModel& vm, const PV& p, float f1, float f2, float f3, int sr, float fsc = 1.f) {
                    const float nyq = 0.46f * static_cast<float>(sr);
                    const float q1v = std::max(0.7f, f1 / std::max(30.f, p[B1]));
                    q2v = std::max(0.7f, f2 / std::max(30.f, p[B2]));
                    q3v = std::max(0.7f, f3 / std::max(30.f, p[B3]));
                    r1.set_params(f1, q1v, sr); r2.set_params(f2, q2v, sr); r3.set_params(f3, q3v, sr);
                    q2.set_params(f2, q2v, sr); q3.set_params(f3, q3v, sr);
                    u4.set_params(std::min(vm.f4 * fsc, nyq), vm.f4 / vm.b4, sr);
                    u5.set_params(std::min(vm.f5 * fsc, nyq), vm.f5 / vm.b5, sr);
                    u6.set_params(std::min(vm.f6 * fsc, nyq), vm.f6 / vm.b6, sr);
                    sg.set_params(std::min(vm.singer_formant_hz * fsc, nyq), vm.singer_formant_hz / vm.singer_formant_bw, sr);
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
                    y += hi * tn.upper_gain * up + sing * tn.sing_gain * sg.tick(hsrc, sr) * ksg;
                    return y;
                }
            };

            // Applies the expression channels to a parameter frame. Written so every channel is a
            // strict no-op at 0.
            inline void apply_expr(PV& p, const Expr& e, double f0) {
                const float vol = e[Ch::Volume];
                if (vol != 0.f) p[AMP] *= std::max(0.f, 1.f + vol);
                const float br = e[Ch::Brightness];
                if (br != 0.f) {
                    p[HI]   = std::max(0.f, p[HI] * (1.f + 0.8f * br));
                    p[TILT] = std::min(16000.f, std::max(250.f, p[TILT] * std::exp2(1.2f * br)));
                    p[SING] = std::max(0.f, p[SING] * (1.f + 0.5f * br));
                }
                float rd_add = 0.f;
                const float bt = e[Ch::Breath];
                if (bt != 0.f) { p[ASP] = std::max(0.f, p[ASP] + 0.3f * bt * p[VOICE]); rd_add += 0.55f * bt; }
                const float te = e[Ch::Tension];
                if (te != 0.f) rd_add -= 0.8f * te;
                float reg = e[Ch::Register];
                const float ra = e[Ch::RegAuto];
                if (ra != 0.f) {
                    const double midi = 69.0 + 12.0 * std::log2(std::max(1.0, f0) / 440.0);
                    reg += ra * static_cast<float>(smoothstep((midi - 72.0) / 8.0));
                }
                if (reg != 0.f) {
                    rd_add += 0.9f * reg;
                    p[SING] = std::max(0.f, p[SING] * (1.f - 0.7f * reg));
                    p[HI]   = std::max(0.f, p[HI] * (1.f - 0.35f * reg));
                    p[TILT] = std::max(250.f, p[TILT] * (1.f - 0.3f * reg));
                    p[ASP] += 0.04f * reg * p[VOICE];
                }
                const float gr = e[Ch::Growl];
                if (gr != 0.f) p[ASP] += 0.10f * gr * p[VOICE];
                const float ai = e[Ch::Air];
                if (ai != 0.f) p[ASP] += 0.04f * ai * p[VOICE];
                if (rd_add != 0.f) p[RD] = std::min(2.5f, std::max(0.4f, p[RD] + rd_add));
                const float fs = e[Ch::FormantShift], mo = e[Ch::MouthOpen];
                if (fs != 0.f || mo != 0.f) {
                    const float m = std::exp2(fs / 12.f);
                    p[F1] *= m * (1.f + 0.22f * mo); p[F2] *= m * (1.f - 0.04f * mo); p[F3] *= m;
                    p[B1] *= 1.f + 0.30f * mo;
                    p[FSC] = (p[FSC] > 0.f ? p[FSC] : 1.f) * m;
                }
            }

            /// Block-based voice synthesiser (V3). The score (key-frames, pitch, expression) is fixed when the stream is
            /// created; samples are then produced on demand in any chunk size. The output does not depend on how it is
            /// chunked: read(1), read(100) and read(all) give identical samples.
            class VoiceStream {
            public:
                VoiceStream(const VoiceModel& vm, int sr, KeyTrack kt, PitchTrack pt, ExprTrack ex, bool has_ex,
                            double total_sec, uint32_t seed = 0xA17E5u)
                    : vm_(vm), sr_(sr), kt_(std::move(kt)), pt_(std::move(pt)), ex_(std::move(ex)), has_ex_(has_ex), seed_(seed),
                      nz_asp_(seed), nz_fric_(seed ^ 0x5bd1e995u), nz_irr_(seed ^ 0x9e3779b9u)
                {
                    total_ = static_cast<size_t>(total_sec * sr_) + 1;
                    fz1_.mode = fz2_.mode = wauvio::filter::SVFilter::Mode::BandPass; npole_.mode = wauvio::filter::SVFilter::Mode::BandPass;
                    ms_a_ = 1.f - std::exp(-1.f / (0.030f * static_cast<float>(sr_)));
                    irr_ = 0.0008f * (1.f - 0.7f * vm_.synthetic);
                    prev_ = kt_.at(0.0);
                    f0_prev_ = 440.0 * std::pow(2.0, (pt_.at(0.0) - 69.0) / 12.0);
                    vg_prev_ = 1.f;
                }

                size_t total_samples() const noexcept { return total_; }
                size_t position() const noexcept { return s0_ - (blen_ - bpos_); }
                double position_seconds() const noexcept { return static_cast<double>(position()) / sr_; }
                bool   finished() const noexcept { return position() >= total_; }
                int    sample_rate() const noexcept { return sr_; }

                /// Writes up to n samples; returns how many were written (0 once the stream is finished).
                size_t read(float* dst, size_t n) {
                    size_t w = 0;
                    while (w < n) {
                        if (bpos_ >= blen_) { if (s0_ >= total_) break; render_block(); }
                        const size_t c = std::min(n - w, blen_ - bpos_);
                        for (size_t i = 0; i < c; ++i) dst[w + i] = blk_[bpos_ + i];
                        w += c; bpos_ += c;
                    }
                    return w;
                }
                Buffer read_all() { Buffer out(total_ - position(), 0.0f); const size_t got = read(out.data(), out.size()); out.resize(got); return out; }

            private:
                static constexpr int B = 16;
                VoiceModel vm_;
                int sr_;
                KeyTrack kt_; PitchTrack pt_; ExprTrack ex_; bool has_ex_;
                uint32_t seed_;
                VoiceNoise nz_asp_, nz_fric_, nz_irr_;
                TractChain main_c_, ref_c_;
                wauvio::filter::SVFilter fz1_, fz2_, npole_;
                Notch nzn_;
                size_t total_ = 0, s0_ = 0, blen_ = 0, bpos_ = 0;
                float blk_[B] = {};
                double ph_ = 0.0, gph_ = 0.0, tph_ = 0.0;
                float rdj_ = 0.f;
                float air_lp_ = 0.f, dc_x_ = 0.f, dc_y_ = 0.f, jit_ = 0.f, shim_ = 1.f, ms_ref_ = 7.0f, jn_ = 0.f, sn_ = 0.f;
                bool fry_t_ = false;
                float ms_a_ = 0.f, irr_ = 0.f, vg_prev_ = 1.f;
                PV prev_;
                double f0_prev_ = 440.0;

                void render_block() {
                    const GlottalTables& gt = glottal_tables();
                    const float sr = static_cast<float>(sr_);
                    const float nyq = 0.46f * sr;
                    const Tune& tn = vm_.tune;
                    const KeyTrack& kt = kt_; const PitchTrack& pt = pt_;
                    const size_t s0 = s0_;
                    const size_t blk = std::min<size_t>(B, total_ - s0);
                    const double t_mid = (static_cast<double>(s0) + 0.5 * static_cast<double>(blk)) / sr;
                    const double t_end = static_cast<double>(s0 + blk) / sr;
                    PV p = kt.at(t_mid);
                    double f0_next = 440.0 * std::pow(2.0, (pt.at(t_end) - 69.0) / 12.0);
                    f0_next = std::min(1800.0, std::max(60.0, f0_next));
                    const double f0_mid = 0.5 * (f0_prev_ + f0_next);

                    // V2 expression channels (all zero => this block is skipped and the output is untouched)
                    Expr e; bool has_e = false;
                    if (has_ex_ && ex_.any) { e = ex_.at(t_mid); has_e = true; apply_expr(p, e, f0_mid); }
                    const float e_growl = has_e ? e[Ch::Growl] : 0.f, e_trem = has_e ? e[Ch::Tremor] : 0.f;
                    const float e_fry = has_e ? e[Ch::Fry] : 0.f, e_air = has_e ? e[Ch::Air] : 0.f;
                    const float air_g = tn.air_gain * (1.f + 4.f * std::max(0.f, e_air));
                    const float e_nat = has_e ? std::max(0.f, e[Ch::Natural]) : 0.f;
                    float nat_amp = 1.f;
                    if (e_nat > 0.f) {                         // slow wander of level, breath and formants (deterministic per seed)
                        nat_amp = 1.f + 0.20f * e_nat * vnoise(seed_ + 21u, t_mid, 1.3) + 0.06f * e_nat * vnoise(seed_ + 22u, t_mid, 4.7);
                        p[ASP] *= 1.f + 0.60f * e_nat * vnoise(seed_ + 23u, t_mid, 1.9) + 0.1f * e_nat;
                        p[B1] *= 1.f + 0.15f * e_nat; p[B2] *= 1.f + 0.12f * e_nat; p[B3] *= 1.f + 0.10f * e_nat;   // less "synthetic" tight resonances
                        p[F1] *= 1.f + 0.028f * e_nat * vnoise(seed_ + 25u, t_mid, 0.8);
                        p[F2] *= 1.f + 0.022f * e_nat * vnoise(seed_ + 27u, t_mid, 0.7);
                        p[F3] *= 1.f + 0.012f * e_nat * vnoise(seed_ + 29u, t_mid, 0.6);
                    }

                    // V3: amplitude vibrato coupled to the pitch vibrato (a little behind it, as in a real larynx/tract)
                    float vg_next = 1.f;
                    {
                        const float am = pt.am_at(t_mid);
                        if (am > 0.f) vg_next = std::max(0.3f, 1.f + 0.8f * am * static_cast<float>(pt.vib_at(t_end - 0.003)));
                    }

                    // V3: nasalisation widens the low resonances (the nasal cavity damps F1 in particular)
                    if (p[NASAL] > 0.001f) {
                        const float nb = p[NASAL] * vm_.nasal_bw_widen;
                        p[B1] *= 1.f + nb; p[B2] *= 1.f + 0.5f * nb; p[B3] *= 1.f + 0.3f * nb;
                    }

                    float f1 = p[F1], f2 = p[F2], f3 = p[F3];
                    if (p[TRK] > 0.f) {                       // keep vowel identity at high pitch
                        const float want = 1.04f * static_cast<float>(f0_mid);
                        if (want > f1) f1 += p[TRK] * (want - f1);
                    }
                    if (p[TRK2] > 0.f) { const float want2 = 1.04f * static_cast<float>(f0_mid); if (want2 > f2) f2 += p[TRK2] * (want2 - f2); }
                    f2 = std::max(f2, f1 * 1.30f + 120.f);
                    f3 = std::max(f3, f2 + 350.f);
                    f1 = std::min(f1, nyq); f2 = std::min(f2, nyq); f3 = std::min(f3, nyq);
                    const float fsc = p[FSC] > 0.f ? p[FSC] : 1.f;
                    main_c_.set(vm_, p, f1, f2, f3, sr_, fsc);
                    {   // reference chain: the vowel's own formants, fixed brightness
                        float g1 = p[RF1], g2 = p[RF2], g3 = p[RF3];
                        if (p[RTRK] > 0.f) { const float want = 1.04f * static_cast<float>(f0_mid); if (want > g1) g1 += p[RTRK] * (want - g1); }
                        if (p[RTRK2] > 0.f) { const float want2 = 1.04f * static_cast<float>(f0_mid); if (want2 > g2) g2 += p[RTRK2] * (want2 - g2); }
                        g2 = std::max(g2, g1 * 1.30f + 120.f); g3 = std::max(g3, g2 + 350.f);
                        PV rp = p; rp[TILT] = 7000.f; rp[B1] = 110.f; rp[B2] = 130.f; rp[B3] = 190.f;
                        ref_c_.set(vm_, rp, std::min(g1, nyq), std::min(g2, nyq), std::min(g3, nyq), sr_, fsc);
                    }
                    const float fc = std::min(nyq, std::max(300.f, p[FRIC_F]));
                    const float fq = std::max(0.6f, fc / std::max(300.f, p[FRIC_BW]));
                    fz1_.set_params(fc, fq, sr_); fz2_.set_params(fc, fq, sr_);
                    nzn_.set(std::min(nyq, std::max(300.f, p[NZ])), vm_.nasal_zero_q, sr_);
                    npole_.set_params(vm_.nasal_pole_hz, 4.0f, sr_);
                    const float fric_norm = 1.f / fq;

                    for (size_t k = 0; k < blk; ++k) {
                        const float u = (static_cast<float>(k) + 0.5f) / static_cast<float>(blk);
                        const float amp   = prev_[AMP]   + (p[AMP]   - prev_[AMP])   * u;
                        const float voice = prev_[VOICE] + (p[VOICE] - prev_[VOICE]) * u;
                        const float asp   = prev_[ASP]   + (p[ASP]   - prev_[ASP])   * u;
                        const float fric  = prev_[FRIC]  + (p[FRIC]  - prev_[FRIC])  * u;
                        const float hi    = prev_[HI]    + (p[HI]    - prev_[HI])    * u;
                        const float sing  = prev_[SING]  + (p[SING]  - prev_[SING])  * u;
                        const float nas   = prev_[NASAL] + (p[NASAL] - prev_[NASAL]) * u;
                        const float vg    = vg_prev_ + (vg_next - vg_prev_) * u;
                        double f0 = f0_prev_ + (f0_next - f0_prev_) * u;
                        if (e_fry > 0.5f) f0 *= 1.0 - 0.30 * static_cast<double>((e_fry - 0.5f) * (e_fry - 0.5f));   // deep creak only; light fry never detunes

                        const double inc = f0 * (1.0 + jit_) / sr;
                        ph_ += inc;
                        if (ph_ >= 1.0) {
                            ph_ -= 1.0;
                            jit_  = irr_ * nz_irr_.tick_white();
                            shim_ = 1.f + 2.f * irr_ * nz_irr_.tick_white();
                            if (e_fry > 0.f) {                          // period doubling: long/short periods, weak/strong pulses
                                fry_t_ = !fry_t_;
                                jit_  += (fry_t_ ? 1.f : -1.f) * 0.10f * e_fry;
                                shim_ *= fry_t_ ? 1.f : 1.f - 0.4f * e_fry;
                            }
                            if (e_growl > 0.f) jit_ += 0.02f * e_growl * nz_irr_.tick_white();
                            if (e_nat > 0.f) {                          // cycle-to-cycle jitter/shimmer, mildly correlated like a real larynx
                                jn_ = 0.55f * jn_ + 0.84f * nz_irr_.tick_white(); sn_ = 0.55f * sn_ + 0.84f * nz_irr_.tick_white();
                                jit_  += 0.0065f * e_nat * jn_;
                                shim_ *= 1.f + 0.065f * e_nat * sn_;
                                rdj_ = 0.6f * rdj_ + 0.8f * nz_irr_.tick_white();            // V3: the pulse shape (spectral tilt) flutters too
                            }
                        }
                        const float g = gt.read(static_cast<float>(ph_), p[RD] * (1.f + 0.045f * e_nat * rdj_), static_cast<float>(inc));
                        const float pm = 0.55f + 0.45f * std::cos(static_cast<float>(TWO_PI * (ph_ - 0.35)));
                        const float wn = nz_asp_.tick_white();

                        // reference chain: constant, fully voiced -> loudness estimate
                        const float yr = ref_c_.tick(g, g, 0.7f, 0.5f, sr_, tn);
                        ms_ref_ += ms_a_ * (yr * yr - ms_ref_);
                        const float norm = 0.22f / std::sqrt(std::max(1e-4f, ms_ref_));

                        float hsrc = g * voice * shim_;
                        if (e_growl > 0.f) {                            // sub-harmonic amplitude roughness
                            gph_ += 75.0 / static_cast<double>(sr); if (gph_ >= 1.0) gph_ -= 1.0;
                            hsrc *= 1.f - 0.55f * e_growl * (0.5f + 0.5f * std::cos(static_cast<float>(TWO_PI * gph_)));
                        }
                        const float src = hsrc + wn * asp * tn.asp_gain * pm * 0.9f;
                        float y = main_c_.tick(src, hsrc, hi, sing, sr_, tn);
                        if (nas > 0.001f) {
                            // V3: nasalisation = low nasal pole (+) and a real anti-formant notch at NZ (-)
                            y += nas * vm_.nasal_pole_gain * npole_.tick(y, sr_) * 0.25f;
                            const float yn = nzn_.tick(y);
                            y += nas * vm_.nasal_zero_depth * (yn - y);
                        }
                        air_lp_ += 0.35f * (wn - air_lp_);
                        y += (wn - air_lp_) * asp * hi * air_g * pm / std::max(0.05f, norm);

                        float o = y * norm * amp * tn.out_scale * nat_amp * vg;
                        if (e_trem > 0.f) {
                            tph_ += static_cast<double>(e.tremor_rate) / static_cast<double>(sr); if (tph_ >= 1.0) tph_ -= 1.0;
                            o *= 1.f + 0.28f * e_trem * std::sin(static_cast<float>(TWO_PI * tph_));
                        }

                        if (fric > 0.0005f) {
                            float fn = nz_fric_.tick_white();
                            fn = fz2_.tick(fz1_.tick(fn, sr_), sr_) * fric_norm;
                            o += fn * fric * tn.fric_gain * 1.6f * (1.f - 0.45f * voice * (1.f - pm));
                        }
                        const float yo = o - dc_x_ + 0.995f * dc_y_; dc_x_ = o; dc_y_ = yo;
                        blk_[k] = yo;
                    }
                    prev_ = p; f0_prev_ = f0_next; vg_prev_ = vg_next;
                    s0_ += blk; blen_ = blk; bpos_ = 0;
                }
            };

            class VoiceRenderer {
            public:
                VoiceRenderer(const VoiceModel& vm, int sr) : vm_(vm), sr_(sr) {}

                /// Whole-buffer render (a VoiceStream read to the end).
                Buffer render(const KeyTrack& kt, const PitchTrack& pt, double total_sec, uint32_t seed = 0xA17E5u,
                              const ExprTrack* ex = nullptr) const {
                    VoiceStream st = stream(kt, pt, total_sec, seed, ex);
                    return st.read_all();
                }
                /// On-demand render: pull samples with VoiceStream::read().
                VoiceStream stream(const KeyTrack& kt, const PitchTrack& pt, double total_sec, uint32_t seed = 0xA17E5u,
                                   const ExprTrack* ex = nullptr) const {
                    return VoiceStream(vm_, sr_, kt, pt, ex ? *ex : ExprTrack(), ex != nullptr, total_sec, seed);
                }
            private:
                VoiceModel vm_;
                int sr_;
            };

        }
    }
}
