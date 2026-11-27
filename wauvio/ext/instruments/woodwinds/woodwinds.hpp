#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        // Transverse flute: near-sinusoidal source, jet noise ~-32 dB re tone, soft ~55 ms attack with chiff, amplitude-led vibrato.
        class Flute : public AirJetInstrument {
        public:
            Flute() : AirJetInstrument("Flute") {
                auto& w = recipe.wind;
            }
        };

        // Short air column: faster attack, purer tone, high-centred jet noise, small vibrato.
        class Piccolo : public AirJetInstrument {
        public:
            Piccolo() : AirJetInstrument("Piccolo") {
                auto& w = recipe.wind;
                w.g_soft = 0.2; w.g_loud = 0.42; w.tilt_per_octave = 0.5; w.cutoff_hz = 8500.0; w.breath_center_hz = 4500.0; w.breath_center_mult = 8.0;
                w.chiff_center_hz = 3800.0; w.attack_ms = 32.0; w.release_ms = 60.0; w.vib_depth_cents = 9.0; w.vib_am = 0.12; w.scoop_cents = 6.0;
            }
        };

        // Longer tube: slower speech, breathier, darker.
        class AltoFlute : public AirJetInstrument {
        public:
            AltoFlute() : AirJetInstrument("Alto Flute") {
                auto& w = recipe.wind;
                w.g_soft = 0.22; w.g_loud = 0.42; w.cutoff_hz = 3200.0; w.breath_db = -28.0; w.breath_center_hz = 2000.0; w.attack_ms = 75.0;
                w.release_ms = 110.0; w.vib_depth_cents = 14.0; w.vib_rate_hz = 5.0; w.chiff_ms = 36.0;
            }
        };

        class BassFlute : public AirJetInstrument {
        public:
            BassFlute() : AirJetInstrument("Bass Flute") {
                auto& w = recipe.wind;
                w.g_soft = 0.18; w.g_loud = 0.36; w.cutoff_hz = 2400.0; w.breath_db = -25.0; w.breath_center_hz = 1500.0; w.attack_ms = 95.0;
                w.release_ms = 130.0; w.vib_depth_cents = 12.0; w.vib_rate_hz = 4.8; w.chiff_ms = 45.0; w.chiff_center_hz = 1800.0;
            }
        };

        class ContrabassFlute : public AirJetInstrument {
        public:
            ContrabassFlute() : AirJetInstrument("Contrabass Flute") {
                auto& w = recipe.wind;
                w.g_soft = 0.14; w.g_loud = 0.3; w.cutoff_hz = 1500.0; w.breath_db = -22.0; w.breath_center_hz = 900.0; w.attack_ms = 140.0;
                w.release_ms = 180.0; w.vib_depth_cents = 8.0; w.vib_rate_hz = 4.5; w.chiff_ms = 70.0; w.chiff_center_hz = 1200.0;
            }
        };

        // Conical double reed: rich pulse-like source, formants near 1.15 kHz and 3 kHz, ~24 ms attack with reed pop.
        class Oboe : public DoubleReedInstrument {
        public:
            Oboe() : DoubleReedInstrument("Oboe") {
                auto& w = recipe.wind;
            }
        };

        class BassOboe : public DoubleReedInstrument {
        public:
            BassOboe() : DoubleReedInstrument("Bass Oboe") {
                auto& w = recipe.wind;
                w.res[0] = { 600.0, 7.0, 1.4 }; w.res[1] = { 1600.0, 3.0, 2.0 }; w.cutoff_hz = 1900.0; w.attack_ms = 38.0; w.breath_center_hz = 1800.0;
                w.vib_depth_cents = 10.0; w.vib_rate_hz = 5.0; w.release_ms = 90.0;
            }
        };

        // Bulb bell: narrower, darker formant near 0.8 kHz (nasal).
        class EnglishHorn : public DoubleReedInstrument {
        public:
            EnglishHorn() : DoubleReedInstrument("English Horn") {
                auto& w = recipe.wind;
                w.res[0] = { 800.0, 8.0, 2.2 }; w.res[1] = { 2000.0, 2.0, 2.0 }; w.cutoff_hz = 2400.0; w.attack_ms = 32.0; w.vib_depth_cents = 12.0;
                w.vib_rate_hz = 5.4; w.breath_center_hz = 2200.0;
            }
        };

        class Heckelphone : public DoubleReedInstrument {
        public:
            Heckelphone() : DoubleReedInstrument("Heckelphone") {
                auto& w = recipe.wind;
                w.res[0] = { 520.0, 7.0, 1.5 }; w.res[1] = { 1400.0, 3.0, 2.0 }; w.cutoff_hz = 1700.0; w.attack_ms = 42.0; w.vib_depth_cents = 10.0;
                w.release_ms = 90.0; w.breath_center_hz = 1600.0;
            }
        };

        // Cylindrical single reed: even harmonics suppressed below ~1.8 kHz, tone-hole cutoff ~2 kHz, fundamental weakens with loudness, 15-50 ms attack by dynamics.
        class Clarinet : public SingleReedInstrument {
        public:
            Clarinet() : SingleReedInstrument("Clarinet") {
                auto& w = recipe.wind;
            }
        };

        class EbClarinet : public SingleReedInstrument {
        public:
            EbClarinet() : SingleReedInstrument("E-flat Clarinet") {
                auto& w = recipe.wind;
                w.cutoff_hz = 2800.0; w.even_open_hz = 2400.0; w.attack_ms = 22.0; w.breath_center_hz = 2800.0; w.chiff_center_hz = 1200.0;
            }
        };

        class BassClarinet : public SingleReedInstrument {
        public:
            BassClarinet() : SingleReedInstrument("Bass Clarinet") {
                auto& w = recipe.wind;
                w.cutoff_hz = 1300.0; w.even_open_hz = 900.0; w.attack_ms = 42.0; w.breath_db = -36.0; w.breath_center_hz = 1400.0; w.chiff_center_hz = 500.0;
                w.chiff_ms = 22.0; w.vib_depth_cents = 8.0; w.vib_rate_hz = 4.8; w.fund_drop_db = 3.0; w.release_ms = 80.0;
            }
        };

        class ContrabassClarinet : public SingleReedInstrument {
        public:
            ContrabassClarinet() : SingleReedInstrument("Contrabass Clarinet") {
                auto& w = recipe.wind;
                w.cutoff_hz = 800.0; w.even_open_hz = 550.0; w.attack_ms = 70.0; w.breath_db = -34.0; w.breath_center_hz = 900.0; w.chiff_center_hz = 350.0;
                w.chiff_ms = 30.0; w.vib_depth_cents = 6.0; w.vib_rate_hz = 4.5; w.fund_drop_db = 3.0; w.release_ms = 110.0;
            }
        };

        // Strong bassoon formant near 440-500 Hz.
        class Bassoon : public DoubleReedInstrument {
        public:
            Bassoon() : DoubleReedInstrument("Bassoon") {
                auto& w = recipe.wind;
                w.res[0] = { 460.0, 9.0, 1.5 }; w.res[1] = { 1150.0, 4.0, 1.5 }; w.cutoff_hz = 1900.0; w.attack_ms = 38.0; w.breath_db = -36.0;
                w.breath_center_hz = 1500.0; w.chiff_center_hz = 700.0; w.chiff_ms = 18.0; w.vib_depth_cents = 10.0; w.vib_rate_hz = 5.2;
                w.vib_delay_s = 0.4; w.vib_am = 0.06; w.release_ms = 90.0;
            }
        };

        class Contrabassoon : public DoubleReedInstrument {
        public:
            Contrabassoon() : DoubleReedInstrument("Contrabassoon") {
                auto& w = recipe.wind;
                w.res[0] = { 230.0, 8.0, 1.3 }; w.res[1] = { 600.0, 3.0, 1.5 }; w.cutoff_hz = 1000.0; w.attack_ms = 85.0; w.release_ms = 110.0;
                w.breath_db = -32.0; w.breath_center_hz = 700.0; w.chiff_center_hz = 400.0; w.chiff_ms = 30.0; w.vib_depth_cents = 6.0; w.vib_rate_hz = 4.5;
            }
        };

        // Fipple (edge-tone) flute: fixed windway, few harmonics, edge chiff, quicker attack, essentially no vibrato.
        class SopranoRecorder : public AirJetInstrument {
        public:
            SopranoRecorder() : AirJetInstrument("Soprano Recorder") {
                auto& w = recipe.wind;
                w.tilt_soft = 1.2; w.tilt_loud = 1.2; w.g_soft = 0.25; w.g_loud = 0.4; w.cutoff_hz = 4600.0; w.breath_db = -38.0; w.breath_follow = 0.3;
                w.breath_tone_mod = 0.1; w.breath_center_hz = 3200.0; w.chiff_db = -20.0; w.chiff_ms = 22.0; w.chiff_center_hz = 3000.0;
                w.attack_ms = 28.0; w.attack_shape = 1.0; w.attack_vel_exp = 0.25; w.pressure_pitch_cents = 12.0; w.scoop_cents = 4.0; w.vib_depth_cents = 0.0;
                w.vib_am = 0.0;
            }
        };
        using Recorder = SopranoRecorder;

        class AltoRecorder : public AirJetInstrument {
        public:
            AltoRecorder() : AirJetInstrument("Alto Recorder") {
                auto& w = recipe.wind;
                w.tilt_soft = 1.2; w.tilt_loud = 1.2; w.g_soft = 0.25; w.g_loud = 0.4; w.cutoff_hz = 3800.0; w.breath_db = -38.0; w.breath_follow = 0.3;
                w.breath_tone_mod = 0.1; w.breath_center_hz = 2800.0; w.chiff_db = -20.0; w.chiff_ms = 24.0; w.chiff_center_hz = 2600.0;
                w.attack_ms = 32.0; w.attack_shape = 1.0; w.attack_vel_exp = 0.25; w.pressure_pitch_cents = 12.0; w.scoop_cents = 4.0; w.vib_depth_cents = 0.0;
                w.vib_am = 0.0;
            }
        };

        class TenorRecorder : public AirJetInstrument {
        public:
            TenorRecorder() : AirJetInstrument("Tenor Recorder") {
                auto& w = recipe.wind;
                w.tilt_soft = 1.2; w.tilt_loud = 1.2; w.g_soft = 0.22; w.g_loud = 0.38; w.cutoff_hz = 3000.0; w.breath_db = -37.0; w.breath_follow = 0.3;
                w.breath_tone_mod = 0.1; w.breath_center_hz = 2200.0; w.chiff_db = -20.0; w.chiff_ms = 28.0; w.chiff_center_hz = 2200.0;
                w.attack_ms = 38.0; w.attack_shape = 1.0; w.attack_vel_exp = 0.25; w.pressure_pitch_cents = 12.0; w.scoop_cents = 4.0; w.vib_depth_cents = 0.0;
                w.vib_am = 0.0;
            }
        };

        class BassRecorder : public AirJetInstrument {
        public:
            BassRecorder() : AirJetInstrument("Bass Recorder") {
                auto& w = recipe.wind;
                w.tilt_soft = 1.3; w.tilt_loud = 1.3; w.g_soft = 0.2; w.g_loud = 0.34; w.cutoff_hz = 2000.0; w.breath_db = -35.0; w.breath_follow = 0.3;
                w.breath_tone_mod = 0.1; w.breath_center_hz = 1400.0; w.chiff_db = -20.0; w.chiff_ms = 36.0; w.chiff_center_hz = 1600.0;
                w.attack_ms = 55.0; w.attack_shape = 1.1; w.attack_vel_exp = 0.25; w.pressure_pitch_cents = 12.0; w.scoop_cents = 4.0; w.vib_depth_cents = 0.0;
                w.vib_am = 0.0; w.release_ms = 100.0;
            }
        };

        class ContrabassRecorder : public AirJetInstrument {
        public:
            ContrabassRecorder() : AirJetInstrument("Contrabass Recorder") {
                auto& w = recipe.wind;
                w.tilt_soft = 1.3; w.tilt_loud = 1.3; w.g_soft = 0.18; w.g_loud = 0.3; w.cutoff_hz = 1200.0; w.breath_db = -33.0; w.breath_follow = 0.3;
                w.breath_tone_mod = 0.1; w.breath_center_hz = 900.0; w.chiff_db = -20.0; w.chiff_ms = 50.0; w.chiff_center_hz = 1100.0; w.attack_ms = 90.0;
                w.attack_shape = 1.2; w.attack_vel_exp = 0.25; w.pressure_pitch_cents = 12.0; w.scoop_cents = 4.0; w.vib_depth_cents = 0.0;
                w.vib_am = 0.0; w.release_ms = 140.0;
            }
        };

        // Stopped pipes: odd harmonics dominate, strongly breathy.
        class PanFlute : public AirJetInstrument {
        public:
            PanFlute() : AirJetInstrument("Pan Flute") {
                auto& w = recipe.wind;
                w.even_low = 0.12; w.even_open_hz = 3500.0; w.g_soft = 0.3; w.g_loud = 0.5; w.tilt_soft = 1.0; w.tilt_loud = 1.0; w.cutoff_hz = 4200.0;
                w.breath_db = -22.0; w.breath_follow = 0.3; w.breath_tone_mod = 0.5; w.chiff_db = -18.0; w.chiff_ms = 35.0; w.attack_ms = 70.0;
                w.vib_depth_cents = 4.0; w.vib_delay_s = 0.6;
            }
        };

        // Helmholtz resonator: almost a pure sine.
        class Ocarina : public AirJetInstrument {
        public:
            Ocarina() : AirJetInstrument("Ocarina") {
                auto& w = recipe.wind;
                w.tilt_soft = 1.6; w.tilt_loud = 1.6; w.g_soft = 0.08; w.g_loud = 0.14; w.cutoff_hz = 3000.0; w.breath_db = -38.0; w.breath_center_hz = 2500.0;
                w.chiff_db = -30.0; w.chiff_ms = 20.0; w.attack_ms = 45.0; w.vib_depth_cents = 8.0; w.vib_am = 0.05; w.pressure_pitch_cents = 10.0;
            }
        };

        // Free reed in a cavity: all harmonics, breath noise from the player, hand tremolo as amplitude modulation.
        class Harmonica : public SingleReedInstrument {
        public:
            Harmonica() : SingleReedInstrument("Harmonica") {
                auto& w = recipe.wind;
                w.tilt_soft = 1.0; w.tilt_loud = 0.6; w.g_soft = 0.85; w.g_loud = 0.97; w.fund_drop_db = 0.0; w.even_low = 1.0; w.cutoff_hz = 4500.0;
                w.res[0] = { 2200.0, 4.0, 1.5 }; w.breath_db = -30.0; w.breath_center_hz = 2600.0; w.breath_follow = 1.0; w.chiff_db = -26.0;
                w.chiff_ms = 20.0; w.chiff_center_hz = 1500.0; w.attack_ms = 30.0; w.vib_depth_cents = 2.0; w.vib_rate_hz = 5.5; w.vib_delay_s = 0.3;
                w.vib_am = 0.12; w.jitter_cents = 0.5;
            }
        };

        // Fipple whistle: ornament-driven, little vibrato.
        class IrishWhistle : public AirJetInstrument {
        public:
            IrishWhistle() : AirJetInstrument("Irish Whistle") {
                auto& w = recipe.wind;
                w.g_soft = 0.3; w.g_loud = 0.5; w.tilt_soft = 1.0; w.tilt_loud = 1.0; w.cutoff_hz = 6200.0; w.breath_db = -28.0; w.breath_center_hz = 3500.0;
                w.breath_tone_mod = 0.25; w.chiff_db = -18.0; w.chiff_ms = 16.0; w.chiff_center_hz = 3500.0; w.attack_ms = 24.0; w.attack_vel_exp = 0.25;
                w.vib_depth_cents = 3.0; w.vib_delay_s = 0.6; w.vib_am = 0.05; w.pressure_pitch_cents = 20.0; w.scoop_cents = 6.0;
            }
        };
        using TinWhistle = IrishWhistle;

        class LowWhistle : public AirJetInstrument {
        public:
            LowWhistle() : AirJetInstrument("Low Whistle") {
                auto& w = recipe.wind;
                w.g_soft = 0.22; w.g_loud = 0.4; w.cutoff_hz = 3400.0; w.breath_db = -24.0; w.breath_center_hz = 1800.0; w.chiff_db = -20.0;
                w.chiff_ms = 35.0; w.chiff_center_hz = 1800.0; w.attack_ms = 55.0; w.vib_depth_cents = 10.0; w.vib_rate_hz = 5.0; w.pressure_pitch_cents = 18.0;
            }
        };

    }
}
