#pragma once
#include "modulation.hpp"

namespace wauvio {
    namespace fx {
        class StereoDelay : public Effect {
        public:
            // Time: either milliseconds or tempo sync (sync_bpm > 0 -> sync_beats_* are used).
            double time_ms        = 350.0;   ///< left (or both) delay time
            double time_ms_right  = -1.0;    ///< < 0 = same as left
            double sync_bpm       = 0.0;
            double sync_beats_left  = div::dotted(div::eighth);   ///< default: dotted eighth
            double sync_beats_right = -1.0;                       ///< < 0 = same as left

            double feedback       = 0.4;     ///< 0..0.98
            bool   ping_pong      = false;   ///< input into the left line, echoes bounce L -> R -> L ...
            double lowcut_hz      = 0.0;     ///< high-pass in the feedback path (0 = off)
            double highcut_hz     = 6000.0;  ///< low-pass in the feedback path (0 = off) - each repeat gets darker
            double mod_depth_ms   = 0.0;     ///< tape-style wobble
            double mod_rate_hz    = 0.4;
            double mix            = 0.35;

            std::string name() const override { return "StereoDelay"; }

            double tail_seconds() const override {
                const double t = std::max(left_seconds(), right_seconds());
                const double fb = std::min(0.98, std::max(0.0, feedback));
                if (fb < 1e-3) return t;
                return std::min(30.0, t * (std::log(1e-3) / std::log(fb)) + t);
            }

            void reset() override { pos_ = 0; dl_[0].clear(); dl_[1].clear(); for (auto& f : lc_) f.reset(); for (auto& f : hc_) f.reset(); }

            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                const double tl = left_seconds() * sr_, tr = right_seconds() * sr_;
                const double mod_s = mod_depth_ms * 0.001 * sr_;
                const size_t cap = static_cast<size_t>(std::max(tl, tr) + mod_s) + 16;
                if (dl_[0].capacity() < cap) { dl_[0].init(cap); dl_[1].init(cap); }
                const double fb = std::min(0.98, std::max(0.0, feedback));
                const double wet = std::min(1.0, std::max(0.0, mix));
                const bool use_lc = lowcut_hz > 1.0, use_hc = highcut_hz > 1.0;
                if (use_lc) { const BiquadCoef c = design_biquad(FilterType::HighPass, sr_, lowcut_hz, 0.7071); lc_[0].set(c); lc_[1].set(c); }
                if (use_hc) { const BiquadCoef c = design_biquad(FilterType::LowPass,  sr_, highcut_hz, 0.7071); hc_[0].set(c); hc_[1].set(c); }

                for (size_t i = 0; i < n; ++i) {
                    const double wob = mod_s > 0.0 ? mod_s * std::sin(TWO_PI * mod_rate_hz * static_cast<double>(pos_ + i) / sr_) : 0.0;
                    float dl = dl_[0].read(std::max(1.0, tl + wob)), dr = dl_[1].read(std::max(1.0, tr - wob));
                    float fl = dl, fr = dr;
                    if (use_lc) { fl = lc_[0].tick(fl); fr = lc_[1].tick(fr); }
                    if (use_hc) { fl = hc_[0].tick(fl); fr = hc_[1].tick(fr); }
                    const float xl = L[i], xr = R[i];
                    if (ping_pong) {
                        dl_[0].push(static_cast<float>(0.5 * (xl + xr) + fr * fb));
                        dl_[1].push(static_cast<float>(fl * fb));
                    } else {
                        dl_[0].push(static_cast<float>(xl + fl * fb));
                        dl_[1].push(static_cast<float>(xr + fr * fb));
                    }
                    L[i] = static_cast<float>(xl * (1.0 - wet) + dl * wet);
                    R[i] = static_cast<float>(xr * (1.0 - wet) + dr * wet);
                }
                pos_ += n;
            }

        private:
            double left_seconds() const {
                return sync_bpm > 0.0 ? beats_to_seconds(sync_bpm, sync_beats_left) : time_ms * 0.001;
            }
            double right_seconds() const {
                if (sync_bpm > 0.0) return beats_to_seconds(sync_bpm, sync_beats_right > 0.0 ? sync_beats_right : sync_beats_left);
                return (time_ms_right >= 0.0 ? time_ms_right : time_ms) * 0.001;
            }
            FracDelay dl_[2]; Biquad lc_[2], hc_[2]; size_t pos_ = 0;
        };
    }
}
