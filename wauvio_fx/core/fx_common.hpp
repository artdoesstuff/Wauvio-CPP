#pragma once
#include "../../wauvio.hpp"
#include <complex>

namespace wauvio {
    namespace fx {
        inline double db_to_lin(double db) noexcept { return std::pow(10.0, db / 20.0); }
        inline double lin_to_db(double lin) noexcept { return 20.0 * std::log10(std::max(lin, 1e-12)); }
        inline float  flush_denormal(float x) noexcept { return (std::fabs(x) < 1e-20f) ? 0.0f : x; }
        namespace div {
            constexpr double whole = 4.0, half = 2.0, quarter = 1.0, eighth = 0.5, sixteenth = 0.25;
            constexpr double dotted(double beats)  noexcept { return beats * 1.5; }
            constexpr double triplet(double beats) noexcept { return beats * 2.0 / 3.0; }
        }
        inline double beats_to_seconds(double bpm, double beats) noexcept {
            return bpm > 0.0 ? beats * 60.0 / bpm : 0.0;
        }

        class Rng {
        public:
            explicit Rng(uint64_t seed = 0x9E3779B97F4A7C15ull) { reseed(seed); }
            void reseed(uint64_t seed) { s_ = seed ? seed : 0x9E3779B97F4A7C15ull; for (int i = 0; i < 4; ++i) next_u64(); }
            uint64_t next_u64() noexcept { s_ ^= s_ >> 12; s_ ^= s_ << 25; s_ ^= s_ >> 27; return s_ * 0x2545F4914F6CDD1Dull; }
            double uniform() noexcept { return (next_u64() >> 11) * (1.0 / 9007199254740992.0); }     // [0,1)
            double bipolar() noexcept { return uniform() * 2.0 - 1.0; }                                // [-1,1)
            double gaussian() noexcept {                                                                // N(0,1), Box-Muller
                double u1 = std::max(uniform(), 1e-12), u2 = uniform();
                return std::sqrt(-2.0 * std::log(u1)) * std::cos(TWO_PI * u2);
            }
        private:
            uint64_t s_;
        };

        enum class FilterType { LowPass, HighPass, BandPass, Notch, Peak, LowShelf, HighShelf, AllPass };

        struct BiquadCoef { double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0; };

        inline BiquadCoef design_biquad(FilterType t, double fs, double f0, double q, double gain_db = 0.0) {
            f0 = std::min(std::max(f0, 1.0), fs * 0.49);
            q  = std::max(q, 0.05);
            const double w0 = TWO_PI * f0 / fs, cw = std::cos(w0), sw = std::sin(w0);
            const double alpha = sw / (2.0 * q);
            const double A = std::pow(10.0, gain_db / 40.0);
            double b0 = 1, b1 = 0, b2 = 0, a0 = 1, a1 = 0, a2 = 0;
            switch (t) {
                case FilterType::LowPass:  b0 = (1 - cw) / 2; b1 = 1 - cw; b2 = (1 - cw) / 2; a0 = 1 + alpha; a1 = -2 * cw; a2 = 1 - alpha; break;
                case FilterType::HighPass: b0 = (1 + cw) / 2; b1 = -(1 + cw); b2 = (1 + cw) / 2; a0 = 1 + alpha; a1 = -2 * cw; a2 = 1 - alpha; break;
                case FilterType::BandPass: b0 = alpha; b1 = 0; b2 = -alpha; a0 = 1 + alpha; a1 = -2 * cw; a2 = 1 - alpha; break;
                case FilterType::Notch:    b0 = 1; b1 = -2 * cw; b2 = 1; a0 = 1 + alpha; a1 = -2 * cw; a2 = 1 - alpha; break;
                case FilterType::AllPass:  b0 = 1 - alpha; b1 = -2 * cw; b2 = 1 + alpha; a0 = 1 + alpha; a1 = -2 * cw; a2 = 1 - alpha; break;
                case FilterType::Peak:
                    b0 = 1 + alpha * A; b1 = -2 * cw; b2 = 1 - alpha * A;
                    a0 = 1 + alpha / A; a1 = -2 * cw; a2 = 1 - alpha / A; break;
                case FilterType::LowShelf: {
                    const double beta = 2 * std::sqrt(A) * alpha;
                    b0 = A * ((A + 1) - (A - 1) * cw + beta);
                    b1 = 2 * A * ((A - 1) - (A + 1) * cw);
                    b2 = A * ((A + 1) - (A - 1) * cw - beta);
                    a0 = (A + 1) + (A - 1) * cw + beta;
                    a1 = -2 * ((A - 1) + (A + 1) * cw);
                    a2 = (A + 1) + (A - 1) * cw - beta; break; }
                case FilterType::HighShelf: {
                    const double beta = 2 * std::sqrt(A) * alpha;
                    b0 = A * ((A + 1) + (A - 1) * cw + beta);
                    b1 = -2 * A * ((A - 1) + (A + 1) * cw);
                    b2 = A * ((A + 1) + (A - 1) * cw - beta);
                    a0 = (A + 1) - (A - 1) * cw + beta;
                    a1 = 2 * ((A - 1) - (A + 1) * cw);
                    a2 = (A + 1) - (A - 1) * cw - beta; break; }
            }
            BiquadCoef c; c.b0 = b0 / a0; c.b1 = b1 / a0; c.b2 = b2 / a0; c.a1 = a1 / a0; c.a2 = a2 / a0;
            return c;
        }

        inline double biquad_magnitude(const BiquadCoef& c, double fs, double f) {
            const double w = TWO_PI * f / fs;
            const std::complex<double> z1 = std::polar(1.0, -w), z2 = std::polar(1.0, -2.0 * w);
            const std::complex<double> num = c.b0 + c.b1 * z1 + c.b2 * z2;
            const std::complex<double> den = 1.0 + c.a1 * z1 + c.a2 * z2;
            return std::abs(num) / std::max(std::abs(den), 1e-30);
        }

        class Biquad {
        public:
            void set(const BiquadCoef& c) noexcept { c_ = c; }
            const BiquadCoef& coef() const noexcept { return c_; }
            float tick(float x) noexcept {
                const double y = c_.b0 * x + z1_;
                z1_ = c_.b1 * x - c_.a1 * y + z2_;
                z2_ = c_.b2 * x - c_.a2 * y;
                return static_cast<float>(y);
            }
            void reset() noexcept { z1_ = z2_ = 0.0; }
        private:
            BiquadCoef c_; double z1_ = 0.0, z2_ = 0.0;
        };

        class DCBlocker {
        public:
            float tick(float x) noexcept { const float y = x - x1_ + 0.9982f * y1_; x1_ = x; y1_ = flush_denormal(y); return y; }
            void reset() noexcept { x1_ = y1_ = 0.0f; }
        private:
            float x1_ = 0.0f, y1_ = 0.0f;
        };

        inline double time_coef(double ms, double sr) noexcept {
            return ms <= 0.0 ? 0.0 : std::exp(-1.0 / (ms * 0.001 * sr));
        }

        enum class LfoShape { Sine, Triangle, Square, SoftSquare };

        inline double lfo_eval(LfoShape s, double phase) noexcept {
            phase -= std::floor(phase);
            switch (s) {
                case LfoShape::Sine:       return std::sin(TWO_PI * phase);
                case LfoShape::Triangle:   return phase < 0.5 ? 4.0 * phase - 1.0 : 3.0 - 4.0 * phase;
                case LfoShape::Square:     return phase < 0.5 ? 1.0 : -1.0;
                case LfoShape::SoftSquare: return std::tanh(4.0 * std::sin(TWO_PI * phase));
            }
            return 0.0;
        }

        inline bool is_pow2(size_t n) noexcept { return n && !(n & (n - 1)); }
        inline size_t next_pow2(size_t n) noexcept { size_t p = 1; while (p < n) p <<= 1; return p; }

        inline void fft(std::vector<std::complex<float>>& a, bool inverse) {
            const size_t n = a.size();
            if (!is_pow2(n)) throw std::invalid_argument("fx::fft: size must be a power of two");
            for (size_t i = 1, j = 0; i < n; ++i) {
                size_t bit = n >> 1;
                for (; j & bit; bit >>= 1) j ^= bit;
                j ^= bit;
                if (i < j) std::swap(a[i], a[j]);
            }
            for (size_t len = 2; len <= n; len <<= 1) {
                const double ang = TWO_PI / static_cast<double>(len) * (inverse ? 1.0 : -1.0);
                const std::complex<float> wl(static_cast<float>(std::cos(ang)), static_cast<float>(std::sin(ang)));
                for (size_t i = 0; i < n; i += len) {
                    std::complex<float> w(1.0f, 0.0f);
                    for (size_t k = 0; k < len / 2; ++k) {
                        if ((k & 31) == 0) { const double a2 = ang * static_cast<double>(k);
                            w = std::complex<float>(static_cast<float>(std::cos(a2)), static_cast<float>(std::sin(a2))); }
                        const std::complex<float> u = a[i + k], v = a[i + k + len / 2] * w;
                        a[i + k] = u + v; a[i + k + len / 2] = u - v;
                        w *= wl;
                    }
                }
            }
            if (inverse) { const float inv = 1.0f / static_cast<float>(n); for (auto& x : a) x *= inv; }
        }
    }
}
