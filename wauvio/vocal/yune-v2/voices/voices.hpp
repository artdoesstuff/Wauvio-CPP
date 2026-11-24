#pragma once
// Built-in voices. builtin_voices() lives in voices_registry.hpp, which `voice_tool gen` rebuilds from voices/ (headers)
// and voicebanks/ (.wvc files). To add a voice: drop its .wvc into voicebanks/ and run `voice_tool gen` (it writes
// voices/voice-<name>.hpp if there is none), or write voices/voice-<name>.hpp by hand and run it to register it.
#include "voices_registry.hpp"

namespace wauvio {
    namespace vocal {
        namespace v2 {

            inline std::vector<std::string> voice_ids() { std::vector<std::string> o; for (auto& kv : builtin_voices()) o.push_back(kv.first); return o; }

            // "yune" -> built-in voice; anything else is read as a .wvc file path. Returns false (and *err) if neither works.
            inline bool make_voice(const std::string& id_or_path, VoiceProfile& out, std::string* err = nullptr) {
                for (auto& kv : builtin_voices()) if (kv.first == id_or_path) { out = kv.second(); return true; }
                VoiceProfile p = voice_yune();               // files override a complete base, so a partial file still works
                if (!load_profile(id_or_path, p, err)) return false;
                out = std::move(p); return true;
            }

        }
    }
}
