#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        // Straight mute: nasal 1.5 kHz peak, reduced brassiness.
        class MutedTrumpet : public LipReedInstrument {
        public:
            MutedTrumpet() : LipReedInstrument("Muted Trumpet") {
                auto& w = recipe.wind;
                w.g_loud = 0.9; w.bell_hz = 1200.0; w.bell_depth = 0.6; w.cutoff_hz = 4200.0; w.res[0] = { 1500.0, 9.0, 3.0 }; w.res[1] = { 2600.0, 4.0, 2.0 };
                w.attack_ms = 28.0; w.vib_depth_cents = 4.0;
            }
        };

        // Wa-wa mute stem in: strong narrow resonance near 2.3 kHz, cut low end.
        class HarmonMuteTrumpet : public LipReedInstrument {
        public:
            HarmonMuteTrumpet() : LipReedInstrument("Harmon Mute Trumpet") {
                auto& w = recipe.wind;
                w.g_loud = 0.9; w.bell_hz = 1300.0; w.bell_depth = 0.5; w.cutoff_hz = 5000.0; w.res[0] = { 2300.0, 11.0, 5.0 }; w.res[1] = { 600.0, -8.0, 1.5 };
                w.attack_ms = 36.0; w.vib_depth_cents = 3.0;
            }
        };

        class MutedTrombone : public LipReedInstrument {
        public:
            MutedTrombone() : LipReedInstrument("Muted Trombone") {
                auto& w = recipe.wind;
                w.g_loud = 0.85; w.bell_hz = 700.0; w.bell_depth = 0.6; w.cutoff_hz = 3200.0; w.res[0] = { 1200.0, 8.0, 3.0 }; w.res[1] = { 500.0, -3.0, 1.2 };
                w.attack_ms = 45.0; w.vib_depth_cents = 8.0;
            }
        };

        // Valveless: notes come only from lip tension, so pitch is unsteady.
        class Bugle : public LipReedInstrument {
        public:
            Bugle() : LipReedInstrument("Bugle") {
                auto& w = recipe.wind;
                w.bell_hz = 800.0; w.cutoff_hz = 4800.0; w.res[0] = { 1300.0, 3.0, 1.0 }; w.attack_ms = 28.0; w.vib_depth_cents = 0.0; w.jitter_cents = 4.5;
            }
        };

        class Mellophone : public LipReedInstrument {
        public:
            Mellophone() : LipReedInstrument("Mellophone") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.9; w.tilt_loud = 0.5; w.bell_depth = 0.7; w.bell_hz = 600.0;
                w.cutoff_hz = 3800.0; w.res[0] = { 900.0, 4.0, 1.0 }; w.attack_ms = 38.0; w.vib_depth_cents = 5.0;
            }
        };

        class Cimbasso : public LipReedInstrument {
        public:
            Cimbasso() : LipReedInstrument("Cimbasso") {
                auto& w = recipe.wind;
                w.bell_hz = 380.0; w.cutoff_hz = 2800.0; w.res[0] = { 480.0, 4.0, 1.0 }; w.attack_ms = 52.0; w.vib_depth_cents = 4.0; w.release_ms = 90.0;
                w.tilt_soft = 0.8; w.tilt_loud = 0.4;
            }
        };

        // Wooden finger-holed lip reed: sweet and breathy.
        class Cornett : public LipReedInstrument {
        public:
            Cornett() : LipReedInstrument("Cornett") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.55; w.tilt_soft = 0.9; w.tilt_loud = 0.6; w.bell_depth = 0.0; w.cutoff_hz = 3300.0; w.res[0] = { 1000.0, 3.0, 1.0 };
                w.breath_db = -30.0; w.attack_ms = 38.0; w.vib_depth_cents = 6.0; w.vib_rate_hz = 5.5; w.jitter_cents = 3.0;
            }
        };

        class PostHorn : public LipReedInstrument {
        public:
            PostHorn() : LipReedInstrument("Post Horn") {
                auto& w = recipe.wind;
                w.bell_hz = 1000.0; w.cutoff_hz = 5800.0; w.res[0] = { 1500.0, 3.0, 1.0 }; w.attack_ms = 26.0; w.vib_depth_cents = 0.0; w.jitter_cents = 3.5;
            }
        };

        class TenorHorn : public LipReedInstrument {
        public:
            TenorHorn() : LipReedInstrument("Tenor Horn") {
                auto& w = recipe.wind;
                w.g_soft = 0.12; w.g_loud = 0.8; w.g_pow = 2.0; w.tilt_soft = 0.9; w.tilt_loud = 0.5; w.bell_depth = 0.7; w.bell_hz = 550.0;
                w.cutoff_hz = 3600.0; w.res[0] = { 800.0, 3.0, 1.0 }; w.attack_ms = 40.0; w.vib_depth_cents = 6.0;
            }
        };

        // Four detuned, staggered voices.
        class FanfareTrumpets : public LipReedInstrument {
        public:
            FanfareTrumpets() : LipReedInstrument("Fanfare Trumpets") {
                auto& w = recipe.wind;
                w.bell_hz = 900.0; w.cutoff_hz = 6800.0; w.res[0] = { 1500.0, 4.0, 1.0 }; w.attack_ms = 34.0; w.ensemble_voices = 4; w.ensemble_cents = 14.0;
                w.ensemble_stagger_ms = 14.0; recipe.stereo_width = 0.35; w.vib_depth_cents = 0.0;
            }
        };

    }
}
