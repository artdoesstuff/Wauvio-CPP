#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class MutedTrumpet : public BrassInstrument {
        public:
            MutedTrumpet() : BrassInstrument("Muted Trumpet") {
                recipe.filter_cutoff = 3200.0; recipe.formant1_hz = 1100.0; recipe.formant1_gain_db = 9.0; recipe.formant1_q = 4.0;
                recipe.pitch_attack_cents = 30.0;
            }
        };

        class HarmonMuteTrumpet : public BrassInstrument {
        public:
            HarmonMuteTrumpet() : BrassInstrument("Harmon Mute Trumpet") {
                recipe.filter_cutoff = 5200.0; recipe.formant1_hz = 2300.0; recipe.formant1_gain_db = 10.0; recipe.formant1_q = 6.0;
                recipe.formant2_hz = 700.0; recipe.formant2_gain_db = -5.0; recipe.formant2_q = 2.5;
                recipe.envelope.attack = 0.04;
            }
        };

        class MutedTrombone : public BrassInstrument {
        public:
            MutedTrombone() : BrassInstrument("Muted Trombone") {
                recipe.filter_cutoff = 2600.0; recipe.formant1_hz = 900.0; recipe.formant1_gain_db = 8.0; recipe.formant1_q = 4.0;
                recipe.envelope.attack = 0.05;
            }
        };

        class Bugle : public BrassInstrument {
        public:
            Bugle() : BrassInstrument("Bugle") {
                recipe.osc_mix = 0.2; recipe.filter_cutoff = 5400.0; recipe.formant1_hz = 1300.0;
                recipe.pitch_attack_cents = 20.0; recipe.vibrato_depth_cents = 3.0;
            }
        };

        class Mellophone : public BrassInstrument {
        public:
            Mellophone() : BrassInstrument("Mellophone") {
                recipe.filter_cutoff = 4300.0; recipe.formant1_hz = 900.0; recipe.formant1_gain_db = 4.0; recipe.pitch_attack_cents = 30.0;
            }
        };

        class Cimbasso : public BrassInstrument {
        public:
            Cimbasso() : BrassInstrument("Cimbasso") {
                recipe.filter_cutoff = 2300.0; recipe.formant1_hz = 500.0; recipe.formant1_gain_db = 5.0;
                recipe.envelope.attack = 0.045; recipe.pitch_attack_cents = 25.0;
            }
        };

        class Cornett : public BrassInstrument {
        public:
            Cornett() : BrassInstrument("Cornett") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.osc_mix = 0.4; recipe.noise_mix = 0.05;
                recipe.filter_cutoff = 3400.0; recipe.formant1_hz = 1000.0; recipe.formant1_gain_db = 3.0;
                recipe.vibrato_depth_cents = 4.0; recipe.pitch_attack_cents = 20.0;
            }
        };

        class PostHorn : public BrassInstrument {
        public:
            PostHorn() : BrassInstrument("Post Horn") {
                recipe.filter_cutoff = 5600.0; recipe.formant1_hz = 1500.0; recipe.pitch_attack_cents = 40.0; recipe.envelope.attack = 0.025;
            }
        };

        class TenorHorn : public BrassInstrument {
        public:
            TenorHorn() : BrassInstrument("Tenor Horn") {
                recipe.filter_cutoff = 4000.0; recipe.formant1_hz = 800.0; recipe.envelope.attack = 0.04;
            }
        };

        class FanfareTrumpets : public BrassInstrument {
        public:
            FanfareTrumpets() : BrassInstrument("Fanfare Trumpets") {
                recipe.filter_cutoff = 7200.0; recipe.formant1_hz = 1500.0; recipe.formant1_gain_db = 5.0;
                recipe.detune_cents = 14.0; recipe.stereo_width = 0.35; recipe.pitch_attack_cents = 45.0;
            }
        };

    }
}
