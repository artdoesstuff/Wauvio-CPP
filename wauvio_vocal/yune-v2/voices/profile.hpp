#pragma once
// A VoiceProfile is everything that makes one *character* what it is, kept out of the engine:
//   model     - tract proportions, vowel centres, source family, range, vibrato defaults (VoiceModel)
//   style     - the character's default look/behaviour (Style; the "Normal" persona)
//   parse     - lyric parsing defaults
//   personas  - the bounded variations this character can take (soft, sweet, ...)
//   natural_amount - recommended Singer::natural() strength
// Voices live in voices/voice-<id>.hpp (C++, typed) and can be exported/loaded as text (.voice).
#include "../persona.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

namespace wauvio {
    namespace vocal {

        struct VoiceProfile {
            std::string id = "voice";            // short key: "yune"
            std::string display = "Voice";       // human name
            std::string language = "ja";
            std::string notes;                   // one line of character description
            VoiceModel model;
            Style style;                         // the character's default (Normal) look
            ParseOptions parse;
            PersonaLibrary personas;
            float natural_amount = 0.6f;         // default strength for Singer::natural()
        };

        namespace profile_detail {
            inline const std::vector<std::string>& style_names() {
                static const std::vector<std::string> v = {
                    "brightness","breathiness","tension","softness","power","resonance","femininity","vibrato_depth","vibrato_rate",
                    "vibrato_delay","vibrato_fade","portamento","pitch_response","overshoot","scoop","exit_fall","drift","micro",
                    "consonant_strength","consonant_duration","vowel_clarity","legato","attack_sharpness","release","volume",
                    "velocity_response","compression","dynamic_timbre","formant_shift","mouth_open","growl","tremor","tremor_rate",
                    "fry","air","reg","reg_auto","glottal_onset","pitch_loud","pitch_snap","undershoot","porta_interval","exit_fall_min",
                    "bidakuon","n_nasalize","mumble","transition_speed","vv_glide","devoice_max","mora_weighting","glottal_final","grit","natural" };
                return v;
            }
            struct MF { const char* name; float VoiceModel::* p; };
            inline const std::vector<MF>& model_floats() {
                static const std::vector<MF> v = {
                    {"f4",&VoiceModel::f4},{"f5",&VoiceModel::f5},{"f6",&VoiceModel::f6},{"b4",&VoiceModel::b4},{"b5",&VoiceModel::b5},{"b6",&VoiceModel::b6},
                    {"singer_formant_hz",&VoiceModel::singer_formant_hz},{"singer_formant_bw",&VoiceModel::singer_formant_bw},
                    {"rd_base",&VoiceModel::rd_base},{"aspiration",&VoiceModel::aspiration},{"tilt_hz_min",&VoiceModel::tilt_hz_min},{"tilt_hz_max",&VoiceModel::tilt_hz_max},
                    {"vib_depth_st",&VoiceModel::vib_depth_st},{"vib_rate_hz",&VoiceModel::vib_rate_hz},{"vib_delay",&VoiceModel::vib_delay},{"vib_fade",&VoiceModel::vib_fade},
                    {"synthetic",&VoiceModel::synthetic} };
                return v;
            }
            struct TF { const char* name; float Tune::* p; };
            inline const std::vector<TF>& tune_floats() {
                static const std::vector<TF> v = {
                    {"out_scale",&Tune::out_scale},{"asp_gain",&Tune::asp_gain},{"fric_gain",&Tune::fric_gain},{"upper_gain",&Tune::upper_gain},
                    {"sing_gain",&Tune::sing_gain},{"air_gain",&Tune::air_gain},{"pre_gain",&Tune::pre_gain},{"f23_gain",&Tune::f23_gain} };
                return v;
            }
            inline std::string fl(float x) {       // shortest text that reads back as the same float
                char b[40]; for (int d = 0; d <= 9; ++d) { std::snprintf(b, sizeof b, "%.*f", d, static_cast<double>(x)); if (std::strtof(b, nullptr) == x) return b; }
                std::snprintf(b, sizeof b, "%.9g", static_cast<double>(x)); return b;
            }
            inline const char* VOW = "aiueo";
        }

        // ---- text export / import ------------------------------------------------------------------------
        inline std::string profile_to_text(const VoiceProfile& p) {
            using namespace profile_detail;
            std::ostringstream o;
            o << "# Wauvio voice profile. Every value is explicit; edit and load with load_profile().\n";
            o << "voice " << p.id << "\ndisplay " << p.display << "\nlanguage " << p.language << "\nnotes " << p.notes << "\nnatural_amount " << fl(p.natural_amount) << "\n";
            o << "\n# model\nmodel_name " << p.model.name << "\n";
            o << "range " << p.model.range_low << " " << p.model.range_high << " " << p.model.hard_low << " " << p.model.hard_high << "\n";
            for (int v = 0; v < 5; ++v) { const VowelTarget& t = p.model.vowel[v];
                o << "vowel " << VOW[v] << " " << fl(t.f1) << " " << fl(t.f2) << " " << fl(t.f3) << " " << fl(t.b1) << " " << fl(t.b2) << " " << fl(t.b3) << " "
                  << fl(t.gain) << " " << fl(t.f1_track) << " " << fl(t.f2_track) << "\n"; }
            for (const MF& m : model_floats()) o << "model " << m.name << " " << fl(p.model.*(m.p)) << "\n";
            for (const TF& t : tune_floats()) o << "tune " << t.name << " " << fl(p.model.tune.*(t.p)) << "\n";
            o << "\n# default style (the character's Normal look)\n";
            for (const std::string& k : style_names()) o << "style " << k << " " << fl(p.style.*(style_field(k))) << "\n";
            o << "style devoicing " << (p.style.devoicing ? 1 : 0) << "\n";
            o << "\n# lyric parsing\nparse merge_long_vowels " << (p.parse.merge_long_vowels ? 1 : 0) << "\nparse cute " << (p.parse.cute ? 1 : 0) << "\n";
            for (const std::string& n : p.personas.names()) {
                const Persona* ps = p.personas.find(n);
                o << "\npersona " << n << "\n";
                for (float Style::* m : expressive_fields()) for (const std::string& k : style_names()) if (style_field(k) == m) { o << "  " << k << " " << fl(ps->look.*m) << "\n"; break; }
            }
            return o.str();
        }

        inline bool save_profile(const VoiceProfile& p, const std::string& path) {
            std::ofstream f(path); if (!f) return false; f << profile_to_text(p); return static_cast<bool>(f);
        }

        // Parses profile text onto `out` (start from a copy of an existing voice to override a few values).
        // Returns false and sets *err on a malformed line; unknown keys are an error too, so typos are caught.
        inline bool profile_from_text(const std::string& text, VoiceProfile& out, std::string* err = nullptr) {
            using namespace profile_detail;
            std::istringstream in(text); std::string line; int ln = 0; Persona cur; bool open = false;
            PersonaLibrary pl = out.personas;
            auto fail = [&](const std::string& m) { if (err) *err = "line " + std::to_string(ln) + ": " + m; return false; };
            auto flush = [&]() { if (open) { pl.add(cur.name, cur.look); open = false; } };
            bool personas_reset = false;
            while (std::getline(in, line)) {
                ++ln; auto h = line.find('#'); if (h != std::string::npos) line.resize(h);
                std::istringstream is(line); std::string k; if (!(is >> k)) continue;
                if (k == "persona") { flush(); if (!personas_reset) { pl = PersonaLibrary(); personas_reset = true; } cur = Persona{}; is >> cur.name; if (cur.name.empty()) return fail("persona needs a name"); cur.look = out.style; open = true; continue; }
                if (open) { float v; if (!(is >> v)) return fail("bad value for " + k); float Style::* m = style_field(k); if (!m) return fail("unknown style field '" + k + "'"); cur.look.*m = v; continue; }
                auto rest = [&]() { std::string r; std::getline(is, r); size_t a = r.find_first_not_of(" \t"); return a == std::string::npos ? std::string() : r.substr(a); };
                if (k == "voice") is >> out.id; else if (k == "display") out.display = rest(); else if (k == "language") is >> out.language;
                else if (k == "notes") out.notes = rest(); else if (k == "natural_amount") { if (!(is >> out.natural_amount)) return fail("bad natural_amount"); }
                else if (k == "model_name") out.model.name = rest();
                else if (k == "range") { if (!(is >> out.model.range_low >> out.model.range_high >> out.model.hard_low >> out.model.hard_high)) return fail("range needs 4 integers"); }
                else if (k == "vowel") { char c; VowelTarget t; if (!(is >> c >> t.f1 >> t.f2 >> t.f3 >> t.b1 >> t.b2 >> t.b3 >> t.gain >> t.f1_track >> t.f2_track)) return fail("vowel needs a letter and 9 numbers");
                    const char* p = std::strchr(VOW, c); if (!p || !c) return fail("vowel must be one of a i u e o"); out.model.vowel[p - VOW] = t; }
                else if (k == "model") { std::string n; float v; if (!(is >> n >> v)) return fail("model needs name and value"); bool ok = false; for (const MF& m : model_floats()) if (n == m.name) { out.model.*(m.p) = v; ok = true; } if (!ok) return fail("unknown model field '" + n + "'"); }
                else if (k == "tune") { std::string n; float v; if (!(is >> n >> v)) return fail("tune needs name and value"); bool ok = false; for (const TF& t : tune_floats()) if (n == t.name) { out.model.tune.*(t.p) = v; ok = true; } if (!ok) return fail("unknown tune field '" + n + "'"); }
                else if (k == "style") { std::string n; float v; if (!(is >> n >> v)) return fail("style needs name and value"); if (n == "devoicing") out.style.devoicing = v != 0.f; else { float Style::* m = style_field(n); if (!m) return fail("unknown style field '" + n + "'"); out.style.*m = v; } }
                else if (k == "parse") { std::string n; int v; if (!(is >> n >> v)) return fail("parse needs name and 0/1"); if (n == "merge_long_vowels") out.parse.merge_long_vowels = v != 0; else if (n == "cute") out.parse.cute = v != 0; else return fail("unknown parse option '" + n + "'"); }
                else return fail("unknown key '" + k + "'");
            }
            flush(); out.personas = pl; return true;
        }
        inline bool load_profile(const std::string& path, VoiceProfile& out, std::string* err = nullptr) {
            std::ifstream f(path); if (!f) { if (err) *err = "cannot open " + path; return false; }
            std::stringstream ss; ss << f.rdbuf(); return profile_from_text(ss.str(), out, err);
        }

    }
}
