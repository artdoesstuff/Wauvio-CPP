#pragma once
// Itsuki (樹): deep, resonant, smooth male voice. Rich low register, rounded vowels, controlled vibrato, a slightly
// grainy texture when pushed; darker, heavier and more restrained than Yune. Own tract, source, range, look, personas.
#include "profile.hpp"

namespace wauvio {
    namespace vocal {

        inline VoiceProfile voice_itsuki() {
            VoiceProfile p;
            p.id = "itsuki"; p.display = "Itsuki (樹)"; p.language = "ja";
            p.notes = "Deep, resonant, smooth male voice: rounded vowels, controlled vibrato, grainy when pushed; dark and restrained.";
            p.natural_amount = 0.5f;

            // ---- model (identity): long male tract (F1 x0.83, F2 x0.80, F3 x0.86 of Yune), low singer's formant, dark source ----
            VoiceModel& m = p.model;
            m.name = "Itsuki: deep, resonant Japanese male";
            m.range_low = 41; m.range_high = 67; m.hard_low = 36; m.hard_high = 76;                 // F2..G4 comfortable
            //                      F1    F2    F3    B1   B2   B3   gain  f1trk f2trk
            m.vowel[0] = {  730, 1184, 2580, 124, 120, 175, 1.00f, 0.85f, 0.0f };   // a
            m.vowel[1] = {  274, 2280, 3010,  87, 138, 184, 1.00f, 0.45f, 0.0f };   // i
            m.vowel[2] = {  307, 1160, 2365,  92, 115, 166, 1.00f, 0.45f, 0.0f };   // u
            m.vowel[3] = {  465, 1960, 2752, 101, 124, 175, 1.00f, 0.75f, 0.0f };   // e
            m.vowel[4] = {  473,  784, 2408, 101, 110, 166, 1.00f, 0.75f, 0.0f };   // o
            m.f4 = 3600.f; m.f5 = 4600.f; m.f6 = 5900.f; m.b4 = 230.f; m.b5 = 290.f; m.b6 = 430.f;
            m.singer_formant_hz = 2750.f; m.singer_formant_bw = 520.f;
            m.rd_base = 0.95f; m.aspiration = 0.016f; m.tilt_hz_min = 1900.f; m.tilt_hz_max = 8000.f;
            m.vib_depth_st = 0.24f; m.vib_rate_hz = 5.3f; m.vib_delay = 0.40f; m.vib_fade = 0.45f;
            m.synthetic = 0.50f;
            m.tune.out_scale = 0.70f; m.tune.asp_gain = 2.60f; m.tune.fric_gain = 0.30f; m.tune.upper_gain = 1.40f;
            m.tune.sing_gain = 1.80f; m.tune.air_gain = 0.06f; m.tune.pre_gain = 1.0f; m.tune.f23_gain = 26.0f;

            // ---- default look: the former "Sweet" persona is this voice's Normal look (male: the gender axis stays untouched, so femininity and formant_shift are NOT raised) ----
            Style& s = p.style;
            s.brightness = 0.45f; s.breathiness = 0.144f; s.tension = 0.38599998f;
            s.softness = 0.4f; s.power = 0.43f; s.resonance = 0.56f;
            s.vibrato_depth = 0.228f; s.vibrato_rate = 5.523158f; s.vibrato_delay = 0.45333335f;
            s.portamento = 0.094000004f; s.pitch_response = 0.45f; s.overshoot = 0.14f;
            s.scoop = 0.33999997f; s.exit_fall = 0.25f; s.micro = 0.08f;
            s.consonant_strength = 0.40999997f; s.consonant_duration = 1.05f; s.vowel_clarity = 0.5f;
            s.legato = 0.73f; s.attack_sharpness = 0.4f; s.release = 0.64000005f;
            s.dynamic_timbre = 0.6f; s.air = 0.080000006f; s.reg = 0.080000006f;
            s.mumble = 0.08f; s.grit = 0.55f;

            p.parse.merge_long_vowels = true; p.parse.cute = false;

            // ---- personas: only Normal and Soft ----
            p.personas.add("normal", s);
            { Style l;
            l.brightness = 0.24999999f; l.breathiness = 0.144f; l.tension = 0.32999998f;
            l.softness = 0.39200002f; l.power = 0.35000002f; l.resonance = 0.56f;
            l.vibrato_depth = 0.17999999f; l.vibrato_delay = 0.55999994f; l.portamento = 0.086f;
            l.pitch_response = 0.45f; l.overshoot = 0.1f; l.scoop = 0.26f;
            l.exit_fall = 0.33f; l.micro = 0.08f; l.consonant_strength = 0.32999998f;
            l.consonant_duration = 1.05f; l.vowel_clarity = 0.5f; l.legato = 0.80999994f;
            l.attack_sharpness = 0.2f; l.release = 0.76f; l.dynamic_timbre = 0.68000007f;
            l.air = 0.120000005f; l.reg = 0.048f; l.mumble = 0.08f;
            l.grit = 0.55f;
              p.personas.add("soft", l); }
            return p;
        }

    }
}
