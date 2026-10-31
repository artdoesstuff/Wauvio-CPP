#pragma once
// Koharu (小春): warm, punchy, expressive female voice. Fuller midrange, crisp consonants, a slightly husky edge,
// energetic delivery; more grounded and chesty than Yune. Own tract, source, range, default look and personas.
#include "profile.hpp"

namespace wauvio {
    namespace vocal {

        inline VoiceProfile voice_koharu() {
            VoiceProfile p;
            p.id = "koharu"; p.display = "Koharu (小春)"; p.language = "ja";
            p.notes = "Warm, punchy, expressive: full midrange, crisp consonants, slightly husky, energetic; grounded and chesty.";
            p.natural_amount = 0.7f;

            // ---- model (identity): slightly larger tract, wider resonances, tense full source, strong singer's formant ----
            VoiceModel& m = p.model;
            m.name = "Koharu: warm, punchy Japanese female";
            m.range_low = 50; m.range_high = 86; m.hard_low = 38; m.hard_high = 94;                 // D3..D6 comfortable
            //                      F1    F2    F3    B1   B2   B3   gain  f1trk f2trk
            m.vowel[0] = {  862, 1436, 2940, 151, 146, 213, 1.00f, 0.85f, 0.0f };   // a
            m.vowel[1] = {  323, 2765, 3430, 106, 168, 224, 1.00f, 0.45f, 0.0f };   // i
            m.vowel[2] = {  363, 1407, 2695, 112, 140, 202, 1.00f, 0.45f, 0.0f };   // u
            m.vowel[3] = {  549, 2377, 3136, 123, 151, 213, 1.00f, 0.75f, 0.0f };   // e
            m.vowel[4] = {  559,  951, 2744, 123, 134, 202, 1.00f, 0.75f, 0.0f };   // o
            m.f4 = 4200.f; m.f5 = 5400.f; m.f6 = 7000.f; m.b4 = 280.f; m.b5 = 340.f; m.b6 = 520.f;
            m.singer_formant_hz = 2950.f; m.singer_formant_bw = 650.f;
            m.rd_base = 0.55f; m.aspiration = 0.026f; m.tilt_hz_min = 2300.f; m.tilt_hz_max = 9500.f;
            m.vib_depth_st = 0.26f; m.vib_rate_hz = 6.0f; m.vib_delay = 0.22f; m.vib_fade = 0.28f;
            m.synthetic = 0.45f;
            m.tune.out_scale = 0.66f; m.tune.asp_gain = 3.20f; m.tune.fric_gain = 0.5f; m.tune.upper_gain = 1.80f;
            m.tune.sing_gain = 2.40f; m.tune.air_gain = 0.08f; m.tune.pre_gain = 1.0f; m.tune.f23_gain = 30.0f;

            // ---- default look: the former "Sweet" persona is this voice's Normal look ----
            Style& s = p.style;
            s.breathiness = 0.18f; s.tension = 0.52f; s.softness = 0.35f;
            s.power = 0.53f; s.resonance = 0.5f; s.femininity = 0.58f;
            s.vibrato_depth = 0.24375f; s.vibrato_rate = 6.3157897f; s.vibrato_delay = 0.25666666f;
            s.portamento = 0.075f; s.pitch_response = 0.68f; s.overshoot = 0.32999998f;
            s.scoop = 0.46999997f; s.exit_fall = 0.12f; s.micro = 0.1f;
            s.consonant_strength = 0.69f; s.consonant_duration = 0.92f; s.vowel_clarity = 0.65f;
            s.legato = 0.45000002f; s.attack_sharpness = 0.68f; s.release = 0.43f;
            s.dynamic_timbre = 0.62f; s.formant_shift = 0.6f; s.growl = 0.15f;
            s.fry = 0.22f; s.air = 0.1f; s.reg = 0.1f;
            s.glottal_onset = 0.06f; s.transition_speed = 1.15f; s.devoice_max = 0.09f;

            p.parse.merge_long_vowels = true; p.parse.cute = false;

            // ---- personas: only Normal and Soft ----
            p.personas.add("normal", s);
            { Style l;
            l.brightness = 0.37f; l.breathiness = 0.18f; l.tension = 0.45000002f;
            l.softness = 0.34f; l.power = 0.43f; l.resonance = 0.5f;
            l.vibrato_depth = 0.17875f; l.vibrato_delay = 0.32999998f; l.portamento = 0.065f;
            l.pitch_response = 0.68f; l.overshoot = 0.28f; l.scoop = 0.36999997f;
            l.exit_fall = 0.22f; l.micro = 0.1f; l.consonant_strength = 0.59000003f;
            l.consonant_duration = 0.92f; l.vowel_clarity = 0.65f; l.legato = 0.54999995f;
            l.attack_sharpness = 0.43f; l.release = 0.58f; l.dynamic_timbre = 0.72f;
            l.growl = 0.03f; l.fry = 0.05f; l.air = 0.15f;
            l.reg = 0.06f; l.glottal_onset = 0.06f; l.transition_speed = 1.15f;
            l.devoice_max = 0.09f;
              p.personas.add("soft", l); }
            return p;
        }

    }
}
