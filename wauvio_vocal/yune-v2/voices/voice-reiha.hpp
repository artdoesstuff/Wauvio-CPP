#pragma once
// Reiha (玲羽): cool, crystalline, airy female voice. Delicate and precise, slightly breathy, clear high notes;
// less warm and less forceful than Yune, more detached. Own tract, source, range, default look and personas.
#include "profile.hpp"

namespace wauvio {
    namespace vocal {

        inline VoiceProfile voice_reiha() {
            VoiceProfile p;
            p.id = "reiha"; p.display = "Reiha (玲羽)"; p.language = "ja";
            p.notes = "Cool, crystalline, airy: delicate, precise, slightly breathy, clear highs; detached and fragile rather than warm.";
            p.natural_amount = 0.5f;

            // ---- model (identity): slightly smaller tract than Yune, narrow resonances, lax airy source, weak singer's formant ----
            VoiceModel& m = p.model;
            m.name = "Reiha: cool, crystalline Japanese female";
            m.range_low = 55; m.range_high = 91; m.hard_low = 43; m.hard_high = 98;                 // G3..G6 comfortable
            //                      F1    F2    F3    B1   B2   B3   gain  f1trk f2trk
            m.vowel[0] = {  906, 1539, 3150, 115, 110, 162, 1.00f, 0.85f, 0.0f };   // a
            m.vowel[1] = {  340, 2964, 3675,  81, 128, 170, 1.00f, 0.45f, 0.0f };   // i
            m.vowel[2] = {  381, 1508, 2888,  85, 106, 153, 1.00f, 0.45f, 0.0f };   // u
            m.vowel[3] = {  577, 2548, 3360,  94, 115, 162, 1.00f, 0.75f, 0.0f };   // e
            m.vowel[4] = {  587, 1019, 2940,  94, 102, 153, 1.00f, 0.75f, 0.0f };   // o
            m.f4 = 4500.f; m.f5 = 5800.f; m.f6 = 7400.f; m.b4 = 240.f; m.b5 = 300.f; m.b6 = 460.f;
            m.singer_formant_hz = 3300.f; m.singer_formant_bw = 650.f;
            m.rd_base = 1.10f; m.aspiration = 0.032f; m.tilt_hz_min = 2800.f; m.tilt_hz_max = 12000.f;
            m.vib_depth_st = 0.20f; m.vib_rate_hz = 5.9f; m.vib_delay = 0.40f; m.vib_fade = 0.45f;
            m.synthetic = 0.80f;
            m.tune.out_scale = 0.65f; m.tune.asp_gain = 3.00f; m.tune.fric_gain = 0.30f; m.tune.upper_gain = 2.20f;
            m.tune.sing_gain = 1.30f; m.tune.air_gain = 0.14f; m.tune.pre_gain = 2.0f; m.tune.f23_gain = 30.0f;

            // ---- default look: the former "Sweet" persona is this voice's Normal look ----
            Style& s = p.style;
            s.brightness = 0.812f; s.breathiness = 0.292f; s.tension = 0.30799997f;
            s.softness = 0.435f; s.power = 0.114999995f; s.resonance = 0.675f;
            s.femininity = 0.57199997f; s.vibrato_depth = 0.18875001f; s.vibrato_rate = 6.179474f;
            s.vibrato_delay = 0.46f; s.portamento = 0.097f; s.pitch_response = 0.45f;
            s.overshoot = 0.16499999f; s.scoop = 0.33499998f; s.exit_fall = 0.3f;
            s.micro = 0.1f; s.consonant_strength = 0.40499997f; s.vowel_clarity = 0.65f;
            s.legato = 0.71000004f; s.attack_sharpness = 0.35f; s.release = 0.665f;
            s.dynamic_timbre = 0.45f; s.formant_shift = 0.54f; s.air = 0.31f;
            s.reg = 0.21f;

            p.parse.merge_long_vowels = true; p.parse.cute = false;

            // ---- personas: only Normal and Soft ----
            p.personas.add("normal", s);
            { Style l;
            l.brightness = 0.515f; l.breathiness = 0.292f; l.tension = 0.24499999f;
            l.softness = 0.426f; l.power = 0.025000006f; l.resonance = 0.675f;
            l.vibrato_depth = 0.14375001f; l.vibrato_delay = 0.58f; l.portamento = 0.088f;
            l.pitch_response = 0.45f; l.overshoot = 0.12f; l.scoop = 0.24499999f;
            l.exit_fall = 0.39000002f; l.micro = 0.1f; l.consonant_strength = 0.315f;
            l.vowel_clarity = 0.65f; l.legato = 0.79999995f; l.attack_sharpness = 0.125f;
            l.release = 0.79999995f; l.dynamic_timbre = 0.54f; l.air = 0.35500002f;
            l.reg = 0.174f;
              p.personas.add("soft", l); }
            return p;
        }

    }
}
