#pragma once

#include "common.hpp"
#include "osc.hpp"
#include "filter.hpp"

namespace wauvio {
    namespace mod {
        // =============================================================================
        //  §12  LFO SYSTEM
        //  Can target: pitch (freq multiplier), amplitude, filter cutoff, pan.
        // =============================================================================

        enum class LFOShape { Sine, Triangle, Square, SampleHold };
        enum class LFOTarget { Pitch, Amplitude, FilterCutoff, Pan };

        class LFO {
        public:
            LFOShape  shape     = LFOShape::Sine;
            LFOTarget target    = LFOTarget::Amplitude;
            double    rate_hz   = 4.0;    ///< Oscillation frequency
            double    depth     = 0.5;    ///< Modulation depth [0,1]
            double    phase_off = 0.0;    ///< Phase offset (radians)

            LFO() = default;
            LFO(LFOShape sh, LFOTarget tgt, double rate, double depth_)
                : shape(sh), target(tgt), rate_hz(rate), depth(depth_) {}

            /// Returns the current LFO value in [-1, 1].
            float tick(int sample_rate) noexcept {
                phase_ += TWO_PI * rate_hz / static_cast<double>(sample_rate);
                if (phase_ >= TWO_PI) phase_ -= TWO_PI;

                const double p = phase_ + phase_off;
                switch (shape) {
                    case LFOShape::Sine:       return static_cast<float>(std::sin(p));
                    case LFOShape::Triangle:   return wauvio::osc::wave::triangle(p);
                    case LFOShape::Square:     return wauvio::osc::wave::square(p);
                    case LFOShape::SampleHold:
                        // New random value on each zero crossing from below
                        if (phase_ < prev_phase_) {
                            sh_value_ = dist_(rng_) * 2.0f - 1.0f;
                        }
                        prev_phase_ = static_cast<float>(phase_);
                        return sh_value_;
                }
                return 0.f;
            }

            /// Returns depth-scaled modulation value.
            float modulate(int sample_rate) noexcept {
                return tick(sample_rate) * static_cast<float>(depth);
            }

            void reset() noexcept { phase_ = 0.0; }

        private:
            double phase_      = 0.0;
            float  prev_phase_ = 0.0f;
            float  sh_value_   = 0.0f;
            std::mt19937 rng_{0xABCDu};
            std::uniform_real_distribution<float> dist_{0.0f, 1.0f};
        };

        // =============================================================================
        //  §22  AUTOMATION SYSTEM
        //  Parameter automation curves applied to buffer data (volume, filter, pitch).
        // =============================================================================

        /// An automation point: time + value.
        struct AutoPoint { double time_sec; float value; };

        /// Compute the linearly interpolated value at `t` from a sorted point list.
        inline float auto_eval(const std::vector<AutoPoint>& pts, double t) noexcept {
            if (pts.empty()) return 0.0f;
            if (t <= pts.front().time_sec) return pts.front().value;
            if (t >= pts.back().time_sec)  return pts.back().value;

            for (size_t i = 0; i + 1 < pts.size(); ++i) {
                if (t >= pts[i].time_sec && t <= pts[i+1].time_sec) {
                    const double span  = pts[i+1].time_sec - pts[i].time_sec;
                    const double frac  = (span > 0) ? (t - pts[i].time_sec) / span : 0.0;
                    return static_cast<float>(pts[i].value + frac * (pts[i+1].value - pts[i].value));
                }
            }
            return pts.back().value;
        }

        /// Apply a volume automation curve to a buffer in-place.
        inline void apply_volume_automation(Buffer& buf,
                                             const std::vector<AutoPoint>& pts,
                                             int sample_rate = 0)
        {
            if (sample_rate <= 0) sample_rate = global_config().sample_rate;
            for (size_t i = 0; i < buf.size(); ++i) {
                const double t = static_cast<double>(i) / sample_rate;
                buf[i] *= auto_eval(pts, t);
            }
        }

        /// Apply a volume automation curve to a stereo buffer in-place.
        inline void apply_volume_automation(StereoBuffer& buf,
                                             const std::vector<AutoPoint>& pts,
                                             int sample_rate = 0)
        {
            if (sample_rate <= 0) sample_rate = global_config().sample_rate;
            for (size_t i = 0; i < buf.size(); ++i) {
                const double t = static_cast<double>(i) / sample_rate;
                const float  v = auto_eval(pts, t);
                buf.L[i] *= v;  buf.R[i] *= v;
            }
        }

        /// Apply an SVF cutoff automation - filter moves from point to point.
        inline Buffer apply_filter_automation(const Buffer& src,
                                               const std::vector<AutoPoint>& cutoff_pts,
                                               double Q,
                                               wauvio::filter::SVFilter::Mode mode = wauvio::filter::SVFilter::Mode::LowPass,
                                               int sample_rate = 0)
        {
            if (sample_rate <= 0) sample_rate = global_config().sample_rate;
            Buffer out(src.size());
            wauvio::filter::SVFilter flt(mode, cutoff_pts.empty() ? 1000.0 : cutoff_pts.front().value, Q);
            for (size_t i = 0; i < src.size(); ++i) {
                const double t  = static_cast<double>(i) / sample_rate;
                const float  fc = auto_eval(cutoff_pts, t);
                flt.cutoff = static_cast<double>(std::max(20.0f, fc));
                flt.update_coeffs(sample_rate);
                out[i] = flt.tick(src[i], sample_rate);
            }
            return out;
        }
    }
}
