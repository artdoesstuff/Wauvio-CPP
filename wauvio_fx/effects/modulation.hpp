#pragma once
#include "../core/fx_effect.hpp"

namespace wauvio {
    namespace fx {
        class FracDelay {
        public:
            void init(size_t max_samples) { buf_.assign(max_samples + 8, 0.0f); pos_ = 0; }
            void clear() { std::fill(buf_.begin(), buf_.end(), 0.0f); pos_ = 0; }
            size_t capacity() const noexcept { return buf_.size() > 8 ? buf_.size() - 8 : 0; }
            void push(float x) noexcept { buf_[pos_] = x; pos_ = (pos_ + 1) % buf_.size(); }
            /// Read `d` samples back (d >= 1; 1 = the most recent push). Fractional delays use
            /// Catmull-Rom interpolation.
            float read(double d) const noexcept {
                const size_t N = buf_.size();
                d = std::min(std::max(d, 1.0), static_cast<double>(N - 4));
                const size_t i = static_cast<size_t>(d);
                const float t = static_cast<float>(1.0 - (d - static_cast<double>(i)));   // 0 -> delay i+1, 1 -> delay i
                const float p0 = at(i + 2), p1 = at(i + 1), p2 = at(i), p3 = at(i > 1 ? i - 1 : 1);
                return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
                               (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
            }
        private:
            float at(size_t back) const noexcept {               // back=1 -> latest sample pushed
                const size_t N = buf_.size();
                return buf_[(pos_ + N - (back % N)) % N];
            }
            std::vector<float> buf_; size_t pos_ = 0;
        };

        class Tremolo : public Effect {
        public:
            double rate_hz   = 5.0;
            double sync_bpm  = 0.0;        ///< > 0: rate follows tempo (one cycle per sync_beats)
            double sync_beats = 1.0;
            double depth     = 0.5;        ///< 0..1
            double stereo_phase_deg = 0.0; ///< right channel LFO offset (180 = alternating L/R)
            LfoShape shape   = LfoShape::Sine;

            std::string name() const override { return "Tremolo"; }
            void reset() override { pos_ = 0; }
            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                const double rate = sync_bpm > 0.0 ? sync_bpm / 60.0 / std::max(sync_beats, 1e-6) : rate_hz;
                const double ph_r = stereo_phase_deg / 360.0, d = std::min(1.0, std::max(0.0, depth));
                for (size_t i = 0; i < n; ++i) {
                    const double ph = rate * static_cast<double>(pos_ + i) / sr_;
                    const double ul = 0.5 + 0.5 * lfo_eval(shape, ph), ur = 0.5 + 0.5 * lfo_eval(shape, ph + ph_r);
                    L[i] *= static_cast<float>(1.0 - d * (1.0 - ul));
                    R[i] *= static_cast<float>(1.0 - d * (1.0 - ur));
                }
                pos_ += n;
            }
        private:
            size_t pos_ = 0;
        };

        class AutoPan : public Effect {
        public:
            double rate_hz = 0.5, sync_bpm = 0.0, sync_beats = 4.0;
            double depth = 1.0;           ///< 0 = centre, 1 = full left <-> right
            LfoShape shape = LfoShape::Sine;
            double phase_deg = 0.0;

            std::string name() const override { return "AutoPan"; }
            void reset() override { pos_ = 0; }
            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                const double rate = sync_bpm > 0.0 ? sync_bpm / 60.0 / std::max(sync_beats, 1e-6) : rate_hz;
                const double d = std::min(1.0, std::max(0.0, depth));
                for (size_t i = 0; i < n; ++i) {
                    const double p = d * lfo_eval(shape, rate * static_cast<double>(pos_ + i) / sr_ + phase_deg / 360.0);   // -1..1
                    const double th = (p + 1.0) * PI / 4.0;
                    L[i] *= static_cast<float>(std::sqrt(2.0) * std::cos(th));
                    R[i] *= static_cast<float>(std::sqrt(2.0) * std::sin(th));
                }
                pos_ += n;
            }
        private:
            size_t pos_ = 0;
        };

        class StereoChorus : public Effect {
        public:
            double rate_hz  = 0.6;
            double depth_ms = 2.5;
            double delay_ms = 14.0;
            int    voices   = 3;          ///< 1..6 per channel
            double spread   = 1.0;        ///< 0..1: how far apart the L/R LFO phases are
            double mix      = 0.5;
            LfoShape shape  = LfoShape::Sine;

            std::string name() const override { return "StereoChorus"; }
            double tail_seconds() const override { return (delay_ms + depth_ms) * 0.001; }
            void reset() override { pos_ = 0; for (auto& d : dl_) d.clear(); }

            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                const size_t cap = static_cast<size_t>((delay_ms + depth_ms + 5.0) * 0.001 * sr_) + 16;
                if (dl_[0].capacity() < cap) { dl_[0].init(cap); dl_[1].init(cap); }
                const int V = std::min(6, std::max(1, voices));
                const double wet = std::min(1.0, std::max(0.0, mix));
                float* ch[2] = { L, R };
                for (size_t i = 0; i < n; ++i) {
                    for (int c = 0; c < 2; ++c) {
                        const float x = ch[c][i];
                        dl_[c].push(x);
                        double acc = 0.0;
                        for (int v = 0; v < V; ++v) {
                            const double ph = rate_hz * static_cast<double>(pos_ + i) / sr_
                                            + static_cast<double>(v) / V + (c ? 0.25 * spread : 0.0);
                            const double d = (delay_ms + depth_ms * lfo_eval(shape, ph)) * 0.001 * sr_;
                            acc += dl_[c].read(d);
                        }
                        ch[c][i] = static_cast<float>(x * (1.0 - wet) + (acc / V) * wet);
                    }
                }
                pos_ += n;
            }
        private:
            FracDelay dl_[2]; size_t pos_ = 0;
        };

        /// Adapter for Wauvio's core mono effects (anything with init(sr) and process(Buffer&)).
        ///   auto& fl = chain.add<DualMono<wauvio::Flanger>>();  fl.left().rate_hz = 0.3;
        /// Use configure() to change both sides at once.
        template <class T>
        class DualMono : public Effect {
        public:
            DualMono() = default;
            std::string name() const override { return "DualMono"; }
            T& left()  { return l_; }
            T& right() { return r_; }
            template <class F> DualMono& configure(F&& f) { f(l_); f(r_); return *this; }
            double extra_tail_seconds = 0.0;   ///< set if the wrapped effect rings (e.g. a flanger with feedback)
            double tail_seconds() const override { return extra_tail_seconds; }
            void reset() override { l_.init(sr_); r_.init(sr_); }
            using Effect::process;
            void process(float* L, float* R, size_t n) override {
                bl_.assign(L, L + n); br_.assign(R, R + n);
                l_.process(bl_); r_.process(br_);
                std::copy(bl_.begin(), bl_.end(), L); std::copy(br_.begin(), br_.end(), R);
            }
        private:
            T l_, r_; Buffer bl_, br_;
        };
    }
}
