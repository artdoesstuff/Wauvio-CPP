#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        // Helmholtz sawtooth through violin body modes (A0 ~275, B1 ~470/560, bridge hill ~2.8k); ~55 ms attack, scrape burst, vibrato after 0.2 s.
        class SoloViolin : public BowedModelInstrument {
        public:
            SoloViolin() : BowedModelInstrument("Solo Violin") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 9000.0; s.attack_ms = 55.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 20.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0;
            }
        };

        class SoloViola : public BowedModelInstrument {
        public:
            SoloViola() : BowedModelInstrument("Solo Viola") {
                auto& s = recipe.bowed;
                s.res[0] = { 225.0, 5.0, 8.0 }; s.res[1] = { 390.0, 4.0, 7.0 }; s.res[2] = { 460.0, 4.0, 7.0 }; s.res[3] = { 850.0, 2.5, 2.0 };
                s.res[4] = { 2200.0, 4.0, 1.2 }; s.cutoff_hz = 6500.0; s.attack_ms = 65.0; s.onset_ms = 38.0; s.vib_rate_hz = 5.3; s.vib_depth_cents = 18.0;
                s.bow_noise_center_hz = 3000.0; s.scrape_ms = 30.0; s.scoop_cents = 9.0; s.pluck_tau_s = 0.3; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.28;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 380.0;
            }
        };
        using Viola = SoloViola;

        class SoloCello : public BowedModelInstrument {
        public:
            SoloCello() : BowedModelInstrument("Solo Cello") {
                auto& s = recipe.bowed;
                s.res[0] = { 105.0, 6.0, 8.0 }; s.res[1] = { 170.0, 5.0, 7.0 }; s.res[2] = { 210.0, 5.0, 7.0 }; s.res[3] = { 600.0, 2.5, 2.0 };
                s.res[4] = { 1200.0, 3.0, 1.2 }; s.cutoff_hz = 5000.0; s.attack_ms = 80.0; s.onset_ms = 50.0; s.vib_rate_hz = 4.9; s.vib_depth_cents = 20.0;
                s.vib_delay_s = 0.25; s.bow_noise_db = -33.0; s.bow_noise_center_hz = 2200.0; s.scrape_ms = 40.0; s.scrape_center_hz = 1800.0;
                s.scoop_cents = 10.0; s.pluck_tau_s = 0.45; s.pluck_pos = 0.12; s.pluck_hf_damp = 0.22; s.pluck_gain = 0.8; s.pluck_thump_hz = 200.0;
            }
        };
        using Cello = SoloCello;

        class DoubleBass : public BowedModelInstrument {
        public:
            DoubleBass() : BowedModelInstrument("Double Bass") {
                auto& s = recipe.bowed;
                s.res[0] = { 62.0, 6.0, 8.0 }; s.res[1] = { 95.0, 5.0, 7.0 }; s.res[2] = { 120.0, 4.0, 7.0 }; s.res[3] = { 350.0, 2.5, 2.0 };
                s.res[4] = { 700.0, 3.0, 1.2 }; s.cutoff_hz = 3000.0; s.attack_ms = 110.0; s.onset_ms = 80.0; s.vib_rate_hz = 4.2; s.vib_depth_cents = 14.0;
                s.vib_delay_s = 0.3; s.bow_noise_db = -31.0; s.bow_noise_center_hz = 1200.0; s.scrape_ms = 60.0; s.scrape_center_hz = 900.0;
                s.scoop_cents = 12.0; s.release_ms = 160.0; s.pluck_tau_s = 0.7; s.pluck_pos = 0.18; s.pluck_hf_damp = 0.2; s.pluck_gain = 0.9;
                s.pluck_thump_hz = 100.0;
            }
        };

        // Jazz bass: more bow noise, longer pizzicato.
        class UprightBass : public BowedModelInstrument {
        public:
            UprightBass() : BowedModelInstrument("Upright Bass") {
                auto& s = recipe.bowed;
                s.res[0] = { 62.0, 6.0, 8.0 }; s.res[1] = { 95.0, 5.0, 7.0 }; s.res[2] = { 120.0, 4.0, 7.0 }; s.res[3] = { 350.0, 2.5, 2.0 };
                s.res[4] = { 700.0, 3.0, 1.2 }; s.cutoff_hz = 2600.0; s.attack_ms = 100.0; s.onset_ms = 80.0; s.vib_rate_hz = 4.2; s.vib_depth_cents = 10.0;
                s.vib_delay_s = 0.3; s.bow_noise_db = -29.0; s.bow_noise_center_hz = 1200.0; s.scrape_ms = 60.0; s.scrape_center_hz = 900.0;
                s.scoop_cents = 12.0; s.release_ms = 160.0; s.pluck_tau_s = 0.55; s.pluck_pos = 0.2; s.pluck_hf_damp = 0.2; s.pluck_gain = 0.9;
                s.pluck_thump_hz = 100.0;
            }
        };

        // Sections: independent detune, chorus drift, vibrato rate and phase, staggered onsets, spread across the stereo field.
        class StringEnsemble : public BowedModelInstrument {
        public:
            StringEnsemble() : BowedModelInstrument("String Ensemble") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 7000.0; s.attack_ms = 95.0; s.onset_ms = 60.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 12.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 6; s.ensemble_cents = 22.0; s.ensemble_stagger_ms = 35.0;
                s.ensemble_drift_cents = 4.0; recipe.stereo_width = 0.4; s.ensemble_vib_spread = 0.15;
            }
        };

        class ChamberStrings : public BowedModelInstrument {
        public:
            ChamberStrings() : BowedModelInstrument("Chamber Strings") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 8000.0; s.attack_ms = 70.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 14.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 4; s.ensemble_cents = 12.0; s.ensemble_stagger_ms = 18.0;
                s.ensemble_drift_cents = 3.0; recipe.stereo_width = 0.3;
            }
        };

        class FullStringSection : public BowedModelInstrument {
        public:
            FullStringSection() : BowedModelInstrument("Full String Section") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 7500.0; s.attack_ms = 110.0; s.onset_ms = 70.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 12.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 10; s.ensemble_cents = 28.0; s.ensemble_stagger_ms = 55.0;
                s.ensemble_drift_cents = 5.0; recipe.stereo_width = 0.5;
            }
        };
        using FullStringEnsemble = FullStringSection;

        class SymphonicStrings : public BowedModelInstrument {
        public:
            SymphonicStrings() : BowedModelInstrument("Symphonic Strings") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 7500.0; s.attack_ms = 125.0; s.onset_ms = 75.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 11.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 12; s.ensemble_cents = 34.0; s.ensemble_stagger_ms = 65.0;
                s.ensemble_drift_cents = 6.0; recipe.stereo_width = 0.6;
            }
        };

        class StringQuartet : public BowedModelInstrument {
        public:
            StringQuartet() : BowedModelInstrument("String Quartet") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 8500.0; s.attack_ms = 55.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 17.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 4; s.ensemble_cents = 8.0; s.ensemble_stagger_ms = 10.0;
                s.ensemble_drift_cents = 2.0; recipe.stereo_width = 0.28;
            }
        };

        // Gut strings, no bridge hill, nasal 1.3 kHz body mode, little vibrato.
        class ViolaDaGamba : public BowedModelInstrument {
        public:
            ViolaDaGamba() : BowedModelInstrument("Viola da Gamba") {
                auto& s = recipe.bowed;
                s.res[0] = { 190.0, 5.0, 8.0 }; s.res[1] = { 330.0, 4.0, 6.0 }; s.res[2] = { 1300.0, 4.0, 2.0 }; s.tilt_soft = 1.5; s.tilt_loud = 1.1;
                s.cutoff_hz = 3800.0; s.attack_ms = 75.0; s.onset_ms = 45.0; s.vib_depth_cents = 6.0; s.vib_delay_s = 0.4; s.bow_noise_db = -29.0;
                s.bow_noise_center_hz = 2500.0; s.pluck_tau_s = 0.35; s.pluck_gain = 0.75;
            }
        };

        class CelloDaSpalla : public BowedModelInstrument {
        public:
            CelloDaSpalla() : BowedModelInstrument("Cello da Spalla") {
                auto& s = recipe.bowed;
                s.res[0] = { 136.5, 6.0, 8.0 }; s.res[1] = { 221.0, 5.0, 7.0 }; s.res[2] = { 273.0, 5.0, 7.0 }; s.res[3] = { 780.0, 2.5, 2.0 };
                s.res[4] = { 1560.0, 3.0, 1.2 }; s.cutoff_hz = 4300.0; s.attack_ms = 65.0; s.onset_ms = 50.0; s.vib_rate_hz = 4.9; s.vib_depth_cents = 8.0;
                s.vib_delay_s = 0.25; s.bow_noise_db = -33.0; s.bow_noise_center_hz = 2200.0; s.scrape_ms = 40.0; s.scrape_center_hz = 1800.0;
                s.scoop_cents = 10.0; s.pluck_tau_s = 0.45; s.pluck_pos = 0.12; s.pluck_hf_damp = 0.22; s.pluck_gain = 0.8; s.pluck_thump_hz = 200.0;
            }
        };

        // Sympathetic strings approximated as a second, slightly detuned voice (estimate).
        class HardangerFiddle : public BowedModelInstrument {
        public:
            HardangerFiddle() : BowedModelInstrument("Hardanger Fiddle") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 3000.0, 6.0, 1.2 }; s.cutoff_hz = 9500.0; s.attack_ms = 45.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 6.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.bow_noise_db = -32.0; s.ensemble_voices = 2; s.ensemble_cents = 10.0; s.ensemble_drift_cents = 2.0;
                recipe.stereo_width = 0.3;
            }
        };

        // Keyed fiddle: nasal, with sympathetic drone strings (second voice).
        class Nyckelharpa : public BowedModelInstrument {
        public:
            Nyckelharpa() : BowedModelInstrument("Nyckelharpa") {
                auto& s = recipe.bowed;
                s.res[0] = { 225.0, 5.0, 8.0 }; s.res[1] = { 390.0, 4.0, 7.0 }; s.res[2] = { 460.0, 4.0, 7.0 }; s.res[3] = { 850.0, 2.5, 2.0 };
                s.res[4] = { 1200.0, 5.0, 3.0 }; s.cutoff_hz = 7500.0; s.attack_ms = 70.0; s.onset_ms = 38.0; s.vib_rate_hz = 5.3; s.vib_depth_cents = 4.0;
                s.bow_noise_center_hz = 3000.0; s.scrape_ms = 30.0; s.scoop_cents = 9.0; s.pluck_tau_s = 0.3; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.28;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 380.0; s.bow_noise_db = -31.0; s.ensemble_voices = 2; s.ensemble_cents = 12.0; s.ensemble_drift_cents = 2.0;
                recipe.stereo_width = 0.32;
            }
        };

        // Python-skin resonator: strong nasal body mode, no bridge hill.
        class Zhonghu : public BowedModelInstrument {
        public:
            Zhonghu() : BowedModelInstrument("Zhonghu") {
                auto& s = recipe.bowed;
                s.res[0] = { 350.0, 5.0, 6.0 }; s.res[1] = { 900.0, 6.0, 2.0 }; s.tilt_soft = 1.2; s.tilt_loud = 0.9; s.cutoff_hz = 4200.0;
                s.attack_ms = 55.0; s.vib_depth_cents = 22.0; s.bow_noise_db = -28.0; s.bow_noise_center_hz = 2500.0; s.pluck_tau_s = 0.25;
            }
        };

        // Gut strings and lower tension: softer tilt, little vibrato.
        class BaroqueViolin : public BowedModelInstrument {
        public:
            BaroqueViolin() : BowedModelInstrument("Baroque Violin") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2600.0, 3.0, 1.2 }; s.cutoff_hz = 6500.0; s.attack_ms = 65.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 6.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.tilt_soft = 1.5; s.tilt_loud = 1.1; s.bow_noise_db = -31.0;
            }
        };

        class BaroqueCello : public BowedModelInstrument {
        public:
            BaroqueCello() : BowedModelInstrument("Baroque Cello") {
                auto& s = recipe.bowed;
                s.res[0] = { 105.0, 6.0, 8.0 }; s.res[1] = { 170.0, 5.0, 7.0 }; s.res[2] = { 210.0, 5.0, 7.0 }; s.res[3] = { 600.0, 2.5, 2.0 };
                s.res[4] = { 1200.0, 3.0, 1.2 }; s.cutoff_hz = 3800.0; s.attack_ms = 90.0; s.onset_ms = 50.0; s.vib_rate_hz = 4.9; s.vib_depth_cents = 4.0;
                s.vib_delay_s = 0.25; s.bow_noise_db = -31.0; s.bow_noise_center_hz = 2200.0; s.scrape_ms = 40.0; s.scrape_center_hz = 1800.0;
                s.scoop_cents = 10.0; s.pluck_tau_s = 0.45; s.pluck_pos = 0.12; s.pluck_hf_damp = 0.22; s.pluck_gain = 0.8; s.pluck_thump_hz = 200.0;
                s.tilt_soft = 1.5; s.tilt_loud = 1.1;
            }
        };

        // Natural harmonics: nearly sinusoidal.
        class ViolinHarmonics : public BowedModelInstrument {
        public:
            ViolinHarmonics() : BowedModelInstrument("Violin Harmonics") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 9000.0; s.attack_ms = 55.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 8.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.tilt_soft = 2.8; s.tilt_loud = 2.4; s.bow_noise_db = -30.0; s.onset_tilt = 0.5;
                s.level = 0.25;
                recipe.art_pitch_shift[Articulation::Sustain] = 12.0;
            }
        };

        class CelloHarmonics : public BowedModelInstrument {
        public:
            CelloHarmonics() : BowedModelInstrument("Cello Harmonics") {
                auto& s = recipe.bowed;
                s.res[0] = { 105.0, 6.0, 8.0 }; s.res[1] = { 170.0, 5.0, 7.0 }; s.res[2] = { 210.0, 5.0, 7.0 }; s.res[3] = { 600.0, 2.5, 2.0 };
                s.res[4] = { 1200.0, 3.0, 1.2 }; s.cutoff_hz = 6000.0; s.attack_ms = 80.0; s.onset_ms = 50.0; s.vib_rate_hz = 4.9; s.vib_depth_cents = 8.0;
                s.vib_delay_s = 0.25; s.bow_noise_db = -30.0; s.bow_noise_center_hz = 2200.0; s.scrape_ms = 40.0; s.scrape_center_hz = 1800.0;
                s.scoop_cents = 10.0; s.pluck_tau_s = 0.45; s.pluck_pos = 0.12; s.pluck_hf_damp = 0.22; s.pluck_gain = 0.8; s.pluck_thump_hz = 200.0;
                s.tilt_soft = 2.8; s.tilt_loud = 2.4; s.onset_tilt = 0.5; s.level = 0.25;
                recipe.art_pitch_shift[Articulation::Sustain] = 12.0;
            }
        };

        // Rapid bow reversals (~13 Hz) with a scrape burst on each stroke.
        class TremoloStrings : public BowedModelInstrument {
        public:
            TremoloStrings() : BowedModelInstrument("Tremolo Strings") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 8000.0; s.attack_ms = 50.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 10.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 6; s.ensemble_cents = 16.0; s.ensemble_stagger_ms = 25.0;
                s.ensemble_drift_cents = 4.0; recipe.stereo_width = 0.4; s.level = 0.38;
            }
            StereoBuffer render_note(const wauvio::music::Note& note, int sample_rate = 0) const override {
                wauvio::music::Note n = note;
                if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Tremolo;
                return BowedStringInstrument::render_note(n, sample_rate);
            }
        };

        // Plucked-string bank: partial k decays at tau/(1+c(k-1)); pluck-position comb; finger thump.
        class PizzicatoStrings : public BowedModelInstrument {
        public:
            PizzicatoStrings() : BowedModelInstrument("Pizzicato Strings") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 8000.0; s.attack_ms = 55.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 20.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 6; s.ensemble_cents = 10.0; s.ensemble_stagger_ms = 6.0;
                s.ensemble_drift_cents = 2.0; recipe.stereo_width = 0.3;
            }
            StereoBuffer render_note(const wauvio::music::Note& note, int sample_rate = 0) const override {
                wauvio::music::Note n = note;
                if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Pizzicato;
                return BowedStringInstrument::render_note(n, sample_rate);
            }
        };

        class SpiccatoStrings : public BowedModelInstrument {
        public:
            SpiccatoStrings() : BowedModelInstrument("Spiccato Strings") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 8000.0; s.attack_ms = 55.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 20.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 6; s.ensemble_cents = 12.0; s.ensemble_stagger_ms = 12.0;
                s.ensemble_drift_cents = 2.0; recipe.stereo_width = 0.32;
            }
            StereoBuffer render_note(const wauvio::music::Note& note, int sample_rate = 0) const override {
                wauvio::music::Note n = note;
                if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Spiccato;
                return BowedStringInstrument::render_note(n, sample_rate);
            }
        };

    }
}
