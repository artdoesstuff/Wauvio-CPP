#pragma once
#include "../core/sfx_recipe.hpp"

namespace wauvio {
    namespace sfx {
        namespace detail {
            struct Svf {            // Simper / Zavalishin TPT state-variable filter (stable under fast modulation)
                double s1 = 0.0, s2 = 0.0;
                void tick(double x, double g, double k, double& lp, double& bp, double& hp) noexcept {
                    const double a1 = 1.0 / (1.0 + g * (g + k)), a2 = g * a1, a3 = g * a2;
                    const double v3 = x - s2, v1 = a1 * s1 + a2 * v3, v2 = s2 + a2 * s1 + a3 * v3;
                    s1 = 2.0 * v1 - s1; s2 = 2.0 * v2 - s2;
                    lp = v2; bp = v1; hp = x - k * v1 - v2;
                }
                void reset() noexcept { s1 = s2 = 0.0; }
            };

            inline double wrap01(double p) noexcept { return p - std::floor(p); }
            inline double unit_pos(size_t i, size_t Nlen) noexcept { return Nlen == 0 ? 1.0 : std::min(1.0, static_cast<double>(i) / static_cast<double>(Nlen)); }

            // one cycle of a band-limited (PolyBLEP) waveform; p in [0,1), dt = f / sr
            inline double wave_sample(Wave w, double p, double dt, double pw) noexcept {
                using wave_bl::polyblep;
                switch (w) {
                    case Wave::Sine:     return std::sin(TWO_PI * p);
                    case Wave::Triangle: return 4.0 * std::fabs(p - 0.5) - 1.0;
                    case Wave::Saw:      return (2.0 * p - 1.0) - polyblep(p, dt);
                    case Wave::Square:   return ((p < 0.5) ? 1.0 : -1.0) + polyblep(p, dt) - polyblep(wrap01(p + 0.5), dt);
                    case Wave::Pulse:    { pw = clampd(pw, 0.05, 0.95); return ((p < pw) ? 1.0 : -1.0) + polyblep(p, dt) - polyblep(wrap01(p + (1.0 - pw)), dt); }
                }
                return 0.0;
            }

            inline Buffer gen_osc(const Source& s, size_t N, size_t Nlen, double sr, uint64_t seed) {
                Buffer out(N, 0.0f); if (!N) return out;
                Rng rng(seed);
                const bool stacked = !s.ratios.empty();
                const int V = stacked ? static_cast<int>(std::min<size_t>(s.ratios.size(), 32)) : std::max(1, std::min(s.voices, 16));
                std::vector<double> ph(static_cast<size_t>(V));
                for (auto& p : ph) p = rng.uniform();
                double fmph = rng.uniform(), subph = 0.0;
                const double nyq = 0.45 * sr, norm = 1.0 / std::sqrt(static_cast<double>(V));
                for (size_t i = 0; i < N; ++i) {
                    const double u = unit_pos(i, Nlen);
                    const double f0 = clampd(midi_to_hz(s.pitch.at(u)), 1.0, nyq);
                    const double cents = s.detune_cents.at(u), pw = s.pulse_width.at(u), idx = clampd(s.fm_index.at(u), 0.0, 40.0);
                    const double pm = idx > 0.0 ? std::sin(TWO_PI * fmph) * idx / TWO_PI : 0.0;
                    double acc = 0.0;
                    for (int k = 0; k < V; ++k) {
                        double f = f0;
                        if (stacked) f *= s.ratios[static_cast<size_t>(k)];
                        else if (V > 1) f *= std::pow(2.0, ((k - (V - 1) * 0.5) / ((V - 1) * 0.5)) * cents / 1200.0);
                        double& p = ph[static_cast<size_t>(k)];
                        const double dt = f / sr; p = wrap01(p + dt);
                        if (f >= nyq * 1.1) continue;
                        acc += wave_sample(s.wave, wrap01(p + pm), dt, pw);
                    }
                    fmph = wrap01(fmph + f0 * s.fm_ratio / sr);
                    double v = acc * norm;
                    if (s.sub > 0.0) { subph = wrap01(subph + 0.5 * f0 / sr); v += s.sub * std::sin(TWO_PI * subph); }
                    out[i] = static_cast<float>(v);
                }
                return out;
            }

            struct NoiseState {
                Rng rng; double b0 = 0, b1 = 0, b2 = 0, brown = 0, w1 = 0, w2 = 0;
                explicit NoiseState(uint64_t seed) : rng(seed) {}
                double tick(NoiseColor c, double dust_p) {
                    const double w = rng.bipolar();
                    switch (c) {
                        case NoiseColor::White:  return w;
                        case NoiseColor::Pink:   b0 = 0.99765 * b0 + w * 0.0990460; b1 = 0.96300 * b1 + w * 0.2965164; b2 = 0.57000 * b2 + w * 1.0526913; return (b0 + b1 + b2 + w * 0.1848) * 0.6;
                        case NoiseColor::Brown:  brown = (brown + 0.04 * w) / 1.02; return brown * 6.0;
                        case NoiseColor::Blue:   { const double o = (w - w1) * 0.7; w1 = w; return o; }
                        case NoiseColor::Violet: { const double o = (w - 2.0 * w1 + w2) * 0.4; w2 = w1; w1 = w; return o; }
                        case NoiseColor::Dust:   return rng.uniform() < dust_p ? (rng.chance(0.5) ? 1.0 : -1.0) * (0.3 + 0.7 * rng.uniform()) : 0.0;
                    }
                    return w;
                }
            };
            inline Buffer gen_noise(const Source& s, size_t N, size_t Nlen, double sr, uint64_t seed) {
                Buffer out(N, 0.0f); NoiseState ns(seed);
                for (size_t i = 0; i < N; ++i) {
                    const double dust_p = s.color == NoiseColor::Dust ? clampd(s.density.at(unit_pos(i, Nlen)), 0.0, sr * 0.5) / sr : 0.0;
                    out[i] = static_cast<float>(ns.tick(s.color, dust_p));
                }
                return out;
            }

            inline Buffer gen_additive(const Source& s, size_t N, size_t Nlen, double sr, uint64_t seed) {
                Buffer out(N, 0.0f); if (!N) return out;
                Rng rng(seed);
                const size_t NN = std::min<size_t>(std::max<size_t>(s.notes.size(), 1), 12);
                const int H = std::max(1, std::min(s.harmonics, 64));
                const size_t P = NN * static_cast<size_t>(H);
                std::vector<double> ph(P), det(P), inc(P, 0.0), amp(P, 0.0);
                for (size_t i = 0; i < P; ++i) { ph[i] = rng.uniform(); det[i] = rng.bipolar(); }
                const double nyq = 0.45 * sr, norm = 1.0 / std::sqrt(static_cast<double>(P));
                const size_t BLK = 32;
                for (size_t i = 0; i < N; ++i) {
                    if (i % BLK == 0) {
                        const double u = unit_pos(i, Nlen), roll = s.rolloff.at(u), spr = s.partial_spread.at(u);
                        for (size_t n = 0; n < NN; ++n) {
                            const double f0 = midi_to_hz(s.pitch.at(u) + (s.notes.empty() ? 0.0 : s.notes[n]));
                            for (int h = 1; h <= H; ++h) {
                                const size_t k = n * static_cast<size_t>(H) + static_cast<size_t>(h - 1);
                                const double hh = static_cast<double>(h);
                                const double f = f0 * hh * std::sqrt(1.0 + s.inharmonicity * hh * hh) * std::pow(2.0, spr * det[k] / 1200.0);
                                if (f >= nyq) { inc[k] = 0.0; amp[k] = 0.0; continue; }
                                inc[k] = f / sr; amp[k] = std::pow(hh, -roll);
                            }
                        }
                    }
                    double acc = 0.0;
                    for (size_t k = 0; k < P; ++k) { if (amp[k] == 0.0) continue; ph[k] = wrap01(ph[k] + inc[k]); acc += amp[k] * std::sin(TWO_PI * ph[k]); }
                    out[i] = static_cast<float>(acc * norm);
                }
                return out;
            }

            inline Buffer gen_modal(const Source& s, size_t N, size_t Nlen, double sr, uint64_t seed) {
                Buffer out(N, 0.0f); if (!N || s.modes.empty()) return out;
                Rng rng(seed);
                const double nyq = 0.45 * sr;
                for (size_t k = 0; k < s.hits.size(); ++k) {
                    double t = clampd(s.hits[k] + s.hit_jitter * rng.bipolar(), 0.0, 1.0);
                    const size_t n0 = static_cast<size_t>(t * static_cast<double>(Nlen));
                    if (n0 >= N) continue;
                    const double note = s.pitch.at(t) + (s.hit_notes.empty() ? 0.0 : s.hit_notes[k % s.hit_notes.size()]);
                    const double f0 = midi_to_hz(note), vel = 1.0 - s.velocity_var * rng.uniform();
                    for (const Mode& m : s.modes) {
                        const double f = f0 * m.ratio; if (f >= nyq || m.amp <= 0.0) continue;
                        const double t60 = std::max(0.005, m.t60 * s.t60_scale);
                        const double d = std::pow(10.0, -3.0 / (t60 * sr)), w = TWO_PI * f / sr, cw = std::cos(w), sw = std::sin(w);
                        double re = 1.0, im = 0.0, a = vel * m.amp; const size_t life = static_cast<size_t>(t60 * sr * 1.05);
                        const size_t end = std::min(N, n0 + life);
                        for (size_t i = n0; i < end; ++i) { out[i] += static_cast<float>(a * im); const double nre = re * cw - im * sw; im = re * sw + im * cw; re = nre; a *= d; }
                    }
                    const size_t sn = static_cast<size_t>(s.strike_ms * 0.001 * sr);                 // attack click
                    for (size_t i = 0; i < sn && n0 + i < N; ++i) out[n0 + i] += static_cast<float>(0.4 * vel * rng.bipolar() * std::exp(-4.0 * static_cast<double>(i) / std::max<double>(1.0, static_cast<double>(sn))));
                }
                return out;
            }

            inline void vowel_formants(double v, double F[3], double A[3]) {
                static const double tab[5][3] = { {800,1150,2900}, {400,1600,2700}, {350,1700,2700}, {450,800,2830}, {325,700,2530} };
                static const double amps[5][3] = { {1.0,0.50,0.25}, {1.0,0.45,0.20}, {1.0,0.40,0.20}, {1.0,0.40,0.25}, {1.0,0.30,0.12} };
                v = clampd(v, 0.0, 4.0); const int a = std::min(3, static_cast<int>(v)); const double f = v - a;
                for (int i = 0; i < 3; ++i) { F[i] = lerp(tab[a][i], tab[a + 1][i], f); A[i] = lerp(amps[a][i], amps[a + 1][i], f); }
            }
            inline Buffer gen_formant(const Source& s, size_t N, size_t Nlen, double sr, uint64_t seed) {
                Buffer out(N, 0.0f); if (!N) return out;
                Rng rng(seed); Svf f[3]; double ph = rng.uniform();
                const double nyq = 0.45 * sr; const double bw[3] = { 90.0, 110.0, 160.0 };
                for (size_t i = 0; i < N; ++i) {
                    const double u = unit_pos(i, Nlen);
                    const double f0 = clampd(midi_to_hz(s.pitch.at(u)), 20.0, nyq), dt = f0 / sr; ph = wrap01(ph + dt);
                    const double voiced = s.noise_excited ? 0.0 : wave_sample(Wave::Saw, ph, dt, 0.5), nz = rng.bipolar();
                    const double exc = (1.0 - s.breath) * voiced + s.breath * nz * 0.8;
                    double F[3], A[3]; vowel_formants(s.vowel.at(u), F, A);
                    const double sh = semis_to_ratio(s.formant_shift.at(u)); double acc = 0.0;
                    for (int k = 0; k < 3; ++k) {
                        const double fc = clampd(F[k] * sh, 60.0, nyq), q = clampd(fc / bw[k], 1.0, 30.0);
                        double lp, bp, hp; f[k].tick(exc, std::tan(PI * fc / sr), 1.0 / q, lp, bp, hp);
                        acc += A[k] * bp / q;
                    }
                    out[i] = static_cast<float>(acc * 4.0);
                }
                return out;
            }

            inline Buffer grain_material(const Source& s, double sr, Rng& rng) {
                const size_t M = static_cast<size_t>(1.6 * sr); Buffer b(M, 0.0f);
                const double root = midi_to_hz(s.pitch.at(0.5));
                switch (s.grain_source) {
                    case GrainSource::Noise: { NoiseState ns(rng.next()); for (auto& v : b) v = static_cast<float>(ns.tick(NoiseColor::Pink, 0.0)); break; }
                    case GrainSource::Tone:  for (size_t i = 0; i < M; ++i) { const double t = static_cast<double>(i) / sr; b[i] = static_cast<float>(std::sin(TWO_PI * root * t) + 0.3 * std::sin(TWO_PI * 2.0 * root * t)); } break;
                    case GrainSource::Chord: for (size_t i = 0; i < M; ++i) { const double t = static_cast<double>(i) / sr; double a = 0; for (double n : s.notes) a += std::sin(TWO_PI * root * semis_to_ratio(n) * t) + 0.25 * std::sin(TWO_PI * 2.0 * root * semis_to_ratio(n) * t); b[i] = static_cast<float>(a / std::max<size_t>(1, s.notes.size())); } break;
                    case GrainSource::Vowel: { Source v; v.kind = SourceKind::Formant; v.pitch = s.pitch; v.vowel = s.vowel; v.breath = s.breath;
                        Buffer f = gen_formant(v, M, M, sr, rng.next()); b = f; float pk = 0; for (float x : b) pk = std::max(pk, std::fabs(x)); if (pk > 1e-9f) for (auto& x : b) x /= pk; break; }
                    case GrainSource::Bell:  for (const Mode& m : modes_bell()) { const double f = root * m.ratio; if (f > 0.45 * sr) continue; const double ph0 = rng.uniform();
                        for (size_t i = 0; i < M; ++i) { const double t = static_cast<double>(i) / sr; b[i] += static_cast<float>(m.amp * std::sin(TWO_PI * f * t + ph0) * std::exp(-t * 2.2 / m.t60)); } } break;
                }
                return b;
            }
            inline Buffer gen_grain(const Source& s, size_t N, size_t Nlen, double sr, uint64_t seed) {
                Buffer out(N, 0.0f); if (!N) return out;
                Rng rng(seed); const Buffer mat = grain_material(s, sr, rng); const size_t M = mat.size();
                struct G { double pos, inc; size_t age, len; double amp; };
                std::vector<G> act; act.reserve(128);
                for (size_t i = 0; i < N; ++i) {
                    const double u = unit_pos(i, Nlen);
                    const double dens = clampd(s.density.at(u), 0.0, 400.0);
                    if (act.size() < 120 && rng.uniform() < dens / sr) {
                        const double len_ms = clampd(s.grain_ms.at(u), 3.0, 600.0);
                        const double semis = s.grain_pitch.at(u) + s.pitch_scatter * rng.bipolar();
                        const double p0 = wrap01(s.position.at(u) + s.position_scatter * rng.bipolar() * 0.5);
                        act.push_back({ p0 * static_cast<double>(M), semis_to_ratio(semis), 0, std::max<size_t>(8, static_cast<size_t>(len_ms * 0.001 * sr)), 0.6 + 0.4 * rng.uniform() });
                    }
                    double acc = 0.0;
                    for (size_t k = 0; k < act.size();) {
                        G& g = act[k];
                        const double w = 0.5 - 0.5 * std::cos(TWO_PI * static_cast<double>(g.age) / static_cast<double>(g.len));
                        const double rp = std::fmod(g.pos, static_cast<double>(M)); const size_t i0 = static_cast<size_t>(rp), i1 = (i0 + 1) % M; const double fr = rp - static_cast<double>(i0);
                        acc += g.amp * w * (mat[i0] + (mat[i1] - mat[i0]) * fr);
                        g.pos += g.inc; ++g.age;
                        if (g.age >= g.len) { act[k] = act.back(); act.pop_back(); } else ++k;
                    }
                    out[i] = static_cast<float>(acc);
                }
                return out;
            }

            inline Buffer gen_pluck(const Source& s, size_t N, size_t Nlen, double sr, uint64_t seed) {
                Buffer out(N, 0.0f); if (!N) return out;
                Rng rng(seed);
                for (size_t k = 0; k < s.hits.size(); ++k) {
                    const double t = clampd(s.hits[k] + s.hit_jitter * rng.bipolar(), 0.0, 1.0);
                    const size_t n0 = static_cast<size_t>(t * static_cast<double>(Nlen)); if (n0 >= N) continue;
                    const double note = s.pitch.at(t) + (s.hit_notes.empty() ? 0.0 : s.hit_notes[k % s.hit_notes.size()]);
                    const double f = clampd(midi_to_hz(note), 30.0, 0.4 * sr); const size_t D = std::max<size_t>(2, static_cast<size_t>(sr / f));
                    std::vector<double> line(D); const double vel = 1.0 - s.velocity_var * rng.uniform();
                    for (auto& v : line) v = rng.bipolar();
                    const double t60 = (0.3 + 3.0 * (1.0 - clampd(s.damping, 0.0, 1.0))) * s.t60_scale;
                    const double g = std::pow(10.0, -3.0 * static_cast<double>(D) / (sr * std::max(0.02, t60))), c = 1.0 - 0.5 * clampd(s.damping, 0.0, 1.0);
                    const size_t end = std::min(N, n0 + static_cast<size_t>(std::min(4.0, t60 * 1.2) * sr)); size_t p = 0;
                    for (size_t i = n0; i < end; ++i) {
                        const double cur = line[p], nxt = line[(p + 1) % D];
                        out[i] += static_cast<float>(cur * vel);
                        line[p] = g * (c * cur + (1.0 - c) * nxt); p = (p + 1) % D;
                    }
                }
                return out;
            }

            inline Buffer gen_chaos(const Source& s, size_t N, size_t Nlen, double sr, uint64_t seed) {
                Buffer out(N, 0.0f); if (!N) return out;
                Rng rng(seed);
                double x = 1.0 + 0.1 * rng.bipolar(), y = 1.0 + 0.1 * rng.bipolar(), z = 1.0 + 0.1 * rng.bipolar();   // Lorenz
                double lx = 0.2 + 0.6 * rng.uniform(), lsm = 0.0, hstep = 0.0;                                         // logistic
                double hx = rng.range(-0.4, 0.4), hy = rng.range(-0.1, 0.1);                                           // henon (start inside its basin of attraction)
                double ph = rng.uniform(), lastc = 0.0, lasty = 0.0;
                for (size_t i = 0; i < N; ++i) {
                    const double u = unit_pos(i, Nlen), rate = clampd(s.chaos_rate.at(u), 0.1, 2000.0);
                    double cx = 0.0, cy = 0.0;
                    if (s.chaos == ChaosKind::Lorenz) {
                        const double dt = rate / sr; const int sub = std::max(1, static_cast<int>(std::ceil(dt / 0.004))); const double h = dt / sub;
                        for (int k = 0; k < sub; ++k) { const double dx = 10.0 * (y - x), dy = x * (28.0 - z) - y, dz = x * y - 8.0 / 3.0 * z; x += h * dx; y += h * dy; z += h * dz; }
                        if (!std::isfinite(x + y + z) || std::fabs(x) > 200.0 || std::fabs(y) > 300.0 || z < -10.0 || z > 300.0) { x = 1.0; y = 1.0; z = 1.0; }
                        cx = clampd(x / 20.0, -1.5, 1.5); cy = clampd(y / 27.0, -1.5, 1.5);
                    } else if (s.chaos == ChaosKind::Logistic) {
                        hstep += rate / sr; const double r = clampd(s.logistic_r.at(u), 3.5, 3.999);
                        while (hstep >= 1.0) { hstep -= 1.0; lx = r * lx * (1.0 - lx); if (!(lx > 1e-6 && lx < 1.0 - 1e-6)) lx = 0.1 + 0.8 * rng.uniform(); }
                        const double a = 1.0 - std::exp(-TWO_PI * std::min(rate, 0.4 * sr) / sr); lsm += a * ((lx * 2.0 - 1.0) - lsm);
                        cx = lsm; cy = lx * 2.0 - 1.0;
                    } else {
                        hstep += rate / sr;
                        while (hstep >= 1.0) { hstep -= 1.0; const double nx = 1.0 - 1.4 * hx * hx + hy; hy = 0.3 * hx; hx = nx; if (!std::isfinite(hx) || std::fabs(hx) > 3.0) { hx = 0.1; hy = 0.1; } }
                        const double a = 1.0 - std::exp(-TWO_PI * std::min(rate, 0.4 * sr) / sr); lastc += a * (clampd(hx / 1.3, -1.5, 1.5) - lastc); lasty += a * (clampd(hy / 0.4, -1.5, 1.5) - lasty);
                        cx = lastc; cy = lasty;
                    }
                    if (s.chaos_direct) { out[i] = static_cast<float>(cx); continue; }
                    const double f = clampd(midi_to_hz(s.pitch.at(u) + s.chaos_depth.at(u) * cx), 10.0, 0.45 * sr), dt = f / sr; ph = wrap01(ph + dt);
                    const double amp = 1.0 - s.chaos_amp * (0.5 + 0.5 * cy);
                    out[i] = static_cast<float>(amp * wave_sample(s.wave, ph, dt, s.pulse_width.at(u)));
                }
                return out;
            }

            inline Buffer generate_source(const Source& s, size_t N, size_t Nlen, double sr, uint64_t seed) {
                switch (s.kind) {
                    case SourceKind::Osc:      return gen_osc(s, N, Nlen, sr, seed);
                    case SourceKind::Noise:    return gen_noise(s, N, Nlen, sr, seed);
                    case SourceKind::Additive: return gen_additive(s, N, Nlen, sr, seed);
                    case SourceKind::Modal:    return gen_modal(s, N, Nlen, sr, seed);
                    case SourceKind::Formant:  return gen_formant(s, N, Nlen, sr, seed);
                    case SourceKind::Grain:    return gen_grain(s, N, Nlen, sr, seed);
                    case SourceKind::Pluck:    return gen_pluck(s, N, Nlen, sr, seed);
                    case SourceKind::Chaos:    return gen_chaos(s, N, Nlen, sr, seed);
                    case SourceKind::None:     break;
                }
                throw Error("layer has no source");
            }
        }
    }
}
