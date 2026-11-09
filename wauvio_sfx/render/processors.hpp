#pragma once
#include "sources.hpp"
#include <complex>

namespace wauvio {
    namespace sfx {
        namespace detail {
            struct ProcCtx { double sr; double bpm; uint64_t seed; size_t Nlen; };

            inline double time_coef_ms(double ms, double sr) noexcept { return ms <= 0.0 ? 0.0 : std::exp(-1.0 / (ms * 0.001 * sr)); }

            inline void proc_filter(Buffer& b, const FilterP& p, const ProcCtx& c) {
                Svf s1, s2; const double nyq = 0.45 * c.sr; const bool two = p.poles >= 4;
                for (size_t i = 0; i < b.size(); ++i) {
                    const double u = unit_pos(i, c.Nlen), t = static_cast<double>(i) / c.sr;
                    double fc = p.cutoff.at(u); if (p.lfo.active()) fc *= std::pow(2.0, p.lfo.eval(t, c.bpm, c.seed));
                    fc = clampd(fc, 10.0, nyq); const double q = clampd(p.q.at(u), 0.5, 30.0), g = std::tan(PI * fc / c.sr), k = 1.0 / q;
                    double lp, bp, hp, y = b[i];
                    auto pick = [&](Svf& s, double x, double kk) { s.tick(x, g, kk, lp, bp, hp);
                        switch (p.type) { case FilterP::LowPass: return lp; case FilterP::HighPass: return hp; case FilterP::BandPass: return bp * kk; default: return x - kk * bp; } };
                    y = pick(s1, y, k);
                    if (two) y = pick(s2, y, 1.4142);
                    if (!std::isfinite(y)) { s1.reset(); s2.reset(); y = 0.0; }
                    b[i] = static_cast<float>(y);
                }
            }

            inline void proc_drive(Buffer& b, const DriveP& p, const ProcCtx& c) {
                const double mix = clampd(p.mix, 0.0, 1.0);
                for (size_t i = 0; i < b.size(); ++i) {
                    const double a = clampd(p.amount.at(unit_pos(i, c.Nlen)), 0.0, 60.0), x = b[i]; double y;
                    switch (p.kind) {
                        case DriveP::Tanh:    y = std::tanh(a * x); break;
                        case DriveP::Hard:    y = clampd(a * x, -1.0, 1.0); break;
                        case DriveP::Fold:    y = std::sin(a * x * 1.5707963267948966); break;
                        case DriveP::Asym:    y = x > 0.0 ? std::tanh(a * x) : std::tanh(0.4 * a * x) * 1.6; break;
                        case DriveP::Rectify: y = 2.0 * std::fabs(std::tanh(a * x)) - 0.5; break;
                        default:              y = x;
                    }
                    b[i] = static_cast<float>(x * (1.0 - mix) + y * mix);
                }
            }

            // Bit reduction is relative to the layer's peak (bits = how many levels span the loudest part), so the result
            // does not depend on whatever envelope was applied before the chain.
            inline void proc_crush(Buffer& b, const CrushP& p, const ProcCtx& c) {
                float pk = 0.0f; for (float v : b) pk = std::max(pk, std::fabs(v));
                const double scale = pk > 1e-9f ? static_cast<double>(pk) : 1.0;
                double phase = 1.0; float held = 0.0f;
                for (size_t i = 0; i < b.size(); ++i) {
                    const double u = unit_pos(i, c.Nlen), bits = clampd(p.bits.at(u), 1.0, 24.0), down = std::max(1.0, p.down.at(u));
                    if (phase >= 1.0) { phase -= 1.0; const double lv = std::pow(2.0, bits - 1.0); held = bits >= 16.0 ? b[i] : static_cast<float>(std::round(b[i] / scale * lv) / lv * scale); }
                    phase += 1.0 / down; b[i] = held;
                }
            }

            inline void proc_gate(Buffer& b, const GateP& p, const ProcCtx& c) {
                if (p.steps.empty()) return;
                const double dur = p.step_sec > 0.0 ? p.step_sec : p.step_beats * 60.0 / c.bpm;
                if (!(dur > 1e-4)) throw Error("gate step length too short");
                const double depth = clampd(p.depth, 0.0, 1.0), sw = clampd(p.swing, 0.0, 0.5), a = time_coef_ms(p.smooth_ms, c.sr);
                double g = 1.0;
                for (size_t i = 0; i < b.size(); ++i) {
                    const double t = static_cast<double>(i) / c.sr, pair = std::floor(t / (2.0 * dur)), local = t - pair * 2.0 * dur;
                    const size_t idx = static_cast<size_t>(pair) * 2 + (local < dur * (1.0 + sw) ? 0 : 1);
                    const double target = 1.0 - depth + depth * clampd(p.steps[idx % p.steps.size()], 0.0, 1.0);
                    g = a * g + (1.0 - a) * target; b[i] = static_cast<float>(b[i] * g);
                }
            }

            inline void proc_trem(Buffer& b, const TremP& p, const ProcCtx& c) {
                double ph = 0.0;
                for (size_t i = 0; i < b.size(); ++i) {
                    const double u = unit_pos(i, c.Nlen); ph = wrap01(ph + clampd(p.rate.at(u), 0.0, 200.0) / c.sr);
                    b[i] = static_cast<float>(b[i] * (1.0 - clampd(p.depth.at(u), 0.0, 1.0) * (0.5 - 0.5 * std::cos(TWO_PI * ph))));
                }
            }

            inline void proc_pulses(Buffer& b, const PulsesP& p, const ProcCtx& c) {
                Rng rng(c.seed); std::vector<size_t> ev;
                if (!p.times.empty()) { for (double t : p.times) { const size_t n = static_cast<size_t>(clampd(t, 0.0, 1.0) * static_cast<double>(c.Nlen)); if (n < b.size()) ev.push_back(n); } std::sort(ev.begin(), ev.end()); }
                else {
                    double ph = 1.0;
                    for (size_t i = 0; i < b.size() && ev.size() < 20000; ++i) {
                        const double r = clampd(p.rate.at(unit_pos(i, c.Nlen)), 0.05, 400.0); ph += r / c.sr;
                        if (ph >= 1.0) { ph -= 1.0; const double j = p.jitter * (rng.bipolar()) * c.sr / r; const double pos = clampd(static_cast<double>(i) + j, 0.0, static_cast<double>(b.size() - 1)); ev.push_back(static_cast<size_t>(pos)); }
                    }
                    std::sort(ev.begin(), ev.end());
                }
                const double att = std::max(1.0, p.attack_ms * 0.001 * c.sr), tau = std::max(2.0, p.decay_ms * 0.001 * c.sr / 6.9), depth = clampd(p.depth, 0.0, 1.0);
                size_t e = 0; bool started = false; double age = 0.0;
                for (size_t i = 0; i < b.size(); ++i) {
                    while (e < ev.size() && ev[e] <= i) { started = true; age = static_cast<double>(i - ev[e]); ++e; }
                    double env = 0.0;
                    if (started) { env = age < att ? age / att : std::exp(-(age - att) / tau); age += 1.0; }
                    b[i] = static_cast<float>(b[i] * (1.0 - depth + depth * env));
                }
            }

            inline void proc_stutter(Buffer& b, const StutterP& p, const ProcCtx& c) {
                Rng rng(c.seed); const size_t seg = std::max<size_t>(16, static_cast<size_t>(p.seg_sec * c.sr)); const int reps = std::max(1, std::min(p.repeats, 32));
                size_t pos = static_cast<size_t>(clampd(p.from, 0.0, 1.0) * static_cast<double>(b.size())); const size_t end = static_cast<size_t>(clampd(p.to, 0.0, 1.0) * static_cast<double>(b.size()));
                const size_t xf = std::min<size_t>(seg / 4, static_cast<size_t>(0.001 * c.sr));
                while (pos + seg <= end && pos + seg <= b.size()) {
                    if (rng.chance(p.prob)) {
                        std::vector<float> copy(b.begin() + static_cast<std::ptrdiff_t>(pos), b.begin() + static_cast<std::ptrdiff_t>(pos + seg));
                        for (size_t i = 0; i < xf; ++i) { const float g = static_cast<float>(i) / static_cast<float>(xf); copy[i] *= g; copy[seg - 1 - i] *= g; }
                        for (int r = 1; r <= reps; ++r) { const size_t o = pos + static_cast<size_t>(r) * seg; if (o + seg > b.size()) break; for (size_t i = 0; i < seg; ++i) b[o + i] = copy[i]; }
                        pos += seg * static_cast<size_t>(1 + reps);
                    } else pos += seg;
                }
            }

            inline void proc_freeze(Buffer& b, const FreezeP& p, double sr) {
                const size_t a = static_cast<size_t>(clampd(p.at, 0.0, 1.0) * static_cast<double>(b.size())); size_t w = std::max<size_t>(64, static_cast<size_t>(p.window_sec * sr));
                if (a < w) w = a;
                if (w < 64) return;
                std::vector<float> seg(b.begin() + static_cast<std::ptrdiff_t>(a - w), b.begin() + static_cast<std::ptrdiff_t>(a));
                auto hann = [](double x) { return 0.5 - 0.5 * std::cos(TWO_PI * x); };
                for (size_t j = a; j < b.size(); ++j) {
                    const size_t pos = (j - a) % w, pos2 = (pos + w / 2) % w;
                    b[j] = static_cast<float>(hann(static_cast<double>(pos) / static_cast<double>(w)) * seg[pos] + hann(static_cast<double>(pos2) / static_cast<double>(w)) * seg[pos2]);
                }
            }

            inline void proc_dropout(Buffer& b, const DropoutP& p, const ProcCtx& c) {
                Rng rng(c.seed); std::vector<float> g(b.size(), 1.0f); const double depth = clampd(p.depth, 0.0, 1.0);
                const size_t ramp = std::max<size_t>(1, static_cast<size_t>(0.001 * c.sr));
                const double span = static_cast<double>(b.size()) / c.sr; const int n = static_cast<int>(p.rate_per_sec * span + 0.5);
                for (int k = 0; k < n; ++k) {
                    const size_t s0 = static_cast<size_t>(rng.uniform() * static_cast<double>(b.size())), len = static_cast<size_t>(rng.range(p.min_ms, std::max(p.min_ms, p.max_ms)) * 0.001 * c.sr);
                    for (size_t i = 0; i < len && s0 + i < b.size(); ++i) { const double e = std::min({ 1.0, static_cast<double>(i) / ramp, static_cast<double>(len - i) / ramp }); g[s0 + i] = std::min(g[s0 + i], static_cast<float>(1.0 - depth * e)); }
                }
                for (size_t i = 0; i < b.size(); ++i) b[i] *= g[i];
            }

            inline void proc_comb(Buffer& b, const CombP& p, const ProcCtx& c) {
                const size_t maxd = static_cast<size_t>(c.sr / 20.0) + 4; std::vector<float> line(maxd, 0.0f); size_t w = 0; const double fb = clampd(p.feedback, 0.0, 0.95), mix = clampd(p.mix, 0.0, 1.0);
                for (size_t i = 0; i < b.size(); ++i) {
                    const double f = clampd(p.freq.at(unit_pos(i, c.Nlen)), 20.0, 0.45 * c.sr), d = c.sr / f; const double rp = static_cast<double>(w) + static_cast<double>(maxd) - d;
                    const size_t i0 = static_cast<size_t>(rp) % maxd, i1 = (i0 + 1) % maxd; const double fr = rp - std::floor(rp);
                    const double dl = line[i0] * (1.0 - fr) + line[i1] * fr; const double y = clampd(b[i] + fb * dl, -8.0, 8.0);
                    line[w] = static_cast<float>(y); w = (w + 1) % maxd; b[i] = static_cast<float>(b[i] * (1.0 - mix) + y * mix);
                }
            }

            inline void proc_ring(Buffer& b, const RingP& p, const ProcCtx& c) {
                double ph = 0.0; const double mix = clampd(p.mix, 0.0, 1.0);
                for (size_t i = 0; i < b.size(); ++i) { ph = wrap01(ph + clampd(p.freq.at(unit_pos(i, c.Nlen)), 0.0, 0.45 * c.sr) / c.sr); b[i] = static_cast<float>(b[i] * (1.0 - mix) + b[i] * std::sin(TWO_PI * ph) * mix); }
            }

            // small Schroeder reverb: 4 damped combs + 2 allpasses. A synthetic tail, not a convolution reverb.
            inline void proc_reverb(Buffer& b, const ReverbP& p, const ProcCtx& c) {
                static const double cd[4] = { 0.0297, 0.0371, 0.0411, 0.0437 }; static const double ad[2] = { 0.0050, 0.0017 };
                const double t60 = clampd(p.t60, 0.1, 30.0), damp = clampd(p.damping, 0.0, 0.98), mix = clampd(p.mix, 0.0, 2.0);
                struct Comb { std::vector<float> l; size_t i = 0; double g = 0, lp = 0; };
                Comb cb[4]; for (int k = 0; k < 4; ++k) { const size_t d = static_cast<size_t>(cd[k] * c.sr); cb[k].l.assign(d, 0.0f); cb[k].g = std::pow(10.0, -3.0 * cd[k] / t60); }
                struct Ap { std::vector<float> l; size_t i = 0; }; Ap ap[2]; for (int k = 0; k < 2; ++k) ap[k].l.assign(static_cast<size_t>(ad[k] * c.sr), 0.0f);
                const size_t pre = static_cast<size_t>(std::max(0.0, p.predelay_ms) * 0.001 * c.sr); std::vector<float> pl(pre + 1, 0.0f); size_t pi = 0;
                for (size_t i = 0; i < b.size(); ++i) {
                    const float x = b[i]; pl[pi] = x; const float xin = pl[(pi + 1) % pl.size()]; pi = (pi + 1) % pl.size();
                    double acc = 0.0;
                    for (auto& k : cb) { const double y = k.l[k.i]; k.lp = y * (1.0 - damp) + k.lp * damp; k.l[k.i] = static_cast<float>(xin + k.g * k.lp); k.i = (k.i + 1) % k.l.size(); acc += y; }
                    double w = acc * 0.25;
                    for (auto& a : ap) { const double d = a.l[a.i]; const double v = w + 0.7 * d; a.l[a.i] = static_cast<float>(v); a.i = (a.i + 1) % a.l.size(); w = d - 0.7 * v; }
                    b[i] = static_cast<float>(x + w * mix * 3.0);
                }
            }

            inline void proc_echo(Buffer& b, const EchoP& p, const ProcCtx& c) {
                const size_t d = std::max<size_t>(8, static_cast<size_t>(p.time_sec * c.sr)); std::vector<float> line(d, 0.0f); size_t w = 0; double lp = 0.0;
                const double fb = clampd(p.feedback, 0.0, 0.92), damp = clampd(p.damping, 0.0, 0.98), mix = clampd(p.mix, 0.0, 2.0);
                for (size_t i = 0; i < b.size(); ++i) { const double y = line[w]; lp = y * (1.0 - damp) + lp * damp; line[w] = static_cast<float>(b[i] + fb * lp); w = (w + 1) % d; b[i] = static_cast<float>(b[i] + mix * y); }
            }

            inline void fft_inplace(std::vector<std::complex<double>>& a, bool inv) {
                const size_t n = a.size();
                for (size_t i = 1, j = 0; i < n; ++i) { size_t bit = n >> 1; for (; j & bit; bit >>= 1) j ^= bit; j ^= bit; if (i < j) std::swap(a[i], a[j]); }
                for (size_t len = 2; len <= n; len <<= 1) {
                    const double ang = TWO_PI / static_cast<double>(len) * (inv ? 1.0 : -1.0); const std::complex<double> wl(std::cos(ang), std::sin(ang));
                    for (size_t i = 0; i < n; i += len) { std::complex<double> w(1, 0); for (size_t k = 0; k < len / 2; ++k) { const auto u = a[i + k], v = a[i + k + len / 2] * w; a[i + k] = u + v; a[i + k + len / 2] = u - v; w *= wl; } }
                }
                if (inv) for (auto& x : a) x /= static_cast<double>(n);
            }
            inline void proc_spectral(Buffer& b, const SpectralP& p, const ProcCtx& c) {
                const size_t N = 2048, H = 512; if (b.size() < N) return;
                const size_t total = b.size() + 2 * N; std::vector<double> in(total, 0.0), out(total, 0.0), win(N);
                for (size_t i = 0; i < N; ++i) win[i] = 0.5 - 0.5 * std::cos(TWO_PI * static_cast<double>(i) / static_cast<double>(N));
                for (size_t i = 0; i < b.size(); ++i) in[N + i] = b[i];
                const size_t frames = (total - N) / H + 1; const size_t freeze_frame = static_cast<size_t>(clampd(p.at, 0.0, 1.0) * static_cast<double>(b.size()) / H) + N / H;
                std::vector<double> frozen(N / 2 + 1, 0.0); std::vector<std::complex<double>> sp(N);
                for (size_t f = 0; f < frames; ++f) {
                    const size_t o = f * H; for (size_t i = 0; i < N; ++i) sp[i] = in[o + i] * win[i];
                    fft_inplace(sp, false);
                    const double u = unit_pos(o > N ? o - N : 0, c.Nlen > 0 ? c.Nlen : b.size()), amt = clampd(p.amount.at(u), 0.0, 1.0);
                    for (size_t k = 0; k <= N / 2; ++k) {
                        double mag = std::abs(sp[k]), ph = std::arg(sp[k]);
                        switch (p.mode) {
                            case SpectralP::Disintegrate: if (hash01(mix_seed(c.seed, k)) < amt) mag = 0.0; break;
                            case SpectralP::Scramble:     ph = hash01(mix_seed(c.seed, f * 4099 + k)) * TWO_PI * amt + ph * (1.0 - amt); break;
                            case SpectralP::TiltFade:     mag *= std::exp(-amt * 9.0 * static_cast<double>(k) / static_cast<double>(N / 2)); break;
                            case SpectralP::Freeze:
                                if (f == freeze_frame) frozen[k] = mag;
                                if (f >= freeze_frame) { mag = lerp(mag, frozen[k], amt); ph = hash01(mix_seed(c.seed, f * 7919 + k)) * TWO_PI * amt + ph * (1.0 - amt); }
                                break;
                        }
                        sp[k] = std::polar(mag, ph); if (k > 0 && k < N / 2) sp[N - k] = std::conj(sp[k]);
                    }
                    sp[N / 2] = std::complex<double>(sp[N / 2].real(), 0.0); sp[0] = std::complex<double>(sp[0].real(), 0.0);
                    fft_inplace(sp, true);
                    for (size_t i = 0; i < N; ++i) out[o + i] += sp[i].real() * win[i];
                }
                for (size_t i = 0; i < b.size(); ++i) b[i] = static_cast<float>(out[N + i] / 1.5);     // Hann^2 overlap at hop N/4 sums to 1.5
            }

            inline void run_proc(Buffer& b, const Proc& pr, const ProcCtx& c) {
                std::visit([&](const auto& p) {
                    using T = std::decay_t<decltype(p)>;
                    if constexpr (std::is_same_v<T, FilterP>) proc_filter(b, p, c);
                    else if constexpr (std::is_same_v<T, DriveP>) proc_drive(b, p, c);
                    else if constexpr (std::is_same_v<T, CrushP>) proc_crush(b, p, c);
                    else if constexpr (std::is_same_v<T, GateP>) proc_gate(b, p, c);
                    else if constexpr (std::is_same_v<T, TremP>) proc_trem(b, p, c);
                    else if constexpr (std::is_same_v<T, PulsesP>) proc_pulses(b, p, c);
                    else if constexpr (std::is_same_v<T, StutterP>) proc_stutter(b, p, c);
                    else if constexpr (std::is_same_v<T, FreezeP>) proc_freeze(b, p, c.sr);
                    else if constexpr (std::is_same_v<T, DropoutP>) proc_dropout(b, p, c);
                    else if constexpr (std::is_same_v<T, CombP>) proc_comb(b, p, c);
                    else if constexpr (std::is_same_v<T, RingP>) proc_ring(b, p, c);
                    else if constexpr (std::is_same_v<T, ReverbP>) proc_reverb(b, p, c);
                    else if constexpr (std::is_same_v<T, EchoP>) proc_echo(b, p, c);
                    else if constexpr (std::is_same_v<T, ReverseP>) std::reverse(b.begin(), b.end());
                    else if constexpr (std::is_same_v<T, SpectralP>) proc_spectral(b, p, c);
                }, pr);
            }
        }
    }
}
