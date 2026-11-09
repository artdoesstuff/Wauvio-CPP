#pragma once
#include "../../wauvio.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace wauvio {
    namespace sfx {
        class Error : public std::runtime_error {
        public:
            explicit Error(const std::string& what) : std::runtime_error(what) {}
        };

        inline bool   is_finite(double v) noexcept { return std::isfinite(v); }
        inline double clampd(double v, double lo, double hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }
        inline double lerp(double a, double b, double t) noexcept { return a + (b - a) * t; }
        inline double midi_to_hz(double m) noexcept { return 440.0 * std::pow(2.0, (m - 69.0) / 12.0); }
        inline double hz_to_midi(double f) noexcept { return 69.0 + 12.0 * std::log2(std::max(f, 1e-6) / 440.0); }
        inline double db_to_lin(double db) noexcept { return std::pow(10.0, db / 20.0); }
        inline double lin_to_db(double v) noexcept { return 20.0 * std::log10(std::max(v, 1e-12)); }
        inline double semis_to_ratio(double s) noexcept { return std::pow(2.0, s / 12.0); }

        inline uint64_t hash64(uint64_t x) noexcept {
            x += 0x9E3779B97F4A7C15ull;
            x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
            x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
            return x ^ (x >> 31);
        }
        inline uint64_t mix_seed(uint64_t a, uint64_t b) noexcept { return hash64(a ^ (hash64(b) + 0x9E3779B97F4A7C15ull + (a << 6) + (a >> 2))); }
        inline double hash01(uint64_t x) noexcept { return static_cast<double>(hash64(x) >> 11) * (1.0 / 9007199254740992.0); }

        class Rng {
        public:
            explicit Rng(uint64_t seed = 1) noexcept { reseed(seed); }
            void reseed(uint64_t seed) noexcept { uint64_t z = seed; for (auto& s : s_) { z += 0x9E3779B97F4A7C15ull; s = hash64(z); } }
            uint64_t next() noexcept {
                const uint64_t r = rotl(s_[1] * 5, 7) * 9, t = s_[1] << 17;
                s_[2] ^= s_[0]; s_[3] ^= s_[1]; s_[1] ^= s_[2]; s_[0] ^= s_[3]; s_[2] ^= t; s_[3] = rotl(s_[3], 45);
                return r;
            }
            double uniform() noexcept { return static_cast<double>(next() >> 11) * (1.0 / 9007199254740992.0); }   // [0,1)
            double bipolar() noexcept { return uniform() * 2.0 - 1.0; }                                              // [-1,1)
            double range(double a, double b) noexcept { return a + (b - a) * uniform(); }
            bool   chance(double p) noexcept { return uniform() < p; }
            int    index(int n) noexcept { return n <= 1 ? 0 : static_cast<int>(uniform() * n) % n; }
            double gaussian() noexcept { const double u1 = std::max(uniform(), 1e-12), u2 = uniform(); return std::sqrt(-2.0 * std::log(u1)) * std::cos(TWO_PI * u2); }
        private:
            static uint64_t rotl(uint64_t x, int k) noexcept { return (x << k) | (x >> (64 - k)); }
            uint64_t s_[4];
        };

        inline double value_noise(double x, uint64_t seed) noexcept {
            const double fl = std::floor(x); const double f = x - fl; const int64_t i = static_cast<int64_t>(fl);
            auto h = [&](int64_t k) { return hash01(mix_seed(seed, static_cast<uint64_t>(k))) * 2.0 - 1.0; };
            const double s = f * f * (3.0 - 2.0 * f);
            return h(i) + (h(i + 1) - h(i)) * s;
        }

        inline const std::vector<std::string>& scale_names() {
            static const std::vector<std::string> n = { "chromatic", "major", "natural_minor", "harmonic_minor", "pentatonic_major",
                "pentatonic_minor", "whole_tone", "dorian", "phrygian", "lydian", "mixolydian" };
            return n;
        }
        inline std::vector<int> scale_degrees(int index) {
            static const std::vector<std::vector<int>> t = {
                {0,1,2,3,4,5,6,7,8,9,10,11}, {0,2,4,5,7,9,11}, {0,2,3,5,7,8,10}, {0,2,3,5,7,8,11}, {0,2,4,7,9},
                {0,3,5,7,10}, {0,2,4,6,8,10}, {0,2,3,5,7,9,10}, {0,1,3,5,7,8,10}, {0,2,4,6,7,9,11}, {0,2,4,5,7,9,10} };
            if (index < 0 || index >= static_cast<int>(t.size())) throw Error("scale index out of range: " + std::to_string(index));
            return t[static_cast<size_t>(index)];
        }
        inline int scale_step(const std::vector<int>& deg, int k) {
            const int n = static_cast<int>(deg.size()); const int oct = (k >= 0) ? k / n : -((-k + n - 1) / n);
            const int idx = k - oct * n; return deg[static_cast<size_t>(idx)] + 12 * oct;
        }
        inline const std::vector<std::string>& chord_names() {
            static const std::vector<std::string> n = { "major", "minor", "sus2", "sus4", "major7", "minor7", "dominant7", "diminished", "augmented", "add9", "cluster" };
            return n;
        }
        inline std::vector<double> chord_intervals(int index) {
            static const std::vector<std::vector<double>> t = {
                {0,4,7}, {0,3,7}, {0,2,7}, {0,5,7}, {0,4,7,11}, {0,3,7,10}, {0,4,7,10}, {0,3,6}, {0,4,8}, {0,4,7,14}, {0,1,2,3} };
            if (index < 0 || index >= static_cast<int>(t.size())) throw Error("chord index out of range: " + std::to_string(index));
            return t[static_cast<size_t>(index)];
        }
    }
}
