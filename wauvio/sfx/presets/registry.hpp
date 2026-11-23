#pragma once
#include "../render/engine.hpp"
#include "../core/sfx_params.hpp"
#include <cctype>

namespace wauvio {
    namespace sfx {
        enum class Category { Risers, Impacts, Glitch, Textures, Bass, EarCandy, Vocal, Rhythmic, Percussion, Experimental, Tonal, Reverses };

        inline const std::vector<Category>& all_categories() {
            static const std::vector<Category> c = { Category::Risers, Category::Impacts, Category::Glitch, Category::Textures, Category::Bass, Category::EarCandy,
                                                     Category::Vocal, Category::Rhythmic, Category::Percussion, Category::Experimental, Category::Tonal, Category::Reverses };
            return c;
        }
        inline const char* category_id(Category c) {
            static const char* n[] = { "risers", "impacts", "glitch", "textures", "bass", "ear_candy", "vocal", "rhythmic", "percussion", "experimental", "tonal", "reverses" };
            return n[static_cast<int>(c)];
        }
        inline const char* category_name(Category c) {
            static const char* n[] = { "Risers & Sweeps", "Impacts & Transitions", "Glitch & Digital", "Textures & Atmospheres", "Bass FX", "Ear Candy & Ornaments",
                                       "Vocal Production FX", "Rhythmic FX", "Synthetic Percussion & Cymbals", "Experimental & Extreme", "Musical Motion & Tonal FX", "Reverses, Tails & Transitions" };
            return n[static_cast<int>(c)];
        }
        inline bool parse_category(std::string_view text, Category& out) {
            std::string t; for (char ch : text) t.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
            for (Category c : all_categories()) { std::string n = category_name(c); for (auto& ch : n) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch))); if (t == category_id(c) || t == n) { out = c; return true; } }
            int hits = 0; Category found = Category::Risers;
            for (Category c : all_categories()) if (!t.empty() && std::string(category_id(c)).rfind(t, 0) == 0) { ++hits; found = c; }
            if (hits == 1) { out = found; return true; }
            return false;
        }

        enum class Behavior { OneShot, Evolving, Sustained };
        inline const char* behavior_name(Behavior b) { return b == Behavior::OneShot ? "one-shot" : b == Behavior::Evolving ? "evolving" : "sustained"; }

        inline std::string make_id(const std::string& name) {
            std::string id; bool us = false;
            for (char ch : name) { if (std::isalnum(static_cast<unsigned char>(ch))) { id.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch)))); us = false; } else if (!us && !id.empty()) { id.push_back('_'); us = true; } }
            while (!id.empty() && id.back() == '_') id.pop_back();
            return id;
        }

        struct BuildCtx {
            const ParamSet& params; double duration; double bpm; uint64_t seed;
            double p(const char* name) const { return params.get(name); }
            double beats(double b) const { return b * 60.0 / bpm; }
            double beat_hz(double beats_per_cycle) const { return bpm / 60.0 / beats_per_cycle; }
            Rng rng(uint64_t tag) const { return Rng(mix_seed(seed, tag)); }
            double lerp01(const char* name, double a, double b) const { return a + (b - a) * p(name); }
            static double lg(double t, double a, double b) { return a * std::pow(b / a, clampd(t, 0.0, 1.0)); }
            std::vector<double> evenly(int n, double a = 0.0, double b = 1.0) const { std::vector<double> t; for (int i = 0; i < n; ++i) t.push_back(n == 1 ? a : a + (b - a) * i / n); return t; }
            std::vector<double> scattered(int n, uint64_t tag, double a = 0.0, double b = 1.0) const { Rng r = rng(tag); std::vector<double> t; for (int i = 0; i < n; ++i) t.push_back(r.range(a, b)); std::sort(t.begin(), t.end()); return t; }
            std::vector<double> chord_notes() const { return chord_intervals(static_cast<int>(p("chord"))); }
            std::vector<double> scale_walk(int n, int from = 0, int stride = 1) const {
                const auto deg = scale_degrees(static_cast<int>(p("scale"))); std::vector<double> v; for (int i = 0; i < n; ++i) v.push_back(scale_step(deg, from + i * stride)); return v;
            }
        };

        using BuildFn = std::function<void(Recipe&, const BuildCtx&)>;

        struct Preset {
            std::string id;
            std::string name;
            std::string description;
            Category category = Category::Risers;
            Behavior behavior = Behavior::OneShot;
            std::vector<std::string> tags;
            std::vector<ParamSpec> params;
            uint64_t default_seed = 1;
            bool reversed = false;
            BuildFn build;

            const ParamSpec* find_param(const std::string& n) const { for (auto& p : params) if (p.name == n) return &p; return nullptr; }
            double default_duration() const { const ParamSpec* d = find_param("duration"); return d ? d->def : 0.0; }
            double default_tail() const { const ParamSpec* d = find_param("tail"); return d ? d->def : 0.0; }
        };

        inline std::vector<ParamSpec> params_from(std::initializer_list<const char*> items) {
            std::vector<ParamSpec> v;
            for (const char* it : items) { std::string s = it; const size_t eq = s.find('=');
                if (eq == std::string::npos) { v.push_back(param(s, param_dictionary().at(s).def)); } else v.push_back(param(s.substr(0, eq), std::stod(s.substr(eq + 1)))); }
            return v;
        }

        class Registry {
        public:
            void add(Preset p) {
                if (p.id.empty()) p.id = make_id(p.name);
                if (p.id.empty()) throw Error("preset needs an id or a name");
                for (char ch : p.id) if (!(std::islower(static_cast<unsigned char>(ch)) || std::isdigit(static_cast<unsigned char>(ch)) || ch == '_')) throw Error("preset id '" + p.id + "' may only use a-z, 0-9 and _");
                if (p.name.empty() || p.description.empty() || p.tags.empty()) throw Error("preset '" + p.id + "' needs a name, a description and tags");
                if (!p.build) throw Error("preset '" + p.id + "' has no build function");
                if (!p.find_param("duration")) throw Error("preset '" + p.id + "' must declare a 'duration' parameter");
                for (auto& s : p.params) if (!(s.def >= s.min && s.def <= s.max)) throw Error("preset '" + p.id + "': default of '" + s.name + "' is out of range");
                if (find(p.id)) throw Error("duplicate preset id '" + p.id + "'");
                presets_.push_back(std::move(p));
            }
            const std::vector<Preset>& all() const noexcept { return presets_; }
            size_t size() const noexcept { return presets_.size(); }
            const Preset* find(std::string_view id) const { for (auto& p : presets_) if (p.id == id) return &p; return nullptr; }
            std::vector<const Preset*> by_category(Category c) const { std::vector<const Preset*> v; for (auto& p : presets_) if (p.category == c) v.push_back(&p); return v; }
            std::vector<const Preset*> search(std::string_view text) const {
                auto low = [](std::string s) { for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch))); return s; };
                const std::string t = low(std::string(text)); std::vector<const Preset*> v;
                for (auto& p : presets_) {
                    bool hit = low(p.id).find(t) != std::string::npos || low(p.name).find(t) != std::string::npos || low(p.description).find(t) != std::string::npos;
                    for (auto& g : p.tags) { if (low(g).find(t) != std::string::npos) hit = true; }
                    if (hit) v.push_back(&p);
                }
                return v;
            }
        private:
            std::vector<Preset> presets_;
        };

        const Registry& builtin_registry();

        class Effect {
        public:
            explicit Effect(const Preset& p) : preset_(p), params_(p.params), seed_(p.default_seed) {}

            Effect& set(const std::string& name, double value) { if (name == "duration") beats_ = 0.0; params_.set(name, value); return *this; }
            double get(const std::string& name) const { return params_.get(name); }
            bool   has(const std::string& name) const { return params_.has(name); }

            Effect& seed(uint64_t s) { seed_ = s; return *this; }
            uint64_t seed() const noexcept { return seed_; }

            Effect& tempo(double bpm) { if (!(bpm >= 20.0 && bpm <= 400.0)) throw Error("tempo must be within 20..400 bpm"); bpm_ = bpm; return *this; }
            double tempo() const noexcept { return bpm_; }

            Effect& duration_beats(double beats) { if (!(beats > 0.0) || !std::isfinite(beats)) throw Error("duration_beats must be positive"); beats_ = beats; return *this; }
            double duration() const { return beats_ > 0.0 ? beats_ * 60.0 / bpm_ : params_.get("duration"); }

            const Preset& preset() const noexcept { return preset_; }
            const ParamSet& params() const noexcept { return params_; }

            Recipe recipe() const {
                const double dur = duration(); const ParamSpec& ds = *preset_.find_param("duration");
                if (dur < ds.min || dur > ds.max) { std::ostringstream o; o << "duration " << dur << " s is outside [" << ds.min << ", " << ds.max << "] for '" << preset_.id << "'"; throw Error(o.str()); }
                Recipe r; r.name = preset_.name; r.duration = dur; r.bpm = bpm_;
                BuildCtx ctx{ params_, dur, bpm_, seed_ };
                preset_.build(r, ctx);
                if (params_.has("tail") && r.tail == 0.0) r.tail = params_.get("tail");
                return r;
            }
            Rendered render(const RenderSettings& st = RenderSettings()) const {
                Rendered r = sfx::render(recipe(), seed_, st);
                if (r.frames() > 0 && r.stats().silent)
                    throw Error("'" + preset_.id + "' rendered as silence with these settings (check pitch and brightness against the sample rate)");
                return r;
            }

        private:
            Preset preset_; ParamSet params_; uint64_t seed_; double bpm_ = 120.0; double beats_ = 0.0;
        };

        inline Effect make(std::string_view id) {
            if (const Preset* p = builtin_registry().find(id)) return Effect(*p);
            std::string msg = "unknown preset id '" + std::string(id) + "'"; auto s = builtin_registry().search(id.substr(0, std::min<size_t>(id.size(), 6)));
            if (!s.empty()) { msg += " (did you mean: "; for (size_t i = 0; i < s.size() && i < 4; ++i) msg += (i ? ", " : "") + s[i]->id; msg += "?)"; }
            throw Error(msg);
        }
        inline Effect make(const Preset& p) { return Effect(p); }
        inline const std::vector<Preset>& all_presets() { return builtin_registry().all(); }
        inline const Preset* find(std::string_view id) { return builtin_registry().find(id); }
        inline std::vector<const Preset*> presets_in(Category c) { return builtin_registry().by_category(c); }
        inline std::vector<const Preset*> search(std::string_view text) { return builtin_registry().search(text); }

        struct Def {
            Registry& reg; Category cat;
            void operator()(const char* name, Behavior b, const char* desc, std::initializer_list<const char*> tags, std::initializer_list<const char*> params, BuildFn fn, bool reversed = false) const {
                Preset p; p.name = name; p.id = make_id(name); p.description = desc; p.category = cat; p.behavior = b; p.tags.assign(tags.begin(), tags.end());
                p.params = params_from(params); p.reversed = reversed; p.build = std::move(fn);
                uint64_t h = 1469598103934665603ull; for (char ch : p.id) { h ^= static_cast<unsigned char>(ch); h *= 1099511628211ull; } p.default_seed = hash64(h);
                reg.add(std::move(p));
            }
        };
    }
}
