#pragma once

#include "common.hpp"
#include "osc.hpp"
#include "env.hpp"
#include "filter.hpp"

namespace wauvio {
    namespace seq {
        // =============================================================================
        //  §25  NOTE / INSTRUMENT / TRACK  (unchanged from v1, stereo-aware)
        // =============================================================================

        struct Note {
            double frequency = 440.0;
            double duration  = 0.25;
            double amplitude = 0.8;
            Note() = default;
            Note(double f, double d, double a = 0.8) : frequency(f), duration(d), amplitude(a) {}
        };

        struct Instrument {
            wauvio::osc::WaveShape shape      = wauvio::osc::WaveShape::Sine;
            wauvio::env::ADSR      envelope   = {0.01, 0.05, 0.7, 0.08};
            float     gain       = 1.0f;
            double    detune     = 0.0;
            bool      use_lpf    = false;
            double    lpf_cutoff = 2000.0;
            double    duty       = 0.5;

            Buffer render_note(const Note& note, int sample_rate = 0) const {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                if (note.frequency < 1.0) return make_buffer(note.duration, sample_rate);
                Buffer buf = make_buffer(note.duration, sample_rate);
                wauvio::osc::Oscillator osc(shape, note.frequency + detune,
                               note.amplitude * static_cast<double>(gain));
                osc.duty = duty;
                for (size_t i = 0; i < buf.size(); ++i) buf[i] = osc.tick(sample_rate);
                envelope.apply(buf, note.duration, -1.0, sample_rate);
                if (use_lpf) { wauvio::filter::LowPassFilter lpf(lpf_cutoff, sample_rate); lpf.process(buf); }
                return buf;
            }
        };

        class Track {
        public:
            Instrument instrument;
            Track() = default;
            explicit Track(const Instrument& i) : instrument(i) {}

            Track& add(double f, double d, double a = 0.8) {
                notes_.push_back({f, d, a}); return *this;
            }
            Track& rest(double d) { notes_.push_back({0.0, d}); return *this; }
            Track& add(const Note& n) { notes_.push_back(n); return *this; }

            double total_duration() const {
                double t = 0.0; for (auto& n : notes_) t += n.duration; return t;
            }

            Buffer render(int sample_rate = 0) const {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                Buffer out = make_buffer(total_duration(), sample_rate);
                size_t off = 0;
                for (auto& note : notes_) {
                    Buffer nb = instrument.render_note(note, sample_rate);
                    for (size_t i = 0; i < nb.size() && off+i < out.size(); ++i)
                        out[off+i] += nb[i];
                    off += nb.size();
                }
                return out;
            }

        private:
            std::vector<Note> notes_;
        };

        // Helpers
        inline Buffer render_chord(const std::vector<double>& freqs, double duration,
                                    const Instrument& instr, int sample_rate = 0)
        {
            if (sample_rate <= 0) sample_rate = global_config().sample_rate;
            Buffer out = make_buffer(duration, sample_rate);
            for (double f : freqs) {
                if (f < 1.0) continue;
                Instrument tmp = instr; tmp.gain = 1.0f;
                mix_into(out, tmp.render_note({f, duration, instr.gain}, sample_rate), instr.gain);
            }
            return out;
        }

        inline Buffer render_arpeggio(const std::vector<double>& freqs, double step_dur,
                                       int cycles, const Instrument& instr, int sample_rate = 0)
        {
            if (sample_rate <= 0) sample_rate = global_config().sample_rate;
            Track tr(instr);
            const int total = static_cast<int>(freqs.size()) * cycles;
            for (int i = 0; i < total; ++i) tr.add(freqs[i % freqs.size()], step_dur);
            return tr.render(sample_rate);
        }

        // =============================================================================
        //  §26  DRUM SEQUENCER  (unchanged from v1)
        // =============================================================================

        class DrumSequencer {
        public:
            double bpm           = 120.0;
            int    steps_per_bar = 16;

            struct Pattern {
                std::vector<bool>          steps;
                std::function<Buffer(int)> sound;
                float                      gain = 1.0f;
            };

            void add_pattern(std::vector<bool> steps,
                             std::function<Buffer(int)> sound, float gain = 1.0f)
            { patterns_.push_back({ std::move(steps), std::move(sound), gain }); }

            void clear_patterns() { patterns_.clear(); }

            Buffer render(int num_bars = 1, int sample_rate = 0) const {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                const double step_dur = 60.0 / (bpm * static_cast<double>(steps_per_bar) / 4.0);
                Buffer out = make_buffer(step_dur * steps_per_bar * num_bars, sample_rate);
                for (auto& pat : patterns_) {
                    const int ns = static_cast<int>(pat.steps.size());
                    for (int bar = 0; bar < num_bars; ++bar) {
                        for (int s = 0; s < steps_per_bar; ++s) {
                            if (!pat.steps[s % ns]) continue;
                            const size_t off = static_cast<size_t>((bar*steps_per_bar+s)*step_dur*sample_rate);
                            Buffer snd = pat.sound(sample_rate);
                            for (size_t i = 0; i < snd.size() && off+i < out.size(); ++i)
                                out[off+i] += snd[i] * pat.gain;
                        }
                    }
                }
                return out;
            }

        private:
            std::vector<Pattern> patterns_;
        };

        // =============================================================================
        //  §27  MASTER BUS  (now handles both mono Buffer and StereoBuffer)
        // =============================================================================

        class MasterBus {
        public:
            int sample_rate;
            explicit MasterBus(int sr = 0)
                : sample_rate(sr > 0 ? sr : global_config().sample_rate) {}

            void schedule(Buffer buf, double onset, float gain = 1.0f) {
                mono_slots_.push_back({ onset, gain, std::move(buf) });
            }
            void schedule(StereoBuffer buf, double onset, float gain = 1.0f) {
                stereo_slots_.push_back({ onset, gain, std::move(buf) });
            }

            void advance(double s) { cursor_ += s; }
            double cursor() const noexcept { return cursor_; }
            void set_cursor(double t) { cursor_ = t; }

            void place(Buffer buf, float gain = 1.0f) {
                schedule(std::move(buf), cursor_, gain);
            }
            void place(StereoBuffer buf, float gain = 1.0f) {
                schedule(std::move(buf), cursor_, gain);
            }

            /// Mix all scheduled content into a stereo output.
            StereoBuffer mix_stereo() const {
                double total = 0.0;
                for (auto& s : mono_slots_)
                    total = std::max(total, s.onset + (double)s.buf.size() / sample_rate);
                for (auto& s : stereo_slots_)
                    total = std::max(total, s.onset + (double)s.buf.size() / sample_rate);

                StereoBuffer out(static_cast<size_t>(total * sample_rate), 0.0f);

                for (auto& s : mono_slots_) {
                    const size_t off = static_cast<size_t>(s.onset * sample_rate);
                    for (size_t i = 0; i < s.buf.size() && off+i < out.size(); ++i) {
                        const float v = s.buf[i] * s.gain;
                        out.L[off+i] += v;
                        out.R[off+i] += v;
                    }
                }
                for (auto& s : stereo_slots_) {
                    const size_t off = static_cast<size_t>(s.onset * sample_rate);
                    for (size_t i = 0; i < s.buf.size() && off+i < out.size(); ++i) {
                        out.L[off+i] += s.buf.L[i] * s.gain;
                        out.R[off+i] += s.buf.R[i] * s.gain;
                    }
                }
                return out;
            }

            /// Mix all scheduled content into a mono output.
            Buffer mix() const {
                StereoBuffer st = mix_stereo();
                return st.to_mono();
            }

            void reset() { mono_slots_.clear(); stereo_slots_.clear(); cursor_ = 0.0; }

        private:
            struct MonoSlot   { double onset; float gain; Buffer       buf; };
            struct StereoSlot { double onset; float gain; StereoBuffer buf; };
            std::vector<MonoSlot>   mono_slots_;
            std::vector<StereoSlot> stereo_slots_;
            double cursor_ = 0.0;
        };
    }
}
