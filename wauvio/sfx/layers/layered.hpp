#pragma once
#include "../presets/registry.hpp"

namespace wauvio {
    namespace sfx {
        /// Per-layer controls of a Layered composite.
        struct LayerOpts {
            double gain_db = 0.0;                 ///< relative level. Effect layers are first rendered at a peak of 0 dBFS; rendered audio is used as it is
            double offset = 0.0;                  ///< start time in seconds
            double offset_beats = -1.0;           ///< if >= 0: start time in beats (overrides offset; uses the Layered tempo)
            double duration = -1.0;               ///< if > 0: sets the layer effect's "duration" parameter (seconds)
            double pitch = 0.0;                   ///< semitones. Presets with a "pitch" parameter are re-pitched through it (clamped to its range);
            ///< others (and rendered audio) are resampled, which also changes their length.
            double pan = 0.0;                     ///< -1 left .. +1 right (equal power)
            bool   has_seed = false; uint64_t seed = 0;   ///< layer-specific seed; otherwise derived from the composite seed and the layer index
            bool   reverse = false;               ///< reverse this layer (offline)
            std::vector<std::pair<std::string, double>> params;   ///< parameter overrides applied to the layer effect
            LayerOpts& gain(double db) { gain_db = db; return *this; }
            LayerOpts& at(double sec) { offset = sec; return *this; }
            LayerOpts& at_beats(double b) { offset_beats = b; return *this; }
            LayerOpts& dur(double sec) { duration = sec; return *this; }
            LayerOpts& transpose(double semis) { pitch = semis; return *this; }
            LayerOpts& place(double p) { pan = p; return *this; }
            LayerOpts& with_seed(uint64_t s) { has_seed = true; seed = s; return *this; }
            LayerOpts& flip() { reverse = true; return *this; }
            LayerOpts& set(const std::string& n, double v) { params.emplace_back(n, v); return *this; }
        };

        namespace detail {
            /// Cubic-interpolated resampling by `ratio` (> 1 = higher pitch and shorter).
            inline Buffer resample(const Buffer& in, double ratio) {
                if (in.size() < 4 || !(ratio > 0.0)) return in;
                const size_t n = static_cast<size_t>(static_cast<double>(in.size()) / ratio); Buffer out(n);
                auto at = [&](long i) { return in[static_cast<size_t>(std::min<long>(std::max<long>(i, 0), static_cast<long>(in.size()) - 1))]; };
                for (size_t k = 0; k < n; ++k) {
                    const double pos = static_cast<double>(k) * ratio; const long i = static_cast<long>(pos); const double f = pos - static_cast<double>(i);
                    const double p0 = at(i - 1), p1 = at(i), p2 = at(i + 1), p3 = at(i + 2);
                    out[k] = static_cast<float>(p1 + 0.5 * f * (p2 - p0 + f * (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3 + f * (3.0 * (p1 - p2) + p3 - p0))));
                }
                return out;
            }
        }

        class Layered {
        public:
            Layered& tempo(double bpm) { if (!(bpm >= 20.0 && bpm <= 400.0)) throw Error("tempo must be within 20..400 bpm"); bpm_ = bpm; return *this; }
            Layered& seed(uint64_t s) { seed_ = s; return *this; }
            size_t size() const noexcept { return layers_.size(); }

            /// Add a built-in preset by id.
            Layered& add(std::string_view preset_id, const LayerOpts& o = LayerOpts()) { return add(make(preset_id), o); }
            /// Add a configured Effect (its own parameters, seed and tempo-independent settings are kept; per-layer options are applied on top).
            Layered& add(const Effect& e, const LayerOpts& o = LayerOpts()) { check(o); Item it; it.effect = std::make_shared<Effect>(e); it.opts = o; layers_.push_back(std::move(it)); return *this; }
            /// Add audio you already rendered (it must have the sample rate you will render the composite at).
            Layered& add(const Rendered& audio, const LayerOpts& o = LayerOpts()) { check(o); Item it; it.audio = std::make_shared<Rendered>(audio); it.opts = o; layers_.push_back(std::move(it)); return *this; }

            /// Render and mix all layers, then normalise the composite according to `st`. Each layer's randomness comes from its own seed:
            /// the explicit LayerOpts seed if given, otherwise a seed derived from the composite seed and the layer's position.
            Rendered render(const RenderSettings& st = RenderSettings()) const {
                validate_settings(st); if (layers_.empty()) throw Error("Layered has no layers");
                std::vector<Rendered> parts; std::vector<size_t> offs; size_t total = 0; const double sr = st.sample_rate;
                RenderSettings child = st; child.normalize = true; child.peak_db = 0.0;
                for (size_t i = 0; i < layers_.size(); ++i) {
                    const Item& it = layers_[i]; Rendered r;
                    if (it.effect) {
                        Effect e = *it.effect; e.tempo(bpm_);
                        e.seed(it.opts.has_seed ? it.opts.seed : mix_seed(seed_, i + 1));
                        if (it.opts.duration > 0.0) e.set("duration", it.opts.duration);
                        for (auto& kv : it.opts.params) e.set(kv.first, kv.second);
                        double resample_semis = it.opts.pitch;
                        if (it.opts.pitch != 0.0 && e.has("pitch")) {
                            const ParamSpec& ps = *e.preset().find_param("pitch");
                            e.set("pitch", clampd(e.get("pitch") + it.opts.pitch, ps.min, ps.max)); resample_semis = 0.0;
                        }
                        r = e.render(child);
                        if (resample_semis != 0.0) { const double ratio = semis_to_ratio(resample_semis); r.left = detail::resample(r.left, ratio); r.right = detail::resample(r.right, ratio); }
                    } else {
                        r = *it.audio;
                        if (r.sample_rate != st.sample_rate) throw Error("layer audio has a different sample rate than the render settings");
                        if (it.opts.pitch != 0.0) { const double ratio = semis_to_ratio(it.opts.pitch); r.left = detail::resample(r.left, ratio); r.right = detail::resample(r.right, ratio); }
                    }
                    if (it.opts.reverse) { std::reverse(r.left.begin(), r.left.end()); std::reverse(r.right.begin(), r.right.end()); }
                    const double th = (clampd(it.opts.pan, -1.0, 1.0) + 1.0) * PI / 4.0, g = db_to_lin(it.opts.gain_db) * std::sqrt(2.0);
                    const float gl = static_cast<float>(g * std::cos(th)), gr = static_cast<float>(g * std::sin(th));
                    for (auto& v : r.left) { v *= gl; }
                    for (auto& v : r.right) { v *= gr; }
                    const double off_s = it.opts.offset_beats >= 0.0 ? it.opts.offset_beats * 60.0 / bpm_ : it.opts.offset;
                    const size_t off = static_cast<size_t>(std::llround(off_s * sr));
                    total = std::max(total, off + r.frames());
                    if (static_cast<double>(total) / sr > st.max_seconds) throw Error("composite exceeds max_seconds");
                    offs.push_back(off); parts.push_back(std::move(r));
                }
                Rendered out; out.sample_rate = st.sample_rate; out.channels = st.channels; out.left.assign(total, 0.0f); out.right.assign(total, 0.0f);
                for (size_t i = 0; i < parts.size(); ++i) for (size_t k = 0; k < parts[i].frames(); ++k) { out.left[offs[i] + k] += parts[i].left[k]; out.right[offs[i] + k] += parts[i].right[k]; }
                if (st.channels == 1) for (size_t i = 0; i < total; ++i) { const float m = 0.5f * (out.left[i] + out.right[i]); out.left[i] = out.right[i] = m; }
                if (st.normalize) { float pk = 0; for (size_t i = 0; i < total; ++i) pk = std::max({ pk, std::fabs(out.left[i]), std::fabs(out.right[i]) });
                    if (pk > 1e-9f) { const float g = static_cast<float>(db_to_lin(st.peak_db)) / pk; for (size_t i = 0; i < total; ++i) { out.left[i] *= g; out.right[i] *= g; } } }
                return out;
            }

        private:
            struct Item { std::shared_ptr<Effect> effect; std::shared_ptr<Rendered> audio; LayerOpts opts; };
            static void check(const LayerOpts& o) {
                for (double v : { o.gain_db, o.offset, o.offset_beats, o.duration, o.pitch, o.pan }) if (!std::isfinite(v)) throw Error("LayerOpts contains a non-finite value");
                if (o.offset < 0.0) throw Error("LayerOpts offset must be >= 0");
            }
            std::vector<Item> layers_; double bpm_ = 120.0; uint64_t seed_ = 1;
        };
    }
}
