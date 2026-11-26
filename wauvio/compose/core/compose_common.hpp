#pragma once

#include "../../../wauvio.hpp"
#include "../../ext.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace wauvio {
    namespace compose {

        class ComposeError : public std::runtime_error {
        public:
            explicit ComposeError(const std::string& what) : std::runtime_error(what) {}
        };

        inline uint64_t hash64(uint64_t x) noexcept {
            x += 0x9E3779B97F4A7C15ull;
            x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
            x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
            return x ^ (x >> 31);
        }

        class Rng {
        public:
            explicit Rng(uint64_t seed = 1) noexcept { reseed(seed); }

            void reseed(uint64_t seed) noexcept {
                uint64_t z = seed ? seed : 1;
                for (auto& s : s_) { z += 0x9E3779B97F4A7C15ull; s = hash64(z); }
            }

            uint64_t next() noexcept {
                const uint64_t r = rotl(s_[1] * 5, 7) * 9, t = s_[1] << 17;
                s_[2] ^= s_[0]; s_[3] ^= s_[1]; s_[1] ^= s_[2]; s_[0] ^= s_[3];
                s_[2] ^= t; s_[3] = rotl(s_[3], 45);
                return r;
            }

            double uniform() noexcept { return static_cast<double>(next() >> 11) * (1.0 / 9007199254740992.0); }
            double bipolar() noexcept { return uniform() * 2.0 - 1.0; }
            double range(double a, double b) noexcept { return a + (b - a) * uniform(); }
            int    range_int(int lo, int hi) noexcept {
                if (hi <= lo) return lo;
                return lo + static_cast<int>(uniform() * static_cast<double>(hi - lo + 1)) % (hi - lo + 1);
            }
            bool   chance(double p) noexcept { return uniform() < p; }

            template <typename T>
            const T& pick(const std::vector<T>& v) {
                if (v.empty()) throw ComposeError("Rng::pick on empty vector");
                return v[static_cast<size_t>(range_int(0, static_cast<int>(v.size()) - 1))];
            }

            size_t weighted_index(const std::vector<double>& weights) noexcept {
                double total = 0.0;
                for (double w : weights) if (w > 0.0) total += w;
                if (total <= 0.0) return 0;
                double r = uniform() * total;
                for (size_t i = 0; i < weights.size(); ++i) {
                    if (weights[i] <= 0.0) continue;
                    r -= weights[i];
                    if (r <= 0.0) return i;
                }
                return weights.size() - 1;
            }

        private:
            static uint64_t rotl(uint64_t x, int k) noexcept { return (x << k) | (x >> (64 - k)); }
            uint64_t s_[4]{};
        };

        enum class PitchClass : int {
            C = 0, Cs = 1, Db = 1, D = 2, Ds = 3, Eb = 3, E = 4, F = 5,
            Fs = 6, Gb = 6, G = 7, Gs = 8, Ab = 8, A = 9, As = 10, Bb = 10, B = 11
        };

        inline int wrap_pc(int pc) noexcept {
            int m = pc % 12;
            return m < 0 ? m + 12 : m;
        }

        inline const char* pitch_class_name(int pc, bool prefer_flats = false) noexcept {
            static const char* sharps[12] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
            static const char* flats[12]  = { "C","Db","D","Eb","E","F","Gb","G","Ab","A","Bb","B" };
            int idx = wrap_pc(pc);
            return prefer_flats ? flats[idx] : sharps[idx];
        }

        inline bool parse_pitch_class(const std::string& text, PitchClass& out) {
            if (text.empty()) return false;
            char base = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
            int pc = -1;
            switch (base) {
                case 'C': pc = 0; break; case 'D': pc = 2; break; case 'E': pc = 4; break;
                case 'F': pc = 5; break; case 'G': pc = 7; break; case 'A': pc = 9; break;
                case 'B': pc = 11; break; default: return false;
            }
            for (size_t i = 1; i < text.size(); ++i) {
                if (text[i] == '#') pc += 1;
                else if (text[i] == 'b') pc -= 1;
                else return false;
            }
            out = static_cast<PitchClass>(wrap_pc(pc));
            return true;
        }

        enum class ScaleType {
            Major,
            NaturalMinor,
            HarmonicMinor,
            MelodicMinor,
            Dorian,
            Phrygian,
            Lydian,
            Mixolydian,
            PentatonicMajor,
            PentatonicMinor,
            Blues,
            WholeTone
        };

        inline const std::vector<int>& scale_intervals(ScaleType st) {
            static const std::vector<int> maj  { 0, 2, 4, 5, 7, 9, 11 };
            static const std::vector<int> min  { 0, 2, 3, 5, 7, 8, 10 };
            static const std::vector<int> hmin { 0, 2, 3, 5, 7, 8, 11 };
            static const std::vector<int> mmin { 0, 2, 3, 5, 7, 9, 11 };
            static const std::vector<int> dor  { 0, 2, 3, 5, 7, 9, 10 };
            static const std::vector<int> phr  { 0, 1, 3, 5, 7, 8, 10 };
            static const std::vector<int> lyd  { 0, 2, 4, 6, 7, 9, 11 };
            static const std::vector<int> mix  { 0, 2, 4, 5, 7, 9, 10 };
            static const std::vector<int> pmaj { 0, 2, 4, 7, 9 };
            static const std::vector<int> pmin { 0, 3, 5, 7, 10 };
            static const std::vector<int> blu  { 0, 3, 5, 6, 7, 10 };
            static const std::vector<int> wt   { 0, 2, 4, 6, 8, 10 };
            switch (st) {
                case ScaleType::Major:           return maj;
                case ScaleType::NaturalMinor:    return min;
                case ScaleType::HarmonicMinor:   return hmin;
                case ScaleType::MelodicMinor:    return mmin;
                case ScaleType::Dorian:          return dor;
                case ScaleType::Phrygian:        return phr;
                case ScaleType::Lydian:          return lyd;
                case ScaleType::Mixolydian:      return mix;
                case ScaleType::PentatonicMajor: return pmaj;
                case ScaleType::PentatonicMinor: return pmin;
                case ScaleType::Blues:           return blu;
                case ScaleType::WholeTone:       return wt;
            }
            return maj;
        }

        inline const char* scale_type_name(ScaleType st) noexcept {
            switch (st) {
                case ScaleType::Major:           return "major";
                case ScaleType::NaturalMinor:    return "minor";
                case ScaleType::HarmonicMinor:   return "harmonic_minor";
                case ScaleType::MelodicMinor:    return "melodic_minor";
                case ScaleType::Dorian:          return "dorian";
                case ScaleType::Phrygian:        return "phrygian";
                case ScaleType::Lydian:          return "lydian";
                case ScaleType::Mixolydian:      return "mixolydian";
                case ScaleType::PentatonicMajor: return "pentatonic_major";
                case ScaleType::PentatonicMinor: return "pentatonic_minor";
                case ScaleType::Blues:           return "blues";
                case ScaleType::WholeTone:       return "whole_tone";
            }
            return "major";
        }

        inline bool parse_scale_type(const std::string& raw, ScaleType& out) {
            std::string s;
            for (char c : raw)
                if (std::isalnum(static_cast<unsigned char>(c)))
                    s.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            if (s == "major" || s == "maj" || s == "ionian")              { out = ScaleType::Major; return true; }
            if (s == "minor" || s == "min" || s == "naturalminor" || s == "aeolian") { out = ScaleType::NaturalMinor; return true; }
            if (s == "harmonicminor" || s == "hminor")                    { out = ScaleType::HarmonicMinor; return true; }
            if (s == "melodicminor"  || s == "mminor")                    { out = ScaleType::MelodicMinor; return true; }
            if (s == "dorian")                                            { out = ScaleType::Dorian; return true; }
            if (s == "phrygian")                                          { out = ScaleType::Phrygian; return true; }
            if (s == "lydian")                                            { out = ScaleType::Lydian; return true; }
            if (s == "mixolydian" || s == "mixo")                         { out = ScaleType::Mixolydian; return true; }
            if (s == "pentatonicmajor" || s == "majorpentatonic" || s == "pentmaj") { out = ScaleType::PentatonicMajor; return true; }
            if (s == "pentatonicminor" || s == "minorpentatonic" || s == "pentmin") { out = ScaleType::PentatonicMinor; return true; }
            if (s == "blues")                                             { out = ScaleType::Blues; return true; }
            if (s == "wholetone")                                         { out = ScaleType::WholeTone; return true; }
            return false;
        }

        inline bool is_minor_scale(ScaleType st) noexcept {
            return st == ScaleType::NaturalMinor || st == ScaleType::HarmonicMinor ||
                   st == ScaleType::MelodicMinor || st == ScaleType::Dorian ||
                   st == ScaleType::Phrygian || st == ScaleType::PentatonicMinor ||
                   st == ScaleType::Blues;
        }

        struct Key {
            PitchClass root  = PitchClass::C;
            ScaleType  scale = ScaleType::Major;

            Key() = default;
            Key(PitchClass r, ScaleType s = ScaleType::Major) : root(r), scale(s) {}

            int root_pc() const noexcept { return wrap_pc(static_cast<int>(root)); }

            int degree_midi(int degree, int octave = 4, int chromatic = 0) const {
                const auto& iv = scale_intervals(scale);
                const int N = static_cast<int>(iv.size());
                const int oct_shift = (degree >= 0) ? (degree / N) : -((-degree + N - 1) / N);
                const int idx = degree - oct_shift * N;
                const int midi = (octave + 1 + oct_shift) * 12 + root_pc() + iv[static_cast<size_t>(idx)] + chromatic;
                return std::clamp(midi, 0, 127);
            }

            int degree_pc(int degree, int chromatic = 0) const {
                const auto& iv = scale_intervals(scale);
                const int N = static_cast<int>(iv.size());
                int idx = degree % N;
                if (idx < 0) idx += N;
                return wrap_pc(root_pc() + iv[static_cast<size_t>(idx)] + chromatic);
            }

            bool is_in_scale(int midi_note) const {
                const int rel = wrap_pc(midi_note - root_pc());
                const auto& iv = scale_intervals(scale);
                return std::find(iv.begin(), iv.end(), rel) != iv.end();
            }

            int snap_to_scale(int midi_note) const {
                if (is_in_scale(midi_note)) return std::clamp(midi_note, 0, 127);
                for (int d = 1; d <= 6; ++d) {
                    if (midi_note + d <= 127 && is_in_scale(midi_note + d)) return midi_note + d;
                    if (midi_note - d >= 0   && is_in_scale(midi_note - d)) return midi_note - d;
                }
                return std::clamp(midi_note, 0, 127);
            }

            int midi_to_step(int midi_note, int base_octave = 4) const {
                int best_step = 0, best_dist = 999;
                for (int s = -28; s <= 28; ++s) {
                    int m = degree_midi(s, base_octave);
                    int d = std::abs(m - midi_note);
                    if (d < best_dist) { best_dist = d; best_step = s; }
                }
                return best_step;
            }

            std::string name() const {
                return std::string(pitch_class_name(root_pc())) + " " + scale_type_name(scale);
            }
        };

        struct Meter {
            int beats_per_bar = 4;
            int beat_unit     = 4;
            double bar_beats() const noexcept { return static_cast<double>(beats_per_bar) * (4.0 / static_cast<double>(beat_unit)); }
        };

        inline double beats_to_seconds(double beats, double bpm) noexcept {
            return (bpm > 0.0) ? (beats * 60.0 / bpm) : 0.0;
        }

    }
}
