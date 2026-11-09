#pragma once
#include "sfx_common.hpp"

namespace wauvio {
    namespace sfx {
        enum class Interp { Linear, Exp, Smooth, Step };

        /// A parameter trajectory over normalised time u in [0, 1] (u = position inside the layer).
        /// Keys are added in time order; each key records how the segment *arriving* at it is
        /// interpolated. Exp interpolation needs both end values to be positive and falls back to
        /// linear otherwise. Non-finite numbers are rejected when building, and at() sanitises its input.
        ///
        ///   Traj::exp(200, 12000)                                           // exponential sweep (musically even)
        ///   Traj(0).to(0.1, 1, Interp::Smooth).to(1, 0.001, Interp::Exp)    // attack then decay
        class Traj {
        public:
            Traj(double constant = 0.0) { check(0.0, constant); k_.push_back({0.0, constant, Interp::Linear}); }   // NOLINT implicit by design

            static Traj linear(double a, double b) { return Traj(a).to(1.0, b, Interp::Linear); }
            static Traj exp(double a, double b)    { return Traj(a).to(1.0, b, Interp::Exp); }
            static Traj smooth(double a, double b) { return Traj(a).to(1.0, b, Interp::Smooth); }
            /// Frequency sweep given in Hz, stored as MIDI notes (so a linear trajectory is an exponential sweep in Hz).
            static Traj pitch_hz(double a_hz, double b_hz, Interp i = Interp::Linear) { return Traj(hz_to_midi(a_hz)).to(1.0, hz_to_midi(b_hz), i); }
            /// Attack to `peak` over the first `attack` fraction, then exponential decay to `floor` at the end.
            static Traj ar(double attack, double peak = 1.0, double floor = 0.001) { attack = clampd(attack, 0.0, 1.0); return Traj(0.0).to(attack, peak, Interp::Smooth).to(1.0, floor, Interp::Exp); }
            /// Exponential decay from 1 to `floor`.
            static Traj decay(double floor = 0.001) { return Traj(1.0).to(1.0, floor, Interp::Exp); }
            /// `n` equal steps from a to b (a staircase), including both end values.
            static Traj stairs(double a, double b, int n) {
                n = std::max(n, 2); Traj t(a);
                for (int i = 1; i < n; ++i) { const double u = static_cast<double>(i) / n; t.to(u, lerp(a, b, static_cast<double>(i) / (n - 1)), Interp::Step); }
                return t;
            }

            /// Add a key at normalised time `t` (must not be earlier than the previous key).
            Traj& to(double t, double v, Interp how = Interp::Linear) {
                check(t, v);
                if (t < 0.0 || t > 1.0) throw Error("Traj key time must be within [0, 1]");
                if (t < k_.back().t) throw Error("Traj keys must be added in time order");
                k_.push_back({t, v, how}); return *this;
            }

            double at(double u) const noexcept {
                if (!std::isfinite(u)) u = 0.0;
                u = u < 0.0 ? 0.0 : (u > 1.0 ? 1.0 : u);
                if (k_.size() == 1 || u <= k_.front().t) return k_.front().v;
                for (size_t i = 1; i < k_.size(); ++i) {
                    if (u <= k_[i].t) {
                        const Key& a = k_[i - 1]; const Key& b = k_[i];
                        const double span = b.t - a.t;
                        if (span <= 0.0) return b.v;
                        const double s = (u - a.t) / span;
                        switch (b.how) {
                            case Interp::Step:   return a.v;
                            case Interp::Smooth: return lerp(a.v, b.v, s * s * (3.0 - 2.0 * s));
                            case Interp::Exp:    if (a.v > 0.0 && b.v > 0.0) return a.v * std::pow(b.v / a.v, s); return lerp(a.v, b.v, s);
                            default:             return lerp(a.v, b.v, s);
                        }
                    }
                }
                return k_.back().v;
            }

            Traj scaled(double k) const { Traj t = *this; for (auto& key : t.k_) key.v *= k; return t; }
            Traj shifted(double d) const { Traj t = *this; for (auto& key : t.k_) key.v += d; return t; }
            double min_value() const { double m = k_[0].v; for (auto& k : k_) m = std::min(m, k.v); return m; }
            double max_value() const { double m = k_[0].v; for (auto& k : k_) m = std::max(m, k.v); return m; }
            size_t key_count() const noexcept { return k_.size(); }

        private:
            struct Key { double t, v; Interp how; };
            static void check(double t, double v) { if (!std::isfinite(t) || !std::isfinite(v)) throw Error("Traj: non-finite value"); }
            std::vector<Key> k_;
        };

        /// Low-frequency modulator, evaluated statelessly from time (so any layer can be rendered in any order).
        struct Mod {
            enum class Shape { Sine, Triangle, Square, Saw, Smooth, Steps };   // Smooth/Steps = random (seeded)
            Shape  shape = Shape::Sine;
            double rate_hz = 0.0;   ///< cycles per second (ignored if `beats` > 0)
            double beats   = 0.0;   ///< if > 0: one cycle per this many beats (tempo-synced)
            double depth   = 0.0;   ///< output range is [-depth, +depth]
            double phase   = 0.0;   ///< cycles

            bool active() const noexcept { return depth != 0.0 && (rate_hz > 0.0 || beats > 0.0); }
            double eval(double t_sec, double bpm, uint64_t seed) const noexcept {
                if (!active()) return 0.0;
                const double rate = beats > 0.0 ? bpm / 60.0 / beats : rate_hz;
                const double x = t_sec * rate + phase; const double f = x - std::floor(x);
                double v = 0.0;
                switch (shape) {
                    case Shape::Sine:     v = std::sin(TWO_PI * x); break;
                    case Shape::Triangle: v = f < 0.5 ? 4.0 * f - 1.0 : 3.0 - 4.0 * f; break;
                    case Shape::Square:   v = f < 0.5 ? 1.0 : -1.0; break;
                    case Shape::Saw:      v = 2.0 * f - 1.0; break;
                    case Shape::Smooth:   v = value_noise(x, seed); break;
                    case Shape::Steps:    v = hash01(mix_seed(seed, static_cast<uint64_t>(static_cast<int64_t>(std::floor(x))))) * 2.0 - 1.0; break;
                }
                return v * depth;
            }
        };
    }
}
