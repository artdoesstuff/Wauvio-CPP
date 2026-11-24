#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class Clavichord : public KeyboardInstrument {
        public:
            Clavichord() : KeyboardInstrument("Clavichord") {
                recipe.percussive = true; recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.filter_cutoff = 4200.0; recipe.noise_attack_burst = 0.12; recipe.gain = 0.8;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.002, 0.0, 0.35, 0.0, 0.08};
                recipe.vibrato_rate_hz = 6.0; recipe.vibrato_depth_cents = 6.0; recipe.vibrato_delay_sec = 0.15;
            }
        };

        class Virginal : public KeyboardInstrument {
        public:
            Virginal() : KeyboardInstrument("Virginal") {
                recipe.percussive = true; recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.filter_cutoff = 6200.0; recipe.noise_attack_burst = 0.18;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.45, 0.0, 0.1};
            }
        };

        class Fortepiano : public KeyboardInstrument {
        public:
            Fortepiano() : KeyboardInstrument("Fortepiano") {
                recipe.filter_cutoff = 5200.0; recipe.noise_attack_burst = 0.08;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.003, 0.0, 0.5, 0.0, 0.2};
            }
        };

        class Pianet : public KeyboardInstrument {
        public:
            Pianet() : KeyboardInstrument("Pianet") {
                recipe.percussive = true; recipe.osc2_ratio = 3.0; recipe.osc_mix = 0.3; recipe.filter_cutoff = 3000.0; recipe.noise_attack_burst = 0.12;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.002, 0.0, 0.5, 0.0, 0.1};
            }
        };

        class FMElectricPiano : public KeyboardInstrument {
        public:
            FMElectricPiano() : KeyboardInstrument("FM Electric Piano") {
                recipe.use_fm = true; recipe.fm_ratio = 1.0; recipe.fm_index = 2.0; recipe.osc_mix = 0.6;
                recipe.filter_cutoff = 6500.0; recipe.envelope.decay = 1.8;
            }
        };

        class FarfisaOrgan : public KeyboardInstrument {
        public:
            FarfisaOrgan() : KeyboardInstrument("Farfisa Organ") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc2_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.5;
                recipe.filter_cutoff = 3800.0; recipe.stereo_width = 0.2;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.005, 0.0, 0.02, 0.95, 0.06};
                recipe.vibrato_rate_hz = 6.0; recipe.vibrato_depth_cents = 6.0; recipe.vibrato_delay_sec = 0.0;
                recipe.gain = 0.32;
            }
        };

        class VoxContinental : public KeyboardInstrument {
        public:
            VoxContinental() : KeyboardInstrument("Vox Continental") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc2_shape = wauvio::osc::WaveShape::Triangle; recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.35;
                recipe.filter_cutoff = 5200.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.005, 0.0, 0.02, 0.95, 0.06};
                recipe.gain = 0.36;
            }
        };

        class Melodica : public KeyboardInstrument {
        public:
            Melodica() : KeyboardInstrument("Melodica") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc_mix = 0.3; recipe.noise_mix = 0.05; recipe.filter_cutoff = 3600.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.03, 0.0, 0.05, 0.9, 0.08};
                recipe.vibrato_depth_cents = 4.0; recipe.vibrato_delay_sec = 0.3;
            }
        };

        class RegalOrgan : public KeyboardInstrument {
        public:
            RegalOrgan() : KeyboardInstrument("Regal Organ") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc2_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.5;
                recipe.filter_cutoff = 3200.0; recipe.formant1_hz = 1000.0; recipe.formant1_gain_db = 4.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.02, 0.0, 0.02, 0.95, 0.08};
                recipe.gain = 0.4;
            }
        };

        class PositiveOrgan : public KeyboardInstrument {
        public:
            PositiveOrgan() : KeyboardInstrument("Positive Organ") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine; recipe.osc2_shape = wauvio::osc::WaveShape::Triangle; recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.5;
                recipe.filter_cutoff = 4200.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.04, 0.0, 0.02, 0.95, 0.12};
            }
        };

    }
}
