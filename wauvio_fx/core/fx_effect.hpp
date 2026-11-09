#pragma once
#include "fx_common.hpp"

namespace wauvio {
    namespace fx {
        /// Base class for every wauvio_fx processor.
        ///
        /// Parameters are plain public fields (like the rest of Wauvio) and are re-read
        /// at the start of every process() call, so they can be changed between calls.
        /// The raw-pointer process() is the streaming entry point (call prepare() first);
        /// the StereoBuffer / Buffer overloads prepare automatically.
        class Effect {
        public:
            virtual ~Effect() = default;
            bool bypass = false;                          ///< honoured by Chain (skips the effect and its latency/tail)

            virtual std::string name() const = 0;
            virtual void reset() = 0;                     ///< clear internal state (keeps parameters)
            virtual void process(float* L, float* R, size_t n) = 0;   ///< in place, streaming
            virtual double tail_seconds() const { return 0.0; }       ///< how long the output rings after silence
            virtual size_t latency_samples() const { return 0; }      ///< lookahead / FIR delay in samples

            /// Allocates for `sr` and resets. Must be called before the raw process().
            void prepare(int sr) { sr_ = sr > 0 ? sr : global_config().sample_rate; prepared_ = true; on_prepare(); reset(); }
            int  sample_rate() const noexcept { return sr_; }
            bool prepared() const noexcept { return prepared_; }
            void ensure_prepared(int sr) { if (sr <= 0) sr = global_config().sample_rate; if (!prepared_ || sr != sr_) prepare(sr); }

            /// Convenience: whole-buffer, in place, no tail extension, latency NOT compensated.
            void process(StereoBuffer& b, int sr = 0) {
                ensure_prepared(sr);
                process(b.L.data(), b.R.data(), b.size());
            }
            /// Mono convenience: runs as dual-mono and returns the channel average.
            void process(Buffer& m, int sr = 0) {
                ensure_prepared(sr);
                Buffer r = m;
                process(m.data(), r.data(), m.size());
                for (size_t i = 0; i < m.size(); ++i) m[i] = 0.5f * (m[i] + r[i]);
            }

        protected:
            virtual void on_prepare() {}
            int  sr_ = 44100;
            bool prepared_ = false;
        };

        /// An ordered list of effects. Whole-buffer processing with automatic
        /// latency compensation and tail extension.
        class Chain {
        public:
            Chain() = default;
            Chain(Chain&&) = default;
            Chain& operator=(Chain&&) = default;

            /// Construct an effect in place and return a reference so you can set parameters:
            ///   auto& c = chain.add<Compressor>(); c.threshold_db = -20;
            template <class E, class... A>
            E& add(A&&... a) {
                auto p = std::make_unique<E>(std::forward<A>(a)...);
                E& ref = *p; fx_.push_back(std::move(p)); return ref;
            }
            template <class E>
            E& add(std::unique_ptr<E> p) { E& ref = *p; fx_.push_back(std::move(p)); return ref; }

            size_t size() const noexcept { return fx_.size(); }
            bool   empty() const noexcept { return fx_.empty(); }
            Effect& at(size_t i) { return *fx_.at(i); }
            const Effect& at(size_t i) const { return *fx_.at(i); }
            void clear() { fx_.clear(); }

            size_t latency_samples() const { size_t t = 0; for (auto& e : fx_) if (!e->bypass) t += e->latency_samples(); return t; }
            double tail_seconds() const   { double t = 0; for (auto& e : fx_) if (!e->bypass) t += e->tail_seconds(); return t; }

            void reset() { for (auto& e : fx_) e->reset(); }

            /// Process a whole buffer from a clean state. Output is aligned with the input
            /// (latency removed) and, if keep_tail, extended by tail_seconds() so reverb and
            /// delay tails are not cut off.
            void process(StereoBuffer& buf, int sr = 0, bool keep_tail = true) {
                if (fx_.empty()) return;
                if (sr <= 0) sr = global_config().sample_rate;
                for (auto& e : fx_) { e->ensure_prepared(sr); e->reset(); }
                const size_t n = buf.size(), lat = latency_samples();
                const size_t tail = keep_tail ? static_cast<size_t>(std::ceil(tail_seconds() * sr)) : 0;
                buf.L.resize(n + lat + tail, 0.0f);
                buf.R.resize(n + lat + tail, 0.0f);
                for (auto& e : fx_) if (!e->bypass) e->process(buf.L.data(), buf.R.data(), buf.size());
                if (lat) {
                    buf.L.erase(buf.L.begin(), buf.L.begin() + static_cast<std::ptrdiff_t>(lat));
                    buf.R.erase(buf.R.begin(), buf.R.begin() + static_cast<std::ptrdiff_t>(lat));
                }
            }
            void process(Buffer& mono, int sr = 0, bool keep_tail = true) {
                StereoBuffer s; s.L = mono; s.R = mono;
                process(s, sr, keep_tail);
                mono.resize(s.size());
                for (size_t i = 0; i < mono.size(); ++i) mono[i] = 0.5f * (s.L[i] + s.R[i]);
            }

        private:
            std::vector<std::unique_ptr<Effect>> fx_;
        };
    }
}
