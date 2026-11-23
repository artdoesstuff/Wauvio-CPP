#pragma once

#include "common.hpp"

namespace wauvio {
    namespace filter {
        // =============================================================================
        //  §13  STATE VARIABLE FILTER (SVF)
        //  Topology-preserving TPT form (Vadim Zavalishin) - no pre-warping needed.
        //  Provides simultaneous LP, HP, and BP outputs.
        //  Q > 0.5: resonance; Q = 0.707: Butterworth; Q → ∞: self-oscillation.
        // =============================================================================

        class SVFilter {
        public:
            enum class Mode { LowPass, HighPass, BandPass };

            Mode   mode     = Mode::LowPass;
            double cutoff   = 1000.0;  ///< Hz
            double Q        = 0.707;   ///< Resonance (0.5–30 typical range)

            SVFilter() = default;
            SVFilter(Mode m, double fc, double q = 0.707) : mode(m), cutoff(fc), Q(q) {}

            void set_params(double fc, double q, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                cutoff = fc;  Q = q;
                update_coeffs(sample_rate);
            }

            /// Process one sample.  Call update_coeffs() after changing params.
            float tick(float x, int sample_rate) noexcept {
                // Lazy coefficient update if sr hasn't been cached
                if (cached_sr_ != sample_rate) update_coeffs(sample_rate);

                // SVF core (TPT)
                const float hp = (x - (2.0f * Q_ + g_) * s1_ - s2_) * h_;
                const float bp = g_ * hp + s1_;
                const float lp = g_ * bp + s2_;

                s1_ = g_ * hp + bp;
                s2_ = g_ * bp + lp;

                switch (mode) {
                    case Mode::LowPass:  return lp;
                    case Mode::HighPass: return hp;
                    case Mode::BandPass: return bp;
                }
                return lp;
            }

            void process(Buffer& buf, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                update_coeffs(sample_rate);
                for (auto& s : buf) s = tick(s, sample_rate);
            }

            void reset() noexcept { s1_ = s2_ = 0.0f; }

            void update_coeffs(int sample_rate) noexcept {
                cached_sr_ = sample_rate;
                const double w  = std::tan(PI * cutoff / sample_rate);
                g_  = static_cast<float>(w);
                Q_  = static_cast<float>(1.0 / (2.0 * Q));
                h_  = 1.0f / (1.0f + 2.0f * Q_ * g_ + g_ * g_);
            }

        private:
            float g_ = 0.0f, Q_ = 0.707f, h_ = 1.0f;
            float s1_ = 0.0f, s2_ = 0.0f;
            int   cached_sr_ = 0;
        };

        // =============================================================================
        //  §14  LEGACY 1-POLE FILTERS  (kept for compatibility with existing modules)
        // =============================================================================

        class LowPassFilter {
        public:
            explicit LowPassFilter(double cutoff_hz = 1000.0, int sample_rate = 0) {
                set_cutoff(cutoff_hz, sample_rate ? sample_rate : global_config().sample_rate);
            }
            void set_cutoff(double fc, int sr = 0) {
                if (sr <= 0) sr = global_config().sample_rate;
                const double rc = 1.0 / (TWO_PI * fc);
                const double dt = 1.0 / sr;
                alpha_ = static_cast<float>(dt / (rc + dt));
                z1_ = 0.0f;
            }
            float tick(float x) noexcept { z1_ += alpha_ * (x - z1_); return z1_; }
            void  process(Buffer& b) noexcept { for (auto& s : b) s = tick(s); }
            void  reset()  noexcept { z1_ = 0.0f; }
        private:
            float alpha_ = 0.1f, z1_ = 0.0f;
        };

        class HighPassFilter {
        public:
            explicit HighPassFilter(double cutoff_hz = 200.0, int sample_rate = 0) {
                set_cutoff(cutoff_hz, sample_rate ? sample_rate : global_config().sample_rate);
            }
            void set_cutoff(double fc, int sr = 0) {
                if (sr <= 0) sr = global_config().sample_rate;
                const double rc = 1.0 / (TWO_PI * fc);
                const double dt = 1.0 / sr;
                alpha_ = static_cast<float>(rc / (rc + dt));
                xp_ = yp_ = 0.0f;
            }
            float tick(float x) noexcept {
                float y = alpha_ * (yp_ + x - xp_);
                xp_ = x;  yp_ = y;
                return y;
            }
            void process(Buffer& b) noexcept { for (auto& s : b) s = tick(s); }
            void reset()  noexcept { xp_ = yp_ = 0.0f; }
        private:
            float alpha_ = 0.9f, xp_ = 0.0f, yp_ = 0.0f;
        };

        // =============================================================================
        //  §14b  PARAMETRIC EQ
        //  Three independent biquad bands: low shelf, peak/notch, high shelf.
        //  Each band is independently bypassable.  Uses Robert Bristow-Johnson's
        //  Audio EQ Cookbook formulae.
        // =============================================================================

        /// Single biquad EQ band.
        class BiquadEQ {
        public:
            enum class Type { LowShelf, HighShelf, Peak };

            Type   type      = Type::Peak;
            double freq_hz   = 1000.0;  ///< Centre / corner frequency
            double gain_db   = 0.0;     ///< Boost/cut in dB (Peak); shelf gain for shelves
            double Q         = 0.707;   ///< Bandwidth / slope control

            BiquadEQ() = default;
            BiquadEQ(Type t, double f, double g, double q = 0.707)
                : type(t), freq_hz(f), gain_db(g), Q(q) {}

            float tick(float x, int sample_rate) noexcept {
                if (cached_sr_ != sample_rate) update_coeffs(sample_rate);
                float y = b0_ * x + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;
                x2_ = x1_;  x1_ = x;
                y2_ = y1_;  y1_ = y;
                return y;
            }

            void process(Buffer& buf, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                update_coeffs(sample_rate);
                for (auto& s : buf) s = tick(s, sample_rate);
            }

            void reset() noexcept { x1_ = x2_ = y1_ = y2_ = 0.0f; }

            void update_coeffs(int sample_rate) noexcept {
                cached_sr_ = sample_rate;
                const double A  = std::pow(10.0, gain_db / 40.0);
                const double w0 = TWO_PI * freq_hz / sample_rate;
                const double cw = std::cos(w0);
                const double sw = std::sin(w0);
                const double alpha = sw / (2.0 * Q);

                double b0 = 0.0, b1 = 0.0, b2 = 0.0, a0 = 1.0, a1 = 0.0, a2 = 0.0;
                switch (type) {
                    case Type::Peak:
                        b0 =  1.0 + alpha * A;
                        b1 = -2.0 * cw;
                        b2 =  1.0 - alpha * A;
                        a0 =  1.0 + alpha / A;
                        a1 = -2.0 * cw;
                        a2 =  1.0 - alpha / A;
                        break;
                    case Type::LowShelf: {
                        const double sqA  = std::sqrt(A);
                        const double beta = sw * std::sqrt((A * A + 1.0) / Q - (A - 1.0) * (A - 1.0));
                        b0 =  A * ((A + 1.0) - (A - 1.0) * cw + beta);
                        b1 =  2.0 * A * ((A - 1.0) - (A + 1.0) * cw);
                        b2 =  A * ((A + 1.0) - (A - 1.0) * cw - beta);
                        a0 =       (A + 1.0) + (A - 1.0) * cw + beta;
                        a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cw);
                        a2 =        (A + 1.0) + (A - 1.0) * cw - beta;
                        (void)sqA;
                        break;
                    }
                    case Type::HighShelf: {
                        const double sqA  = std::sqrt(A);
                        const double beta = sw * std::sqrt((A * A + 1.0) / Q - (A - 1.0) * (A - 1.0));
                        b0 =  A * ((A + 1.0) + (A - 1.0) * cw + beta);
                        b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cw);
                        b2 =  A * ((A + 1.0) + (A - 1.0) * cw - beta);
                        a0 =        (A + 1.0) - (A - 1.0) * cw + beta;
                        a1 =  2.0 * ((A - 1.0) - (A + 1.0) * cw);
                        a2 =        (A + 1.0) - (A - 1.0) * cw - beta;
                        (void)sqA;
                        break;
                    }
                }
                b0_ = static_cast<float>(b0 / a0);
                b1_ = static_cast<float>(b1 / a0);
                b2_ = static_cast<float>(b2 / a0);
                a1_ = static_cast<float>(a1 / a0);
                a2_ = static_cast<float>(a2 / a0);
            }

        private:
            float b0_ = 1.f, b1_ = 0.f, b2_ = 0.f, a1_ = 0.f, a2_ = 0.f;
            float x1_ = 0.f, x2_ = 0.f, y1_ = 0.f, y2_ = 0.f;
            int   cached_sr_ = 0;
        };

        /// Three-band parametric EQ: low shelf + peak + high shelf.
        /// Each band can be bypassed independently by setting gain_db = 0.
        class ParametricEQ {
        public:
            BiquadEQ low_shelf  { BiquadEQ::Type::LowShelf,  200.0,  0.0, 0.707 };
            BiquadEQ peak       { BiquadEQ::Type::Peak,      1000.0,  0.0, 0.707 };
            BiquadEQ high_shelf { BiquadEQ::Type::HighShelf, 6000.0,  0.0, 0.707 };

            float tick(float x, int sample_rate) noexcept {
                x = low_shelf.tick(x, sample_rate);
                x = peak.tick(x, sample_rate);
                x = high_shelf.tick(x, sample_rate);
                return x;
            }

            void process(Buffer& buf, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                for (auto& s : buf) s = tick(s, sample_rate);
            }

            void process(StereoBuffer& buf, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                for (size_t i = 0; i < buf.size(); ++i) {
                    buf.L[i] = tick(buf.L[i], sample_rate);
                    buf.R[i] = tick(buf.R[i], sample_rate);
                }
            }

            void reset() noexcept {
                low_shelf.reset();
                peak.reset();
                high_shelf.reset();
            }
        };
    }
}
