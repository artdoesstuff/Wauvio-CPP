#pragma once
#include "sfx_common.hpp"

namespace wauvio {
    namespace sfx {
        struct ParamSpec {
            std::string name;
            std::string description;
            std::string unit;
            double def = 0.0, min = 0.0, max = 1.0;
            bool   integer = false;
        };

        inline const std::map<std::string, ParamSpec>& param_dictionary() {
            static const std::map<std::string, ParamSpec> d = [] {
                std::map<std::string, ParamSpec> m;
                auto add = [&](const char* n, const char* desc, const char* unit, double def, double lo, double hi, bool integer = false) {
                    m[n] = ParamSpec{n, desc, unit, def, lo, hi, integer}; };
                add("duration",    "Nominal length of the effect (the tail, if any, is appended after it).", "s", 2.0, 0.05, 60.0);
                add("intensity",   "Overall aggressiveness: drive, resonance, modulation strength.", "0..1", 0.6, 0.0, 1.0);
                add("brightness",  "Spectral brightness: filter ceilings and high-frequency content.", "0..1", 0.6, 0.0, 1.0);
                add("pitch",       "Root note of the effect.", "midi", 60.0, 12.0, 108.0);
                add("range",       "Size of the pitch or frequency movement.", "semitones", 24.0, 0.0, 72.0);
                add("density",     "Event or grain density, from sparse to dense.", "0..1", 0.5, 0.0, 1.0);
                add("rate",        "Modulation, retrigger, flutter or attractor-evolution rate.", "Hz", 8.0, 0.1, 1000.0);
                add("depth",       "Modulation or gating depth.", "0..1", 0.7, 0.0, 1.0);
                add("decay",       "Decay length of the resonant or percussive part, from short to long.", "0..1", 0.5, 0.0, 1.0);
                add("tail",        "Seconds of synthetic tail appended after the duration.", "s", 0.0, 0.0, 20.0);
                add("width",       "Stereo width, from mono to fully decorrelated.", "0..1", 0.6, 0.0, 1.0);
                add("subdivision", "Length of one rhythmic step in beats (0.25 = sixteenth note).", "beats", 0.25, 0.0625, 4.0);
                add("scale",       "Scale used for tonal content (see scale_names()).", "index", 1.0, 0.0, 10.0, true);
                add("chord",       "Chord type used for tonal content (see chord_names()).", "index", 0.0, 0.0, 10.0, true);
                add("transient",   "Strength of the attack transient.", "0..1", 0.7, 0.0, 1.0);
                add("body",        "Strength of the main body layer.", "0..1", 0.8, 0.0, 1.0);
                add("sub",         "Strength of the sub-bass layer.", "0..1", 0.6, 0.0, 1.0);
                add("swing",       "Delay applied to every second rhythmic step, as a fraction of a step.", "0..0.5", 0.0, 0.0, 0.5);
                add("bits",        "Bit depth used by digital degradation (lower is rougher).", "bits", 6.0, 1.0, 16.0);
                add("steps",       "Number of steps or notes.", "count", 8.0, 2.0, 32.0, true);
                add("spread",      "Detune, inharmonicity or cluster width.", "0..1", 0.5, 0.0, 1.0);
                return m;
            }();
            return d;
        }

        inline ParamSpec param(const std::string& name, double def) {
            auto it = param_dictionary().find(name);
            if (it == param_dictionary().end()) throw Error("unknown common parameter '" + name + "'");
            ParamSpec s = it->second;
            if (!(def >= s.min && def <= s.max)) throw Error("default of '" + name + "' is outside its range");
            s.def = def; return s;
        }

        class ParamSet {
        public:
            ParamSet() = default;
            explicit ParamSet(std::vector<ParamSpec> specs) : specs_(std::move(specs)) { for (auto& s : specs_) values_.push_back(s.def); }

            bool has(const std::string& name) const { return index(name) >= 0; }
            double get(const std::string& name) const { const int i = index(name); if (i < 0) throw Error("no parameter '" + name + "'"); return values_[static_cast<size_t>(i)]; }
            void set(const std::string& name, double v) {
                const int i = index(name);
                if (i < 0) throw Error("no parameter '" + name + "' (valid: " + names() + ")");
                const ParamSpec& s = specs_[static_cast<size_t>(i)];
                if (!std::isfinite(v)) throw Error("parameter '" + name + "' must be finite");
                if (v < s.min || v > s.max) { std::ostringstream o; o << "parameter '" << name << "' = " << v << " is outside [" << s.min << ", " << s.max << "]"; throw Error(o.str()); }
                if (s.integer && v != std::floor(v)) throw Error("parameter '" + name + "' must be a whole number");
                values_[static_cast<size_t>(i)] = v;
            }
            bool try_set(const std::string& name, double v, std::string* why = nullptr) { try { set(name, v); return true; } catch (const Error& e) { if (why) *why = e.what(); return false; } }
            void reset() { for (size_t i = 0; i < specs_.size(); ++i) values_[i] = specs_[i].def; }
            const std::vector<ParamSpec>& specs() const noexcept { return specs_; }
            std::string names() const { std::string s; for (auto& p : specs_) { if (!s.empty()) s += ", "; s += p.name; } return s; }

        private:
            int index(const std::string& name) const { for (size_t i = 0; i < specs_.size(); ++i) if (specs_[i].name == name) return static_cast<int>(i); return -1; }
            std::vector<ParamSpec> specs_;
            std::vector<double> values_;
        };
    }
}
