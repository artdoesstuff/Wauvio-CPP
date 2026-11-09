#pragma once
#include "modulation.hpp"
#include "../core/fx_io.hpp"

namespace wauvio {
    namespace fx {
        class FdnReverb : public Effect {
        public:
            double decay_s      = 2.5;      ///< RT60 at low frequencies, 0.1 .. 30 s (damping makes highs shorter)
            double predelay_ms  = 15.0;
            double size         = 1.0;      ///< scales all delay lengths (0.4 small room .. 2.5 huge hall)
            double damping_hz   = 6000.0;   ///< high frequencies decay faster above this; 0 = no damping
            double diffusion    = 0.6;      ///< 0..1, input smearing (allpass chain)
            double mod_depth_ms = 0.25;     ///< slow delay-length wobble; breaks up metallic ringing (0 = off)
            double mod_rate_hz  = 0.35;
            double width        = 1.0;      ///< wet stereo width: 0 = mono, 1 = full
            double lowcut_hz    = 80.0;     ///< wet high-pass (0 = off)
            double highcut_hz   = 12000.0;  ///< wet low-pass (0 = off)
            double mix          = 0.3;

            std::string name() const override { return "FdnReverb"; }
            double tail_seconds() const override { return predelay_ms * 0.001 + std::min(30.0, std::max(0.1, decay_s)) * 1.1; }

            void reset() override {
                for (auto& l : line_) l.clear();
                pre_.clear();
                for (auto& a : ap_) { a.clear(); }
                for (auto& z : lp_) z = 0.0f;
                for (auto& f : hp_) f.reset();
                for (auto& f : lc_) f.reset();
                pos_ = 0;
            }

            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                static const double base_ms[N] = { 29.7, 37.1, 41.1, 43.7, 53.3, 59.3, 67.1, 71.9 };
                static const double ap_ms[4]   = { 5.3, 7.9, 11.3, 13.7 };
                static const float  sl[N] = { 1, 1, 1, 1, -1, -1, -1, -1 };
                static const float  sgn_r[N] = { 1, -1, 1, -1, 1, -1, 1, -1 };

                const double sr = sr_samplerate();
                const double sz = std::min(3.0, std::max(0.2, size));
                const double mod_s = mod_depth_ms * 0.001 * sr;
                double d[N], g[N];
                const double rt = std::min(30.0, std::max(0.1, decay_s));
                size_t need = 0;
                for (int i = 0; i < N; ++i) {
                    d[i] = std::max(8.0, std::round(base_ms[i] * sz * 0.001 * sr));
                    g[i] = std::pow(10.0, -3.0 * d[i] / (rt * sr));          // per-trip gain so RT60 = decay_s
                    need = std::max(need, static_cast<size_t>(d[i] + mod_s) + 16);
                }
                if (line_[0].capacity() < need) for (auto& l : line_) l.init(need);
                const size_t pre_n = static_cast<size_t>(std::max(0.0, predelay_ms) * 0.001 * sr);
                if (pre_.capacity() < pre_n + 16) pre_.init(pre_n + 16);
                for (int a = 0; a < 4; ++a) {
                    const size_t len = static_cast<size_t>(ap_ms[a] * 0.001 * sr) + 2;
                    if (ap_[a].capacity() < len + 16) ap_[a].init(len + 16);
                    ap_len_[a] = len;
                }
                const bool damp_on = damping_hz > 1.0 && damping_hz < 0.45 * sr;
                const double damp_a = damp_on ? 1.0 - std::exp(-TWO_PI * damping_hz / sr) : 1.0;
                const float apg = static_cast<float>(0.7 * std::min(1.0, std::max(0.0, diffusion)));
                const bool use_lc = lowcut_hz > 1.0, use_hc = highcut_hz > 1.0;
                if (use_lc) { const BiquadCoef c = design_biquad(FilterType::HighPass, sr, lowcut_hz, 0.7071); lc_[0].set(c); lc_[1].set(c); }
                if (use_hc) { const BiquadCoef c = design_biquad(FilterType::LowPass,  sr, highcut_hz, 0.7071); hp_[0].set(c); hp_[1].set(c); }
                const double wet = std::min(1.0, std::max(0.0, mix));
                const float wd = static_cast<float>(std::min(1.0, std::max(0.0, width)));
                const float out_scale = 0.35355339f;    // 0.5/sqrt(2): wet RMS ~= input RMS for noise input

                for (size_t i = 0; i < n; ++i) {
                    const float xl = L[i], xr = R[i];
                    // pre-delay -> allpass diffusion
                    pre_.push(0.5f * (xl + xr));
                    float s = pre_n > 0 ? pre_.read(static_cast<double>(pre_n)) : 0.5f * (xl + xr);
                    for (int a = 0; a < 4; ++a) {
                        const float y = ap_[a].read(static_cast<double>(ap_len_[a]));
                        const float w = s + apg * y;
                        ap_[a].push(w);
                        s = y - apg * w;
                    }
                    // FDN
                    float y[N], f[N]; float sum = 0.0f;
                    const double t = static_cast<double>(pos_ + i) / sr;
                    for (int k = 0; k < N; ++k) {
                        const double wob = mod_s > 0.0 ? mod_s * std::sin(TWO_PI * mod_rate_hz * (0.8 + 0.05 * k) * t + k * 0.9) : 0.0;
                        y[k] = line_[k].read(d[k] + wob);
                        lp_[k] += static_cast<float>(damp_a) * (y[k] - lp_[k]);        // damping
                        lp_[k] = flush_denormal(lp_[k]);
                        f[k] = lp_[k] * static_cast<float>(g[k]);
                        sum += f[k];
                    }
                    const float h = sum * (2.0f / N);                                 // Householder reflection
                    float wl = 0.0f, wr = 0.0f;
                    for (int k = 0; k < N; ++k) {
                        line_[k].push(s + f[k] - h);
                        wl += sl[k] * y[k]; wr += sgn_r[k] * y[k];
                    }
                    wl *= out_scale; wr *= out_scale;
                    const float m = 0.5f * (wl + wr), sd = 0.5f * (wl - wr) * wd;
                    wl = m + sd; wr = m - sd;
                    if (use_lc) { wl = lc_[0].tick(wl); wr = lc_[1].tick(wr); }
                    if (use_hc) { wl = hp_[0].tick(wl); wr = hp_[1].tick(wr); }
                    L[i] = static_cast<float>(xl * (1.0 - wet) + wl * wet);
                    R[i] = static_cast<float>(xr * (1.0 - wet) + wr * wet);
                }
                pos_ += n;
            }

        private:
            static constexpr int N = 8;
            double sr_samplerate() const { return static_cast<double>(sr_); }
            FracDelay line_[N], pre_, ap_[4];
            size_t ap_len_[4] = {0, 0, 0, 0};
            float lp_[N] = {};
            Biquad hp_[2], lc_[2];
            size_t pos_ = 0;
        };

        namespace ir {
            struct Params {
                double rt60        = 1.8;     ///< seconds to -60 dB
                double predelay_ms = 8.0;
                double hf_damping  = 0.5;     ///< 0 = bright tail, 1 = very dark tail
                double early       = 0.5;     ///< early-reflection level 0..1 (0 = none)
                double stereo      = 1.0;     ///< 0 = both channels identical, 1 = fully decorrelated
                uint64_t seed      = 1;
            };

            inline Params room()      { Params p; p.rt60 = 0.6;  p.predelay_ms = 3;  p.hf_damping = 0.55; p.early = 0.7; return p; }
            inline Params chamber()   { Params p; p.rt60 = 1.3;  p.predelay_ms = 6;  p.hf_damping = 0.45; p.early = 0.55; return p; }
            inline Params hall()      { Params p; p.rt60 = 2.8;  p.predelay_ms = 22; p.hf_damping = 0.5;  p.early = 0.35; return p; }
            inline Params cathedral() { Params p; p.rt60 = 6.0;  p.predelay_ms = 35; p.hf_damping = 0.65; p.early = 0.25; return p; }
            inline Params plate()     { Params p; p.rt60 = 2.0;  p.predelay_ms = 0;  p.hf_damping = 0.15; p.early = 0.0;  return p; }

            /// Stereo impulse response: sparse early reflections + a gaussian-noise tail that
            /// decays exponentially and gets darker over time. Same params + seed = same IR.
            inline StereoBuffer make(const Params& p, int sr) {
                const double rt = std::max(0.1, p.rt60);
                const double len_s = p.predelay_ms * 0.001 + rt * 1.15;
                const size_t n = static_cast<size_t>(len_s * sr);
                StereoBuffer out(n);
                Rng rng(p.seed);
                std::vector<float> a(n, 0.0f), b(n, 0.0f);
                const size_t pre = static_cast<size_t>(p.predelay_ms * 0.001 * sr);
                const double st = std::min(1.0, std::max(0.0, p.stereo));
                const double k_decay = 6.907755 / rt;                       // ln(1000)/rt60
                float ya = 0.0f, yb = 0.0f;
                for (size_t i = pre; i < n; ++i) {
                    const double t = static_cast<double>(i - pre) / sr;
                    const double env = std::exp(-k_decay * t) * std::min(1.0, t / 0.006);
                    const double fc = 16000.0 * std::exp(-t * p.hf_damping * 1.2) + 800.0;
                    const float c = static_cast<float>(1.0 - std::exp(-TWO_PI * std::min(fc, 0.45 * sr) / sr));
                    const float na = static_cast<float>(rng.gaussian()), nb0 = static_cast<float>(rng.gaussian());
                    const float nb = static_cast<float>(st * nb0 + (1.0 - st) * na);
                    ya += c * (na - ya); yb += c * (nb - yb);
                    a[i] = static_cast<float>(ya * env); b[i] = static_cast<float>(yb * env);
                }
                if (p.early > 0.0) {
                    const int taps = 18;
                    for (int ch = 0; ch < 2; ++ch) {
                        auto& v = ch ? b : a;
                        Rng er(p.seed * 7919 + static_cast<uint64_t>(ch) * 104729 + 13);
                        for (int k = 0; k < taps; ++k) {
                            const double tt = 0.003 + 0.075 * std::pow(er.uniform(), 1.4);
                            const size_t idx = pre + static_cast<size_t>(tt * sr);
                            if (idx < n) v[idx] += static_cast<float>((er.bipolar() > 0 ? 1 : -1) * p.early * 0.9 * std::exp(-tt * 18.0));
                        }
                    }
                }
                out.L = a; out.R = b;
                // unit energy
                double e = 0.0; for (size_t i = 0; i < n; ++i) e += static_cast<double>(out.L[i]) * out.L[i] + static_cast<double>(out.R[i]) * out.R[i];
                if (e > 0.0) { const float g = static_cast<float>(1.0 / std::sqrt(e * 0.5)); for (size_t i = 0; i < n; ++i) { out.L[i] *= g; out.R[i] *= g; } }
                return out;
            }
        }

        // ----------------------------------------------------------------------------
        //  ConvolutionReverb - uniform-partitioned overlap-save FFT convolution
        //  Stereo IR: left input * IR left, right input * IR right. Mono IR: used on both.
        //  Latency = partition size (512 samples); Chain compensates it.
        // ----------------------------------------------------------------------------
        class ConvolutionReverb : public Effect {
        public:
            double mix         = 0.3;
            double predelay_ms = 0.0;
            double wet_gain_db = 0.0;
            bool   normalize_ir = true;   ///< scale IR to unit energy so wet level ~ input level

            std::string name() const override { return "ConvolutionReverb"; }
            size_t latency_samples() const override { return B; }
            double tail_seconds() const override { return ir_seconds_ + predelay_ms * 0.001; }

            void set_ir(const StereoBuffer& ir, int ir_sr) { ir_[0] = ir.L; ir_[1] = ir.R; ir_sr_ = ir_sr; dirty_ = true; update_len(); }
            void set_ir(const Buffer& mono, int ir_sr)     { ir_[0] = mono; ir_[1] = mono; ir_sr_ = ir_sr; dirty_ = true; update_len(); }
            /// Procedural IR (see ir::room() etc.). Generated at the current sample rate on prepare.
            void set_ir(const ir::Params& p) { proc_ = p; use_proc_ = true; dirty_ = true; ir_seconds_ = p.predelay_ms * 0.001 + p.rt60 * 1.15; }
            /// Load a WAV impulse response (mono or stereo; resampled to the render rate if needed).
            void load_ir(const std::string& path) {
                AudioData a = read_wav(path);
                if (a.channels.empty()) throw std::runtime_error("ConvolutionReverb: empty IR " + path);
                ir_[0] = a.channels[0]; ir_[1] = a.channels.size() > 1 ? a.channels[1] : a.channels[0];
                ir_sr_ = a.sample_rate; use_proc_ = false; dirty_ = true; update_len();
            }

            void reset() override {
                for (int c = 0; c < 2; ++c) { std::fill(inbuf_[c].begin(), inbuf_[c].end(), 0.0f); std::fill(outbuf_[c].begin(), outbuf_[c].end(), 0.0f);
                    std::fill(prev_[c].begin(), prev_[c].end(), 0.0f); std::fill(dry_[c].begin(), dry_[c].end(), 0.0f);
                    for (auto& s : fdl_[c]) std::fill(s.begin(), s.end(), std::complex<float>(0, 0)); }
                pos_ = 0; fdl_pos_ = 0;
            }

            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                if (dirty_ || built_sr_ != sr_) build();
                if (parts_ == 0) return;
                const float wet = static_cast<float>(std::min(1.0, std::max(0.0, mix)) * db_to_lin(wet_gain_db));
                const float dry = static_cast<float>(1.0 - std::min(1.0, std::max(0.0, mix)));
                // dry path is delayed by B so it lines up with the wet path
                float* ch[2] = { L, R };
                for (size_t i = 0; i < n; ++i) {
                    for (int c = 0; c < 2; ++c) {
                        const float x = ch[c][i];
                        const float w = outbuf_[c][pos_];
                        const float dd = dry_[c][pos_];
                        dry_[c][pos_] = x;
                        inbuf_[c][pos_] = x;
                        ch[c][i] = dd * dry + w * wet;
                    }
                    if (++pos_ == B) { pos_ = 0; block(); }
                }
            }

        private:
            static constexpr size_t B = 512;              // partition (hop) size
            static constexpr size_t F = 2 * B;            // FFT size

            void update_len() { ir_seconds_ = ir_[0].empty() ? 0.0 : static_cast<double>(ir_[0].size()) / std::max(1, ir_sr_); }

            void build() {
                std::vector<float> irs[2];
                if (use_proc_) { StereoBuffer s = ir::make(proc_, sr_); irs[0] = s.L; irs[1] = s.R; }
                else { irs[0] = resample_linear(ir_[0], ir_sr_, sr_); irs[1] = resample_linear(ir_[1], ir_sr_, sr_); }
                const size_t pre = static_cast<size_t>(std::max(0.0, predelay_ms) * 0.001 * sr_);
                double e = 0.0;
                for (int c = 0; c < 2; ++c) { irs[c].insert(irs[c].begin(), pre, 0.0f); for (float v : irs[c]) e += static_cast<double>(v) * v; }
                float g = 1.0f;
                if (normalize_ir && e > 0.0) g = static_cast<float>(1.0 / std::sqrt(e * 0.5));
                parts_ = (irs[0].empty()) ? 0 : (irs[0].size() + B - 1) / B;
                for (int c = 0; c < 2; ++c) {
                    H_[c].assign(parts_, std::vector<std::complex<float>>(F));
                    fdl_[c].assign(parts_, std::vector<std::complex<float>>(F, std::complex<float>(0, 0)));
                    for (size_t p = 0; p < parts_; ++p) {
                        std::vector<std::complex<float>> a(F, std::complex<float>(0, 0));
                        for (size_t k = 0; k < B; ++k) { const size_t idx = p * B + k; if (idx < irs[c].size()) a[k] = irs[c][idx] * g; }
                        fft(a, false); H_[c][p] = std::move(a);
                    }
                    inbuf_[c].assign(B, 0.0f); outbuf_[c].assign(B, 0.0f); prev_[c].assign(B, 0.0f); dry_[c].assign(B, 0.0f);
                }
                pos_ = 0; fdl_pos_ = 0; dirty_ = false; built_sr_ = sr_;
            }

            void block() {
                for (int c = 0; c < 2; ++c) {
                    std::vector<std::complex<float>> x(F);
                    for (size_t k = 0; k < B; ++k) { x[k] = prev_[c][k]; x[B + k] = inbuf_[c][k]; }
                    std::copy(inbuf_[c].begin(), inbuf_[c].end(), prev_[c].begin());
                    fft(x, false);
                    fdl_[c][fdl_pos_] = std::move(x);
                    std::vector<std::complex<float>> acc(F, std::complex<float>(0, 0));
                    for (size_t p = 0; p < parts_; ++p) {
                        const auto& X = fdl_[c][(fdl_pos_ + parts_ - p) % parts_];
                        const auto& Hh = H_[c][p];
                        for (size_t k = 0; k < F; ++k) acc[k] += X[k] * Hh[k];
                    }
                    fft(acc, true);
                    for (size_t k = 0; k < B; ++k) outbuf_[c][k] = acc[B + k].real();
                }
                fdl_pos_ = (fdl_pos_ + 1) % parts_;
            }

            std::vector<float> ir_[2]; int ir_sr_ = 44100; double ir_seconds_ = 0.0;
            ir::Params proc_; bool use_proc_ = false;
            bool dirty_ = true; int built_sr_ = 0;
            size_t parts_ = 0, pos_ = 0, fdl_pos_ = 0;
            std::vector<std::vector<std::complex<float>>> H_[2], fdl_[2];
            std::vector<float> inbuf_[2], outbuf_[2], prev_[2], dry_[2];
        };
    }
}
