#pragma once

#include "common.hpp"

namespace wauvio {
    namespace env {
        // =============================================================================
        //  §9  ADSR ENVELOPE  (unchanged from v1, retained for compatibility)
        // =============================================================================

        class ADSR {
        public:
            double attack  = 0.01;
            double decay   = 0.05;
            double sustain = 0.7;
            double release = 0.1;

            ADSR() = default;
            ADSR(double a, double d, double s, double r)
                : attack(a), decay(d), sustain(s), release(r) {}

            void apply(Buffer& buf, double total_seconds,
                       double note_off = -1.0, int sample_rate = 0) const
            {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                const double sr = static_cast<double>(sample_rate);
                const size_t n  = buf.size();

                const size_t a_end   = static_cast<size_t>(attack * sr);
                const size_t d_end   = a_end + static_cast<size_t>(decay * sr);
                if (note_off < 0.0) note_off = total_seconds - release;
                note_off = std::max(0.0, note_off);
                const size_t r_start = static_cast<size_t>(note_off * sr);
                const size_t r_end   = r_start + static_cast<size_t>(release * sr);

                for (size_t i = 0; i < n; ++i) {
                    double env;
                    if      (i < a_end)   env = a_end > 0 ? (double)i / a_end : 1.0;
                    else if (i < d_end)   env = 1.0 - (double)(i-a_end)/(d_end-a_end) * (1.0-sustain);
                    else if (i < r_start) env = sustain;
                    else if (i < r_end)   env = sustain * (1.0 - (double)(i-r_start)/(r_end-r_start+1));
                    else                  env = 0.0;
                    buf[i] *= static_cast<float>(env);
                }
            }

            /// Sample-by-sample stateful version (for voice systems).
            struct State {
                enum Phase { ATTACK, DECAY, SUSTAIN, RELEASE, DONE } phase = ATTACK;
                double value    = 0.0;
                double note_off = -1.0;  ///< Call note_off() to trigger release
                size_t sample   = 0;

                void note_off_trigger() { phase = RELEASE; }
            };

            float tick(State& st, int sample_rate) const noexcept {
                const double sr     = static_cast<double>(sample_rate);
                const double a_rate = (attack  > 0) ? 1.0 / (attack  * sr) : 1.0;
                const double d_rate = (decay   > 0) ? 1.0 / (decay   * sr) : 1.0;
                const double r_rate = (release > 0) ? 1.0 / (release * sr) : 1.0;

                switch (st.phase) {
                    case State::ATTACK:
                        st.value += a_rate;
                        if (st.value >= 1.0) { st.value = 1.0; st.phase = State::DECAY; }
                        break;
                    case State::DECAY:
                        st.value -= d_rate * (1.0 - sustain);
                        if (st.value <= sustain) { st.value = sustain; st.phase = State::SUSTAIN; }
                        break;
                    case State::SUSTAIN:
                        st.value = sustain;
                        break;
                    case State::RELEASE:
                        st.value -= r_rate * sustain;
                        if (st.value <= 0.0) { st.value = 0.0; st.phase = State::DONE; }
                        break;
                    case State::DONE:
                        st.value = 0.0;
                        break;
                }
                return static_cast<float>(st.value);
            }
        };

        // =============================================================================
        //  §10  AHDSR / DAHDSR MULTI-STAGE ENVELOPE
        //  Hold stage between attack and decay.
        //  Optional pre-delay (D = initial delay before attack).
        // =============================================================================

        class DAHDSR {
        public:
            double pre_delay = 0.0;   ///< D: seconds of silence before attack
            double attack    = 0.01;
            double hold      = 0.0;   ///< H: seconds at peak before decay
            double decay     = 0.05;
            double sustain   = 0.7;
            double release   = 0.1;

            DAHDSR() = default;
            DAHDSR(double d, double a, double h, double dec, double s, double r)
                : pre_delay(d), attack(a), hold(h), decay(dec), sustain(s), release(r) {}

            void apply(Buffer& buf, double total_seconds,
                       double note_off = -1.0, int sample_rate = 0) const
            {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                const double sr = sample_rate;
                const size_t n  = buf.size();
                // default note-off = release before the end, but never earlier than half the note: a release longer than the note
                // would otherwise start at 0 and silence it
                if (note_off < 0.0) note_off = std::max(total_seconds - release, 0.5 * total_seconds);
                note_off = std::max(0.0, note_off);

                const size_t d_end   = static_cast<size_t>(pre_delay * sr);
                const size_t a_end   = d_end + static_cast<size_t>(attack * sr);
                const size_t h_end   = a_end + static_cast<size_t>(hold * sr);
                const size_t dec_end = h_end + static_cast<size_t>(decay * sr);
                const size_t r_start = static_cast<size_t>(note_off * sr);
                const size_t r_end   = r_start + static_cast<size_t>(release * sr);

                // level before the release: follows the stages up to note-off. A note-off that arrives while the attack or decay is
                // still running releases from the level it had reached (it used to jump straight to the release curve).
                auto stage_level = [&](size_t i) -> double {
                    if      (i < d_end)   return 0.0;
                    else if (i < a_end)   return a_end > d_end ? (double)(i-d_end)/(a_end-d_end) : 1.0;
                    else if (i < h_end)   return 1.0;
                    else if (i < dec_end) return 1.0 - (double)(i-h_end)/(dec_end-h_end)*(1.0-sustain);
                    return sustain;
                };
                const double release_from = stage_level(r_start);

                for (size_t i = 0; i < n; ++i) {
                    double env;
                    if      (i < r_start) env = stage_level(i);
                    else if (i < r_end)   env = release_from*(1.0-(double)(i-r_start)/(r_end-r_start+1));
                    else                  env = 0.0;
                    buf[i] *= static_cast<float>(env);
                }
            }
        };

        // =============================================================================
        //  §11  LOOPABLE ENVELOPE
        //  Loops the attack→decay→sustain cycle indefinitely - behaves like a
        //  slow LFO with a shaped waveform.
        // =============================================================================

        class LoopableEnvelope {
        public:
            double attack  = 0.1;
            double decay   = 0.2;
            double sustain = 0.5;   ///< Level held between decay and next attack
            bool   loop    = true;

            struct State {
                enum Phase { ATT, DEC, SUS } phase = ATT;
                double value = 0.0;
            };

            float tick(State& st, int sample_rate) const noexcept {
                const double sr     = sample_rate;
                const double a_rate = attack > 0 ? 1.0 / (attack * sr) : 1.0;
                const double d_rate = decay  > 0 ? 1.0 / (decay  * sr) : 1.0;

                switch (st.phase) {
                    case State::ATT:
                        st.value += a_rate;
                        if (st.value >= 1.0) { st.value = 1.0; st.phase = State::DEC; }
                        break;
                    case State::DEC:
                        st.value -= d_rate * (1.0 - sustain);
                        if (st.value <= sustain) {
                            st.value = sustain;
                            st.phase = loop ? State::ATT : State::SUS;
                        }
                        break;
                    case State::SUS:
                        break;
                }
                return static_cast<float>(st.value);
            }
        };
    } // namespace env
} // namespace wauvio
