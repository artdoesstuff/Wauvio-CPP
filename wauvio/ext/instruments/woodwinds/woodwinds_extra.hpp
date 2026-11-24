#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class Fife : public WoodwindInstrument {
        public:
            Fife() : WoodwindInstrument("Fife") {
                recipe.filter_cutoff = 6800.0; recipe.noise_mix = 0.08; recipe.osc_mix = 0.1; recipe.vibrato_depth_cents = 4.0; recipe.envelope.attack = 0.03;
            }
        };

        class Quena : public WoodwindInstrument {
        public:
            Quena() : WoodwindInstrument("Quena") {
                recipe.filter_cutoff = 3600.0; recipe.noise_mix = 0.13; recipe.osc_mix = 0.1; recipe.noise_attack_burst = 0.15;
                recipe.vibrato_depth_cents = 8.0; recipe.envelope.attack = 0.07;
            }
        };

        class Kaval : public WoodwindInstrument {
        public:
            Kaval() : WoodwindInstrument("Kaval") {
                recipe.filter_cutoff = 3200.0; recipe.noise_mix = 0.10; recipe.osc_mix = 0.12; recipe.vibrato_depth_cents = 14.0; recipe.envelope.attack = 0.08;
            }
        };

        class BassetHorn : public WoodwindInstrument {
        public:
            BassetHorn() : WoodwindInstrument("Basset Horn") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc_mix = 0.2; recipe.noise_mix = 0.03;
                recipe.filter_cutoff = 3000.0; recipe.formant1_hz = 700.0; recipe.formant1_gain_db = 3.0;
            }
        };

        class AltoClarinet : public WoodwindInstrument {
        public:
            AltoClarinet() : WoodwindInstrument("Alto Clarinet") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc_mix = 0.25; recipe.noise_mix = 0.03;
                recipe.filter_cutoff = 3200.0; recipe.formant1_hz = 650.0; recipe.formant1_gain_db = 3.0;
            }
        };

        class ClarinetInA : public WoodwindInstrument {
        public:
            ClarinetInA() : WoodwindInstrument("Clarinet in A") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc_mix = 0.25; recipe.noise_mix = 0.03;
                recipe.filter_cutoff = 3900.0; recipe.formant1_hz = 850.0; recipe.formant1_gain_db = 3.0;
            }
        };

        class OboeDAmore : public WoodwindInstrument {
        public:
            OboeDAmore() : WoodwindInstrument("Oboe d'Amore") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc_mix = 0.3; recipe.noise_mix = 0.03;
                recipe.formant1_hz = 1100.0; recipe.formant1_gain_db = 5.0; recipe.filter_cutoff = 4600.0;
            }
        };

        class SopraninoSaxophone : public SaxophoneInstrument {
        public:
            SopraninoSaxophone() : SaxophoneInstrument("Sopranino Saxophone") {
                recipe.filter_cutoff = 6600.0; recipe.formant1_hz = 1700.0;
            }
        };

        class Shawm : public WoodwindInstrument {
        public:
            Shawm() : WoodwindInstrument("Shawm") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc_mix = 0.55; recipe.noise_mix = 0.04;
                recipe.filter_cutoff = 6200.0; recipe.formant1_hz = 1500.0; recipe.formant1_gain_db = 8.0; recipe.formant1_q = 2.0;
                recipe.vibrato_depth_cents = 3.0; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.02, 0.0, 0.05, 0.95, 0.1};
            }
        };

        class Crumhorn : public WoodwindInstrument {
        public:
            Crumhorn() : WoodwindInstrument("Crumhorn") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc2_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc_mix = 0.6; recipe.noise_mix = 0.02;
                recipe.filter_cutoff = 2300.0; recipe.formant1_hz = 1000.0; recipe.formant1_gain_db = 4.0;
                recipe.vibrato_depth_cents = 0.0; recipe.envelope.attack = 0.03;
            }
        };

        class Dulcian : public WoodwindInstrument {
        public:
            Dulcian() : WoodwindInstrument("Dulcian") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc_mix = 0.3;
                recipe.filter_cutoff = 2400.0; recipe.formant1_hz = 500.0; recipe.formant1_gain_db = 4.0; recipe.envelope.attack = 0.05;
            }
        };

        class Chalumeau : public WoodwindInstrument {
        public:
            Chalumeau() : WoodwindInstrument("Chalumeau") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc_mix = 0.25; recipe.noise_mix = 0.04;
                recipe.filter_cutoff = 2400.0; recipe.formant1_hz = 600.0; recipe.formant1_gain_db = 3.0; recipe.envelope.attack = 0.06;
            }
        };

        class Kazoo : public WoodwindInstrument {
        public:
            Kazoo() : WoodwindInstrument("Kazoo") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc_mix = 0.6; recipe.noise_mix = 0.12;
                recipe.filter_cutoff = 3800.0; recipe.formant1_hz = 1500.0; recipe.formant1_gain_db = 8.0; recipe.formant1_q = 3.0;
                recipe.vibrato_depth_cents = 6.0; recipe.envelope.attack = 0.02;
            }
        };

        class Gemshorn : public WoodwindInstrument {
        public:
            Gemshorn() : WoodwindInstrument("Gemshorn") {
                recipe.osc_mix = 0.2; recipe.noise_mix = 0.03; recipe.filter_cutoff = 2800.0; recipe.envelope.attack = 0.06;
            }
        };

    }
}
