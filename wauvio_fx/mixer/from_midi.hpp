#pragma once
#include "../../wauvio_midi.hpp"
#include "from_arrangement.hpp"

namespace wauvio {
    namespace fx {
        /// One Source per MIDI part (names come from the file). Parts all start at 0 and are
        /// rendered by the same instruments / genre settings as the normal MIDI render.
        inline Mixer mixer_from_midi(const track::MidiMusic& music, const RenderOptions& opts = RenderOptions()) {
            const int sr = opts.sample_rate > 0 ? opts.sample_rate : global_config().sample_rate;
            auto prior = audio::default_interpolation_quality();
            audio::default_interpolation_quality() = opts.interpolation;
            const size_t total = music.total_samples(sr), n = music.part_count();
            std::vector<StereoBuffer> rendered(n);
            const unsigned workers = render_detail::resolve_worker_count(opts.worker_threads);
            if (workers <= 1 || n <= 1) {
                for (size_t i = 0; i < n; ++i) rendered[i] = music.render_part(i, sr, total);
            } else {
                std::atomic<size_t> next{0};
                std::vector<std::thread> pool;
                for (unsigned w = 0; w < workers; ++w)
                    pool.emplace_back([&]() { size_t i; while ((i = next.fetch_add(1)) < n) rendered[i] = music.render_part(i, sr, total); });
                for (auto& t : pool) t.join();
            }
            audio::default_interpolation_quality() = prior;
            Mixer mix(sr);
            for (size_t i = 0; i < n; ++i) {
                Source s; const auto& p = music.part(i);
                s.name = p.name.empty() ? "Part " + std::to_string(i) : p.name; s.audio = std::move(rendered[i]);
                mix.add_source(std::move(s));
            }
            return mix;
        }
    }
}
