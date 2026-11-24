#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class Saz : public GuitarInstrument {
        public:
            Saz() : GuitarInstrument("Saz") {
                recipe.filter_cutoff = 5400.0; recipe.detune_cents = 8.0; recipe.envelope.decay = 0.6; recipe.noise_attack_burst = 0.2; recipe.formant1_hz = 1400.0; recipe.formant1_gain_db = 3.0;
            }
        };

        class Qanun : public GuitarInstrument {
        public:
            Qanun() : GuitarInstrument("Qanun") {
                recipe.filter_cutoff = 7600.0; recipe.envelope.decay = 1.2; recipe.detune_cents = 12.0; recipe.noise_attack_burst = 0.15; recipe.stereo_width = 0.35;
            }
        };

        class Cimbalom : public GuitarInstrument {
        public:
            Cimbalom() : GuitarInstrument("Cimbalom") {
                recipe.filter_cutoff = 7000.0; recipe.envelope.decay = 1.1; recipe.detune_cents = 10.0; recipe.noise_attack_burst = 0.3;
            }
        };

        class Yangqin : public GuitarInstrument {
        public:
            Yangqin() : GuitarInstrument("Yangqin") {
                recipe.filter_cutoff = 7600.0; recipe.envelope.decay = 0.9; recipe.detune_cents = 10.0; recipe.noise_attack_burst = 0.3;
            }
        };

        class Ruan : public GuitarInstrument {
        public:
            Ruan() : GuitarInstrument("Ruan") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.filter_cutoff = 5400.0; recipe.envelope.decay = 0.5; recipe.noise_attack_burst = 0.15;
            }
        };

        class Liuqin : public GuitarInstrument {
        public:
            Liuqin() : GuitarInstrument("Liuqin") {
                recipe.filter_cutoff = 7200.0; recipe.envelope.decay = 0.35; recipe.detune_cents = 6.0; recipe.noise_attack_burst = 0.2;
            }
        };

        class Kora : public GuitarInstrument {
        public:
            Kora() : GuitarInstrument("Kora") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.filter_cutoff = 7600.0; recipe.envelope.decay = 1.1; recipe.stereo_width = 0.35; recipe.noise_attack_burst = 0.1;
            }
        };

        class Ngoni : public GuitarInstrument {
        public:
            Ngoni() : GuitarInstrument("Ngoni") {
                recipe.osc_mix = 0.3; recipe.filter_cutoff = 5000.0; recipe.envelope.decay = 0.45; recipe.noise_attack_burst = 0.25;
            }
        };

        class Veena : public GuitarInstrument {
        public:
            Veena() : GuitarInstrument("Veena") {
                recipe.filter_cutoff = 6800.0; recipe.envelope.decay = 1.6; recipe.detune_cents = 12.0; recipe.formant1_hz = 1800.0; recipe.formant1_gain_db = 3.0; recipe.noise_attack_burst = 0.15;
            }
        };

        class Tanpura : public GuitarInstrument {
        public:
            Tanpura() : GuitarInstrument("Tanpura") {
                recipe.filter_cutoff = 6000.0; recipe.detune_cents = 14.0; recipe.noise_attack_burst = 0.05; recipe.stereo_width = 0.4;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.004, 0.0, 3.2, 0.1, 0.5};
                recipe.formant1_hz = 3000.0; recipe.formant1_gain_db = 5.0; recipe.formant1_q = 2.0;
            }
        };

        class Tar : public GuitarInstrument {
        public:
            Tar() : GuitarInstrument("Tar") {
                recipe.filter_cutoff = 5200.0; recipe.envelope.decay = 0.7; recipe.noise_attack_burst = 0.2;
            }
        };

        class Setar : public GuitarInstrument {
        public:
            Setar() : GuitarInstrument("Setar") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.filter_cutoff = 6200.0; recipe.envelope.decay = 0.5; recipe.noise_attack_burst = 0.12;
            }
        };

        class Dutar : public GuitarInstrument {
        public:
            Dutar() : GuitarInstrument("Dutar") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.filter_cutoff = 5800.0; recipe.envelope.decay = 0.6;
            }
        };

        class Hichiriki : public WorldInstrument {
        public:
            Hichiriki() : WorldInstrument("Hichiriki") {
                recipe.osc_mix = 0.5; recipe.noise_mix = 0.04; recipe.filter_cutoff = 5200.0; recipe.formant1_hz = 1800.0; recipe.formant1_gain_db = 8.0; recipe.pitch_attack_cents = 25.0;
            }
        };

        class Ryuteki : public WorldInstrument {
        public:
            Ryuteki() : WorldInstrument("Ryuteki") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine; recipe.osc_mix = 0.12; recipe.noise_mix = 0.1; recipe.filter_cutoff = 4600.0; recipe.vibrato_depth_cents = 10.0; recipe.envelope.attack = 0.08;
            }
        };

        class Sho : public WorldInstrument {
        public:
            Sho() : WorldInstrument("Sho") {
                recipe.osc_mix = 0.4; recipe.detune_cents = 10.0; recipe.filter_cutoff = 3600.0; recipe.stereo_width = 0.3;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.12, 0.0, 0.1, 0.9, 0.25};
            }
        };

        class Nadaswaram : public WorldInstrument {
        public:
            Nadaswaram() : WorldInstrument("Nadaswaram") {
                recipe.osc_mix = 0.5; recipe.noise_mix = 0.03; recipe.filter_cutoff = 7000.0; recipe.formant1_hz = 1400.0; recipe.formant1_gain_db = 9.0; recipe.vibrato_depth_cents = 5.0;
            }
        };

        class Pungi : public WorldInstrument {
        public:
            Pungi() : WorldInstrument("Pungi") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc_mix = 0.6; recipe.filter_cutoff = 4800.0; recipe.formant1_hz = 1600.0; recipe.formant1_gain_db = 6.0; recipe.vibrato_depth_cents = 0.0;
            }
        };

        class ShrutiBox : public WorldInstrument {
        public:
            ShrutiBox() : WorldInstrument("Shruti Box") {
                recipe.osc_mix = 0.35; recipe.noise_mix = 0.03; recipe.filter_cutoff = 2400.0; recipe.stereo_width = 0.3; recipe.vibrato_depth_cents = 0.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.25, 0.0, 0.1, 1.0, 0.4};
            }
        };

    }
}
