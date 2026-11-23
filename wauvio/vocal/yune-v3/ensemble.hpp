#pragma once

#include "singer.hpp"

#include <string>
#include <vector>

namespace wauvio {
    namespace vocal {
        namespace v3 {

            /// V3: several Singers rendered together and mixed to stereo: harmonies, doubled leads, small choirs, duets.
            ///
            ///   Singer lead; ... add notes ...
            ///   Ensemble e;
            ///   e.add(lead, "lead");                                  // centre
            ///   e.add_double(lead, 2);                                // two other takes, detuned and spread left/right
            ///   e.add_harmony(lead, +4, "third", -0.4f, 0.6f);        // a third above, a little left, quieter
            ///   e.add_harmony(lead, -5, "low",   0.4f, 0.6f, &voice_itsuki());   // a fourth below, sung by another voice
            ///   StereoBuffer mix = e.render_stereo();
            ///
            /// Every part is an independent monophonic Singer, so each keeps its own coarticulation, pitch tracker and
            /// voice. Parts are mixed with constant-power panning; no normalisation is applied (use normalize or a chain).
            class Ensemble {
            public:
                struct Part {
                    Singer singer;
                    std::string name;
                    float  pan   = 0.f;     // -1 left .. +1 right
                    float  gain  = 1.f;
                    double delay = 0.0;     // seconds of silence added in front of this part
                    bool   mute  = false;
                };

                /// Adds a copy of `s` as a part and returns it (set .pan/.gain/.delay or edit .singer afterwards).
                Part& add(const Singer& s, const std::string& name = "", float pan = 0.f, float gain = 1.f, double delay_s = 0.0) {
                    parts_.push_back(Part{ s, name.empty() ? "part" + std::to_string(parts_.size() + 1) : name, pan, gain, delay_s, false });
                    return parts_.back();
                }
                /// Adds an empty part with its own voice for you to fill in (`ensemble[i].singer.note(...)`).
                Part& add_voice(const VoiceProfile& p, const std::string& name = "", float pan = 0.f, float gain = 1.f) {
                    return add(Singer(p), name, pan, gain);
                }

                /// Same notes, shifted by `semitones`. Pass a profile to have another voice sing it.
                Part& add_harmony(const Singer& lead, int semitones, const std::string& name = "", float pan = 0.f, float gain = 0.7f,
                                  const VoiceProfile* voice = nullptr, double delay_s = 0.0) {
                    Singer h = lead.transposed(semitones);
                    if (voice) h.set_voice(voice->model);
                    return add(h, name.empty() ? "harmony" + std::to_string(semitones) : name, pan, gain, delay_s);
                }

                /// Double-tracking: `copies` extra takes of the same notes, each with its own timing/pitch imperfection,
                /// a few cents of detune and a small entry delay, spread across the stereo field.
                /// spread: 0..1 how far left/right the copies go; detune_cents: largest detune; gain: level of each copy.
                Ensemble& add_double(const Singer& lead, int copies = 2, float spread = 0.7f, double detune_cents = 6.0,
                                     float gain = 0.55f, double max_delay_s = 0.012, uint32_t first_take = 11) {
                    for (int k = 0; k < copies; ++k) {
                        const uint32_t take = first_take + static_cast<uint32_t>(k) * 17u;
                        const double side = (k % 2 == 0) ? -1.0 : 1.0;
                        const double rank = static_cast<double>(k / 2 + 1) / static_cast<double>((copies + 1) / 2);
                        const double cents = side * detune_cents * (0.45 + 0.55 * unit_(take));
                        const double delay = max_delay_s * (0.25 + 0.75 * unit_(take + 3u));
                        Singer d = lead.doubled(take, 0.3f, cents);
                        add(d, "double" + std::to_string(k + 1), static_cast<float>(side * spread * rank), gain, delay);
                    }
                    return *this;
                }

                size_t size() const noexcept { return parts_.size(); }
                Part& operator[](size_t i) { return parts_.at(i); }
                const Part& operator[](size_t i) const { return parts_.at(i); }
                void clear() { parts_.clear(); }

                /// Renders every part and mixes them (parts are rendered independently, so this is deterministic).
                StereoBuffer render_stereo(int sample_rate = 0) const {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    StereoBuffer mix;
                    for (const Part& p : parts_) {
                        if (p.mute) continue;
                        Buffer m = p.singer.render(sample_rate);
                        if (m.empty()) continue;
                        const size_t lead = static_cast<size_t>(std::max(0.0, p.delay) * sample_rate);
                        StereoBuffer s = pan_mono(m, p.pan, sample_rate);
                        const size_t need = lead + s.size();
                        if (mix.size() < need) { mix.L.resize(need, 0.f); mix.R.resize(need, 0.f); }
                        for (size_t i = 0; i < s.size(); ++i) { mix.L[lead + i] += s.L[i] * p.gain; mix.R[lead + i] += s.R[i] * p.gain; }
                    }
                    return mix;
                }

                /// Renders, normalises to 0.9 peak and writes a stereo WAV.
                void save(const std::string& path, int sample_rate = 0) const {
                    StereoBuffer b = render_stereo(sample_rate);
                    float pk = 0.f;
                    for (size_t i = 0; i < b.size(); ++i) pk = std::max(pk, std::max(std::fabs(b.L[i]), std::fabs(b.R[i])));
                    if (pk > 1e-6f) { const float g = 0.9f / pk; for (size_t i = 0; i < b.size(); ++i) { b.L[i] *= g; b.R[i] *= g; } }
                    save_wav_stereo(b, path, sample_rate);
                }

            private:
                std::vector<Part> parts_;
                static double unit_(uint32_t x) {            // deterministic 0..1 from a take number
                    x = (x ^ 61u) ^ (x >> 16); x *= 9u; x ^= x >> 4; x *= 0x27d4eb2du; x ^= x >> 15;
                    return static_cast<double>(x & 0xFFFFu) / 65535.0;
                }
            };

        }
    }
}
