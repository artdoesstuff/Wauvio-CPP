#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class FolkFiddle : public BowedModelInstrument {
        public:
            FolkFiddle() : BowedModelInstrument("Folk Fiddle") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 8000.0; s.attack_ms = 45.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 8.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.bow_noise_db = -30.0;
            }
        };

        // Sympathetic strings approximated as a second voice (estimate).
        class ViolaDAmore : public BowedModelInstrument {
        public:
            ViolaDAmore() : BowedModelInstrument("Viola d'Amore") {
                auto& s = recipe.bowed;
                s.res[0] = { 225.0, 5.0, 8.0 }; s.res[1] = { 390.0, 4.0, 7.0 }; s.res[2] = { 460.0, 4.0, 7.0 }; s.res[3] = { 850.0, 2.5, 2.0 };
                s.res[4] = { 2500.0, 4.0, 1.2 }; s.cutoff_hz = 6000.0; s.attack_ms = 75.0; s.onset_ms = 38.0; s.vib_rate_hz = 5.3; s.vib_depth_cents = 18.0;
                s.bow_noise_center_hz = 3000.0; s.scrape_ms = 30.0; s.scoop_cents = 9.0; s.pluck_tau_s = 0.3; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.28;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 380.0; s.tilt_soft = 1.5; s.tilt_loud = 1.1; s.ensemble_voices = 2; s.ensemble_cents = 6.0;
                s.ensemble_drift_cents = 1.5; recipe.stereo_width = 0.3;
            }
        };

        class Violone : public BowedModelInstrument {
        public:
            Violone() : BowedModelInstrument("Violone") {
                auto& s = recipe.bowed;
                s.res[0] = { 80.6, 6.0, 8.0 }; s.res[1] = { 123.5, 5.0, 7.0 }; s.res[2] = { 156.0, 4.0, 7.0 }; s.res[3] = { 455.0, 2.5, 2.0 };
                s.res[4] = { 910.0, 3.0, 1.2 }; s.cutoff_hz = 2800.0; s.attack_ms = 100.0; s.onset_ms = 80.0; s.vib_rate_hz = 4.2; s.vib_depth_cents = 14.0;
                s.vib_delay_s = 0.3; s.bow_noise_db = -31.0; s.bow_noise_center_hz = 1200.0; s.scrape_ms = 60.0; s.scrape_center_hz = 900.0;
                s.scoop_cents = 12.0; s.release_ms = 160.0; s.pluck_tau_s = 0.7; s.pluck_pos = 0.18; s.pluck_hf_damp = 0.2; s.pluck_gain = 0.9;
                s.pluck_thump_hz = 100.0; s.tilt_soft = 1.5; s.tilt_loud = 1.1;
            }
        };

        class Kemenche : public BowedModelInstrument {
        public:
            Kemenche() : BowedModelInstrument("Kemenche") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 1500.0, 5.0, 2.0 }; s.cutoff_hz = 6500.0; s.attack_ms = 45.0; s.onset_ms = 30.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 14.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.bow_noise_db = -30.0;
            }
        };

        // Skin belly and sympathetic strings (approximated as three slightly detuned voices; estimate).
        class Sarangi : public BowedModelInstrument {
        public:
            Sarangi() : BowedModelInstrument("Sarangi") {
                auto& s = recipe.bowed;
                s.res[0] = { 260.0, 5.0, 6.0 }; s.res[1] = { 1300.0, 6.0, 3.0 }; s.res[2] = { 2600.0, 3.0, 1.5 }; s.tilt_soft = 1.2; s.tilt_loud = 0.9;
                s.cutoff_hz = 5200.0; s.attack_ms = 55.0; s.vib_depth_cents = 16.0; s.bow_noise_db = -28.0; s.ensemble_voices = 3; s.ensemble_cents = 14.0;
                s.ensemble_drift_cents = 2.0; recipe.stereo_width = 0.3;
            }
        };

        // Skin-faced body: nasal, noisy.
        class Rebab : public BowedModelInstrument {
        public:
            Rebab() : BowedModelInstrument("Rebab") {
                auto& s = recipe.bowed;
                s.res[0] = { 400.0, 4.0, 6.0 }; s.res[1] = { 1200.0, 6.0, 2.0 }; s.tilt_soft = 1.2; s.tilt_loud = 0.9; s.cutoff_hz = 3800.0;
                s.attack_ms = 50.0; s.vib_depth_cents = 10.0; s.bow_noise_db = -27.0; s.bow_noise_center_hz = 2400.0;
            }
        };

        class MorinKhuur : public BowedModelInstrument {
        public:
            MorinKhuur() : BowedModelInstrument("Morin Khuur") {
                auto& s = recipe.bowed;
                s.res[0] = { 250.0, 5.0, 6.0 }; s.res[1] = { 600.0, 4.0, 2.0 }; s.cutoff_hz = 3300.0; s.attack_ms = 90.0; s.vib_depth_cents = 6.0;
                s.bow_noise_db = -28.0; s.bow_noise_center_hz = 2000.0; s.onset_ms = 55.0;
            }
        };

        // Giant bass: very slow string speech.
        class Octobass : public BowedModelInstrument {
        public:
            Octobass() : BowedModelInstrument("Octobass") {
                auto& s = recipe.bowed;
                s.res[0] = { 35.0, 6.0, 8.0 }; s.res[1] = { 55.0, 5.0, 7.0 }; s.res[2] = { 80.0, 3.0, 2.0 }; s.cutoff_hz = 1200.0; s.attack_ms = 200.0;
                s.onset_ms = 140.0; s.bow_noise_db = -30.0; s.bow_noise_center_hz = 700.0; s.scrape_center_hz = 600.0; s.scrape_ms = 90.0;
                s.vib_rate_hz = 3.5; s.vib_depth_cents = 3.0; s.vib_delay_s = 0.4; s.release_ms = 220.0; s.scoop_cents = 14.0;
            }
        };

        class Rebec : public BowedModelInstrument {
        public:
            Rebec() : BowedModelInstrument("Rebec") {
                auto& s = recipe.bowed;
                s.res[0] = { 500.0, 4.0, 6.0 }; s.res[1] = { 1400.0, 5.0, 2.0 }; s.cutoff_hz = 5500.0; s.attack_ms = 45.0; s.vib_depth_cents = 5.0;
                s.bow_noise_db = -29.0;
            }
        };

        class BowedPsaltery : public BowedModelInstrument {
        public:
            BowedPsaltery() : BowedModelInstrument("Bowed Psaltery") {
                auto& s = recipe.bowed;
                s.res[0] = { 800.0, 3.0, 2.0 }; s.tilt_soft = 1.8; s.tilt_loud = 1.4; s.cutoff_hz = 5000.0; s.attack_ms = 120.0; s.vib_depth_cents = 0.0;
                s.bow_noise_db = -30.0; s.onset_ms = 60.0; recipe.stereo_width = 0.3;
            }
        };

        class ViolinSection : public BowedModelInstrument {
        public:
            ViolinSection() : BowedModelInstrument("Violin Section") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, 5.0, 1.2 }; s.cutoff_hz = 8000.0; s.attack_ms = 85.0; s.onset_ms = 50.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 13.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 8; s.ensemble_cents = 24.0; s.ensemble_stagger_ms = 40.0;
                s.ensemble_drift_cents = 4.0; recipe.stereo_width = 0.45;
            }
        };

        class ViolaSection : public BowedModelInstrument {
        public:
            ViolaSection() : BowedModelInstrument("Viola Section") {
                auto& s = recipe.bowed;
                s.res[0] = { 225.0, 5.0, 8.0 }; s.res[1] = { 390.0, 4.0, 7.0 }; s.res[2] = { 460.0, 4.0, 7.0 }; s.res[3] = { 850.0, 2.5, 2.0 };
                s.res[4] = { 2200.0, 4.0, 1.2 }; s.cutoff_hz = 6500.0; s.attack_ms = 95.0; s.onset_ms = 55.0; s.vib_rate_hz = 5.3; s.vib_depth_cents = 12.0;
                s.bow_noise_center_hz = 3000.0; s.scrape_ms = 30.0; s.scoop_cents = 9.0; s.pluck_tau_s = 0.3; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.28;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 380.0; s.ensemble_voices = 8; s.ensemble_cents = 24.0; s.ensemble_stagger_ms = 40.0;
                s.ensemble_drift_cents = 4.0; recipe.stereo_width = 0.45;
            }
        };

        class CelloSection : public BowedModelInstrument {
        public:
            CelloSection() : BowedModelInstrument("Cello Section") {
                auto& s = recipe.bowed;
                s.res[0] = { 105.0, 6.0, 8.0 }; s.res[1] = { 170.0, 5.0, 7.0 }; s.res[2] = { 210.0, 5.0, 7.0 }; s.res[3] = { 600.0, 2.5, 2.0 };
                s.res[4] = { 1200.0, 3.0, 1.2 }; s.cutoff_hz = 5000.0; s.attack_ms = 100.0; s.onset_ms = 65.0; s.vib_rate_hz = 4.9; s.vib_depth_cents = 13.0;
                s.vib_delay_s = 0.25; s.bow_noise_db = -33.0; s.bow_noise_center_hz = 2200.0; s.scrape_ms = 40.0; s.scrape_center_hz = 1800.0;
                s.scoop_cents = 10.0; s.pluck_tau_s = 0.45; s.pluck_pos = 0.12; s.pluck_hf_damp = 0.22; s.pluck_gain = 0.8; s.pluck_thump_hz = 200.0;
                s.ensemble_voices = 8; s.ensemble_cents = 20.0; s.ensemble_stagger_ms = 45.0; s.ensemble_drift_cents = 4.0; recipe.stereo_width = 0.42;
            }
        };

        class DoubleBassSection : public BowedModelInstrument {
        public:
            DoubleBassSection() : BowedModelInstrument("Double Bass Section") {
                auto& s = recipe.bowed;
                s.res[0] = { 62.0, 6.0, 8.0 }; s.res[1] = { 95.0, 5.0, 7.0 }; s.res[2] = { 120.0, 4.0, 7.0 }; s.res[3] = { 350.0, 2.5, 2.0 };
                s.res[4] = { 700.0, 3.0, 1.2 }; s.cutoff_hz = 3000.0; s.attack_ms = 130.0; s.onset_ms = 90.0; s.vib_rate_hz = 4.2; s.vib_depth_cents = 9.0;
                s.vib_delay_s = 0.3; s.bow_noise_db = -31.0; s.bow_noise_center_hz = 1200.0; s.scrape_ms = 60.0; s.scrape_center_hz = 900.0;
                s.scoop_cents = 12.0; s.release_ms = 160.0; s.pluck_tau_s = 0.7; s.pluck_pos = 0.18; s.pluck_hf_damp = 0.2; s.pluck_gain = 0.9;
                s.pluck_thump_hz = 100.0; s.ensemble_voices = 6; s.ensemble_cents = 18.0; s.ensemble_stagger_ms = 55.0; s.ensemble_drift_cents = 3.0;
                recipe.stereo_width = 0.4;
            }
        };

        // Con sordino: the mute damps the bridge hill and high partials.
        class MutedStrings : public BowedModelInstrument {
        public:
            MutedStrings() : BowedModelInstrument("Muted Strings") {
                auto& s = recipe.bowed;
                s.res[0] = { 275.0, 5.0, 8.0 }; s.res[1] = { 470.0, 5.0, 7.0 }; s.res[2] = { 560.0, 5.0, 7.0 }; s.res[3] = { 1000.0, 2.5, 2.0 };
                s.res[4] = { 2800.0, -2.0, 1.2 }; s.cutoff_hz = 3200.0; s.attack_ms = 100.0; s.onset_ms = 55.0; s.vib_rate_hz = 5.6; s.vib_depth_cents = 10.0;
                s.bow_noise_center_hz = 3500.0; s.scrape_ms = 25.0; s.scoop_cents = 8.0; s.pluck_tau_s = 0.22; s.pluck_pos = 0.13; s.pluck_hf_damp = 0.3;
                s.pluck_gain = 0.75; s.pluck_thump_hz = 450.0; s.ensemble_voices = 6; s.ensemble_cents = 20.0; s.ensemble_stagger_ms = 35.0;
                s.ensemble_drift_cents = 4.0; recipe.stereo_width = 0.4; s.bow_noise_db = -36.0;
            }
        };

    }
}
