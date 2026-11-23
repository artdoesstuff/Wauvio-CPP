#pragma once
#include "../effects/dynamics.hpp"
#include "../effects/saturation.hpp"

namespace wauvio {
    namespace fx {
        struct LoudnessReport {
            bool   valid = false;              ///< false if the audio is shorter than one 400 ms block or entirely gated out
            double integrated_lufs   = -120.0; ///< gated programme loudness
            double momentary_max_lufs = -120.0;///< loudest 400 ms
            double short_term_max_lufs = -120.0;///< loudest 3 s
            double loudness_range_lu = 0.0;    ///< LRA (10th..95th percentile of gated short-term values)
            double sample_peak_db    = -120.0; ///< dBFS
            double true_peak_db      = -120.0; ///< dBTP (4x oversampled)
        };

        /// K-weighting filter (BS.1770): high-shelf (+4 dB @ 1.68 kHz) then RLB high-pass (38 Hz).
        /// Coefficients are derived for any sample rate (same method as libebur128).
        class KWeighting {
        public:
            void prepare(double sr) {
                {   const double f0 = 1681.974450955533, G = 3.999843853973347, Q = 0.7071752369554196;
                    const double K = std::tan(PI * f0 / sr), Vh = std::pow(10.0, G / 20.0), Vb = std::pow(Vh, 0.4996667741545416);
                    const double a0 = 1.0 + K / Q + K * K;
                    BiquadCoef c; c.b0 = (Vh + Vb * K / Q + K * K) / a0; c.b1 = 2.0 * (K * K - Vh) / a0; c.b2 = (Vh - Vb * K / Q + K * K) / a0;
                    c.a1 = 2.0 * (K * K - 1.0) / a0; c.a2 = (1.0 - K / Q + K * K) / a0; s1_.set(c); }
                {   const double f0 = 38.13547087602444, Q = 0.5003270373238773;
                    const double K = std::tan(PI * f0 / sr);
                    const double a0 = 1.0 + K / Q + K * K;
                    BiquadCoef c; c.b0 = 1.0; c.b1 = -2.0; c.b2 = 1.0; c.a1 = 2.0 * (K * K - 1.0) / a0; c.a2 = (1.0 - K / Q + K * K) / a0; s2_.set(c); }
                s1_.reset(); s2_.reset();
            }
            float tick(float x) noexcept { return s2_.tick(s1_.tick(x)); }
        private:
            Biquad s1_, s2_;
        };

        inline LoudnessReport measure_loudness(const StereoBuffer& buf, int sr = 0) {
            if (sr <= 0) sr = global_config().sample_rate;
            LoudnessReport r;
            const size_t n = buf.size();
            if (n == 0) return r;

            // peaks
            float sp = 0.0f;
            for (size_t i = 0; i < n; ++i) sp = std::max(sp, std::max(std::fabs(buf.L[i]), std::fabs(buf.R[i])));
            r.sample_peak_db = lin_to_db(sp);
            {   float tp = 0.0f;
                for (int c = 0; c < 2; ++c) {
                    Oversampler1 os; os.prepare(4);
                    const Buffer& x = c ? buf.R : buf.L; Buffer tmp(n);
                    os.process(x.data(), tmp.data(), n, [&](float* hi, size_t m) { for (size_t i = 0; i < m; ++i) tp = std::max(tp, std::fabs(hi[i])); });
                }
                r.true_peak_db = lin_to_db(std::max(tp, sp)); }

            // K-weighted energy per 100 ms sub-block
            const size_t hop = static_cast<size_t>(std::llround(sr * 0.1));
            const size_t nsub = n / hop;
            if (nsub < 4) return r;
            KWeighting kl, kr; kl.prepare(sr); kr.prepare(sr);
            std::vector<double> sub(nsub, 0.0);
            for (size_t j = 0; j < nsub; ++j) {
                double e = 0.0;
                for (size_t i = j * hop; i < (j + 1) * hop; ++i) { const double a = kl.tick(buf.L[i]), b = kr.tick(buf.R[i]); e += a * a + b * b; }
                sub[j] = e / static_cast<double>(hop);                    // sum over channels of mean square (weights 1.0)
            }
            auto to_lufs = [](double z) { return z > 0.0 ? -0.691 + 10.0 * std::log10(z) : -120.0; };

            // momentary (400 ms, 75 % overlap)
            std::vector<double> blocks;
            for (size_t j = 0; j + 4 <= nsub; ++j) blocks.push_back(0.25 * (sub[j] + sub[j + 1] + sub[j + 2] + sub[j + 3]));
            for (double z : blocks) r.momentary_max_lufs = std::max(r.momentary_max_lufs, to_lufs(z));

            // integrated with absolute (-70) then relative (-10 LU) gates
            auto gated_mean = [&](double thr_lufs, double& count) {
                double s = 0.0; count = 0.0;
                for (double z : blocks) if (to_lufs(z) > thr_lufs) { s += z; count += 1.0; }
                return count > 0.0 ? s / count : 0.0; };
            double cnt = 0.0; const double m_abs = gated_mean(-70.0, cnt);
            if (cnt == 0.0) return r;
            const double rel = to_lufs(m_abs) - 10.0;
            const double m_rel = gated_mean(rel, cnt);
            if (cnt == 0.0) return r;
            r.integrated_lufs = to_lufs(m_rel);
            r.valid = true;

            // short-term (3 s) and LRA
            if (nsub >= 30) {
                std::vector<double> st;
                double run = 0.0;
                for (size_t j = 0; j < 30; ++j) run += sub[j];
                st.push_back(run / 30.0);
                for (size_t j = 30; j < nsub; ++j) { run += sub[j] - sub[j - 30]; st.push_back(run / 30.0); }
                for (double z : st) r.short_term_max_lufs = std::max(r.short_term_max_lufs, to_lufs(z));
                std::vector<double> above; double s = 0.0;
                for (double z : st) if (to_lufs(z) > -70.0) { above.push_back(z); s += z; }
                if (!above.empty()) {
                    const double rel_lra = to_lufs(s / above.size()) - 20.0;
                    std::vector<double> keep;
                    for (double z : above) if (to_lufs(z) > rel_lra) keep.push_back(to_lufs(z));
                    if (keep.size() > 1) {
                        std::sort(keep.begin(), keep.end());
                        auto pct = [&](double p) { const double pos = p * (keep.size() - 1); const size_t i0 = static_cast<size_t>(pos);
                            const size_t i1 = std::min(keep.size() - 1, i0 + 1); return keep[i0] + (keep[i1] - keep[i0]) * (pos - i0); };
                        r.loudness_range_lu = pct(0.95) - pct(0.10);
                    }
                }
            }
            return r;
        }

        /// Bring `buf` to `target_lufs` (integrated). If the true peak would exceed `ceiling_dbtp`
        /// a lookahead limiter is used (limit = true) or the gain is reduced to fit (limit = false).
        /// The buffer keeps its length. Returns the loudness measured after processing.
        inline LoudnessReport normalize_loudness(StereoBuffer& buf, int sr, double target_lufs,
                                                 double ceiling_dbtp = -1.0, bool limit = true) {
            if (sr <= 0) sr = global_config().sample_rate;
            LoudnessReport r = measure_loudness(buf, sr);
            if (!r.valid) return r;
            double gain = std::max(-60.0, std::min(60.0, target_lufs - r.integrated_lufs));
            if (!limit) {
                const double max_gain = ceiling_dbtp - r.true_peak_db;
                gain = std::min(gain, max_gain);
                apply_gain(buf, static_cast<float>(db_to_lin(gain)));
                return measure_loudness(buf, sr);
            }
            const StereoBuffer original = buf;
            LoudnessReport best = r; StereoBuffer best_buf = original;
            double in_gain = gain, best_err = 1e9;
            for (int it = 0; it < 4; ++it) {
                StereoBuffer work = original;
                Chain c; auto& lim = c.add<Limiter>(); lim.input_gain_db = in_gain; lim.ceiling_db = ceiling_dbtp; lim.true_peak = true;
                c.process(work, sr, false);
                const LoudnessReport m = measure_loudness(work, sr);
                const double err = std::fabs(m.integrated_lufs - target_lufs);
                if (err < best_err) { best_err = err; best = m; best_buf = std::move(work); }
                if (err < 0.1) break;
                in_gain += (target_lufs - m.integrated_lufs);
            }
            buf = std::move(best_buf);
            return best;
        }
    }
}
