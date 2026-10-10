#pragma once

#include "japanese.hpp"
#include "../wauvio.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace wauvio {
    namespace vocal {
        struct VowelTarget {
            float f1, f2, f3;       // Hz
            float b1, b2, b3;       // bandwidths, Hz
            float gain;             // per-vowel loudness trim (measured, see tests)
            float f1_track;         // 0..1: how strongly F1 follows F0 when F0 approaches F1
            float f2_track;         // 0..1: same for F2 (only matters for very high notes)
        };

        // All static data describing the singer. Only one voice ships; the struct exists
        // so the numbers live in one place, not to host a family of voices.
        struct VoiceModel {
            const char* name = "Wauvio Japanese Female (default)";

            // Range (MIDI notes)
            int range_low = 52, range_high = 88;       // E3 .. E6 comfortable
            int hard_low = 40,  hard_high = 96;

            // Japanese vowels, female, deliberately peripheral for clarity.
            VowelTarget vowel[5] = {
                /* a */ { 880, 1480, 3000, 135, 130, 190, 1.00f, 0.85f, 0.0f },
                /* i */ { 330, 2850, 3500,  95, 150, 200, 1.00f, 0.45f, 0.0f },
                /* u */ { 370, 1450, 2750, 100, 125, 180, 1.00f, 0.45f, 0.0f },
                /* e */ { 560, 2450, 3200, 110, 135, 190, 1.00f, 0.75f, 0.0f },
                /* o */ { 570,  980, 2800, 110, 120, 180, 1.00f, 0.75f, 0.0f },
            };
            // Fixed upper formants / brightness structure
            float f4 = 4300.f, f5 = 5600.f, f6 = 7200.f;
            float b4 = 260.f,  b5 = 320.f,  b6 = 500.f;
            float singer_formant_hz = 3100.f, singer_formant_bw = 600.f;

            // Glottal source defaults (Rd parameter of the LF model; lower = tenser/brighter)
            float rd_base      = 0.95f;
            float aspiration   = 0.020f;    // steady aspiration level at breathiness 0
            float tilt_hz_min  = 2600.f;    // source low-pass at brightness 0
            float tilt_hz_max  = 11000.f;   // ... at brightness 1

            // Vibrato defaults
            float vib_depth_st = 0.32f;     // semitones, peak
            float vib_rate_hz  = 5.7f;
            float vib_delay    = 0.30f;     // s before vibrato begins
            float vib_fade     = 0.35f;     // s to reach full depth

            // Timbre of the synthetic character
            float synthetic    = 0.65f;     // 0..1: tighter bandwidths, steadier source, brighter uppers
        };

        inline const VoiceModel& default_voice() { static const VoiceModel v; return v; }

        // A control curve (time, value). Time units depend on use (see Singer docs).
        using Curve = std::vector<std::pair<double, double>>;

        inline double curve_eval(const Curve& c, double t, double outside = 0.0) {
            if (c.empty()) return outside;
            if (t <= c.front().first) return c.front().second;
            if (t >= c.back().first)  return c.back().second;
            size_t lo = 0, hi = c.size() - 1;
            while (hi - lo > 1) { size_t mid = (lo + hi) / 2; (c[mid].first <= t ? lo : hi) = mid; }
            const double t0 = c[lo].first, t1 = c[hi].first;
            const double f = t1 > t0 ? (t - t0) / (t1 - t0) : 0.0;
            return c[lo].second + (c[hi].second - c[lo].second) * f;
        }

        // Musical controls. All 0..1 unless a unit is given. Every field can be set as a
        // singer-wide default and overridden per note.
        struct Style {
            // voice
            float brightness  = 0.70f;   // high-frequency presence (source tilt + upper formants)
            float breathiness = 0.12f;   // aspiration + more open glottis
            float tension     = 0.50f;   // glottal tenseness (low Rd -> sharper, brighter)
            float softness    = 0.00f;   // gentle, rounded onset and breathier tone
            float power       = 0.50f;   // vocal effort: singer's formant + tenser source
            float resonance   = 0.60f;   // formant sharpness (narrower bandwidths)
            float femininity  = 0.50f;   // 0.5 = model default; shifts formants by about +-6%

            // pitch
            float vibrato_depth = -1.f;  // semitones peak; <0 = voice default
            float vibrato_rate  = -1.f;  // Hz; <0 = voice default
            float vibrato_delay = -1.f;  // s before onset; <0 = voice default
            float vibrato_fade  = -1.f;  // s fade-in;     <0 = voice default
            float portamento    = 0.060f;   // s glide between connected notes
            float pitch_response= 0.55f;   // 0..1 how fast pitch settles on target
            float overshoot     = 0.20f;   // 0..1 pitch overshoot (0 = none)
            float scoop         = 0.30f;   // semitones: note-entry slide from below after a rest
            float exit_fall     = 0.20f;   // semitones: pitch sag at the end of a phrase-final note
            float drift         = 0.00f;   // semitones: slow deterministic drift
            float micro         = 0.00f;   // 0..1 tiny deterministic microvariation of vibrato

            // articulation
            float consonant_strength = 0.55f;
            float consonant_duration = 1.00f;   // scale on consonant length
            float vowel_clarity      = 0.60f;
            float legato             = 0.50f;   // 1 = notes run together
            float attack_sharpness   = 0.50f;
            float release            = 0.50f;   // 0 = abrupt, 1 = long fade
            bool  devoicing          = true;    // devoice short i/u between voiceless consonants

            // dynamics
            float volume            = 1.00f;
            float velocity_response = 0.65f;
            float compression       = 0.30f;
            float dynamic_timbre    = 0.50f;    // louder -> brighter/tenser, softer -> breathier
        };

        // Per-note override container: the effective Style for the note is already
        // baked in at note creation time; these are note-specific extras.
        struct NoteExtras {
            Curve bend;          // semitones vs seconds from note start
            double glide_from = -1.0;  // explicit portamento source pitch (MIDI), <0 = auto
        };

        inline float clamp01(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }

    }
}
