#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        // Small cylindrical flute: loud and bright at ff.
        class Fife : public AirJetInstrument {
        public:
            Fife() : AirJetInstrument("Fife") {
                auto& w = recipe.wind;
                w.tilt_soft = 0.8; w.tilt_loud = 0.8; w.g_soft = 0.35; w.g_loud = 0.7; w.cutoff_hz = 6500.0; w.breath_db = -34.0; w.chiff_db = -20.0;
                w.chiff_center_hz = 3800.0; w.chiff_ms = 14.0; w.attack_ms = 22.0; w.vib_depth_cents = 4.0; w.pressure_pitch_cents = 20.0;
                w.scoop_cents = 8.0;
            }
        };

        // Notched end-blown flute: breathy, scooped onset.
        class Quena : public AirJetInstrument {
        public:
            Quena() : AirJetInstrument("Quena") {
                auto& w = recipe.wind;
                w.g_soft = 0.22; w.g_loud = 0.4; w.cutoff_hz = 3600.0; w.breath_db = -22.0; w.breath_center_hz = 2200.0; w.breath_follow = 0.4;
                w.breath_tone_mod = 0.5; w.chiff_db = -16.0; w.chiff_ms = 45.0; w.chiff_center_hz = 2200.0; w.attack_ms = 70.0; w.vib_depth_cents = 14.0;
                w.vib_rate_hz = 5.2; w.vib_delay_s = 0.5; w.vib_am = 0.15; w.scoop_cents = 18.0; w.scoop_ms = 60.0;
            }
        };

        class Kaval : public AirJetInstrument {
        public:
            Kaval() : AirJetInstrument("Kaval") {
                auto& w = recipe.wind;
                w.g_soft = 0.25; w.g_loud = 0.45; w.cutoff_hz = 3200.0; w.breath_db = -24.0; w.breath_center_hz = 2000.0; w.chiff_db = -18.0;
                w.chiff_ms = 40.0; w.chiff_center_hz = 2000.0; w.attack_ms = 80.0; w.vib_depth_cents = 22.0; w.vib_rate_hz = 5.5; w.vib_am = 0.2;
                w.scoop_cents = 14.0;
            }
        };

        class BassetHorn : public SingleReedInstrument {
        public:
            BassetHorn() : SingleReedInstrument("Basset Horn") {
                auto& w = recipe.wind;
                w.cutoff_hz = 1500.0; w.even_open_hz = 1000.0; w.attack_ms = 32.0; w.breath_center_hz = 1600.0; w.chiff_center_hz = 600.0;
                w.vib_depth_cents = 7.0; w.release_ms = 75.0;
            }
        };

        class AltoClarinet : public SingleReedInstrument {
        public:
            AltoClarinet() : SingleReedInstrument("Alto Clarinet") {
                auto& w = recipe.wind;
                w.cutoff_hz = 1500.0; w.even_open_hz = 1050.0; w.attack_ms = 34.0; w.breath_center_hz = 1600.0; w.chiff_center_hz = 600.0;
                w.vib_depth_cents = 7.0; w.release_ms = 75.0;
            }
        };

        class ClarinetInA : public SingleReedInstrument {
        public:
            ClarinetInA() : SingleReedInstrument("Clarinet in A") {
                auto& w = recipe.wind;
                w.cutoff_hz = 1900.0; w.even_open_hz = 1700.0; w.attack_ms = 28.0;
            }
        };

        class OboeDAmore : public DoubleReedInstrument {
        public:
            OboeDAmore() : DoubleReedInstrument("Oboe d'Amore") {
                auto& w = recipe.wind;
                w.res[0] = { 950.0, 7.0, 1.4 }; w.res[1] = { 2600.0, 3.0, 2.0 }; w.cutoff_hz = 2800.0; w.attack_ms = 28.0; w.vib_depth_cents = 11.0;
            }
        };

        class SopraninoSaxophone : public SaxophoneInstrument {
        public:
            SopraninoSaxophone() : SaxophoneInstrument("Sopranino Saxophone") {
                auto& w = recipe.wind;
                w.cutoff_hz = 2300.0; w.res[0] = { 2200.0, 5.0, 1.2 }; w.res[1] = { 4000.0, 2.0, 1.8 }; w.attack_ms = 26.0; w.breath_center_hz = 3000.0;
                w.vib_depth_cents = 14.0;
            }
        };

        // Pirouette-free reed: loud, bright, no vibrato, abrupt onset.
        class Shawm : public DoubleReedInstrument {
        public:
            Shawm() : DoubleReedInstrument("Shawm") {
                auto& w = recipe.wind;
                w.tilt_soft = 0.5; w.tilt_loud = 0.2; w.g_soft = 0.93; w.g_loud = 0.99; w.cutoff_hz = 5200.0; w.res[0] = { 1500.0, 8.0, 2.0 };
                w.res[1] = { 3200.0, 3.0, 2.0 }; w.breath_db = -42.0; w.chiff_db = -28.0; w.chiff_ms = 10.0; w.attack_ms = 14.0; w.attack_shape = 0.7;
                w.overshoot = 0.05; w.release_ms = 40.0; w.vib_depth_cents = 0.0; w.vib_am = 0.0; w.jitter_cents = 0.6; w.level = 0.36;
            }
        };

        // Capped reed: no lip control, constant buzzy tone, cylindrical (odd-weighted).
        class Crumhorn : public SingleReedInstrument {
        public:
            Crumhorn() : SingleReedInstrument("Crumhorn") {
                auto& w = recipe.wind;
                w.tilt_soft = 0.6; w.tilt_loud = 0.6; w.g_soft = 0.95; w.g_loud = 0.98; w.fund_drop_db = 0.0; w.even_low = 0.25; w.even_open_hz = 2200.0;
                w.cutoff_hz = 2600.0; w.res[0] = { 1000.0, 4.0, 1.5 }; w.breath_db = -46.0; w.chiff_db = -36.0; w.attack_ms = 26.0; w.attack_shape = 0.7;
                w.overshoot = 0.0; w.vib_depth_cents = 0.0; w.vib_am = 0.0; w.jitter_cents = 0.3; w.amp_jitter = 0.0; w.pressure_pitch_cents = 0.0;
            }
        };

        class Dulcian : public DoubleReedInstrument {
        public:
            Dulcian() : DoubleReedInstrument("Dulcian") {
                auto& w = recipe.wind;
                w.res[0] = { 480.0, 6.0, 1.5 }; w.res[1] = { 1100.0, 3.0, 1.5 }; w.cutoff_hz = 2000.0; w.attack_ms = 40.0; w.vib_depth_cents = 2.0;
                w.vib_am = 0.0; w.breath_center_hz = 1400.0; w.release_ms = 85.0;
            }
        };

        class Chalumeau : public SingleReedInstrument {
        public:
            Chalumeau() : SingleReedInstrument("Chalumeau") {
                auto& w = recipe.wind;
                w.even_low = 0.1; w.cutoff_hz = 1400.0; w.even_open_hz = 1200.0; w.attack_ms = 45.0; w.breath_db = -36.0; w.vib_depth_cents = 0.0;
                w.vib_am = 0.0; w.release_ms = 80.0;
            }
        };

        // Voice-driven membrane buzz with a nasal formant.
        class Kazoo : public SingleReedInstrument {
        public:
            Kazoo() : SingleReedInstrument("Kazoo") {
                auto& w = recipe.wind;
                w.tilt_soft = 0.9; w.tilt_loud = 0.5; w.g_soft = 0.9; w.g_loud = 0.97; w.fund_drop_db = 0.0; w.even_low = 1.0; w.cutoff_hz = 3800.0;
                w.res[0] = { 1500.0, 9.0, 3.0 }; w.breath_db = -24.0; w.chiff_db = -28.0; w.attack_ms = 18.0; w.vib_depth_cents = 9.0; w.vib_rate_hz = 5.5;
                w.vib_delay_s = 0.2; w.amp_jitter = 0.04; w.jitter_cents = 2.0;
            }
        };

        class Gemshorn : public AirJetInstrument {
        public:
            Gemshorn() : AirJetInstrument("Gemshorn") {
                auto& w = recipe.wind;
                w.tilt_soft = 1.4; w.tilt_loud = 1.4; w.g_soft = 0.12; w.g_loud = 0.2; w.cutoff_hz = 2800.0; w.breath_db = -36.0; w.breath_center_hz = 1800.0;
                w.chiff_db = -28.0; w.chiff_ms = 40.0; w.attack_ms = 60.0; w.vib_depth_cents = 4.0; w.vib_am = 0.03;
            }
        };

    }
}
