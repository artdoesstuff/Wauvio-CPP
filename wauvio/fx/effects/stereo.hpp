#pragma once
#include "../core/fx_effect.hpp"

namespace wauvio {
    namespace fx {
        class StereoImager : public Effect {
        public:
            double width         = 1.0;    ///< 0 = mono, 1 = unchanged, 2 = twice as wide
            double mid_gain_db   = 0.0;
            double side_gain_db  = 0.0;
            double mono_below_hz = 0.0;    ///< side signal is high-passed (4th order): bass stays centred. 0 = off
            double balance       = 0.0;    ///< -1 = left only, +1 = right only

            std::string name() const override { return "StereoImager"; }
            void reset() override { for (auto& b : hp_) b.reset(); }

            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                const float gm = static_cast<float>(db_to_lin(mid_gain_db)), gs = static_cast<float>(db_to_lin(side_gain_db) * width);
                const bool use_hp = mono_below_hz > 1.0;
                if (use_hp) { const BiquadCoef c = design_biquad(FilterType::HighPass, sr_, mono_below_hz, 0.70710678);
                    for (auto& b : hp_) b.set(c); }
                const float gl = static_cast<float>(std::min(1.0, 1.0 - balance)), gr = static_cast<float>(std::min(1.0, 1.0 + balance));
                for (size_t i = 0; i < n; ++i) {
                    const float m = 0.5f * (L[i] + R[i]);
                    float s = 0.5f * (L[i] - R[i]);
                    if (use_hp) s = hp_[1].tick(hp_[0].tick(s));
                    const float mo = m * gm, so = s * gs;
                    L[i] = (mo + so) * gl; R[i] = (mo - so) * gr;
                }
            }
        private:
            Biquad hp_[2];
        };

        /// Gain, pan, polarity, channel swap, mono sum, DC removal.
        class Utility : public Effect {
        public:
            double gain_db = 0.0;
            double pan     = 0.0;          ///< -1..1, equal power (0 = unchanged)
            bool   mute = false, invert_left = false, invert_right = false, swap_channels = false, mono = false, remove_dc = false;

            std::string name() const override { return "Utility"; }
            void reset() override { dc_[0].reset(); dc_[1].reset(); }

            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                const float g = mute ? 0.0f : static_cast<float>(db_to_lin(gain_db));
                const double th = (pan + 1.0) * PI / 4.0;
                const float pl = static_cast<float>(std::sqrt(2.0) * std::cos(th)), pr = static_cast<float>(std::sqrt(2.0) * std::sin(th));
                for (size_t i = 0; i < n; ++i) {
                    float l = L[i], r = R[i];
                    if (remove_dc) { l = dc_[0].tick(l); r = dc_[1].tick(r); }
                    if (invert_left) l = -l;
                    if (invert_right) r = -r;
                    if (swap_channels) std::swap(l, r);
                    if (mono) { l = r = 0.5f * (l + r); }
                    L[i] = l * g * pl; R[i] = r * g * pr;
                }
            }
        private:
            DCBlocker dc_[2];
        };
    }
}
