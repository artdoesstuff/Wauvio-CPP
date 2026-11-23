#pragma once

#include "common.hpp"
#include "osc.hpp"
#include "env.hpp"
#include "filter.hpp"

namespace wauvio {
    namespace synth {
        // =============================================================================
        //  §15  FM SYNTHESIS  (unchanged from v1)
        // =============================================================================

        class FMSynth {
        public:
            double carrier_freq   = 440.0;
            double modulator_freq = 440.0;
            double mod_index      = 1.0;
            double amplitude      = 1.0;

            Buffer render(double seconds, int sample_rate = 0) const {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                const size_t n = static_cast<size_t>(seconds * sample_rate);
                Buffer buf(n);
                const double c_inc = TWO_PI * carrier_freq   / sample_rate;
                const double m_inc = TWO_PI * modulator_freq / sample_rate;
                double cp = 0.0, mp = 0.0;
                for (size_t i = 0; i < n; ++i) {
                    buf[i] = static_cast<float>(amplitude * std::sin(cp + mod_index * std::sin(mp)));
                    cp += c_inc;  mp += m_inc;
                    if (cp >= TWO_PI) cp -= TWO_PI;
                    if (mp >= TWO_PI) mp -= TWO_PI;
                }
                return buf;
            }
        };

        // =============================================================================
        //  §23  POLYPHONIC VOICE SYSTEM
        // =============================================================================

        /// One voice in a polyphonic synth patch.
        struct Voice {
            bool        active    = false;
            double      frequency = 440.0;
            double      velocity  = 1.0;
            uint64_t    birth_sample = 0;   ///< When was this voice triggered (for stealing)
            float       current_amp  = 0.0f;

            wauvio::osc::Oscillator  osc;
            wauvio::osc::Oscillator  osc2;     ///< Optional second oscillator (detune layer)
            wauvio::env::ADSR        adsr;
            wauvio::env::ADSR::State adsr_state;
            wauvio::filter::SVFilter    filt;

            void trigger(double freq, double vel, const wauvio::env::ADSR& env,
                         wauvio::osc::WaveShape shape1, wauvio::osc::WaveShape shape2,
                         double detune_hz, uint64_t timestamp, int /*sample_rate*/)
            {
                frequency     = freq;
                velocity      = vel;
                active        = true;
                birth_sample  = timestamp;
                current_amp   = 0.0f;
                adsr          = env;
                adsr_state    = wauvio::env::ADSR::State{};

                osc  = wauvio::osc::Oscillator(shape1, freq, vel);
                osc2 = wauvio::osc::Oscillator(shape2, freq + detune_hz, vel * 0.6);

                filt.reset();
            }

            void release() { adsr_state.note_off_trigger(); }

            /// Returns next sample (mono).
            float tick(int sample_rate) noexcept {
                if (!active) return 0.0f;
                const float env = adsr.tick(adsr_state, sample_rate);
                if (adsr_state.phase == wauvio::env::ADSR::State::DONE) { active = false; return 0.0f; }
                current_amp  = env;
                const float s = (osc.tick(sample_rate) + osc2.tick(sample_rate)) * 0.5f;
                return filt.tick(s * env, sample_rate);
            }
        };

        /// Polyphonic voice manager - allocates voices, handles stealing.
        class VoiceManager {
        public:
            static constexpr int MAX_VOICES = 16;

            wauvio::osc::WaveShape shape1       = wauvio::osc::WaveShape::Sawtooth;
            wauvio::osc::WaveShape shape2       = wauvio::osc::WaveShape::Square;
            double    detune_hz    = 2.5;
            wauvio::env::ADSR      adsr         = {0.01, 0.08, 0.7, 0.15};
            wauvio::filter::SVFilter  filter_proto;   ///< Prototype filter settings copied per voice

            void note_on(double freq, double velocity = 1.0) {
                Voice* v = find_free_voice();
                if (!v) v = steal_voice();
                v->filt = filter_proto;
                v->trigger(freq, velocity, adsr, shape1, shape2, detune_hz,
                           sample_counter_, global_config().sample_rate);
            }

            void note_off(double freq) {
                for (auto& v : voices_)
                    if (v.active && std::abs(v.frequency - freq) < 0.1)
                        v.release();
            }

            void all_notes_off() {
                for (auto& v : voices_) v.release();
            }

            float tick(int sample_rate) noexcept {
                float sum = 0.0f;
                for (auto& v : voices_) sum += v.tick(sample_rate);
                ++sample_counter_;
                return sum;
            }

            Buffer render(double seconds, int sample_rate = 0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                const size_t n = static_cast<size_t>(seconds * sample_rate);
                Buffer out(n);
                for (size_t i = 0; i < n; ++i) out[i] = tick(sample_rate);
                return out;
            }

            int active_voices() const {
                int n = 0;
                for (const auto& v : voices_) if (v.active) ++n;
                return n;
            }

        private:
            Voice    voices_[MAX_VOICES];
            uint64_t sample_counter_ = 0;

            Voice* find_free_voice() {
                for (auto& v : voices_) if (!v.active) return &v;
                return nullptr;
            }

            Voice* steal_voice() {
                // Steal the oldest voice
                Voice* oldest = &voices_[0];
                for (auto& v : voices_)
                    if (v.birth_sample < oldest->birth_sample) oldest = &v;
                return oldest;
            }
        };
    }
}
