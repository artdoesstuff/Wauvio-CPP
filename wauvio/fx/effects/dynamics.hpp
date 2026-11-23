#pragma once
#include "../core/fx_effect.hpp"
#include <deque>

namespace wauvio {
    namespace fx {
        class Compressor : public Effect {
        public:
            enum class Detect { Peak, RMS };

            double threshold_db = -18.0;
            double ratio        = 4.0;      ///< 1 = off, large = limiting
            double attack_ms    = 10.0;
            double release_ms   = 100.0;
            double knee_db      = 6.0;      ///< 0 = hard knee
            double makeup_db    = 0.0;
            double mix          = 1.0;      ///< <1 = parallel ("New York") compression
            double stereo_link  = 1.0;      ///< 1 = both channels get the same gain reduction
            double sidechain_hpf_hz = 0.0;  ///< high-pass the detector (keeps bass from pumping the mix); 0 = off
            Detect detector     = Detect::Peak;

            std::string name() const override { return "Compressor"; }

            /// Deepest gain reduction (dB, >= 0) seen during the last process() call.
            double gain_reduction_db() const noexcept { return max_gr_db_; }

            void reset() override { gr_[0] = gr_[1] = 0.0; ms_[0] = ms_[1] = 0.0; hp_[0].reset(); hp_[1].reset(); max_gr_db_ = 0.0; }

            using Effect::process;

            void process(float* L, float* R, size_t n) override { process_sidechain(L, R, n, nullptr, nullptr); }

            /// Detect on an external key signal (e.g. a kick) instead of the input itself.
            /// kL/kR may be null (= use the input). Key buffers must hold n samples.
            void process_sidechain(float* L, float* R, size_t n, const float* kL, const float* kR) {
                const double sr = sr_;
                const double att = time_coef(attack_ms, sr), rel = time_coef(release_ms, sr);
                const double rms_c = 1.0 - time_coef(10.0, sr);
                const double inv_r = 1.0 / std::max(ratio, 1.0);
                const bool use_hpf = sidechain_hpf_hz > 1.0;
                if (use_hpf) { const BiquadCoef c = design_biquad(FilterType::HighPass, sr, sidechain_hpf_hz, 0.7071);
                    hp_[0].set(c); hp_[1].set(c); }
                const double mk = db_to_lin(makeup_db);
                const double link = std::min(1.0, std::max(0.0, stereo_link));
                const double wet = std::min(1.0, std::max(0.0, mix));
                double worst = 0.0;

                for (size_t i = 0; i < n; ++i) {
                    const float xl = L[i], xr = R[i];
                    float kl = kL ? kL[i] : xl, kr = kR ? kR[i] : xr;
                    if (use_hpf) { kl = hp_[0].tick(kl); kr = hp_[1].tick(kr); }
                    double dl, dr;
                    if (detector == Detect::RMS) {
                        ms_[0] += rms_c * (static_cast<double>(kl) * kl - ms_[0]);
                        ms_[1] += rms_c * (static_cast<double>(kr) * kr - ms_[1]);
                        dl = std::sqrt(2.0 * ms_[0]); dr = std::sqrt(2.0 * ms_[1]);   // sine reads like its peak
                    } else { dl = std::fabs(kl); dr = std::fabs(kr); }
                    const double mx = std::max(dl, dr);
                    const double lv[2] = { link * mx + (1.0 - link) * dl, link * mx + (1.0 - link) * dr };

                    double g[2];
                    for (int c = 0; c < 2; ++c) {
                        const double x_db = 20.0 * std::log10(lv[c] + 1e-9);
                        const double over = x_db - threshold_db;
                        double change;                                         // dB, <= 0
                        if (knee_db <= 0.0)              change = over > 0.0 ? over * (inv_r - 1.0) : 0.0;
                        else if (2.0 * over < -knee_db)  change = 0.0;
                        else if (2.0 * std::fabs(over) <= knee_db) { const double t = over + knee_db * 0.5;
                            change = (inv_r - 1.0) * t * t / (2.0 * knee_db); }
                        else                             change = over * (inv_r - 1.0);
                        const double coef = (change < gr_[c]) ? att : rel;     // more reduction -> attack
                        gr_[c] = coef * gr_[c] + (1.0 - coef) * change;
                        worst = std::min(worst, gr_[c]);
                        g[c] = std::pow(10.0, (gr_[c]) / 20.0) * mk;
                    }
                    L[i] = static_cast<float>(xl * (1.0 - wet) + xl * g[0] * wet);
                    R[i] = static_cast<float>(xr * (1.0 - wet) + xr * g[1] * wet);
                }
                max_gr_db_ = -worst;
            }

        private:
            double gr_[2] = {0, 0}, ms_[2] = {0, 0}, max_gr_db_ = 0.0;
            Biquad hp_[2];
        };

        // ----------------------------------------------------------------------------
        //  Limiter - lookahead brickwall
        //  Guarantees |output sample| <= ceiling. With true_peak the detector also looks at
        //  4x-interpolated points between samples (16-tap Lanczos), so the inter-sample
        //  (true) peak stays within a fraction of a dB of the ceiling. The ceiling is only
        //  mathematically guaranteed for sample peaks; fx::master() additionally verifies the
        //  true peak with the full oversampling meter.
        // ----------------------------------------------------------------------------
        class Limiter : public Effect {
        public:
            double ceiling_db    = -1.0;
            double input_gain_db = 0.0;   ///< push into the limiter (loudness maximiser)
            double release_ms    = 80.0;
            double lookahead_ms  = 5.0;
            bool   true_peak     = true;

            std::string name() const override { return "Limiter"; }
            size_t latency_samples() const override { return la_samples() + extra(); }
            double gain_reduction_db() const noexcept { return max_gr_db_; }

            void reset() override {
                D_ = la_samples(); K_ = extra();
                for (auto& v : dly_) v.assign(D_ + K_, 0.0f);
                dpos_ = 0;
                for (auto& h : hist_) for (auto& x : h) x = 0.0f;
                dq_.clear(); t_ = 0;
                ring_.assign(D_ + 1, 1.0); rpos_ = 0; sum_ = static_cast<double>(D_ + 1); r_ = 1.0;
                max_gr_db_ = 0.0;
                build_taps();
            }

            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                if (la_samples() != D_ || extra() != K_) reset();
                const double ceil_lin = db_to_lin(ceiling_db);
                const float  in_gain  = static_cast<float>(db_to_lin(input_gain_db));
                const double rel = time_coef(release_ms, sr_);
                const size_t D = D_, K = K_;
                const bool tp = (K > 0);
                double worst = 1.0;

                for (size_t i = 0; i < n; ++i) {
                    const float xl = L[i] * in_gain, xr = R[i] * in_gain;
                    double p = 0.0;
                    if (tp) {
                        for (int c = 0; c < 2; ++c) {
                            float* h = hist_[c];                              // h[0..15] = x[t-15] .. x[t]
                            for (int k = 0; k < 15; ++k) h[k] = h[k + 1];
                            h[15] = (c == 0) ? xl : xr;
                            // sample s = t-7 is h[8]; the pair (s-1, s) = (h[7], h[8]); needs h[0]..h[15]
                            p = std::max(p, static_cast<double>(std::fabs(h[8])));
                            for (int f = 0; f < 3; ++f) {
                                double acc = 0.0;
                                for (int k = 0; k < 16; ++k) acc += static_cast<double>(taps_[f][k]) * h[k];
                                p = std::max(p, std::fabs(acc));
                            }
                        }
                    } else {
                        p = std::max(std::fabs(static_cast<double>(xl)), std::fabs(static_cast<double>(xr)));
                    }
                    const double g = (p > ceil_lin) ? ceil_lin / p : 1.0;

                    // sliding minimum of g over the last D+1 arrivals
                    while (!dq_.empty() && dq_.back().second >= g) dq_.pop_back();
                    dq_.emplace_back(t_, g);
                    while (dq_.front().first + static_cast<long long>(D) < t_) dq_.pop_front();
                    const double m = dq_.front().second;
                    ++t_;

                    // instant attack, exponential release, then moving average = smooth attack ramp
                    r_ = (m < r_) ? m : rel * r_ + (1.0 - rel) * m;
                    sum_ += r_ - ring_[rpos_]; ring_[rpos_] = r_; rpos_ = (rpos_ + 1) % ring_.size();
                    const double G = sum_ / static_cast<double>(ring_.size());
                    worst = std::min(worst, G);

                    // audio delayed by D+K samples
                    const float ol = dly_[0][dpos_], orr = dly_[1][dpos_];
                    dly_[0][dpos_] = xl; dly_[1][dpos_] = xr;
                    dpos_ = (dpos_ + 1) % dly_[0].size();
                    const float cl = static_cast<float>(ceil_lin);
                    L[i] = std::max(-cl, std::min(cl, static_cast<float>(ol * G)));
                    R[i] = std::max(-cl, std::min(cl, static_cast<float>(orr * G)));
                }
                max_gr_db_ = -lin_to_db(worst);
            }

        private:
            size_t la_samples() const { return std::max<size_t>(1, static_cast<size_t>(std::llround(lookahead_ms * 0.001 * sr_))); }
            size_t extra() const { return true_peak ? 7 : 0; }     // detector look-ahead beyond the pair being tested
            void build_taps() {
                // Lanczos-8 interpolation of the point u (in samples) after h[7], using h[0..15]
                static const double fr[3] = { 0.25, 0.5, 0.75 };
                for (int f = 0; f < 3; ++f) {
                    double sum = 0.0, w[16];
                    for (int k = 0; k < 16; ++k) {
                        const double x = (k - 7) - fr[f];                     // distance from the interpolation point
                        const double sinc = (std::fabs(x) < 1e-12) ? 1.0 : std::sin(PI * x) / (PI * x);
                        const double lz = (std::fabs(x) < 8.0) ? ((std::fabs(x) < 1e-12) ? 1.0 : std::sin(PI * x / 8.0) / (PI * x / 8.0)) : 0.0;
                        w[k] = sinc * lz; sum += w[k];
                    }
                    for (int k = 0; k < 16; ++k) taps_[f][k] = static_cast<float>(w[k] / sum);
                }
            }
            size_t D_ = 0, K_ = 0;
            std::vector<float> dly_[2];
            size_t dpos_ = 0;
            float hist_[2][16] = {};
            float taps_[3][16] = {};
            std::deque<std::pair<long long, double>> dq_;
            long long t_ = 0;
            std::vector<double> ring_; size_t rpos_ = 0; double sum_ = 0.0, r_ = 1.0;
            double max_gr_db_ = 0.0;
        };

        class Gate : public Effect {
        public:
            double threshold_db   = -50.0;
            double range_db       = -80.0;   ///< attenuation when closed
            double attack_ms      = 1.0;
            double hold_ms        = 30.0;
            double release_ms     = 150.0;
            double hysteresis_db  = 4.0;

            std::string name() const override { return "Gate"; }
            void reset() override { env_ = 0.0; open_ = false; hold_ = 0; g_ = 1.0; }

            using Effect::process;

            void process(float* L, float* R, size_t n) override {
                const double det_rel = time_coef(10.0, sr_);
                const double att = time_coef(attack_ms, sr_), rel = time_coef(release_ms, sr_);
                const double floor_g = db_to_lin(range_db);
                const long long hold_n = static_cast<long long>(hold_ms * 0.001 * sr_);
                for (size_t i = 0; i < n; ++i) {
                    const double lv = std::max(std::fabs(static_cast<double>(L[i])), std::fabs(static_cast<double>(R[i])));
                    env_ = (lv > env_) ? lv : det_rel * env_ + (1.0 - det_rel) * lv;
                    const double e_db = lin_to_db(env_);
                    if (e_db > threshold_db) { open_ = true; hold_ = hold_n; }
                    else if (open_ && e_db < threshold_db - hysteresis_db) { if (hold_ > 0) --hold_; else open_ = false; }
                    const double target = open_ ? 1.0 : floor_g;
                    const double coef = (target > g_) ? att : rel;
                    g_ = coef * g_ + (1.0 - coef) * target;
                    L[i] = static_cast<float>(L[i] * g_); R[i] = static_cast<float>(R[i] * g_);
                }
            }
        private:
            double env_ = 0.0, g_ = 1.0; bool open_ = false; long long hold_ = 0;
        };

        // ----------------------------------------------------------------------------
        //  Pump - tempo-synced ducking. Gives the house/trance "sidechain" pulse without
        //  needing a separate kick track. For real track-to-track sidechaining use
        //  Compressor::process_sidechain (or fx::Mixer::sidechain).
        // ----------------------------------------------------------------------------
        class Pump : public Effect {
        public:
            double bpm          = 128.0;
            double depth        = 0.8;      ///< 0 = no ducking, 1 = full silence at the hit
            double attack_ms    = 4.0;
            double release_ms   = 250.0;
            double shape        = 1.6;      ///< 1 = linear recovery, >1 = fast then settle, <1 = slow start
            double offset_sec   = 0.0;      ///< time of the first hit
            std::vector<double> pattern { 0.0 };  ///< hit positions in beats inside one loop
            double loop_beats   = 1.0;      ///< loop length in beats (1 = every beat; 4 = hits placed in a bar)

            std::string name() const override { return "Pump"; }
            void reset() override { pos_ = 0; }

            /// Gain at `t` seconds (the curve the effect applies).
            double gain_at(double t) const {
                if (bpm <= 0.0 || pattern.empty()) return 1.0;
                const double tb = (t - offset_sec) * bpm / 60.0;
                if (tb < 0.0) return 1.0;
                const double loop = std::max(loop_beats, 1e-6);
                const double lp = std::fmod(tb, loop);
                double best = -1e300;                          // latest hit at or before lp (possibly in the previous loop)
                for (double h : pattern) { double hh = std::fmod(h, loop); if (hh < 0) hh += loop;
                    const double d = (hh <= lp) ? hh : hh - loop;
                    if (d > best) best = d; }
                double tau = (lp - best) * 60.0 / bpm;         // seconds since the hit
                if (tb < loop && best < 0.0) return 1.0;       // no hit has happened yet
                const double att = attack_ms * 0.001, rel = std::max(release_ms * 0.001, 1e-4);
                const double low = 1.0 - std::min(1.0, std::max(0.0, depth));
                if (tau < att) { const double u = att > 0 ? tau / att : 1.0; return 1.0 - (1.0 - low) * u; }
                const double u = (tau - att) / rel;
                if (u >= 1.0) return 1.0;
                return 1.0 - (1.0 - low) * std::pow(1.0 - u, shape);
            }

            using Effect::process;

            void process(float* L, float* R, size_t n) override {
                for (size_t i = 0; i < n; ++i) {
                    const float g = static_cast<float>(gain_at(static_cast<double>(pos_ + i) / sr_));
                    L[i] *= g; R[i] *= g;
                }
                pos_ += n;
            }
        private:
            size_t pos_ = 0;
        };

        class MultibandCompressor : public Effect {
        public:
            struct BandSettings {
                double threshold_db = -20.0, ratio = 2.0, attack_ms = 20.0, release_ms = 150.0, makeup_db = 0.0, knee_db = 6.0;
            };
            double low_freq  = 120.0;     ///< low / mid crossover
            double high_freq = 2500.0;    ///< mid / high crossover
            BandSettings low, mid, high;
            double mix = 1.0;

            MultibandCompressor() {
                low.threshold_db = -22; low.ratio = 3.0; low.attack_ms = 30; low.release_ms = 200;
                mid.threshold_db = -22; mid.ratio = 2.0; mid.attack_ms = 15; mid.release_ms = 150;
                high.threshold_db = -26; high.ratio = 2.0; high.attack_ms = 5; high.release_ms = 100;
            }
            std::string name() const override { return "MultibandCompressor"; }

            /// Deepest gain reduction (dB) per band during the last process() call.
            double gain_reduction_db(int band) const { return gr_[std::min(2, std::max(0, band))]; }

            void reset() override {
                for (auto& ch : xo_) for (auto& b : ch) b.reset();
                for (auto& c : comp_) c.reset();
            }

            using Effect::process;

            void process(float* L, float* R, size_t n) override {
                const double f1 = low_freq, f2 = std::max(high_freq, low_freq * 1.5);
                const BiquadCoef lp1 = design_biquad(FilterType::LowPass,  sr_, f1, 0.70710678);
                const BiquadCoef hp1 = design_biquad(FilterType::HighPass, sr_, f1, 0.70710678);
                const BiquadCoef lp2 = design_biquad(FilterType::LowPass,  sr_, f2, 0.70710678);
                const BiquadCoef hp2 = design_biquad(FilterType::HighPass, sr_, f2, 0.70710678);
                for (auto& ch : xo_) {
                    ch[0].set(lp1); ch[1].set(lp1); ch[2].set(hp1); ch[3].set(hp1);     // split 1 (LR4 = 2x Butterworth)
                    ch[4].set(lp2); ch[5].set(lp2); ch[6].set(hp2); ch[7].set(hp2);     // split 2 on the upper part
                    ch[8].set(lp2); ch[9].set(lp2); ch[10].set(hp2); ch[11].set(hp2);   // phase compensation on the low band
                }
                const BandSettings* bs[3] = { &low, &mid, &high };
                for (int b = 0; b < 3; ++b) {
                    Compressor& c = comp_[b];
                    c.threshold_db = bs[b]->threshold_db; c.ratio = bs[b]->ratio; c.attack_ms = bs[b]->attack_ms;
                    c.release_ms = bs[b]->release_ms; c.makeup_db = bs[b]->makeup_db; c.knee_db = bs[b]->knee_db;
                    c.stereo_link = 1.0; c.mix = 1.0;
                    c.ensure_prepared(static_cast<int>(sr_));
                }
                const size_t BLK = 4096;
                std::vector<float> bl[3], br[3];
                for (int b = 0; b < 3; ++b) { bl[b].resize(BLK); br[b].resize(BLK); }
                double worst[3] = {0, 0, 0};
                for (size_t off = 0; off < n; off += BLK) {
                    const size_t m = std::min(BLK, n - off);
                    for (size_t i = 0; i < m; ++i) {
                        float in[2] = { L[off + i], R[off + i] };
                        float lo[2], mi[2], hi[2];
                        for (int c = 0; c < 2; ++c) {
                            auto& s = xo_[c];
                            float a = s[1].tick(s[0].tick(in[c]));              // LP1
                            float rest = s[3].tick(s[2].tick(in[c]));           // HP1
                            float m2 = s[5].tick(s[4].tick(rest));              // LP2 of rest = mid
                            float h2 = s[7].tick(s[6].tick(rest));              // HP2 of rest = high
                            float al = s[9].tick(s[8].tick(a));                 // low through LP2 ...
                            float ah = s[11].tick(s[10].tick(a));               // ... + HP2 (allpass sum)
                            lo[c] = al + ah; mi[c] = m2; hi[c] = h2;
                        }
                        bl[0][i] = lo[0]; br[0][i] = lo[1];
                        bl[1][i] = mi[0]; br[1][i] = mi[1];
                        bl[2][i] = hi[0]; br[2][i] = hi[1];
                    }
                    for (int b = 0; b < 3; ++b) {
                        comp_[b].process(bl[b].data(), br[b].data(), m);
                        worst[b] = std::max(worst[b], comp_[b].gain_reduction_db());
                    }
                    const float wet = static_cast<float>(std::min(1.0, std::max(0.0, mix)));
                    for (size_t i = 0; i < m; ++i) {
                        const float sl = bl[0][i] + bl[1][i] + bl[2][i], sr = br[0][i] + br[1][i] + br[2][i];
                        L[off + i] = L[off + i] * (1.0f - wet) + sl * wet;
                        R[off + i] = R[off + i] * (1.0f - wet) + sr * wet;
                    }
                }
                for (int b = 0; b < 3; ++b) gr_[b] = worst[b];
            }

        private:
            Biquad xo_[2][12];
            Compressor comp_[3];
            double gr_[3] = {0, 0, 0};
        };
    }
}
