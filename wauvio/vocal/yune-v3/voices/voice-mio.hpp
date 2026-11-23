#pragma once
// Mio (澪): a calm, warm relative of Yune
#include "profile.hpp"

namespace wauvio {
    namespace vocal {
        namespace v3 {

            inline VoiceProfile voice_mio() {
                VoiceProfile p;
                p.id = "mio"; p.display = "Mio (澪)"; p.language = "ja";
                p.notes = "Calm, warm and smooth: a slightly darker, rounder cousin of Yune with a touch more breath, steady slower vibrato and gentle glides; soft consonants, flowing legato.";
                p.natural_amount = 0.27f;   // default humanisation strength (singer-sized at 0.3)

                // ---- model (identity): a close relative of Yune: formants within about 1.5% of hers, similar range ----
                VoiceModel& m = p.model;
                m.name = "Mio: calm, warm Japanese female";
                m.range_low = 51; m.range_high = 86; m.hard_low = 40; m.hard_high = 94;                 // D#3..D6 comfortable
                //                      F1    F2    F3    B1   B2   B3   gain  f1trk f2trk
                m.vowel[0] = {  867, 1458, 2955, 140, 135, 198, 1.00f, 0.85f, 0.0f };   // a
                m.vowel[1] = {  325, 2807, 3448,  99, 156, 208, 1.00f, 0.45f, 0.0f };   // i
                m.vowel[2] = {  364, 1428, 2709, 104, 130, 187, 1.00f, 0.45f, 0.0f };   // u
                m.vowel[3] = {  552, 2413, 3152, 114, 140, 198, 1.00f, 0.75f, 0.0f };   // e
                m.vowel[4] = {  561,  965, 2758, 114, 125, 187, 1.00f, 0.75f, 0.0f };   // o
                m.f4 = 4250.f; m.f5 = 5550.f; m.f6 = 7100.f; m.b4 = 270.f; m.b5 = 330.f; m.b6 = 520.f;
                m.singer_formant_hz = 3050.f; m.singer_formant_bw = 620.f;
                m.rd_base = 1.02f; m.aspiration = 0.024f; m.tilt_hz_min = 2400.f; m.tilt_hz_max = 10000.f;
                m.vib_depth_st = 0.30f; m.vib_rate_hz = 5.4f; m.vib_delay = 0.30f; m.vib_fade = 0.35f;
                m.synthetic = 0.60f;
                m.tune.out_scale = 0.67f;   // level-matched to Yune (measured unnormalised) m.tune.asp_gain = 3.10f; m.tune.fric_gain = 0.28f; m.tune.upper_gain = 4.400f;
                m.tune.sing_gain = 2.200f; m.tune.air_gain = 0.12f; m.tune.pre_gain = 1.0f; m.tune.f23_gain = 30.0f;

                // ---- default look ----
                Style& s = p.style;
                s.brightness = 0.68f; s.breathiness = 0.24f; s.tension = 0.36f;
                s.softness = 0.42f; s.power = 0.30f; s.resonance = 0.58f;
                s.femininity = 0.58f; s.vibrato_depth = 0.26f; s.vibrato_rate = 5.4f;
                // singing technique at the V3 standard (JSUT-song): vibrato starts at once, entries almost on pitch, short portamento
                s.vibrato_delay = 0.03f; s.vibrato_fade = 0.26f; s.portamento = 0.075f; s.overshoot = 0.25f;
                s.scoop = 0.09f; s.consonant_strength = 0.45f; s.legato = 0.70f;
                s.release = 0.62f; s.formant_shift = 0.6f; s.air = 0.12f;
                s.reg = 0.1f;

                p.parse.merge_long_vowels = true; p.parse.cute = false;

                // ---- personas: Normal and Soft ----
                p.personas.add("normal", s);
                { Style l;
                l.brightness = 0.38f; l.breathiness = 0.26f; l.tension = 0.30f;
                l.softness = 0.40f; l.power = 0.22f; l.resonance = 0.58f;
                l.vibrato_depth = 0.20f; l.vibrato_delay = 0.10f; l.portamento = 0.09f;
                l.scoop = 0.30f; l.exit_fall = 0.30f; l.consonant_strength = 0.36f;
                l.legato = 0.78f; l.attack_sharpness = 0.22f; l.release = 0.76f;
                l.dynamic_timbre = 0.6f; l.air = 0.17f; l.reg = 0.06f;
                  p.personas.add("soft", l); }
                return p;
            }

        }
    }
}
