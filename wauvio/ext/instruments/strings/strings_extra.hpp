#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class FolkFiddle : public BowedStringInstrument {
        public:
            FolkFiddle() : BowedStringInstrument("Folk Fiddle") {
                recipe.filter_cutoff = 6400.0; recipe.formant1_hz = 1100.0; recipe.detune_cents = 8.0; recipe.noise_mix = 0.03;
                recipe.vibrato_depth_cents = 8.0; recipe.envelope.attack = 0.05;
            }
        };

        class ViolaDAmore : public BowedStringInstrument {
        public:
            ViolaDAmore() : BowedStringInstrument("Viola d'Amore") {
                recipe.filter_cutoff = 5200.0; recipe.formant1_hz = 1000.0; recipe.detune_cents = 9.0; recipe.envelope.attack = 0.08;
            }
        };

        class Violone : public BowedStringInstrument {
        public:
            Violone() : BowedStringInstrument("Violone") {
                recipe.filter_cutoff = 2600.0; recipe.formant1_hz = 350.0; recipe.envelope.attack = 0.1;
            }
        };

        class Kemenche : public BowedStringInstrument {
        public:
            Kemenche() : BowedStringInstrument("Kemenche") {
                recipe.filter_cutoff = 5800.0; recipe.formant1_hz = 1500.0; recipe.formant1_gain_db = 5.0; recipe.vibrato_depth_cents = 14.0; recipe.noise_mix = 0.03;
            }
        };

        class Sarangi : public BowedStringInstrument {
        public:
            Sarangi() : BowedStringInstrument("Sarangi") {
                recipe.filter_cutoff = 4600.0; recipe.formant1_hz = 1300.0; recipe.formant1_gain_db = 6.0; recipe.formant1_q = 3.0;
                recipe.detune_cents = 12.0; recipe.noise_mix = 0.05; recipe.vibrato_depth_cents = 18.0;
            }
        };

        class Rebab : public BowedStringInstrument {
        public:
            Rebab() : BowedStringInstrument("Rebab") {
                recipe.filter_cutoff = 3600.0; recipe.formant1_hz = 1200.0; recipe.formant1_gain_db = 6.0; recipe.noise_mix = 0.06; recipe.envelope.attack = 0.06; recipe.vibrato_depth_cents = 10.0;
            }
        };

        class MorinKhuur : public BowedStringInstrument {
        public:
            MorinKhuur() : BowedStringInstrument("Morin Khuur") {
                recipe.filter_cutoff = 3000.0; recipe.formant1_hz = 600.0; recipe.formant1_gain_db = 4.0; recipe.noise_mix = 0.05; recipe.envelope.attack = 0.1;
            }
        };

        class Octobass : public BowedStringInstrument {
        public:
            Octobass() : BowedStringInstrument("Octobass") {
                recipe.filter_cutoff = 900.0; recipe.formant1_hz = 120.0; recipe.envelope.attack = 0.2;
            }
        };

        class Rebec : public BowedStringInstrument {
        public:
            Rebec() : BowedStringInstrument("Rebec") {
                recipe.filter_cutoff = 5200.0; recipe.formant1_hz = 1400.0; recipe.noise_mix = 0.05;
            }
        };

        class BowedPsaltery : public BowedStringInstrument {
        public:
            BowedPsaltery() : BowedStringInstrument("Bowed Psaltery") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.osc2_shape = wauvio::osc::WaveShape::Sine; recipe.filter_cutoff = 6000.0; recipe.noise_mix = 0.03; recipe.vibrato_depth_cents = 0.0; recipe.stereo_width = 0.3;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.12, 0.0, 0.1, 0.9, 0.3};
            }
        };

        class ViolinSection : public BowedStringInstrument {
        public:
            ViolinSection() : BowedStringInstrument("Violin Section") {
                recipe.detune_cents = 16.0; recipe.stereo_width = 0.45; recipe.filter_cutoff = 6800.0; recipe.formant1_hz = 1200.0; recipe.envelope.attack = 0.12;
            }
        };

        class ViolaSection : public BowedStringInstrument {
        public:
            ViolaSection() : BowedStringInstrument("Viola Section") {
                recipe.detune_cents = 16.0; recipe.stereo_width = 0.45; recipe.filter_cutoff = 5200.0; recipe.formant1_hz = 900.0; recipe.envelope.attack = 0.13;
            }
        };

        class CelloSection : public BowedStringInstrument {
        public:
            CelloSection() : BowedStringInstrument("Cello Section") {
                recipe.detune_cents = 14.0; recipe.stereo_width = 0.42; recipe.filter_cutoff = 3800.0; recipe.formant1_hz = 600.0; recipe.envelope.attack = 0.14;
            }
        };

        class DoubleBassSection : public BowedStringInstrument {
        public:
            DoubleBassSection() : BowedStringInstrument("Double Bass Section") {
                recipe.detune_cents = 12.0; recipe.stereo_width = 0.4; recipe.filter_cutoff = 1800.0; recipe.formant1_hz = 250.0; recipe.envelope.attack = 0.16;
            }
        };

        class MutedStrings : public BowedStringInstrument {
        public:
            MutedStrings() : BowedStringInstrument("Muted Strings") {
                recipe.detune_cents = 14.0; recipe.stereo_width = 0.4; recipe.filter_cutoff = 3000.0; recipe.envelope.attack = 0.15;
            }
        };

    }
}
