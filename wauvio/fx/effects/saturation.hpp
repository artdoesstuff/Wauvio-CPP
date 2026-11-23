#pragma once
#include "../core/fx_effect.hpp"

namespace wauvio {
    namespace fx {
        // ----------------------------------------------------------------------------
        //  Oversampler1 - one channel. Each 2x stage is a 97-tap Kaiser-windowed half-band
        //  low-pass (beta 9, ~90 dB stop-band). Latency is an integer number of samples.
        // ----------------------------------------------------------------------------
        class Oversampler1 {
        public:
            static constexpr int D = 48;              // half length; taps = 2D + 1
            static constexpr int T = 2 * D + 1;

            /// factor: 1, 2, 4 or 8
            void prepare(int factor) {
                factor_ = (factor >= 8) ? 8 : (factor >= 4) ? 4 : (factor >= 2) ? 2 : 1;
                stages_ = factor_ == 8 ? 3 : factor_ == 4 ? 2 : factor_ == 2 ? 1 : 0;
                if (h_.empty()) build_kernel();
                st_.assign(static_cast<size_t>(stages_), Stage());
                for (auto& s : st_) { s.up_hist.assign(D, 0.0f); s.dn_hist.assign(2 * D, 0.0f); }
                bufs_.assign(static_cast<size_t>(stages_) + 1, std::vector<float>());
            }
            int factor() const noexcept { return factor_; }
            static int normalise_factor(int f) noexcept { return f >= 8 ? 8 : f >= 4 ? 4 : f >= 2 ? 2 : 1; }
            /// Round-trip delay (base-rate samples) for a given factor, before prepare().
            static size_t latency_for(int f) noexcept { f = normalise_factor(f); return f == 1 ? 0 : static_cast<size_t>(D) * (2 * static_cast<size_t>(f) - 2) / static_cast<size_t>(f); }
            /// Round-trip delay in base-rate samples (up + down).
            size_t latency() const noexcept { return latency_for(factor_); }
            void reset() { for (auto& s : st_) { std::fill(s.up_hist.begin(), s.up_hist.end(), 0.0f);
                std::fill(s.dn_hist.begin(), s.dn_hist.end(), 0.0f); } }

            /// Up-sample, run fn(float* hi, size_t m) in place at the high rate, down-sample.
            /// `in` and `out` may be the same pointer.
            template <class F>
            void process(const float* in, float* out, size_t n, F&& fn) {
                if (factor_ == 1) { if (in != out) std::copy(in, in + n, out); fn(out, n); return; }
                const size_t B = 1024;
                for (size_t off = 0; off < n; off += B) {
                    const size_t m = std::min(B, n - off);
                    bufs_[0].assign(in + off, in + off + m);
                    for (int s = 0; s < stages_; ++s) up(static_cast<size_t>(s));
                    fn(bufs_[static_cast<size_t>(stages_)].data(), bufs_[static_cast<size_t>(stages_)].size());
                    for (int s = stages_ - 1; s >= 0; --s) down(static_cast<size_t>(s));
                    std::copy(bufs_[0].begin(), bufs_[0].end(), out + off);
                }
            }

        private:
            struct Stage { std::vector<float> up_hist, dn_hist; };

            static double bessel_i0(double x) {
                double sum = 1.0, term = 1.0; const double q = x * x * 0.25;
                for (int k = 1; k < 60; ++k) { term *= q / (static_cast<double>(k) * k); sum += term; if (term < 1e-14 * sum) break; }
                return sum;
            }
            void build_kernel() {
                h_.assign(T, 0.0f);
                const double beta = 9.0, i0b = bessel_i0(beta);
                std::vector<double> k(T); double sum = 0.0;
                for (int i = 0; i < T; ++i) {
                    const double x = 0.5 * (i - D);                                  // cutoff = 0.25 cycles/sample of the high rate
                    const double sinc = (std::fabs(x) < 1e-12) ? 1.0 : std::sin(PI * x) / (PI * x);
                    const double r = static_cast<double>(i - D) / D;
                    const double win = bessel_i0(beta * std::sqrt(std::max(0.0, 1.0 - r * r))) / i0b;
                    k[static_cast<size_t>(i)] = 0.5 * sinc * win; sum += k[static_cast<size_t>(i)];
                }
                for (int i = 0; i < T; ++i) h_[static_cast<size_t>(i)] = static_cast<float>(k[static_cast<size_t>(i)] / sum);
            }
            void up(size_t s) {
                auto& x = bufs_[s]; auto& y = bufs_[s + 1]; auto& hist = st_[s].up_hist;
                const size_t n = x.size();
                std::vector<float> ext(D + n);
                std::copy(hist.begin(), hist.end(), ext.begin());
                std::copy(x.begin(), x.end(), ext.begin() + D);
                y.assign(2 * n, 0.0f);
                for (size_t i = 0; i < n; ++i) {
                    double a = 0.0, b = 0.0;
                    for (int j = 0; j <= D; ++j)     a += static_cast<double>(h_[static_cast<size_t>(2 * j)])     * ext[D + i - static_cast<size_t>(j)];
                    for (int j = 0; j <  D; ++j)     b += static_cast<double>(h_[static_cast<size_t>(2 * j + 1)]) * ext[D + i - static_cast<size_t>(j)];
                    y[2 * i] = static_cast<float>(2.0 * a); y[2 * i + 1] = static_cast<float>(2.0 * b);
                }
                std::copy(ext.end() - D, ext.end(), hist.begin());
            }
            void down(size_t s) {
                auto& u = bufs_[s + 1]; auto& y = bufs_[s]; auto& hist = st_[s].dn_hist;
                const size_t m = u.size(), n = m / 2;
                std::vector<float> ext(2 * D + m);
                std::copy(hist.begin(), hist.end(), ext.begin());
                std::copy(u.begin(), u.end(), ext.begin() + 2 * D);
                y.assign(n, 0.0f);
                for (size_t i = 0; i < n; ++i) {
                    double a = 0.0;
                    for (int k = 0; k < T; ++k) a += static_cast<double>(h_[static_cast<size_t>(k)]) * ext[2 * D + 2 * i - static_cast<size_t>(k)];
                    y[i] = static_cast<float>(a);
                }
                std::copy(ext.end() - 2 * D, ext.end(), hist.begin());
            }

            int factor_ = 1, stages_ = 0;
            std::vector<float> h_;
            std::vector<Stage> st_;
            std::vector<std::vector<float>> bufs_;
        };

        class Saturator : public Effect {
        public:
            enum class Mode { Soft, Tube, Tape, Hard, Fold, Asymmetric };

            Mode   mode       = Mode::Soft;
            double drive_db   = 12.0;   ///< input gain into the shaper
            double bias       = 0.0;    ///< -1..1, adds even harmonics (Tube/Asymmetric); Tube always adds a little
            double tone_hz    = 0.0;    ///< post low-pass; 0 = off
            double mix        = 1.0;
            double output_db  = 0.0;
            int    oversample = 2;      ///< 1 (off), 2, 4 or 8 - reduces aliasing of the added harmonics

            std::string name() const override { return "Saturator"; }
            size_t latency_samples() const override { return Oversampler1::latency_for(oversample); }

            void reset() override {
                for (int c = 0; c < 2; ++c) { os_[c].reset(); dc_[c].reset(); tone_[c].reset(); bump_[c].reset(); lp_[c] = 0.0f;
                    std::fill(dry_[c].begin(), dry_[c].end(), 0.0f); }
                dpos_ = 0;
            }

            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                const int want = Oversampler1::normalise_factor(oversample);
                if (os_[0].factor() != want) {
                    for (int c = 0; c < 2; ++c) os_[c].prepare(want);
                    const size_t lat = os_[0].latency();
                    for (int c = 0; c < 2; ++c) dry_[c].assign(lat, 0.0f);
                    dpos_ = 0;
                }
                const double g = db_to_lin(drive_db), outg = db_to_lin(output_db);
                const double hi_sr = sr_ * want;
                const double tb = (mode == Mode::Tube) ? 0.2 + 0.5 * std::max(0.0, bias) : bias * 0.6;
                const bool tape = (mode == Mode::Tape);
                if (tone_hz > 1.0) { const BiquadCoef c = design_biquad(FilterType::LowPass, sr_, tone_hz, 0.7071); tone_[0].set(c); tone_[1].set(c); }
                const BiquadCoef bc = design_biquad(FilterType::Peak, sr_, 90.0, 0.7, 2.0);
                bump_[0].set(bc); bump_[1].set(bc);
                const float tape_lp = static_cast<float>(1.0 - std::exp(-TWO_PI * 16000.0 / hi_sr));
                const double wet = std::min(1.0, std::max(0.0, mix));
                const size_t lat = os_[0].latency();
                float* ch[2] = { L, R };
                const Mode md = mode;

                for (int c = 0; c < 2; ++c) {
                    float* x = ch[c];
                    // delayed dry copy, aligned with the oversampler's latency (for the mix control)
                    std::vector<float> dry(n);
                    if (lat == 0) std::copy(x, x + n, dry.begin());
                    else { auto& ring = dry_[c]; size_t pp = dpos_;
                        for (size_t i = 0; i < n; ++i) { dry[i] = ring[pp]; ring[pp] = x[i]; pp = (pp + 1) % ring.size(); } }
                    if (tape) for (size_t i = 0; i < n; ++i) x[i] = bump_[c].tick(x[i]);
                    float lp = lp_[c];
                    os_[c].process(x, x, n, [&](float* hi, size_t m) {
                        for (size_t i = 0; i < m; ++i) {
                            double v = static_cast<double>(hi[i]) * g, y;
                            switch (md) {
                            case Mode::Soft:       y = std::tanh(v); break;
                            case Mode::Tube:       y = std::tanh(v + tb) - std::tanh(tb); break;
                            case Mode::Tape:       y = std::tanh(v * 0.9); break;
                            case Mode::Hard:       y = std::max(-1.0, std::min(1.0, v)); break;
                            case Mode::Fold:       y = std::sin(v * 1.5707963267948966); break;
                            case Mode::Asymmetric: { const double vb = v + tb; y = vb > 0.0 ? std::tanh(vb) : vb / (1.0 + std::fabs(vb)); break; }
                            default: y = v;
                            }
                            float f = static_cast<float>(y);
                            if (tape) { lp += tape_lp * (f - lp); f = lp; }
                            hi[i] = f;
                        }
                    });
                    lp_[c] = lp;
                    for (size_t i = 0; i < n; ++i) {
                        float w = x[i];
                        if (md == Mode::Tube || md == Mode::Asymmetric) w = dc_[c].tick(w);
                        if (tone_hz > 1.0) w = tone_[c].tick(w);
                        x[i] = static_cast<float>((dry[i] * (1.0 - wet) + w * wet * outg));
                    }
                }
                if (lat > 0) dpos_ = (dpos_ + n) % dry_[0].size();
            }

        private:
            Oversampler1 os_[2];
            DCBlocker dc_[2];
            Biquad tone_[2], bump_[2];
            float lp_[2] = {0, 0};
            std::vector<float> dry_[2];
            size_t dpos_ = 0;
        };

        class Bitcrusher : public Effect {
        public:
            double bits      = 8.0;     ///< 2..24 -> 2^bits - 1 levels (mid-tread: silence stays silent); fractions allowed
            double downsample = 1.0;    ///< hold each sample for this many samples (1 = off, 4 = quarter rate)
            bool   dither    = false;   ///< TPDF dither before quantising (seeded, deterministic)
            double mix       = 1.0;
            uint64_t seed    = 1;

            std::string name() const override { return "Bitcrusher"; }
            void reset() override { phase_[0] = phase_[1] = 1.0; held_[0] = held_[1] = 0.0f; rng_.reseed(seed); }

            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                const double levels = std::max(1.0, std::pow(2.0, std::min(24.0, std::max(1.0, bits)) - 1.0) - 1.0);   // steps per polarity
                const double inc = 1.0 / std::max(1.0, downsample);
                const double wet = std::min(1.0, std::max(0.0, mix));
                float* ch[2] = { L, R };
                for (int c = 0; c < 2; ++c)
                    for (size_t i = 0; i < n; ++i) {
                        const float x = ch[c][i];
                        if (phase_[c] >= 1.0) {                       // capture, then hold for `downsample` samples
                            phase_[c] -= 1.0;
                            double v = static_cast<double>(x) * levels;
                            if (dither) v += (rng_.uniform() - rng_.uniform());
                            held_[c] = static_cast<float>(std::round(v) / levels);
                        }
                        phase_[c] += inc;
                        ch[c][i] = static_cast<float>(x * (1.0 - wet) + held_[c] * wet);
                    }
            }
        private:
            double phase_[2] = {1.0, 1.0}; float held_[2] = {0, 0}; Rng rng_;
        };
    }
}
