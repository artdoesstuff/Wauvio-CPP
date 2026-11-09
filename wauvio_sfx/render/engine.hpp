#pragma once
#include "processors.hpp"

namespace wauvio {
    namespace sfx {
        struct RenderSettings {
            int    sample_rate = 44100;      ///< 8000 .. 192000
            int    channels    = 2;          ///< 1 (mono) or 2 (stereo)
            bool   normalize   = true;       ///< scale the finished effect so its peak equals peak_db
            double peak_db     = -1.0;       ///< target peak when normalising (<= 0 dBFS)
            double max_seconds = 120.0;      ///< refuse to render anything longer (memory guard)
            double dc_cutoff_hz = 12.0;      ///< DC blocker (0 = off)
        };

        struct Stats {
            double seconds = 0.0, peak = 0.0, rms = 0.0, dc = 0.0, clip_ratio = 0.0, crest = 0.0;
            bool silent = true;
            double peak_db() const { return lin_to_db(peak); }
            double rms_db() const { return lin_to_db(rms); }
        };

        struct Rendered {
            Buffer left, right;
            int sample_rate = 44100;
            int channels = 2;
            size_t frames() const noexcept { return left.size(); }
            double seconds() const noexcept { return sample_rate > 0 ? static_cast<double>(left.size()) / sample_rate : 0.0; }

            Stats stats() const {
                Stats s; s.seconds = seconds(); const size_t n = left.size(); if (!n) return s;
                double sq = 0, sum = 0; size_t clip = 0;
                for (size_t i = 0; i < n; ++i) { const double a = left[i], b = right[i];
                    s.peak = std::max({ s.peak, std::fabs(a), std::fabs(b) }); sq += a * a + b * b; sum += a + b; if (std::fabs(a) >= 0.999 || std::fabs(b) >= 0.999) ++clip; }
                s.rms = std::sqrt(sq / (2.0 * static_cast<double>(n))); s.dc = sum / (2.0 * static_cast<double>(n)); s.clip_ratio = static_cast<double>(clip) / static_cast<double>(n);
                s.crest = s.rms > 1e-12 ? s.peak / s.rms : 0.0; s.silent = s.peak < 1e-6; return s;
            }
            StereoBuffer to_stereo() const { StereoBuffer b; b.L = left; b.R = right; return b; }
            Buffer to_mono() const { Buffer m(left.size()); for (size_t i = 0; i < m.size(); ++i) m[i] = 0.5f * (left[i] + right[i]); return m; }
            void save(const std::string& path) const { if (channels == 1) save_wav(to_mono(), path, sample_rate); else save_wav_stereo(to_stereo(), path, sample_rate); }
        };

        inline void validate_settings(const RenderSettings& st) {
            if (st.sample_rate < 8000 || st.sample_rate > 192000) throw Error("sample_rate must be within 8000..192000");
            if (st.channels != 1 && st.channels != 2) throw Error("channels must be 1 or 2");
            if (!std::isfinite(st.peak_db) || st.peak_db > 0.0 || st.peak_db < -60.0) throw Error("peak_db must be within -60..0");
            if (!std::isfinite(st.max_seconds) || st.max_seconds <= 0.0) throw Error("max_seconds must be positive");
            if (!std::isfinite(st.dc_cutoff_hz) || st.dc_cutoff_hz < 0.0) throw Error("dc_cutoff_hz must be >= 0");
        }

        inline void validate_recipe(const Recipe& r) {
            if (!std::isfinite(r.duration) || r.duration < 0.0) throw Error("recipe duration must be finite and >= 0");
            if (!std::isfinite(r.tail) || r.tail < 0.0) throw Error("recipe tail must be finite and >= 0");
            if (!std::isfinite(r.bpm) || r.bpm < 20.0 || r.bpm > 400.0) throw Error("recipe bpm must be within 20..400");
            if (r.layers.empty()) throw Error("recipe has no layers");
            if (r.layers.size() > 64) throw Error("recipe has more than 64 layers");
            for (const auto& l : r.layers) {
                if (l.src.kind == SourceKind::None) throw Error("layer '" + l.name + "' has no source");
                if (l.chain.size() > 32) throw Error("layer '" + l.name + "' has more than 32 processors");
                for (double v : { l.level, l.start, l.length, l.tail, l.pan, l.width, l.fade_in_ms, l.fade_out_ms })
                    if (!std::isfinite(v)) throw Error("layer '" + l.name + "' has a non-finite setting");
                if (l.start < 0.0 || l.tail < 0.0) throw Error("layer '" + l.name + "': start and tail must be >= 0");
            }
        }

        namespace detail {
            struct LayerCtx { double sr; double bpm; double duration; };

            inline Buffer render_layer(const LayerSpec& L, const LayerCtx& c, uint64_t seed) {
                const double len = L.length < 0.0 ? std::max(0.0, c.duration - L.start) : L.length;
                const size_t Nlen = static_cast<size_t>(std::llround(len * c.sr)), N = static_cast<size_t>(std::llround((len + L.tail) * c.sr));
                if (N == 0) return Buffer();
                Buffer b = generate_source(L.src, N, Nlen, c.sr, mix_seed(seed, 0x51));
                auto peak_of = [](const Buffer& x) { float p = 0; for (float v : x) p = std::max(p, std::fabs(v)); return p; };
                float pk = peak_of(b); if (pk > 1e-9f) for (auto& v : b) v *= 0.9f / pk;                // standard level into env / chain
                for (size_t i = 0; i < N; ++i) b[i] = static_cast<float>(b[i] * std::max(0.0, L.env.at(unit_pos(i, Nlen))));
                uint64_t pseed = mix_seed(seed, 0x7001);
                for (const Proc& p : L.chain) { run_proc(b, p, ProcCtx{ c.sr, c.bpm, pseed, Nlen }); pseed = hash64(pseed); }
                pk = peak_of(b); if (pk > 1e-9f) for (auto& v : b) v *= 1.0f / pk;                      // layer peak = 1 before post/level
                for (size_t i = 0; i < N; ++i) b[i] = static_cast<float>(b[i] * std::max(0.0, L.post.at(unit_pos(i, Nlen))));
                // clean edges
                const size_t fi = std::min(N / 2, static_cast<size_t>(L.fade_in_ms * 0.001 * c.sr)), fo = std::min(N / 2, static_cast<size_t>(L.fade_out_ms * 0.001 * c.sr));
                for (size_t i = 0; i < fi; ++i) b[i] *= static_cast<float>(static_cast<double>(i) / static_cast<double>(fi));
                for (size_t i = 0; i < fo; ++i) b[N - 1 - i] *= static_cast<float>(static_cast<double>(i) / static_cast<double>(fo));
                for (auto& v : b) if (!std::isfinite(v)) v = 0.0f;
                return b;
            }
        }

        /// Render a recipe. `seed` makes every random decision reproducible; the same recipe, seed and
        /// settings give the same audio (see the README for the cross-platform caveat).
        inline Rendered render(const Recipe& r, uint64_t seed = 1, const RenderSettings& st = RenderSettings()) {
            validate_settings(st); validate_recipe(r);
            const double total_s = r.total_seconds();
            if (total_s > st.max_seconds) { std::ostringstream o; o << "effect would be " << total_s << " s, which exceeds max_seconds (" << st.max_seconds << ")"; throw Error(o.str()); }
            const double sr = st.sample_rate; const size_t N = static_cast<size_t>(std::llround(total_s * sr));
            Rendered out; out.sample_rate = st.sample_rate; out.channels = st.channels; out.left.assign(N, 0.0f); out.right.assign(N, 0.0f);
            if (N == 0) return out;

            detail::LayerCtx lc{ sr, r.bpm, r.duration };
            for (size_t li = 0; li < r.layers.size(); ++li) {
                const LayerSpec& L = r.layers[li];
                const uint64_t ls = mix_seed(mix_seed(seed, li + 1), L.seed_offset);
                const Buffer m = detail::render_layer(L, lc, ls);
                if (m.empty()) continue;
                const double w = clampd(L.width, 0.0, 1.0);
                Buffer m2; if (w > 0.0) m2 = detail::render_layer(L, lc, mix_seed(ls, 0xB0B0));     // independently rendered second channel
                const double th = (clampd(L.pan, -1.0, 1.0) + 1.0) * PI / 4.0, gl = std::cos(th) * std::sqrt(2.0) * L.level, gr = std::sin(th) * std::sqrt(2.0) * L.level;
                const size_t off = static_cast<size_t>(std::llround(L.start * sr));
                for (size_t i = 0; i < m.size() && off + i < N; ++i) {
                    const double a = m[i], b = (w > 0.0 && i < m2.size()) ? (1.0 - w) * m[i] + w * m2[i] : m[i];
                    out.left[off + i] += static_cast<float>(a * gl); out.right[off + i] += static_cast<float>(b * gr);
                }
            }
            if (r.reverse_all) { std::reverse(out.left.begin(), out.left.end()); std::reverse(out.right.begin(), out.right.end()); }

            if (st.dc_cutoff_hz > 0.0) {
                const double a = std::exp(-TWO_PI * st.dc_cutoff_hz / sr);
                for (Buffer* ch : { &out.left, &out.right }) { double x1 = 0, y1 = 0; for (auto& v : *ch) { const double x = v, y = x - x1 + a * y1; x1 = x; y1 = y; v = static_cast<float>(y); } }
            }
            const size_t fi = std::min(N / 2, static_cast<size_t>(r.fade_in_ms * 0.001 * sr)), fo = std::min(N / 2, static_cast<size_t>(r.fade_out_ms * 0.001 * sr));
            for (Buffer* ch : { &out.left, &out.right }) {
                for (size_t i = 0; i < fi; ++i) (*ch)[i] *= static_cast<float>(static_cast<double>(i) / static_cast<double>(fi));
                for (size_t i = 0; i < fo; ++i) (*ch)[N - 1 - i] *= static_cast<float>(static_cast<double>(i) / static_cast<double>(fo));
            }
            if (st.channels == 1) for (size_t i = 0; i < N; ++i) { const float m = 0.5f * (out.left[i] + out.right[i]); out.left[i] = out.right[i] = m; }
            if (st.normalize) {
                float pk = 0; for (size_t i = 0; i < N; ++i) pk = std::max({ pk, std::fabs(out.left[i]), std::fabs(out.right[i]) });
                if (pk > 1e-9f) { const float g = static_cast<float>(db_to_lin(st.peak_db)) / pk; for (size_t i = 0; i < N; ++i) { out.left[i] *= g; out.right[i] *= g; } }
            }
            return out;
        }
    }
}
