#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        // Cylindrical-bore trumpet: strong steepening, bright at ff, ~24-35 ms attack, fundamental weak, higher partials enter later in the attack.
        class PiccoloTrumpet : public LipReedInstrument {
        public:
            PiccoloTrumpet() : LipReedInstrument("Piccolo Trumpet") {
                auto& w = recipe.wind;
                w.g_soft = 0.3; w.bell_hz = 1100.0; w.cutoff_hz = 8000.0; w.res[0] = { 2000.0, 3.0, 1.0 }; w.attack_ms = 24.0; w.scoop_cents = 10.0;
                w.vib_depth_cents = 5.0; w.release_ms = 55.0;
            }
        };

        class Trumpet : public LipReedInstrument {
        public:
            Trumpet() : LipReedInstrument("Trumpet") {
                auto& w = recipe.wind;
                w.cutoff_hz = 6200.0; w.res[0] = { 1300.0, 3.0, 1.0 }; w.attack_ms = 32.0;
            }
        };

        // More conical than the trumpet: mellower.
        class Cornet : public LipReedInstrument {
        public:
            Cornet() : LipReedInstrument("Cornet") {
                auto& w = recipe.wind;
                w.g_loud = 0.92; w.bell_hz = 800.0; w.cutoff_hz = 4600.0; w.res[0] = { 1100.0, 3.0, 1.0 }; w.attack_ms = 32.0;
            }
        };

        // Mostly conical: dark and round.
        class Flugelhorn : public LipReedInstrument {
        public:
            Flugelhorn() : LipReedInstrument("Flugelhorn") {
                auto& w = recipe.wind;
                w.g_soft = 0.15; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.8; w.tilt_loud = 0.5; w.bell_depth = 0.7; w.bell_hz = 650.0;
                w.cutoff_hz = 3000.0; w.res[0] = { 900.0, 3.0, 1.0 }; w.attack_ms = 42.0;
            }
        };

        class BassTrumpet : public LipReedInstrument {
        public:
            BassTrumpet() : LipReedInstrument("Bass Trumpet") {
                auto& w = recipe.wind;
                w.bell_hz = 550.0; w.cutoff_hz = 3400.0; w.res[0] = { 750.0, 3.0, 1.0 }; w.attack_ms = 40.0; w.release_ms = 80.0;
            }
        };

        // Slide trombone: slower lip start, wider vibrato.
        class Trombone : public LipReedInstrument {
        public:
            Trombone() : LipReedInstrument("Trombone") {
                auto& w = recipe.wind;
                w.bell_hz = 650.0; w.cutoff_hz = 4200.0; w.res[0] = { 600.0, 3.0, 1.0 }; w.attack_ms = 42.0; w.scoop_cents = 18.0; w.scoop_ms = 50.0;
                w.vib_depth_cents = 11.0; w.vib_rate_hz = 5.3; w.vib_delay_s = 0.5;
            }
        };

        class BassTrombone : public LipReedInstrument {
        public:
            BassTrombone() : LipReedInstrument("Bass Trombone") {
                auto& w = recipe.wind;
                w.bell_hz = 450.0; w.cutoff_hz = 3000.0; w.res[0] = { 450.0, 3.0, 1.0 }; w.attack_ms = 55.0; w.scoop_cents = 18.0; w.scoop_ms = 55.0;
                w.vib_depth_cents = 9.0; w.release_ms = 90.0;
            }
        };

        // Long conical tube, hand in the bell: soft rounded onset, scooped lip start, darker spectrum.
        class FrenchHorn : public LipReedInstrument {
        public:
            FrenchHorn() : LipReedInstrument("French Horn") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.85; w.g_pow = 2.0; w.tilt_soft = 1.0; w.tilt_loud = 0.6; w.bell_depth = 0.6; w.attack_shape = 1.6;
                w.attack_vel_exp = 0.4; w.jitter_cents = 3.5; w.bell_hz = 500.0; w.cutoff_hz = 2800.0; w.res[0] = { 450.0, 4.0, 1.2 }; w.attack_ms = 55.0;
                w.scoop_cents = 25.0; w.scoop_ms = 70.0; w.vib_depth_cents = 5.0; w.release_ms = 90.0;
            }
        };

        // Four detuned, staggered voices.
        class FrenchHornEnsemble : public LipReedInstrument {
        public:
            FrenchHornEnsemble() : LipReedInstrument("French Horn Ensemble") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.85; w.g_pow = 2.0; w.tilt_soft = 1.0; w.tilt_loud = 0.6; w.bell_depth = 0.6; w.attack_shape = 1.6;
                w.attack_vel_exp = 0.4; w.jitter_cents = 3.5; w.bell_hz = 500.0; w.cutoff_hz = 2800.0; w.res[0] = { 450.0, 4.0, 1.2 }; w.attack_ms = 60.0;
                w.scoop_cents = 25.0; w.scoop_ms = 70.0; w.vib_depth_cents = 4.0; w.release_ms = 100.0; w.ensemble_voices = 4; w.ensemble_cents = 14.0;
                w.ensemble_stagger_ms = 18.0; recipe.stereo_width = 0.35;
            }
        };

        class Euphonium : public LipReedInstrument {
        public:
            Euphonium() : LipReedInstrument("Euphonium") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.9; w.tilt_loud = 0.5; w.bell_depth = 0.7; w.bell_hz = 400.0;
                w.cutoff_hz = 2600.0; w.res[0] = { 420.0, 3.0, 1.0 }; w.attack_ms = 48.0; w.vib_depth_cents = 8.0; w.release_ms = 85.0;
            }
        };

        class BaritoneHorn : public LipReedInstrument {
        public:
            BaritoneHorn() : LipReedInstrument("Baritone Horn") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.9; w.tilt_loud = 0.5; w.bell_depth = 0.7; w.bell_hz = 420.0;
                w.cutoff_hz = 2800.0; w.res[0] = { 400.0, 3.0, 1.0 }; w.attack_ms = 46.0; w.vib_depth_cents = 7.0; w.release_ms = 85.0;
            }
        };

        // Large bore: slow lip start (~80 ms), low cutoff.
        class Tuba : public LipReedInstrument {
        public:
            Tuba() : LipReedInstrument("Tuba") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.8; w.tilt_loud = 0.4; w.bell_depth = 0.7; w.bell_hz = 230.0;
                w.cutoff_hz = 1500.0; w.cutoff_loud_gain = 1.2; w.res[0] = { 230.0, 4.0, 1.0 }; w.attack_ms = 80.0; w.attack_shape = 1.5;
                w.scoop_cents = 28.0; w.scoop_ms = 90.0; w.vib_depth_cents = 4.0; w.release_ms = 110.0; w.jitter_cents = 4.0;
            }
        };

        class Sousaphone : public LipReedInstrument {
        public:
            Sousaphone() : LipReedInstrument("Sousaphone") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.8; w.tilt_loud = 0.4; w.bell_depth = 0.7; w.bell_hz = 210.0;
                w.cutoff_hz = 1400.0; w.cutoff_loud_gain = 1.2; w.res[0] = { 210.0, 4.0, 1.0 }; w.attack_ms = 95.0; w.attack_shape = 1.5;
                w.scoop_cents = 30.0; w.scoop_ms = 100.0; w.vib_depth_cents = 3.0; w.release_ms = 120.0; w.jitter_cents = 4.0; recipe.stereo_width = 0.2;
            }
        };

        // Horn-like mouthpiece on a tuba-like tube.
        class WagnerTuba : public LipReedInstrument {
        public:
            WagnerTuba() : LipReedInstrument("Wagner Tuba") {
                auto& w = recipe.wind;
                w.g_soft = 0.15; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 1.0; w.tilt_loud = 0.6; w.bell_depth = 0.6; w.attack_shape = 1.6;
                w.attack_vel_exp = 0.4; w.jitter_cents = 3.5; w.bell_hz = 300.0; w.cutoff_hz = 2200.0; w.res[0] = { 350.0, 4.0, 1.1 }; w.attack_ms = 60.0;
                w.scoop_cents = 20.0; w.scoop_ms = 70.0; w.vib_depth_cents = 5.0; w.release_ms = 95.0;
            }
        };

        class BassCornet : public LipReedInstrument {
        public:
            BassCornet() : LipReedInstrument("Bass Cornet") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.9; w.tilt_loud = 0.5; w.bell_depth = 0.7; w.bell_hz = 480.0;
                w.cutoff_hz = 3000.0; w.res[0] = { 480.0, 3.0, 1.0 }; w.attack_ms = 44.0; w.vib_depth_cents = 7.0; w.release_ms = 80.0;
            }
        };

        // Four detuned, staggered voices.
        class BrassSection : public LipReedInstrument {
        public:
            BrassSection() : LipReedInstrument("Brass Section") {
                auto& w = recipe.wind;
                w.bell_hz = 800.0; w.cutoff_hz = 5200.0; w.res[0] = { 1100.0, 3.0, 1.0 }; w.attack_ms = 40.0; w.ensemble_voices = 4; w.ensemble_cents = 18.0;
                w.ensemble_stagger_ms = 16.0; recipe.stereo_width = 0.4; w.vib_depth_cents = 4.0;
            }
        };

        class LowBrassSection : public LipReedInstrument {
        public:
            LowBrassSection() : LipReedInstrument("Low Brass Section") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.9; w.tilt_loud = 0.5; w.bell_depth = 0.7; w.bell_hz = 400.0;
                w.cutoff_hz = 2800.0; w.res[0] = { 400.0, 3.0, 1.0 }; w.attack_ms = 70.0; w.ensemble_voices = 4; w.ensemble_cents = 18.0;
                w.ensemble_stagger_ms = 22.0; recipe.stereo_width = 0.4; w.vib_depth_cents = 4.0; w.release_ms = 100.0;
            }
        };

        // Hand-stopping only, no valves: slightly breathier and less steady than the valved horn.
        class NaturalHorn : public LipReedInstrument {
        public:
            NaturalHorn() : LipReedInstrument("Natural Horn") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.75; w.g_pow = 2.0; w.tilt_soft = 1.0; w.tilt_loud = 0.6; w.bell_depth = 0.6; w.attack_shape = 1.6;
                w.attack_vel_exp = 0.4; w.jitter_cents = 4.0; w.bell_hz = 450.0; w.cutoff_hz = 2600.0; w.res[0] = { 450.0, 4.0, 1.2 }; w.attack_ms = 60.0;
                w.scoop_cents = 22.0; w.scoop_ms = 70.0; w.vib_depth_cents = 3.0; w.breath_db = -36.0;
            }
        };

        // Long wooden conical tube without a flared bell: very dull, slow to speak.
        class Alphorn : public LipReedInstrument {
        public:
            Alphorn() : LipReedInstrument("Alphorn") {
                auto& w = recipe.wind;
                w.g_soft = 0.1; w.g_loud = 0.55; w.g_pow = 1.6; w.tilt_soft = 1.2; w.tilt_loud = 0.8; w.bell_depth = 0.0; w.cutoff_hz = 1200.0;
                w.res[0] = { 220.0, 4.0, 1.0 }; w.attack_ms = 110.0; w.attack_shape = 1.5; w.scoop_cents = 30.0; w.scoop_ms = 100.0; w.vib_depth_cents = 0.0;
                w.release_ms = 140.0; w.breath_db = -34.0; w.jitter_cents = 5.0;
            }
        };

        class BaroqueTrumpet : public LipReedInstrument {
        public:
            BaroqueTrumpet() : LipReedInstrument("Baroque Trumpet") {
                auto& w = recipe.wind;
                w.bell_hz = 900.0; w.cutoff_hz = 5200.0; w.res[0] = { 1500.0, 3.0, 1.0 }; w.attack_ms = 36.0; w.vib_depth_cents = 4.0; w.jitter_cents = 3.0;
                w.breath_db = -37.0;
            }
        };

        // Sackbut: narrower bore, gentler.
        class BaroqueTrombone : public LipReedInstrument {
        public:
            BaroqueTrombone() : LipReedInstrument("Baroque Trombone") {
                auto& w = recipe.wind;
                w.bell_hz = 500.0; w.cutoff_hz = 3200.0; w.res[0] = { 600.0, 3.0, 1.0 }; w.attack_ms = 48.0; w.vib_depth_cents = 3.0; w.jitter_cents = 3.0;
                w.breath_db = -37.0; w.release_ms = 80.0;
            }
        };

        // Wooden, finger-holed, cup mouthpiece: dull and unsteady.
        class Serpent : public LipReedInstrument {
        public:
            Serpent() : LipReedInstrument("Serpent") {
                auto& w = recipe.wind;
                w.g_soft = 0.15; w.g_loud = 0.7; w.g_pow = 2.0; w.tilt_soft = 0.9; w.tilt_loud = 0.5; w.bell_depth = 0.0; w.cutoff_hz = 1500.0;
                w.res[0] = { 260.0, 4.0, 1.0 }; w.attack_ms = 90.0; w.breath_db = -30.0; w.vib_depth_cents = 0.0; w.release_ms = 110.0; w.jitter_cents = 6.0;
                w.scoop_cents = 26.0; w.scoop_ms = 90.0;
            }
        };

        class Ophicleide : public LipReedInstrument {
        public:
            Ophicleide() : LipReedInstrument("Ophicleide") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.9; w.tilt_loud = 0.5; w.bell_depth = 0.7; w.bell_hz = 330.0;
                w.cutoff_hz = 2100.0; w.res[0] = { 320.0, 4.0, 1.0 }; w.attack_ms = 70.0; w.breath_db = -34.0; w.jitter_cents = 4.0; w.vib_depth_cents = 3.0;
                w.release_ms = 100.0;
            }
        };

    }
}
