#pragma once

#include "common.hpp"

namespace wauvio {
    namespace osc {
        // =============================================================================
        //  §4  WAVEFORM PRIMITIVES  (stateless, phase in [0,2π))
        // =============================================================================

        namespace wave {
            inline float sine    (double p) noexcept { return static_cast<float>(std::sin(p)); }
            inline float square  (double p) noexcept { return (std::fmod(p,TWO_PI) < PI) ? 1.f : -1.f; }
            inline float triangle(double p) noexcept {
                double t = std::fmod(p / TWO_PI, 1.0);
                return static_cast<float>(t < 0.5 ? 4.0*t - 1.0 : 3.0 - 4.0*t);
            }
            inline float sawtooth(double p) noexcept {
                return static_cast<float>(2.0 * std::fmod(p / TWO_PI, 1.0) - 1.0);
            }
            inline float pulse(double p, double duty = 0.5) noexcept {
                return (std::fmod(p, TWO_PI) < TWO_PI * duty) ? 1.f : -1.f;
            }
        }

        // =============================================================================
        //  §4b  POLYBLEP BAND-LIMITED WAVEFORMS
        //  PolyBLEP (Polynomial Band-Limited Step) correction reduces aliasing at
        //  discontinuities in square, sawtooth, and triangle waves.  Drop-in
        //  replacements for the naive wave:: functions; phase in [0, 2π).
        // =============================================================================

        namespace wave_bl {
            /// PolyBLEP residual: smooths a unit step at normalised phase t ∈ [0,1).
            /// dt = phase_increment / TWO_PI  (one sample's worth of normalised phase)
            inline double polyblep(double t, double dt) noexcept {
                if (t < dt) {
                    t /= dt;
                    return t + t - t * t - 1.0;
                } else if (t > 1.0 - dt) {
                    t = (t - 1.0) / dt;
                    return t * t + t + t + 1.0;
                }
                return 0.0;
            }

            /// Band-limited sawtooth.  dt = freq / sample_rate.
            inline float sawtooth(double phase, double dt) noexcept {
                double t   = phase / TWO_PI;
                double saw = 2.0 * t - 1.0;
                saw -= polyblep(t, dt);
                return static_cast<float>(saw);
            }

            /// Band-limited square.  dt = freq / sample_rate.
            inline float square(double phase, double dt) noexcept {
                double t  = phase / TWO_PI;
                double sq = (t < 0.5) ? 1.0 : -1.0;
                sq += polyblep(t, dt);
                sq -= polyblep(std::fmod(t + 0.5, 1.0), dt);
                return static_cast<float>(sq);
            }

            /// Band-limited triangle (integrated square, naturally alias-free below Nyquist).
            /// Requires one sample of state; pass `prev` by reference.
            inline float triangle(double phase, double dt, float& prev) noexcept {
                float sq    = square(phase, dt);
                float tri   = static_cast<float>(4.0 * dt) * sq + (1.0f - static_cast<float>(4.0 * dt)) * prev;
                prev = tri;
                return tri;
            }

            /// Band-limited pulse with variable duty cycle.  dt = freq / sample_rate.
            inline float pulse(double phase, double dt, double duty = 0.5) noexcept {
                double t  = phase / TWO_PI;
                double pw = (t < duty) ? 1.0 : -1.0;
                pw += polyblep(t, dt);
                pw -= polyblep(std::fmod(t + (1.0 - duty), 1.0), dt);
                return static_cast<float>(pw);
            }
        }

        // =============================================================================
        //  §5  WAVESHAPE ENUM
        // =============================================================================

        enum class WaveShape { Sine, Square, Triangle, Sawtooth, Pulse,
                               BL_Square, BL_Triangle, BL_Sawtooth, BL_Pulse };

        // =============================================================================
        //  §6  OSCILLATOR
        //  Phase-accumulating oscillator with hard-sync and portamento support.
        //  Hard sync: when sync_source phase resets, this oscillator resets too.
        //  Portamento: call glide_to(target, seconds) and tick() handles the ramp.
        // =============================================================================

        class Oscillator {
        public:
            WaveShape shape        = WaveShape::Sine;
            double    frequency    = 440.0;
            double    amplitude    = 1.0;
            double    phase_offset = 0.0;
            double    duty         = 0.5;
            bool      hard_sync    = false;   ///< If true, caller must call sync_reset() at slave reset

            Oscillator() = default;
            explicit Oscillator(WaveShape s, double f = 440.0, double a = 1.0)
                : shape(s), frequency(f), amplitude(a) {}

            float tick(int sample_rate) noexcept {
                // Portamento: smoothly move frequency toward glide_target_
                if (glide_samples_left_ > 0) {
                    frequency += glide_step_;
                    --glide_samples_left_;
                    if (glide_samples_left_ == 0) frequency = glide_target_;
                }

                const double dt = frequency / static_cast<double>(sample_rate);
                phase_ += TWO_PI * dt;
                if (phase_ >= TWO_PI) phase_ -= TWO_PI;
                return sample_at(phase_ + phase_offset, dt) * static_cast<float>(amplitude);
            }

            /// Begin a smooth frequency glide to `target_freq` over `seconds`.
            /// Call before the first tick() of a new note.
            void glide_to(double target_freq, double seconds, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                glide_target_      = target_freq;
                glide_samples_left_ = static_cast<int>(seconds * sample_rate);
                glide_step_ = (glide_samples_left_ > 0)
                              ? (target_freq - frequency) / glide_samples_left_
                              : 0.0;
            }

            /// Cancel any in-progress glide and snap to current frequency.
            void cancel_glide() noexcept { glide_samples_left_ = 0; }

            bool is_gliding() const noexcept { return glide_samples_left_ > 0; }

            /// Hard sync: reset phase when master crosses zero upward.
            void sync_reset() noexcept { phase_ = 0.0; }

            void   reset()             noexcept { phase_ = 0.0; }
            double phase()       const noexcept { return phase_; }
            void   set_phase(double p) noexcept { phase_ = p; }

            Buffer render(double seconds, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                Buffer buf(static_cast<size_t>(seconds * sample_rate));
                for (auto& s : buf) s = tick(sample_rate);
                return buf;
            }

        private:
            double phase_              = 0.0;
            double glide_target_       = 440.0;
            double glide_step_         = 0.0;
            int    glide_samples_left_ = 0;
            float  bl_tri_prev_        = 0.0f;  ///< State for BL triangle integrator

            float sample_at(double p, double dt) const noexcept {
                switch (shape) {
                    case WaveShape::Sine:        return wave::sine(p);
                    case WaveShape::Square:      return wave::square(p);
                    case WaveShape::Triangle:    return wave::triangle(p);
                    case WaveShape::Sawtooth:    return wave::sawtooth(p);
                    case WaveShape::Pulse:       return wave::pulse(p, duty);
                    case WaveShape::BL_Square:   return wave_bl::square(p, dt);
                    case WaveShape::BL_Sawtooth: return wave_bl::sawtooth(p, dt);
                    case WaveShape::BL_Triangle: return wave_bl::triangle(p, dt,
                                                     const_cast<Oscillator*>(this)->bl_tri_prev_);
                    case WaveShape::BL_Pulse:    return wave_bl::pulse(p, dt, duty);
                }
                return 0.f;
            }
        };

        // =============================================================================
        //  §7  SUPERSAW HELPER
        //  Renders N detuned sawtooth oscillators into a mono buffer.
        //  Classic JP-8000 supersaw: 7 voices, center at 0 Hz detune, outer voices
        //  spread symmetrically up to ±detune_cents cents.
        // =============================================================================

        class Supersaw {
        public:
            int    voices       = 7;      ///< Number of oscillator voices
            double frequency    = 440.0;
            double detune_cents = 12.0;   ///< Total spread in cents (split across voices)
            double amplitude    = 1.0;
            double mix_center   = 0.7;    ///< Gain of centre voice relative to side voices

            /// Render `seconds` of supersaw audio (mono).
            Buffer render(double seconds, int sample_rate = 0) const {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                const size_t n = static_cast<size_t>(seconds * sample_rate);
                Buffer out(n, 0.0f);

                const int    v     = std::max(1, voices);
                const double span  = detune_cents / 100.0 / 12.0; // semitones
                const double step  = (v > 1) ? 2.0 * span / (v - 1) : 0.0;

                for (int i = 0; i < v; ++i) {
                    const double detune_st = -span + i * step;
                    const double freq      = frequency * std::pow(2.0, detune_st);
                    const double gain      = (i == v / 2) ? mix_center : (1.0 - mix_center) / (v - 1);

                    double phase = 0.0;
                    const double inc = TWO_PI * freq / sample_rate;
                    for (size_t s = 0; s < n; ++s) {
                        out[s] += static_cast<float>(wave::sawtooth(phase) * gain * amplitude);
                        phase += inc;
                    }
                }
                return out;
            }

            /// Render into a stereo buffer with panning spread across voices.
            StereoBuffer render_stereo(double seconds, int sample_rate = 0) const {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                const size_t n = static_cast<size_t>(seconds * sample_rate);
                StereoBuffer out(n, 0.0f);

                const int    v    = std::max(1, voices);
                const double span = detune_cents / 100.0 / 12.0;
                const double step = (v > 1) ? 2.0 * span / (v - 1) : 0.0;

                for (int i = 0; i < v; ++i) {
                    const double detune_st = -span + i * step;
                    const double freq      = frequency * std::pow(2.0, detune_st);
                    const double gain      = (i == v / 2)
                        ? mix_center : (1.0 - mix_center) / (v - 1);

                    // Pan: spread voices from full left to full right
                    const double pan  = (v > 1) ? static_cast<double>(i) / (v - 1) : 0.5;
                    const float  gainL = static_cast<float>(gain * std::cos(pan * PI * 0.5));
                    const float  gainR = static_cast<float>(gain * std::sin(pan * PI * 0.5));

                    double phase = 0.0;
                    const double inc = TWO_PI * freq / sample_rate;
                    for (size_t s = 0; s < n; ++s) {
                        const float samp = static_cast<float>(wave::sawtooth(phase) * amplitude);
                        out.L[s] += samp * gainL;
                        out.R[s] += samp * gainR;
                        phase += inc;
                    }
                }
                return out;
            }
        };

        // =============================================================================
        //  §7b  WAVETABLE OSCILLATOR
        //  Lookup-table oscillator with linear interpolation.  The table can be any
        //  arbitrary waveform — fill it from the wave:: helpers, load from disk, or
        //  compute your own.  Linear-interpolated reads give good quality; combine
        //  with PolyBLEP (wave_bl::) tables for alias-free output.
        //
        //  Usage:
        //    WavetableOscillator osc;
        //    osc.load_wave(WaveShape::Sawtooth);   // or fill osc.table manually
        //    float s = osc.tick(440.0, sample_rate);
        // =============================================================================

        class WavetableOscillator {
        public:
            static constexpr size_t TABLE_SIZE = 2048;

            std::array<float, TABLE_SIZE> table{};

            WavetableOscillator() { load_sine(); }

            /// Fill the table with a standard WaveShape.
            /// BL_ variants are pre-computed at a reference frequency of 440 Hz with
            /// dt = 440/44100 — good alias reduction for most musical use.
            void load_wave(WaveShape shape, double ref_freq = 440.0, int ref_sr = 44100) {
                const double dt = ref_freq / ref_sr;
                float bl_tri_prev = 0.0f;
                for (size_t i = 0; i < TABLE_SIZE; ++i) {
                    const double phase = TWO_PI * static_cast<double>(i) / TABLE_SIZE;
                    switch (shape) {
                        case WaveShape::Sine:        table[i] = wave::sine(phase);        break;
                        case WaveShape::Square:      table[i] = wave::square(phase);      break;
                        case WaveShape::Triangle:    table[i] = wave::triangle(phase);    break;
                        case WaveShape::Sawtooth:    table[i] = wave::sawtooth(phase);    break;
                        case WaveShape::Pulse:       table[i] = wave::pulse(phase);       break;
                        case WaveShape::BL_Square:   table[i] = wave_bl::square(phase, dt);   break;
                        case WaveShape::BL_Sawtooth: table[i] = wave_bl::sawtooth(phase, dt); break;
                        case WaveShape::BL_Triangle: table[i] = wave_bl::triangle(phase, dt, bl_tri_prev); break;
                        case WaveShape::BL_Pulse:    table[i] = wave_bl::pulse(phase, dt);    break;
                    }
                }
            }

            void load_sine() { load_wave(WaveShape::Sine); }

            /// Fill table from an arbitrary functor: float fn(double phase_0_to_2pi).
            template<typename Fn>
            void load_custom(Fn&& fn) {
                for (size_t i = 0; i < TABLE_SIZE; ++i)
                    table[i] = fn(TWO_PI * static_cast<double>(i) / TABLE_SIZE);
            }

            /// Advance phase and return the next sample (linearly interpolated).
            float tick(double frequency, int sample_rate) noexcept {
                phase_ = std::fmod(phase_ + frequency / sample_rate, 1.0);
                const double idx  = phase_ * TABLE_SIZE;
                const size_t i0   = static_cast<size_t>(idx) % TABLE_SIZE;
                const size_t i1   = (i0 + 1) % TABLE_SIZE;
                const float  frac = static_cast<float>(idx - static_cast<double>(i0));
                return table[i0] * (1.0f - frac) + table[i1] * frac;
            }

            float tick(double frequency, double amplitude, int sample_rate) noexcept {
                return tick(frequency, sample_rate) * static_cast<float>(amplitude);
            }

            void  reset()              noexcept { phase_ = 0.0; }
            double phase()       const noexcept { return phase_; }
            void   set_phase(double p) noexcept { phase_ = std::fmod(p, 1.0); }

            Buffer render(double frequency, double amplitude,
                          double seconds, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                Buffer buf(static_cast<size_t>(seconds * sample_rate));
                for (auto& s : buf) s = tick(frequency, amplitude, sample_rate);
                return buf;
            }

        private:
            double phase_ = 0.0;  ///< Normalised phase in [0, 1)
        };

        // =============================================================================
        //  §8  NOISE GENERATOR
        // =============================================================================

        class NoiseGenerator {
        public:
            explicit NoiseGenerator(uint32_t seed = 42u)
                : rng_(seed), dist_(-1.0f, 1.0f) {}

            /// White noise sample in [-1, 1].
            float tick_white() { return dist_(rng_); }

            /// Pink noise sample (Paul Kellet's refined method, 1/f spectrum).
            /// Amplitude is approximately normalised to [-1, 1].
            float tick_pink() {
                float white = dist_(rng_);
                b0_ = 0.99886f * b0_ + white * 0.0555179f;
                b1_ = 0.99332f * b1_ + white * 0.0750759f;
                b2_ = 0.96900f * b2_ + white * 0.1538520f;
                b3_ = 0.86650f * b3_ + white * 0.3104856f;
                b4_ = 0.55000f * b4_ + white * 0.5329522f;
                b5_ = -0.7616f * b5_ - white * 0.0168980f;
                float pink = (b0_ + b1_ + b2_ + b3_ + b4_ + b5_ + b6_ + white * 0.5362f) * 0.11f;
                b6_ = white * 0.115926f;
                return std::max(-1.0f, std::min(1.0f, pink));
            }

            /// Convenience alias: tick() returns white noise (preserves prior behaviour).
            float tick() { return tick_white(); }

            void seed(uint32_t s) {
                rng_.seed(s);
                b0_ = b1_ = b2_ = b3_ = b4_ = b5_ = b6_ = 0.0f;
            }

            Buffer render(double seconds, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                Buffer buf(static_cast<size_t>(seconds * sample_rate));
                for (auto& s : buf) s = tick_white();
                return buf;
            }

            Buffer render_pink(double seconds, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                Buffer buf(static_cast<size_t>(seconds * sample_rate));
                for (auto& s : buf) s = tick_pink();
                return buf;
            }

        private:
            std::mt19937 rng_;
            std::uniform_real_distribution<float> dist_;
            // Pink noise state (Kellet's method)
            float b0_ = 0.0f, b1_ = 0.0f, b2_ = 0.0f, b3_ = 0.0f,
                  b4_ = 0.0f, b5_ = 0.0f, b6_ = 0.0f;
        };
    }
}
