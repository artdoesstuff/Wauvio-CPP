#pragma once
// Registry of the built-in voices. To add a voice: write voices/voice-<id>.hpp (copy voice-template.md's checklist),
// include it here and add one line to builtin_voices().
#include "voice-yune.hpp"
#include "voice-yune_old.hpp"
#include "voice-reiha.hpp"
#include "voice-koharu.hpp"
#include "voice-itsuki.hpp"

namespace wauvio {
    namespace vocal {

        inline const std::vector<std::pair<std::string, std::function<VoiceProfile()>>>& builtin_voices() {
            static const std::vector<std::pair<std::string, std::function<VoiceProfile()>>> v = {
                { "yune", [] { return voice_yune(); } },
                { "yune-old", [] { return voice_yune_old(); } },
                { "reiha", [] { return voice_reiha(); } },
                { "koharu", [] { return voice_koharu(); } },
                { "itsuki", [] { return voice_itsuki(); } },
            };
            return v;
        }
        inline std::vector<std::string> voice_ids() { std::vector<std::string> o; for (auto& kv : builtin_voices()) o.push_back(kv.first); return o; }

        // "yune" -> built-in voice; anything else is read as a .voice file path. Returns false (and *err) if neither works.
        inline bool make_voice(const std::string& id_or_path, VoiceProfile& out, std::string* err = nullptr) {
            for (auto& kv : builtin_voices()) if (kv.first == id_or_path) { out = kv.second(); return true; }
            VoiceProfile p = voice_yune();               // files override a complete base, so a partial file still works
            if (!load_profile(id_or_path, p, err)) return false;
            out = std::move(p); return true;
        }

    }
}
