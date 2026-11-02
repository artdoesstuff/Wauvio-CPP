#pragma once

#include "voice.hpp"
#include "expr.hpp"

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace wauvio {
    namespace vocal {

        // A persona is a named, absolute "look" for the expressive Style fields. A voice stays itself:
        // the identity (VoiceModel: tract proportions, vowel centres, source family) is not touched,
        // and persona values are kept inside a bounded envelope (see tests/identity).
        // Blending lerps the expressive fields only; volume, velocity response, compression and
        // devoicing are never changed by a persona.
        inline const std::vector<float Style::*>& expressive_fields() {
            static const std::vector<float Style::*> f = {
                &Style::brightness, &Style::breathiness, &Style::tension, &Style::softness, &Style::power,
                &Style::resonance, &Style::femininity,
                &Style::vibrato_depth, &Style::vibrato_rate, &Style::vibrato_delay, &Style::vibrato_fade,
                &Style::portamento, &Style::pitch_response, &Style::overshoot, &Style::scoop, &Style::exit_fall,
                &Style::drift, &Style::micro,
                &Style::consonant_strength, &Style::consonant_duration, &Style::vowel_clarity, &Style::legato,
                &Style::attack_sharpness, &Style::release, &Style::dynamic_timbre,
                &Style::formant_shift, &Style::mouth_open, &Style::growl, &Style::tremor, &Style::tremor_rate,
                &Style::fry, &Style::air, &Style::reg, &Style::glottal_onset, &Style::undershoot, &Style::porta_interval,
                &Style::mumble, &Style::grit };
            return f;
        }

        // lerp on the expressive fields. Vibrato fields use -1 as "voice default": if only one side
        // is explicit the explicit value wins, so blending never produces a negative depth by accident.
        inline Style blend_expressive(const Style& a, const Style& b, float t) {
            Style o = a;
            for (float Style::* f : expressive_fields()) {
                const float x = a.*f, y = b.*f;
                const bool sentinel = (f == &Style::vibrato_depth || f == &Style::vibrato_rate ||
                                       f == &Style::vibrato_delay || f == &Style::vibrato_fade);
                if (sentinel && (x < 0.f || y < 0.f)) o.*f = (x < 0.f && y < 0.f) ? -1.f : (t >= 0.5f ? y : x);
                else o.*f = x + (y - x) * t;
            }
            return o;
        }

        struct Persona {
            std::string name;
            Style look;     // absolute values for the expressive fields
        };

        class PersonaLibrary {
        public:
            PersonaLibrary() = default;     // empty; each voice (voices/voice-*.hpp) supplies its own personas

            void add(const std::string& name, const Style& look) { map_[name] = Persona{ name, look }; }
            const Persona* find(const std::string& name) const { auto it = map_.find(name); return it == map_.end() ? nullptr : &it->second; }
            std::vector<std::string> names() const { std::vector<std::string> v; for (auto& kv : map_) v.push_back(kv.first); return v; }

            // Text format (one persona per block):
            //   persona sweet
            //   brightness 0.78      # any Style field name, others stay at the Normal look
            // Returns the number of personas read.
            size_t load(const std::string& path) {
                std::ifstream f(path); if (!f) return 0;
                std::string line; Persona cur; bool open = false; size_t n = 0;
                auto flush = [&]() { if (open) { add(cur.name, cur.look); ++n; open = false; } };
                while (std::getline(f, line)) {
                    auto h = line.find('#'); if (h != std::string::npos) line.resize(h);
                    std::istringstream is(line); std::string k; if (!(is >> k)) continue;
                    if (k == "persona") { flush(); cur = Persona{}; is >> cur.name; cur.look = Style(); open = !cur.name.empty(); continue; }
                    if (!open) continue;
                    float v; if (!(is >> v)) continue;
                    if (float Style::* m = style_field(k)) cur.look.*m = v;
                }
                flush(); return n;
            }
            bool save(const std::string& name, const std::string& path, bool append = false) const {
                const Persona* p = find(name); if (!p) return false;
                std::ofstream f(path, append ? std::ios::app : std::ios::trunc); if (!f) return false;
                const Style N;
                f << "persona " << p->name << "\n";
                for (float Style::* m : expressive_fields()) {
                    if (p->look.*m == N.*m) continue;
                    for (const char* k : { "brightness","breathiness","tension","softness","power","resonance","femininity","vibrato_depth",
                        "vibrato_rate","vibrato_delay","vibrato_fade","portamento","pitch_response","overshoot","scoop","exit_fall","drift","micro",
                        "consonant_strength","consonant_duration","vowel_clarity","legato","attack_sharpness","release","dynamic_timbre",
                        "formant_shift","mouth_open","growl","tremor","tremor_rate","fry","air","reg","glottal_onset","undershoot","porta_interval","mumble","grit" })
                        if (style_field(k) == m) { f << k << " " << p->look.*m << "\n"; break; }
                }
                return true;
            }
        private:
            std::map<std::string, Persona> map_;
        };
        // Moves the expressive fields of `s` toward persona `name` by `amount` (0..1). Unknown names
        // are ignored (returns false).
        inline bool apply_persona(const PersonaLibrary& lib, Style& s, const std::string& name, float amount = 1.f) {
            const Persona* p = lib.find(name); if (!p) return false;
            s = blend_expressive(s, p->look, clamp01(amount)); return true;
        }
        // Blend of two personas (t = 0 -> a, 1 -> b), then moved toward by `amount`.
        inline bool apply_persona_blend(const PersonaLibrary& lib, Style& s, const std::string& a, const std::string& b, float t, float amount = 1.f) {
            const Persona *pa = lib.find(a), *pb = lib.find(b); if (!pa || !pb) return false;
            s = blend_expressive(s, blend_expressive(pa->look, pb->look, clamp01(t)), clamp01(amount)); return true;
        }

        // Lanes that morph a note's current look toward persona `name` over time. `amount` is a
        // curve of 0..1 vs seconds. Only fields that have a render-level channel can move inside a
        // note: brightness, breathiness, tension, volume, growl, fry, air, register, formant shift,
        // mouth opening. Values are converted into the channel's own units.
        inline std::vector<Lane> persona_morph_lanes(const Style& cur, const Persona& p, const Curve& amount, double t_end, double t_begin = 0.0) {
            std::vector<double> ts{ t_begin, t_end };
            for (auto& k : amount) if (k.first > t_begin && k.first < t_end) ts.push_back(k.first);
            for (double t = t_begin + 0.05; t < t_end; t += 0.05) ts.push_back(t);
            std::sort(ts.begin(), ts.end()); ts.erase(std::unique(ts.begin(), ts.end()), ts.end());
            struct M { Ch ch; float Style::* f; float scale; bool relative; };
            static const M map[] = {
                { Ch::Brightness, &Style::brightness, 1.5f, false }, { Ch::Breath, &Style::breathiness, 1.0f, false },
                { Ch::Tension, &Style::tension, 1.0f, false },       { Ch::Volume, &Style::volume, 1.0f, true },
                { Ch::Growl, &Style::growl, 1.0f, false },           { Ch::Fry, &Style::fry, 1.0f, false },
                { Ch::Air, &Style::air, 1.0f, false },               { Ch::Register, &Style::reg, 1.0f, false },
                { Ch::FormantShift, &Style::formant_shift, 1.0f, false }, { Ch::MouthOpen, &Style::mouth_open, 1.0f, false } };
            std::vector<Lane> out;
            for (const M& m : map) {
                const float c = cur.*m.f, tgt = p.look.*m.f;
                if (c == tgt) continue;
                Lane ln; ln.ch = m.ch;
                for (double t : ts) {
                    const float a = clamp01(static_cast<float>(curve_eval(amount, t, 0.0)));
                    float d = m.relative ? (c != 0.f ? (tgt / c - 1.f) : 0.f) : (tgt - c) * m.scale;
                    // growl/fry/air/register/formant/mouth are additive channels whose Style value is already
                    // folded in at note level, so the lane carries only the change.
                    ln.curve.push_back({ t, static_cast<double>(d * a) });
                }
                out.push_back(std::move(ln));
            }
            return out;
        }

    }
}
