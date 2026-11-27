#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        // Conical single reed: full harmonic series. Cutoff and formant scale with instrument size (estimates).
        class SopranoSaxophone : public SaxophoneInstrument {
        public:
            SopranoSaxophone() : SaxophoneInstrument("Soprano Saxophone") {
                auto& w = recipe.wind;
                w.cutoff_hz = 1900.0; w.res[0] = { 1700.0, 5.0, 1.2 }; w.res[1] = { 3600.0, 2.0, 1.8 }; w.attack_ms = 28.0; w.breath_center_hz = 2600.0;
                w.vib_depth_cents = 14.0;
            }
        };

        class AltoSaxophone : public SaxophoneInstrument {
        public:
            AltoSaxophone() : SaxophoneInstrument("Alto Saxophone") {
                auto& w = recipe.wind;
                w.cutoff_hz = 1300.0; w.res[0] = { 1300.0, 5.0, 1.2 }; w.res[1] = { 3000.0, 2.0, 1.8 }; w.attack_ms = 32.0;
            }
        };

        class TenorSaxophone : public SaxophoneInstrument {
        public:
            TenorSaxophone() : SaxophoneInstrument("Tenor Saxophone") {
                auto& w = recipe.wind;
                w.cutoff_hz = 1000.0; w.res[0] = { 1050.0, 5.0, 1.2 }; w.res[1] = { 2500.0, 2.0, 1.8 }; w.attack_ms = 36.0; w.breath_center_hz = 1700.0;
                w.vib_depth_cents = 17.0; w.release_ms = 70.0;
            }
        };

        class BaritoneSaxophone : public SaxophoneInstrument {
        public:
            BaritoneSaxophone() : SaxophoneInstrument("Baritone Saxophone") {
                auto& w = recipe.wind;
                w.cutoff_hz = 780.0; w.res[0] = { 800.0, 5.0, 1.2 }; w.res[1] = { 2000.0, 2.0, 1.8 }; w.attack_ms = 46.0; w.breath_center_hz = 1400.0;
                w.chiff_center_hz = 800.0; w.vib_depth_cents = 17.0; w.vib_rate_hz = 5.2; w.release_ms = 85.0;
            }
        };

        class BassSaxophone : public SaxophoneInstrument {
        public:
            BassSaxophone() : SaxophoneInstrument("Bass Saxophone") {
                auto& w = recipe.wind;
                w.cutoff_hz = 620.0; w.res[0] = { 650.0, 5.0, 1.2 }; w.res[1] = { 1600.0, 2.0, 1.8 }; w.attack_ms = 60.0; w.breath_center_hz = 1100.0;
                w.chiff_center_hz = 600.0; w.vib_depth_cents = 15.0; w.vib_rate_hz = 5.0; w.release_ms = 100.0;
            }
        };

        class ContrabassSaxophone : public SaxophoneInstrument {
        public:
            ContrabassSaxophone() : SaxophoneInstrument("Contrabass Saxophone") {
                auto& w = recipe.wind;
                w.cutoff_hz = 480.0; w.res[0] = { 500.0, 5.0, 1.2 }; w.res[1] = { 1300.0, 2.0, 1.8 }; w.attack_ms = 90.0; w.breath_center_hz = 800.0;
                w.chiff_center_hz = 450.0; w.vib_depth_cents = 12.0; w.vib_rate_hz = 4.8; w.release_ms = 130.0;
            }
        };

    }
}
