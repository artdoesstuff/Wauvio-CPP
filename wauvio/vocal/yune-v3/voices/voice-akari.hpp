#pragma once
// Akari (灯): a warm, luminous relative of Yune
#include "profile.hpp"

namespace wauvio {
    namespace vocal {
        namespace v3 {

            inline VoiceProfile voice_akari() {
                VoiceProfile p;
                p.id = "akari"; p.display = "Akari (灯)"; p.language = "ja";
                p.notes = "Warm and luminous: a little brighter and more present than Yune with a stronger carrying formant, a livelier vibrato and clean, gently crisp consonants; glowing rather than icy.";
                p.natural_amount = 0.32f;   // default humanisation strength (singer-sized at 0.3)

                // ---- model (identity): a close relative of Yune: formants within about 1.5% of hers, similar range ----
                VoiceModel& m = p.model;
                m.name = "Akari: warm, luminous Japanese female";
                m.range_low = 54; m.range_high = 89; m.hard_low = 42; m.hard_high = 96;                 // F#3..F6 comfortable
                //                      F1    F2    F3    B1   B2   B3   gain  f1trk f2trk
                m.vowel[0] = {  891, 1498, 3036, 130, 125, 182, 1.00f, 0.85f, 0.0f };   // a
                m.vowel[1] = {  334, 2884, 3542,  91, 144, 192, 1.00f, 0.45f, 0.0f };   // i
                m.vowel[2] = {  374, 1467, 2783,  96, 120, 173, 1.00f, 0.45f, 0.0f };   // u
                m.vowel[3] = {  567, 2479, 3238, 106, 130, 182, 1.00f, 0.75f, 0.0f };   // e
                m.vowel[4] = {  577,  992, 2834, 106, 115, 173, 1.00f, 0.75f, 0.0f };   // o
                m.f4 = 4350.f; m.f5 = 5650.f; m.f6 = 7300.f; m.b4 = 250.f; m.b5 = 310.f; m.b6 = 480.f;
                m.singer_formant_hz = 3150.f; m.singer_formant_bw = 600.f;
                m.rd_base = 0.88f; m.aspiration = 0.018f; m.tilt_hz_min = 2700.f; m.tilt_hz_max = 11500.f;
                m.vib_depth_st = 0.32f; m.vib_rate_hz = 5.9f; m.vib_delay = 0.30f; m.vib_fade = 0.35f;
                m.synthetic = 0.60f;
                m.tune.out_scale = 0.65f;   // level-matched to Yune (measured unnormalised) 
                m.tune.asp_gain = 2.90f; m.tune.fric_gain = 0.32f; m.tune.upper_gain = 4.500f;
                m.tune.sing_gain = 2.700f; m.tune.air_gain = 0.09f; m.tune.pre_gain = 1.0f; m.tune.f23_gain = 30.0f;

                // ---- default look ----
                Style& s = p.style;
                s.brightness = 0.82f; s.breathiness = 0.17f; s.tension = 0.45f;
                s.softness = 0.32f; s.power = 0.40f; s.resonance = 0.52f;
                s.femininity = 0.58f; s.vibrato_depth = 0.24f; s.vibrato_rate = 5.9f;
                // singing technique at the V3 standard (JSUT-song): vibrato starts at once, entries almost on pitch, short portamento
                s.vibrato_delay = 0.0f; s.vibrato_fade = 0.20f; s.portamento = 0.055f; s.overshoot = 0.25f;
                s.scoop = 0.10f; s.consonant_strength = 0.55f; s.legato = 0.55f;
                s.release = 0.50f; s.formant_shift = 0.6f; s.air = 0.09f;
                s.reg = 0.1f;
                s.attack_sharpness = 0.45f;
                s.vib_am = 0.27f;   // her stronger carrying formant already couples level to pitch (measured 1.64 dB at the default 0.35)

                p.parse.merge_long_vowels = true; p.parse.cute = false;

                // ---- personas: Normal and Soft ----
                p.personas.add("normal", s);
                { Style l;
                l.brightness = 0.50f; l.breathiness = 0.20f; l.tension = 0.38f;
                l.softness = 0.34f; l.power = 0.28f; l.resonance = 0.52f;
                l.vibrato_depth = 0.21f; l.vibrato_delay = 0.10f; l.portamento = 0.07f;
                l.scoop = 0.30f; l.exit_fall = 0.28f; l.consonant_strength = 0.44f;
                l.legato = 0.66f; l.attack_sharpness = 0.30f; l.release = 0.64f;
                l.dynamic_timbre = 0.6f; l.air = 0.13f; l.reg = 0.06f;
                  p.personas.add("soft", l); }
                return p;
            }

        }
    }
}
