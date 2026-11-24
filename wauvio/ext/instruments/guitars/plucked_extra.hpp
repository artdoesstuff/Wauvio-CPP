#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class PedalSteelGuitar : public GuitarInstrument {
        public:
            PedalSteelGuitar() : GuitarInstrument("Pedal Steel Guitar") {
                recipe.filter_cutoff = 5200.0; recipe.noise_attack_burst = 0.08; recipe.stereo_width = 0.3;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.004, 0.0, 1.8, 0.12, 0.3};
                recipe.vibrato_rate_hz = 5.5; recipe.vibrato_depth_cents = 18.0; recipe.vibrato_delay_sec = 0.2;
            }
        };

        class LapSteelGuitar : public GuitarInstrument {
        public:
            LapSteelGuitar() : GuitarInstrument("Lap Steel Guitar") {
                recipe.filter_cutoff = 4600.0; recipe.noise_attack_burst = 0.08;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.004, 0.0, 1.5, 0.1, 0.3};
                recipe.vibrato_rate_hz = 5.5; recipe.vibrato_depth_cents = 12.0; recipe.vibrato_delay_sec = 0.2;
            }
        };

        class JazzGuitar : public GuitarInstrument {
        public:
            JazzGuitar() : GuitarInstrument("Jazz Guitar") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.osc_mix = 0.15; recipe.filter_cutoff = 2600.0; recipe.envelope.decay = 0.9; recipe.noise_attack_burst = 0.1;
            }
        };

        class BaritoneGuitar : public GuitarInstrument {
        public:
            BaritoneGuitar() : GuitarInstrument("Baritone Guitar") {
                recipe.filter_cutoff = 3600.0; recipe.envelope.decay = 0.9;
            }
        };

        class AcousticBassGuitar : public GuitarInstrument {
        public:
            AcousticBassGuitar() : GuitarInstrument("Acoustic Bass Guitar") {
                recipe.filter_cutoff = 1800.0; recipe.envelope.decay = 0.9; recipe.noise_attack_burst = 0.1; recipe.stereo_width = 0.1;
                recipe.gain = 1.4;
            }
        };

        class BassUkulele : public GuitarInstrument {
        public:
            BassUkulele() : GuitarInstrument("Bass Ukulele") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.filter_cutoff = 1500.0; recipe.envelope.decay = 0.5; recipe.stereo_width = 0.1;
                recipe.gain = 1.5;
            }
        };

        class Cavaquinho : public GuitarInstrument {
        public:
            Cavaquinho() : GuitarInstrument("Cavaquinho") {
                recipe.filter_cutoff = 7000.0; recipe.detune_cents = 6.0; recipe.envelope.decay = 0.35; recipe.noise_attack_burst = 0.25;
            }
        };

        class TresCubano : public GuitarInstrument {
        public:
            TresCubano() : GuitarInstrument("Tres Cubano") {
                recipe.filter_cutoff = 7200.0; recipe.detune_cents = 12.0; recipe.envelope.decay = 0.45; recipe.noise_attack_burst = 0.2;
            }
        };

        class Tiple : public GuitarInstrument {
        public:
            Tiple() : GuitarInstrument("Tiple") {
                recipe.filter_cutoff = 7000.0; recipe.detune_cents = 10.0; recipe.envelope.decay = 0.5;
            }
        };

        class Vihuela : public GuitarInstrument {
        public:
            Vihuela() : GuitarInstrument("Vihuela") {
                recipe.filter_cutoff = 6000.0; recipe.envelope.decay = 0.6;
            }
        };

        class Domra : public GuitarInstrument {
        public:
            Domra() : GuitarInstrument("Domra") {
                recipe.filter_cutoff = 6800.0; recipe.detune_cents = 8.0; recipe.envelope.decay = 0.35; recipe.noise_attack_burst = 0.25;
            }
        };

        class Gusli : public GuitarInstrument {
        public:
            Gusli() : GuitarInstrument("Gusli") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.filter_cutoff = 7400.0; recipe.envelope.decay = 1.4; recipe.stereo_width = 0.3;
            }
        };

        class ElectricSitar : public GuitarInstrument {
        public:
            ElectricSitar() : GuitarInstrument("Electric Sitar") {
                recipe.filter_cutoff = 6500.0; recipe.detune_cents = 12.0; recipe.envelope.decay = 0.5;
                recipe.formant1_hz = 2500.0; recipe.formant1_gain_db = 5.0; recipe.formant1_q = 2.5;
            }
        };

    }
}
