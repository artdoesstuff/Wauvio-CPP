#pragma once
#include "../../wauvio_ext.hpp"
#include "../../wauvio_fx.hpp"

namespace wauvio {
    namespace fx {
        /// Build a Mixer from an Arrangement: one Source per track (start time and gain preserved).
        /// The Arrangement's own master_reverb()/limiter() are NOT applied - use fx effects instead.
        inline Mixer mixer_from_arrangement(const audio::Arrangement& arr, const RenderOptions& opts = RenderOptions()) {
            const int sr = opts.sample_rate > 0 ? opts.sample_rate : global_config().sample_rate;
            auto prior = audio::default_interpolation_quality();
            audio::default_interpolation_quality() = opts.interpolation;
            const size_t n = arr.track_count();
            std::vector<StereoBuffer> rendered(n);
            const unsigned workers = render_detail::resolve_worker_count(opts.worker_threads);
            if (workers <= 1 || n <= 1) {
                for (size_t i = 0; i < n; ++i) rendered[i] = arr.track_at(i).render(sr);
            } else {
                std::atomic<size_t> next{0};
                std::vector<std::thread> pool;
                for (unsigned w = 0; w < workers; ++w)
                    pool.emplace_back([&]() { size_t i; while ((i = next.fetch_add(1)) < n) rendered[i] = arr.track_at(i).render(sr); });
                for (auto& t : pool) t.join();
            }
            audio::default_interpolation_quality() = prior;
            Mixer mix(sr);
            for (size_t i = 0; i < n; ++i) {
                Source s; s.name = "Track " + std::to_string(i); s.audio = std::move(rendered[i]);
                s.start_sec = arr.start_time_at(i); s.gain = arr.gain_at(i);
                mix.add_source(std::move(s));
            }
            return mix;
        }

        /// Write a stereo buffer exactly as it is (clamped to [-1, 1]; never normalised), so a
        /// loudness-mastered mix keeps its level. Uses the RenderOptions bit depth / dither / seed.
        inline void write_wav(const StereoBuffer& in, const std::string& path, const RenderOptions& opts = RenderOptions()) {
            StereoBuffer b = in;
            RenderOptions o = opts; o.normalize = false; o.render_tail_sec = 0.0; o.loop_count = 1;
            apply_render_options(b, o);
            render_detail::write_wav_custom(b, path, o.sample_rate, o.bit_depth, o.dither, o.seed, o.deterministic_seed_enabled);
        }
    }
}
