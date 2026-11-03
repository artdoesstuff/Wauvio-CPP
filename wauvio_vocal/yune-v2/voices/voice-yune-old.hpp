#pragma once
// Yune (old): the previous default Japanese female voice, kept for reference / A-B comparison.
// Ported from voice-yune-old.voice. Style fields left unset keep their Style defaults
// (which match this voice's old "normal" look); -1 values in the .voice file mean "inherit".
#include "profile.hpp"

namespace wauvio {
    namespace vocal {

        inline VoiceProfile voice_yune_old() {
            VoiceProfile p;
            p.id = "yune"; p.display = "Yune"; p.language = "ja";
            p.notes = "Bright, clear Japanese female singing voice; peripheral vowels for clarity, tight resonances (synthetic 0.65).";
            p.natural_amount = 0.6f;

            // ---- model (identity) ----
            VoiceModel& m = p.model;
            m.name = "Wauvio Japanese Female (default)";
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

            // ---- default look: the plain "Normal" persona ----
            Style& s = p.style;
            s.brightness = 0.7f; s.breathiness = 0.12f; s.tension = 0.5f; s.softness = 0.0f;
            s.power = 0.5f; s.resonance = 0.6f; s.femininity = 0.5f; s.portamento = 0.06f;
            s.pitch_response = 0.55f; s.overshoot = 0.2f; s.scoop = 0.3f; s.exit_fall = 0.2f;
            s.drift = 0.0f; s.micro = 0.0f; s.consonant_strength = 0.55f; s.consonant_duration = 1.0f;
            s.vowel_clarity = 0.6f; s.legato = 0.5f; s.attack_sharpness = 0.5f; s.release = 0.5f;
            s.dynamic_timbre = 0.5f; s.formant_shift = 0.0f; s.mouth_open = 0.0f; s.growl = 0.0f;
            s.tremor = 0.0f; s.tremor_rate = 9.0f; s.fry = 0.0f; s.air = 0.0f;
            s.reg = 0.0f; s.glottal_onset = 0.0f; s.undershoot = 0.0f; s.porta_interval = 0.0f;
            s.mumble = 0.0f; s.grit = 0.0f;

            p.parse.merge_long_vowels = true; p.parse.cute = false;

            // ---- personas (only values that differ from the Style defaults are listed) ----
            p.personas.add("normal", s);
            { Style l;  // airy
              l.brightness = 0.55f; l.breathiness = 0.3f; l.tension = 0.3f; l.softness = 0.26f;
              l.power = 0.22f; l.resonance = 0.5f; l.vibrato_depth = 0.15f; l.vibrato_delay = 0.5f;
              l.portamento = 0.09f; l.exit_fall = 0.35f; l.consonant_strength = 0.35f; l.legato = 0.75f;
              l.attack_sharpness = 0.15f; l.release = 0.75f; l.dynamic_timbre = 0.7f; l.air = 0.5f;
              l.reg = 0.1f;
              p.personas.add("airy", l); }
            { Style l;  // cool
              l.brightness = 0.6f; l.breathiness = 0.06f; l.tension = 0.6f; l.power = 0.4f;
              l.resonance = 0.55f; l.vibrato_depth = 0.08f; l.vibrato_rate = 5.5f; l.vibrato_delay = 0.6f;
              l.portamento = 0.03f; l.overshoot = 0.05f; l.scoop = 0.05f; l.exit_fall = 0.05f;
              l.consonant_strength = 0.6f; l.vowel_clarity = 0.7f; l.legato = 0.35f; l.release = 0.35f;
              l.dynamic_timbre = 0.3f; l.mumble = -0.25f;
              p.personas.add("cool", l); }
            { Style l;  // dark
              l.brightness = 0.35f; l.breathiness = 0.15f; l.tension = 0.6f; l.power = 0.55f;
              l.resonance = 0.65f; l.femininity = 0.42f; l.vibrato_depth = 0.26f; l.vibrato_rate = 5.2f;
              l.portamento = 0.07f; l.drift = 0.03f; l.consonant_strength = 0.5f; l.release = 0.6f;
              l.formant_shift = -0.8f; l.mouth_open = 0.1f; l.growl = 0.08f;
              p.personas.add("dark", l); }
            { Style l;  // giddy
              l.brightness = 0.85f; l.breathiness = 0.15f; l.tension = 0.4f; l.vibrato_depth = 0.2f;
              l.portamento = 0.05f; l.overshoot = 0.4f; l.scoop = 0.6f; l.consonant_strength = 0.7f;
              l.legato = 0.3f; l.attack_sharpness = 0.7f; l.release = 0.3f; l.formant_shift = 0.8f;
              p.personas.add("giddy", l); }
            { Style l;  // nervous
              l.breathiness = 0.25f; l.tension = 0.6f; l.vibrato_depth = 0.1f; l.pitch_response = 0.6f;
              l.scoop = 0.1f; l.drift = 0.1f; l.tremor = 0.4f; l.tremor_rate = 10.0f;
              l.glottal_onset = 0.2f;
              p.personas.add("nervous", l); }
            { Style l;  // power
              l.brightness = 0.85f; l.breathiness = 0.05f; l.tension = 0.75f; l.power = 0.95f;
              l.resonance = 0.7f; l.vibrato_depth = 0.45f; l.vibrato_rate = 5.8f; l.vibrato_delay = 0.25f;
              l.overshoot = 0.1f; l.scoop = 0.2f; l.exit_fall = 0.1f; l.consonant_strength = 0.75f;
              l.legato = 0.4f; l.attack_sharpness = 0.75f; l.release = 0.35f; l.dynamic_timbre = 0.7f;
              l.mouth_open = 0.35f; l.glottal_onset = 0.15f; l.mumble = -0.1f;
              p.personas.add("power", l); }
            { Style l;  // shy
              l.brightness = 0.5f; l.breathiness = 0.25f; l.tension = 0.38f; l.softness = 0.3f;
              l.power = 0.25f; l.vibrato_depth = 0.12f; l.vibrato_delay = 0.5f; l.scoop = 0.4f;
              l.exit_fall = 0.4f; l.drift = 0.06f; l.consonant_strength = 0.35f; l.tremor = 0.15f;
              l.tremor_rate = 8.0f; l.air = 0.2f; l.reg = 0.06f; l.mumble = 0.15f;
              p.personas.add("shy", l); }
            { Style l;  // sleepy
              l.brightness = 0.35f; l.breathiness = 0.14f; l.tension = 0.25f; l.softness = 0.24f;
              l.power = 0.18f; l.vibrato_depth = 0.12f; l.portamento = 0.12f; l.pitch_response = 0.35f;
              l.exit_fall = 0.5f; l.drift = 0.12f; l.consonant_strength = 0.3f; l.vowel_clarity = 0.3f;
              l.legato = 0.8f; l.attack_sharpness = 0.1f; l.release = 0.8f; l.fry = 0.12f;
              l.air = 0.15f; l.mumble = 0.35f;
              p.personas.add("sleepy", l); }
            { Style l;  // soft
              l.brightness = 0.45f; l.breathiness = 0.2f; l.tension = 0.35f; l.softness = 0.34f;
              l.power = 0.25f; l.resonance = 0.55f; l.vibrato_depth = 0.22f; l.vibrato_delay = 0.45f;
              l.portamento = 0.08f; l.scoop = 0.35f; l.exit_fall = 0.3f; l.consonant_strength = 0.4f;
              l.legato = 0.7f; l.attack_sharpness = 0.25f; l.release = 0.7f; l.dynamic_timbre = 0.6f;
              l.air = 0.15f; l.reg = 0.06f;
              p.personas.add("soft", l); }
            { Style l;  // sweet
              l.brightness = 0.78f; l.breathiness = 0.2f; l.tension = 0.42f; l.softness = 0.35f;
              l.power = 0.35f; l.resonance = 0.55f; l.femininity = 0.58f; l.vibrato_depth = 0.3f;
              l.vibrato_rate = 6.0f; l.vibrato_delay = 0.35f; l.portamento = 0.09f; l.overshoot = 0.25f;
              l.scoop = 0.45f; l.consonant_strength = 0.5f; l.legato = 0.6f; l.release = 0.55f;
              l.formant_shift = 0.6f; l.air = 0.1f; l.reg = 0.1f;
              p.personas.add("sweet", l); }
            { Style l;  // tearful
              l.breathiness = 0.27f; l.tension = 0.55f; l.softness = 0.25f; l.vibrato_depth = 0.35f;
              l.portamento = 0.11f; l.scoop = 0.5f; l.exit_fall = 0.5f; l.drift = 0.08f;
              l.formant_shift = 0.3f; l.tremor = 0.35f; l.tremor_rate = 7.0f; l.fry = 0.1f;
              l.reg = 0.08f;
              p.personas.add("tearful", l); }
            return p;
        }

    }
}
