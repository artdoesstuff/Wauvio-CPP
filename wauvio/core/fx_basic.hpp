#pragma once

#include "common.hpp"

namespace wauvio {
    namespace fx {
        namespace basic {
            // =============================================================================
            //  §16  DISTORTION
            // =============================================================================

            namespace distortion {
                /// Soft clip via hyperbolic tangent.  drive > 1 = more distortion.
                inline float soft_clip(float x, float drive = 2.0f) noexcept {
                    return std::tanh(x * drive) / std::tanh(drive);
                }

                /// Hard clip at ±threshold.
                inline float hard_clip(float x, float threshold = 0.5f) noexcept {
                    return std::max(-threshold, std::min(threshold, x)) / threshold;
                }

                /// Asymmetric waveshaper: polynomial with tunable knee.
                ///   knee = 0.0 → linear; knee = 1.0 → maximum curvature
                inline float waveshaper(float x, float knee = 0.7f) noexcept {
                    // Cubic soft saturation with asymmetric bias
                    const float k   = knee * 3.0f;
                    const float abs_x = std::abs(x);
                    if (abs_x >= 1.0f) return (x > 0) ? 1.0f : -1.0f;
                    return x * (1.0f + k * (1.0f - abs_x));
                }

                /// Process a buffer in place with a chosen distortion function.
                inline void apply_soft_clip(Buffer& buf, float drive = 2.0f) {
                    for (auto& s : buf) s = soft_clip(s, drive);
                }

                inline void apply_hard_clip(Buffer& buf, float threshold = 0.5f) {
                    for (auto& s : buf) s = hard_clip(s, threshold);
                }

                inline void apply_waveshaper(Buffer& buf, float knee = 0.7f) {
                    for (auto& s : buf) s = waveshaper(s, knee);
                }
            }

            // =============================================================================
            //  §17  DELAY LINE
            //  Feedback digital delay.  Supports tempo-synced delays (pass time_ms).
            // =============================================================================

            class DelayLine {
            public:
                double time_ms  = 250.0;  ///< Delay time in milliseconds
                float  feedback = 0.4f;   ///< Feedback amount [0,1)
                float  wet      = 0.5f;   ///< Wet level
                float  dry      = 1.0f;   ///< Dry level

                void init(int sample_rate = 0) {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    sr_    = sample_rate;
                    size_  = static_cast<size_t>(time_ms * 0.001 * sample_rate) + 1;
                    buf_.assign(size_, 0.0f);
                    head_  = 0;
                }

                float tick(float x) noexcept {
                    const float delayed = buf_[head_];
                    buf_[head_] = x + delayed * feedback;
                    head_ = (head_ + 1) % size_;
                    return x * dry + delayed * wet;
                }

                void process(Buffer& buf) {
                    if (buf_.empty()) init(sr_);
                    for (auto& s : buf) s = tick(s);
                }

                void reset() noexcept { std::fill(buf_.begin(), buf_.end(), 0.0f); head_ = 0; }

            private:
                std::vector<float> buf_;
                size_t             head_ = 0;
                size_t             size_ = 1;
                int                sr_   = 44100;
            };

            // =============================================================================
            //  §18  REVERB  - Schroeder structure
            //  4 parallel feedback comb filters → 2 series allpass filters.
            //  Classic digital reverb suitable for atmospheric tails.
            // =============================================================================

            class Reverb {
            public:
                float room_size = 0.8f;   ///< [0,1] - longer = larger room
                float damping   = 0.5f;   ///< [0,1] - higher = duller high frequencies
                float wet       = 0.3f;
                float dry       = 1.0f;

                void init(int sample_rate = 0) {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    // Comb filter delay lengths (in samples) - prime-ish values
                    static const int comb_ms[] = { 29, 37, 41, 43 };
                    for (int i = 0; i < 4; ++i) {
                        const int len = comb_ms[i] * sample_rate / 1000;
                        comb_buf_[i].assign(len, 0.0f);
                        comb_head_[i] = 0;
                        comb_fb_[i]   = 0.0f;
                    }
                    static const int ap_ms[] = { 5, 13 };
                    for (int i = 0; i < 2; ++i) {
                        const int len = ap_ms[i] * sample_rate / 1000;
                        ap_buf_[i].assign(len, 0.0f);
                        ap_head_[i] = 0;
                    }
                }

                float tick(float x) noexcept {
                    // Comb bank
                    float comb_out = 0.0f;
                    for (int i = 0; i < 4; ++i) {
                        auto& cb = comb_buf_[i];
                        const float delayed  = cb[comb_head_[i]];
                        const float dampened = comb_fb_[i] * (1.0f - damping) + delayed * damping;
                        comb_fb_[i]          = dampened;
                        cb[comb_head_[i]]    = x + dampened * room_size;
                        comb_head_[i]        = (comb_head_[i] + 1) % cb.size();
                        comb_out            += delayed;
                    }
                    comb_out *= 0.25f;

                    // Allpass chain
                    float ap_in = comb_out;
                    for (int i = 0; i < 2; ++i) {
                        auto& ab = ap_buf_[i];
                        const float v    = ab[ap_head_[i]];
                        const float out  = -ap_in + v;
                        ab[ap_head_[i]]  = ap_in + v * 0.5f;
                        ap_head_[i]      = (ap_head_[i] + 1) % ab.size();
                        ap_in            = out;
                    }

                    return x * dry + ap_in * wet;
                }

                void process(Buffer& buf) {
                    if (comb_buf_[0].empty()) init();
                    for (auto& s : buf) s = tick(s);
                }

                void reset() noexcept {
                    for (auto& b : comb_buf_) std::fill(b.begin(), b.end(), 0.0f);
                    for (auto& b : ap_buf_)   std::fill(b.begin(), b.end(), 0.0f);
                }

            private:
                std::vector<float> comb_buf_[4];
                int                comb_head_[4] = {};
                float              comb_fb_[4]   = {};

                std::vector<float> ap_buf_[2];
                int                ap_head_[2] = {};
            };

            // =============================================================================
            //  §19  CHORUS
            //  Modulated delay line that creates pitch/timbre widening.
            //  Uses an LFO to vary delay time, detuning copies of the signal slightly.
            // =============================================================================

            class Chorus {
            public:
                double rate_hz    = 0.5;    ///< LFO rate
                double depth_ms   = 1.5;    ///< Modulation depth in ms
                double delay_ms   = 8.0;    ///< Centre delay time in ms
                float  wet        = 0.5f;
                float  dry        = 1.0f;

                void init(int sample_rate = 0) {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    sr_  = sample_rate;
                    max_delay_ = static_cast<size_t>((delay_ms + depth_ms * 2.0) * 0.001 * sr_) + 2;
                    buf_.assign(max_delay_, 0.0f);
                    head_ = 0;
                }

                float tick(float x) noexcept {
                    buf_[head_] = x;

                    lfo_phase_ += TWO_PI * rate_hz / sr_;
                    if (lfo_phase_ >= TWO_PI) lfo_phase_ -= TWO_PI;

                    const double mod    = depth_ms * 0.001 * sr_ * std::sin(lfo_phase_);
                    const double delay  = delay_ms * 0.001 * sr_ + mod;
                    const size_t d_int  = static_cast<size_t>(delay);
                    const float  frac   = static_cast<float>(delay - d_int);

                    const size_t i0 = (head_ + max_delay_ - d_int)     % max_delay_;
                    const size_t i1 = (head_ + max_delay_ - d_int - 1) % max_delay_;
                    const float  delayed = buf_[i0] * (1.0f - frac) + buf_[i1] * frac;

                    head_ = (head_ + 1) % max_delay_;
                    return x * dry + delayed * wet;
                }

                void process(Buffer& buf) {
                    if (buf_.empty()) init(sr_);
                    for (auto& s : buf) s = tick(s);
                }

            private:
                std::vector<float> buf_;
                size_t             head_      = 0;
                size_t             max_delay_ = 1;
                double             lfo_phase_ = 0.0;
                int                sr_        = 44100;
            };

            // =============================================================================
            //  §19b  FLANGER
            //  Short modulated delay (0.1–10 ms) mixed with the dry signal, creating
            //  comb-filter notches that sweep with the LFO.  Feedback intensifies the
            //  metallic/jet-plane sweep.
            // =============================================================================

            class Flanger {
            public:
                double rate_hz   = 0.25;   ///< LFO rate
                double depth_ms  = 2.0;    ///< Modulation depth in ms
                double delay_ms  = 3.0;    ///< Centre delay time in ms (keep < 10 ms)
                float  feedback  = 0.5f;   ///< Feedback amount [-1, 1]; negative = through-zero
                float  wet       = 0.5f;
                float  dry       = 1.0f;

                void init(int sample_rate = 0) {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    sr_ = sample_rate;
                    max_delay_ = static_cast<size_t>((delay_ms + depth_ms) * 0.001 * sr_) + 2;
                    buf_.assign(max_delay_, 0.0f);
                    head_ = 0;
                    lfo_phase_ = 0.0;
                }

                float tick(float x) noexcept {
                    lfo_phase_ += TWO_PI * rate_hz / sr_;
                    if (lfo_phase_ >= TWO_PI) lfo_phase_ -= TWO_PI;

                    const double mod   = depth_ms * 0.001 * sr_ * std::sin(lfo_phase_);
                    const double delay = delay_ms * 0.001 * sr_ + mod;
                    const size_t d_int = static_cast<size_t>(delay);
                    const float  frac  = static_cast<float>(delay - d_int);

                    const size_t i0 = (head_ + max_delay_ - d_int)     % max_delay_;
                    const size_t i1 = (head_ + max_delay_ - d_int - 1) % max_delay_;
                    const float  delayed = buf_[i0] * (1.0f - frac) + buf_[i1] * frac;

                    buf_[head_] = x + delayed * feedback;
                    head_ = (head_ + 1) % max_delay_;

                    return x * dry + delayed * wet;
                }

                void process(Buffer& buf) {
                    if (buf_.empty()) init(sr_);
                    for (auto& s : buf) s = tick(s);
                }

                void reset() noexcept {
                    std::fill(buf_.begin(), buf_.end(), 0.0f);
                    head_ = 0;
                    lfo_phase_ = 0.0;
                }

            private:
                std::vector<float> buf_;
                size_t             head_      = 0;
                size_t             max_delay_ = 1;
                double             lfo_phase_ = 0.0;
                int                sr_        = 44100;
            };

            // =============================================================================
            //  §19c  PHASER
            //  N all-pass stages whose centre frequency is modulated by an LFO.
            //  The phase-shifted signal is mixed with dry, creating notches that sweep
            //  through the spectrum — smoother and more musical than flanging.
            //  Default: 4 stages (classic phaser); 8 stages = richer sweep.
            // =============================================================================

            class Phaser {
            public:
                int    stages   = 4;       ///< Number of all-pass stages (2, 4, 6, 8 …)
                double rate_hz  = 0.5;     ///< LFO rate
                double depth    = 0.9;     ///< LFO depth [0, 1]
                double base_hz  = 400.0;   ///< Lowest sweep frequency
                double range_hz = 1600.0;  ///< Frequency range of the sweep
                float  feedback = 0.4f;    ///< Feedback amount [0, 1)
                float  wet      = 0.5f;
                float  dry      = 1.0f;

                void init(int sample_rate = 0) {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    sr_ = sample_rate;
                    a_.assign(static_cast<size_t>(stages), 0.0f);
                    z_.assign(static_cast<size_t>(stages), 0.0f);
                    lfo_phase_ = 0.0;
                    fb_sample_ = 0.0f;
                }

                float tick(float x) noexcept {
                    lfo_phase_ += TWO_PI * rate_hz / sr_;
                    if (lfo_phase_ >= TWO_PI) lfo_phase_ -= TWO_PI;

                    // Sweep: map LFO [-1,1] → [base_hz, base_hz + range_hz]
                    const double lfo = 0.5 * (1.0 + std::sin(lfo_phase_));
                    const double fc  = base_hz + depth * range_hz * lfo;
                    // First-order all-pass coefficient
                    const float  kf  = static_cast<float>(
                                           std::tan(PI * fc / sr_) - 1.0) /
                                       static_cast<float>(
                                           std::tan(PI * fc / sr_) + 1.0);

                    // Re-cache a_[] only when coefficient changes meaningfully
                    for (auto& av : a_) av = kf;

                    float s = x + fb_sample_ * feedback;
                    for (int i = 0; i < stages; ++i) {
                        float out = a_[i] * s + z_[i];
                        z_[i] = s - a_[i] * out;
                        s = out;
                    }
                    fb_sample_ = s;
                    return x * dry + s * wet;
                }

                void process(Buffer& buf) {
                    if (a_.empty()) init(sr_);
                    for (auto& s : buf) s = tick(s);
                }

                void reset() noexcept {
                    std::fill(z_.begin(), z_.end(), 0.0f);
                    fb_sample_  = 0.0f;
                    lfo_phase_  = 0.0;
                }

            private:
                std::vector<float> a_;          ///< All-pass coefficients (one per stage)
                std::vector<float> z_;          ///< All-pass state (one per stage)
                float              fb_sample_  = 0.0f;
                double             lfo_phase_  = 0.0;
                int                sr_         = 44100;
            };

            // =============================================================================
            //  §21  SIDECHAIN COMPRESSOR
            //  Ducker: attenuates `target` buffer based on an envelope follower applied
            //  to the `trigger` buffer.  Classic pumping / sidechaining effect.
            // =============================================================================

            class SidechainCompressor {
            public:
                float attack_ms   = 1.0f;    ///< Envelope follower attack  (ms)
                float release_ms  = 100.0f;  ///< Envelope follower release (ms)
                float strength    = 0.9f;    ///< Duck depth [0=no effect, 1=full silence]
                float threshold   = 0.05f;   ///< RMS level above which ducking begins

                /// Apply sidechain ducking: modulate `target` using envelope of `trigger`.
                /// Both buffers must have the same length.
                void apply(Buffer& target, const Buffer& trigger, int sample_rate = 0) const {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    const float att_coef = std::exp(-1.0f / (attack_ms  * 0.001f * sample_rate));
                    const float rel_coef = std::exp(-1.0f / (release_ms * 0.001f * sample_rate));

                    const size_t n = std::min(target.size(), trigger.size());
                    float env = 0.0f;
                    for (size_t i = 0; i < n; ++i) {
                        const float level = std::abs(trigger[i]);
                        env = (level > env) ? att_coef * env + (1.0f - att_coef) * level
                                            : rel_coef * env + (1.0f - rel_coef) * level;
                        const float duck = (env > threshold)
                            ? 1.0f - strength * std::min(1.0f, (env - threshold) / (1.0f - threshold))
                            : 1.0f;
                        target[i] *= duck;
                    }
                }

                /// Stereo version - applies same gain reduction to both channels.
                void apply(StereoBuffer& target, const Buffer& trigger, int sample_rate = 0) const {
                    Buffer L_copy = target.L;
                    apply(target.L, trigger, sample_rate);
                    // Compute same gain from L, apply to R
                    const float att_coef = std::exp(-1.0f / (attack_ms  * 0.001f * sample_rate));
                    const float rel_coef = std::exp(-1.0f / (release_ms * 0.001f * sample_rate));
                    const size_t n = std::min(target.R.size(), trigger.size());
                    float env = 0.0f;
                    for (size_t i = 0; i < n; ++i) {
                        const float level = std::abs(trigger[i]);
                        env = (level > env) ? att_coef * env + (1.0f - att_coef) * level
                                            : rel_coef * env + (1.0f - rel_coef) * level;
                        const float duck = (env > threshold)
                            ? 1.0f - strength * std::min(1.0f, (env - threshold) / (1.0f - threshold))
                            : 1.0f;
                        target.R[i] *= duck;
                    }
                }
            };
        }
    }
}
