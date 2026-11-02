#pragma once
// Yune: the default Japanese female voice.
#include "profile.hpp"

namespace wauvio {
    namespace vocal {

        inline VoiceProfile voice_yune() {
            VoiceProfile p;
            p.id = "yune"; p.display = "Yune"; p.language = "ja";
            p.notes = "Bright, clear Japanese female singing voice; peripheral vowels for clarity, tight resonances.";
            p.natural_amount = 0.6f;

            // ---- model (identity) ----
            VoiceModel& m = p.model;
            m.name = "Yune: Japanese default female voice";
            m.range_low = 52; m.range_high = 88; m.hard_low = 40; m.hard_high = 96;                 // E3..E6 comfortable
            //                      F1    F2    F3    B1   B2   B3   gain  f1trk f2trk
            m.vowel[0] = {  880, 1480, 3000, 135, 130, 190, 1.00f, 0.85f, 0.0f };   // a
            m.vowel[1] = {  330, 2850, 3500,  95, 150, 200, 1.00f, 0.45f, 0.0f };   // i
            m.vowel[2] = {  370, 1450, 2750, 100, 125, 180, 1.00f, 0.45f, 0.0f };   // u
            m.vowel[3] = {  560, 2450, 3200, 110, 135, 190, 1.00f, 0.75f, 0.0f };   // e
            m.vowel[4] = {  570,  980, 2800, 110, 120, 180, 1.00f, 0.75f, 0.0f };   // o
            m.f4 = 4300.f; m.f5 = 5600.f; m.f6 = 7200.f; m.b4 = 260.f; m.b5 = 320.f; m.b6 = 500.f;
            m.singer_formant_hz = 3100.f; m.singer_formant_bw = 600.f;
            m.rd_base = 0.95f; m.aspiration = 0.020f; m.tilt_hz_min = 2600.f; m.tilt_hz_max = 11000.f;
            m.vib_depth_st = 0.32f; m.vib_rate_hz = 5.7f; m.vib_delay = 0.30f; m.vib_fade = 0.35f;
            m.synthetic = 0.65f;
            m.tune.out_scale = 0.65f; m.tune.asp_gain = 3.00f; m.tune.fric_gain = 0.30f; m.tune.upper_gain = 2.00f;
            m.tune.sing_gain = 2.00f; m.tune.air_gain = 0.10f; m.tune.pre_gain = 1.0f; m.tune.f23_gain = 30.0f;

            // ---- default look: the former "Sweet" persona is this voice's Normal look ----
            Style& s = p.style;
            s.brightness = 0.78f; s.breathiness = 0.2f; s.tension = 0.42f;
            s.softness = 0.35f; s.power = 0.35f; s.resonance = 0.55f;
            s.femininity = 0.58f; s.vibrato_depth = 0.3f; s.vibrato_rate = 6.f;
            s.vibrato_delay = 0.35f; s.portamento = 0.09f; s.overshoot = 0.25f;
            s.scoop = 0.45f; s.consonant_strength = 0.5f; s.legato = 0.6f;
            s.release = 0.55f; s.formant_shift = 0.6f; s.air = 0.1f;
            s.reg = 0.1f;

            p.parse.merge_long_vowels = true; p.parse.cute = false;

            // ---- personas: only Normal and Soft ----
            p.personas.add("normal", s);
            { Style l;
            l.brightness = 0.45f; l.breathiness = 0.2f; l.tension = 0.35f;
            l.softness = 0.34f; l.power = 0.25f; l.resonance = 0.55f;
            l.vibrato_depth = 0.22f; l.vibrato_delay = 0.45f; l.portamento = 0.08f;
            l.scoop = 0.35f; l.exit_fall = 0.3f; l.consonant_strength = 0.4f;
            l.legato = 0.7f; l.attack_sharpness = 0.25f; l.release = 0.7f;
            l.dynamic_timbre = 0.6f; l.air = 0.15f; l.reg = 0.06f;
              p.personas.add("soft", l); }
            return p;
        }

    }
}
