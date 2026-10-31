#pragma once

#include "../core/core.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace wauvio {
    namespace genre {

        enum class Role { Melodic, Bass, Percussion };

        struct RoleProfile {
            double velocity_scale    = 1.0;
            double velocity_floor    = 0.0;
            double velocity_variance = 0.0;

            std::vector<double> accent_pattern;

            double swing_amount             = 0.0;
            double timing_humanize_sec      = 0.0;
            double quantize_grid_fraction   = 0.0;

            double timing_offset_sec = 0.0;

            double downbeat_deemphasis = 0.0;

            double thin_probability = 0.0;

            int register_shift_semitones = 0;

            bool               force_articulation = false;
            audio::Articulation articulation      = audio::Articulation::Sustain;
            double             fallback_gate_ratio = 1.0;

            double filter_cutoff_mult    = 1.0;
            double detune_cents_delta    = 0.0;
            double stereo_width_override = -1.0;
            double noise_mix_delta       = 0.0;
            double wobble_amount         = 0.0;

            bool  want_reverb = false;
            float reverb_room = 0.5f, reverb_wet = 0.25f, reverb_damping = 0.5f;

            bool   want_chorus = false;
            double chorus_rate_hz = 0.5, chorus_depth_ms = 1.5;
            float  chorus_wet = 0.3f;

            bool   want_delay = false;
            double delay_time_ms = 250.0;
            float  delay_feedback = 0.3f, delay_wet = 0.25f;

            bool  want_distortion = false;
            float distortion_drive = 2.0f;
        };

        class GenrePreset {
        public:
            std::string name;
            std::string description;

            double tempo_scale = 1.0;

            RoleProfile default_profile;

            RoleProfile bass_profile;
            bool        has_bass_profile = false;

            RoleProfile percussion_profile;
            bool        has_percussion_profile = false;

            const RoleProfile& profile_for(Role role) const noexcept {
                if (role == Role::Bass && has_bass_profile) return bass_profile;
                if (role == Role::Percussion && has_percussion_profile) return percussion_profile;
                return default_profile;
            }

            GenrePreset& set_bass(RoleProfile p) { bass_profile = std::move(p); has_bass_profile = true; return *this; }
            GenrePreset& set_percussion(RoleProfile p) { percussion_profile = std::move(p); has_percussion_profile = true; return *this; }
        };

        using GenrePresetPtr = std::shared_ptr<const GenrePreset>;

        namespace detail {

            inline uint64_t splitmix64(uint64_t x) noexcept {
                x += 0x9E3779B97F4A7C15ULL;
                x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
                x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
                return x ^ (x >> 31);
            }

            inline uint64_t note_hash(uint64_t seed, double t, int midi_note, size_t index, uint64_t salt) noexcept {
                uint64_t bits;
                static_assert(sizeof(bits) == sizeof(t), "expected 64-bit double");
                std::memcpy(&bits, &t, sizeof(bits));
                uint64_t h = seed;
                h = splitmix64(h ^ bits);
                h = splitmix64(h ^ (static_cast<uint64_t>(midi_note) * 0x100000001B3ULL));
                h = splitmix64(h ^ (static_cast<uint64_t>(index) * 0xD6E8FEB86659FD93ULL));
                h = splitmix64(h ^ salt);
                return h;
            }

            inline double unit(uint64_t h) noexcept {
                return static_cast<double>(h >> 11) * (1.0 / 9007199254740992.0);
            }

            inline double signed_unit(uint64_t h) noexcept { return unit(h) * 2.0 - 1.0; }

            inline int clampi(int v, int lo, int hi) noexcept { return std::max(lo, std::min(hi, v)); }
            inline double clampd(double v, double lo, double hi) noexcept { return std::max(lo, std::min(hi, v)); }

            inline double accent_multiplier(const std::vector<double>& pattern, int beat_index) {
                if (pattern.empty()) return 1.0;
                double peak = *std::max_element(pattern.begin(), pattern.end());
                if (peak <= 0.0) return 1.0;
                int idx = beat_index % static_cast<int>(pattern.size());
                if (idx < 0) idx += static_cast<int>(pattern.size());
                return pattern[static_cast<size_t>(idx)] / peak;
            }

        }
    }
}