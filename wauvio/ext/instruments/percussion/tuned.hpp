#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class Crotales : public MalletInstrument {
        public:
            Crotales() : MalletInstrument("Crotales") {
                recipe.fm_ratio = 4.2; recipe.fm_index = 1.8; recipe.filter_cutoff = 12000.0;
                recipe.envelope.decay = 3.0;
            }
        };

        class Glockenspiel : public MalletInstrument {
        public:
            Glockenspiel() : MalletInstrument("Glockenspiel") {
                recipe.use_fm = false; recipe.percussive = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                // uniform steel bars: 1 : 2.756 : 5.404 : 8.933
                recipe.modes = { {1.0, 1.0, 2.4}, {2.756, 0.55, 1.4}, {5.404, 0.30, 0.7}, {8.933, 0.15, 0.35} };
                recipe.filter_cutoff = 14000.0;
                recipe.gain = 1.0;
            }
        };

        class SoftGlockenspiel : public MalletInstrument {
        public:
            SoftGlockenspiel() : MalletInstrument("Soft Glockenspiel") {
                recipe.use_fm = false; recipe.percussive = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 2.6}, {2.756, 0.28, 1.3}, {5.404, 0.12, 0.6}, {8.933, 0.05, 0.3} };
                recipe.filter_cutoff = 9000.0; recipe.gain = 0.8;
            }
        };

        class HardGlockenspiel : public MalletInstrument {
        public:
            HardGlockenspiel() : MalletInstrument("Hard Glockenspiel") {
                recipe.use_fm = false; recipe.percussive = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 2.0}, {2.756, 0.75, 1.2}, {5.404, 0.45, 0.6}, {8.933, 0.25, 0.3} };
                recipe.filter_cutoff = 15000.0; recipe.noise_attack_burst = 0.1;
                recipe.gain = 1.0;
            }
        };

        class Vibraphone : public MalletInstrument {
        public:
            Vibraphone() : MalletInstrument("Vibraphone") {
                recipe.use_fm = false; recipe.percussive = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                // aluminium bars tuned 1 : 4 : 10; the motor adds the vibrato
                recipe.modes = { {1.0, 1.0, 4.5}, {4.0, 0.28, 2.2}, {10.0, 0.07, 0.8} };
                recipe.filter_cutoff = 8000.0;
                recipe.vibrato_rate_hz = 4.5; recipe.vibrato_depth_cents = 10.0; recipe.vibrato_delay_sec = 0.0;

        using VibraphoneWithMotor = Vibraphone;
                recipe.gain = 1.0;
            }
        };

        class VibraphoneWithoutMotor : public MalletInstrument {
        public:
            VibraphoneWithoutMotor() : MalletInstrument("Vibraphone without Motor") {
                recipe.use_fm = false; recipe.percussive = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 5.0}, {4.0, 0.28, 2.4}, {10.0, 0.07, 0.9} };
                recipe.filter_cutoff = 8000.0;
                recipe.gain = 1.0;
            }
        };

        class Marimba : public MalletInstrument {
        public:
            Marimba() : MalletInstrument("Marimba") {
                recipe.use_fm = false; recipe.percussive = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                // rosewood bars tuned 1 : 4 : 10
                recipe.modes = { {1.0, 1.0, 1.8}, {4.0, 0.45, 0.6}, {10.0, 0.15, 0.2} };
                recipe.filter_cutoff = 6000.0; recipe.noise_attack_burst = 0.05;
                recipe.gain = 1.0;
            }
        };

        class BassMarimba : public MalletInstrument {
        public:
            BassMarimba() : MalletInstrument("Bass Marimba") {
                recipe.use_fm = false; recipe.percussive = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 2.4}, {4.0, 0.4, 0.7}, {10.0, 0.12, 0.2} };
                recipe.filter_cutoff = 3500.0; recipe.noise_attack_burst = 0.06;
                recipe.gain = 1.03;
            }
        };

        class Xylophone : public MalletInstrument {
        public:
            Xylophone() : MalletInstrument("Xylophone") {
                recipe.use_fm = false; recipe.percussive = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                // rosewood bars tuned 1 : 3 : 6
                recipe.modes = { {1.0, 1.0, 0.9}, {3.0, 0.55, 0.4}, {6.0, 0.25, 0.15} };
                recipe.filter_cutoff = 11000.0; recipe.noise_attack_burst = 0.08;
                recipe.gain = 1.4;
            }
        };

        class TubularBells : public MalletInstrument {
        public:
            TubularBells() : MalletInstrument("Tubular Bells") {
                recipe.fm_ratio = 2.4; recipe.fm_index = 2.2; recipe.filter_cutoff = 7000.0;
                recipe.envelope.decay = 3.2; recipe.stereo_width = 0.35;
            }
        };
        using OrchestralBells = TubularBells;

        class Timpani : public MalletInstrument {
        public:
            Timpani() : MalletInstrument("Timpani") {
                recipe.fm_ratio = 1.5; recipe.fm_index = 0.6; recipe.filter_cutoff = 1200.0;
                recipe.envelope.decay = 1.4; recipe.noise_attack_burst = 0.08; recipe.stereo_width = 0.2;
            }
        };

        class Handbells : public MalletInstrument {
        public:
            Handbells() : MalletInstrument("Handbells") {
                recipe.fm_ratio = 3.0; recipe.fm_index = 1.4; recipe.filter_cutoff = 7500.0;
                recipe.envelope.decay = 2.0; recipe.stereo_width = 0.25;
            }
        };

        class GlassHarmonica : public MalletInstrument {
        public:
            GlassHarmonica() : MalletInstrument("Glass Harmonica") {
                recipe.percussive = false; recipe.fm_ratio = 1.0; recipe.fm_index = 0.2;
                recipe.filter_cutoff = 5000.0; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.2, 0.0, 0.2, 0.85, 0.6};
                recipe.stereo_width = 0.3;
            }
        };

        class GlassMarimba : public MalletInstrument {
        public:
            GlassMarimba() : MalletInstrument("Glass Marimba") {
                recipe.fm_ratio = 5.2; recipe.fm_index = 1.5; recipe.filter_cutoff = 10000.0;
                recipe.envelope.decay = 1.5; recipe.noise_attack_burst = 0.04;
            }
        };

        class MusicBox : public MalletInstrument {
        public:
            MusicBox() : MalletInstrument("Music Box") {
                recipe.use_fm = false; recipe.percussive = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                // clamped-free tines: 1 : 6.267 : 17.55
                recipe.modes = { {1.0, 1.0, 1.2}, {6.267, 0.28, 0.35}, {17.55, 0.06, 0.1} };
                recipe.filter_cutoff = 12000.0; recipe.noise_attack_burst = 0.18;
                recipe.gain = 1.17;
            }
        };

        class Waterphone : public MalletInstrument {
        public:
            Waterphone() : MalletInstrument("Waterphone") {
                recipe.use_fm = true; recipe.fm_ratio = 3.73; recipe.fm_index = 3.2; recipe.osc_mix = 0.75;
                recipe.noise_mix = 0.1; recipe.pink_noise = true;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.01, 0.0, 3.5, 0.15, 2.5};
                recipe.filter_cutoff = 6500.0; recipe.filter_q = 1.4;
                recipe.pitch_instability = 65.0;
                recipe.formant1_hz = 1800.0; recipe.formant1_gain_db = 5.0; recipe.formant1_q = 4.0;
                recipe.formant2_hz = 3200.0; recipe.formant2_gain_db = 3.5; recipe.formant2_q = 5.0;
                recipe.stereo_width = 0.4; recipe.gain = 0.75;
            }
        };

    }
}