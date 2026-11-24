#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class Saron : public MalletInstrument {
        public:
            Saron() : MalletInstrument("Saron") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 3.0}, {2.756, 0.6, 1.5}, {5.404, 0.3, 0.7}, {8.933, 0.12, 0.3} };
                recipe.filter_cutoff = 12000.0; recipe.gain = 1.0;
            }
        };

        class Peking : public MalletInstrument {
        public:
            Peking() : MalletInstrument("Peking") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 1.8}, {2.756, 0.66, 0.9}, {5.404, 0.33, 0.42}, {8.933, 0.132, 0.18} };
                recipe.filter_cutoff = 13000.0; recipe.gain = 1.0;
            }
        };

        class Demung : public MalletInstrument {
        public:
            Demung() : MalletInstrument("Demung") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 4.2}, {2.756, 0.54, 2.1}, {5.404, 0.27, 0.98}, {8.933, 0.108, 0.42} };
                recipe.filter_cutoff = 10000.0; recipe.gain = 1.0;
            }
        };

        class Slentem : public MalletInstrument {
        public:
            Slentem() : MalletInstrument("Slentem") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 5.0}, {2.756, 0.35, 2.4}, {5.404, 0.12, 1.0} };
                recipe.filter_cutoff = 6000.0; recipe.gain = 1.0;
            }
        };

        class Gender : public MalletInstrument {
        public:
            Gender() : MalletInstrument("Gender") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 4.5}, {2.756, 0.35, 2.0}, {5.404, 0.1, 0.8} };
                recipe.filter_cutoff = 7000.0; recipe.vibrato_rate_hz = 0.0; recipe.gain = 1.0;
            }
        };

        class Bonang : public MalletInstrument {
        public:
            Bonang() : MalletInstrument("Bonang") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 2.0}, {2.04, 0.55, 1.2}, {2.87, 0.4, 0.7}, {4.13, 0.22, 0.35} };
                recipe.filter_cutoff = 9000.0; recipe.gain = 1.0;
            }
        };

        class Kenong : public MalletInstrument {
        public:
            Kenong() : MalletInstrument("Kenong") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 2.5}, {2.04, 0.5, 1.5}, {2.87, 0.35, 0.8}, {4.13, 0.2, 0.4} };
                recipe.filter_cutoff = 6000.0; recipe.gain = 1.0;
            }
        };

        class Kempul : public MalletInstrument {
        public:
            Kempul() : MalletInstrument("Kempul") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 5.0}, {2.04, 0.5, 3.0}, {2.87, 0.35, 1.8}, {4.13, 0.2, 0.9}, {5.36, 0.1, 0.5} };
                recipe.filter_cutoff = 5000.0; recipe.gain = 1.0;
            }
        };

        class GongAgeng : public MalletInstrument {
        public:
            GongAgeng() : MalletInstrument("Gong Ageng") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 8.0}, {2.04, 0.55, 5.0}, {2.87, 0.4, 3.0}, {4.13, 0.25, 1.8}, {5.36, 0.12, 1.0} };
                recipe.filter_cutoff = 4500.0; recipe.stereo_width = 0.4; recipe.gain = 1.0;
            }
        };

        class ChurchBell : public MalletInstrument {
        public:
            ChurchBell() : MalletInstrument("Church Bell") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {0.5, 0.7, 12.0}, {1.0, 1.0, 9.0}, {1.2, 0.6, 6.0}, {1.5, 0.45, 5.0}, {2.0, 0.9, 4.0}, {2.5, 0.25, 3.0}, {3.0, 0.3, 2.5}, {4.0, 0.2, 1.5} };
                recipe.filter_cutoff = 9000.0; recipe.stereo_width = 0.3; recipe.mode_decay_pitch_exp = 0.4; recipe.gain = 1.0;
            }
        };

        class Carillon : public MalletInstrument {
        public:
            Carillon() : MalletInstrument("Carillon") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {0.5, 0.7, 4.2}, {1.0, 1.0, 3.15}, {1.2, 0.6, 2.1}, {1.5, 0.45, 1.75}, {2.0, 0.9, 1.4}, {2.5, 0.25, 1.05}, {3.0, 0.3, 0.875}, {4.0, 0.2, 0.525} };
                recipe.filter_cutoff = 10000.0; recipe.stereo_width = 0.3; recipe.gain = 1.0;
            }
        };

        class SingingBowl : public MalletInstrument {
        public:
            SingingBowl() : MalletInstrument("Singing Bowl") {
                recipe.use_fm = false; recipe.percussive = false; recipe.envelope = wauvio::env::DAHDSR{0.0, 0.0008, 0.0, 0.0, 1.0, 0.06};
                recipe.modes = { {1.0, 1.0, 10.0}, {1.004, 0.5, 9.0}, {2.71, 0.45, 6.0}, {5.15, 0.2, 3.5} };
                recipe.filter_cutoff = 8000.0; recipe.gain = 1.0;
            }
        };

        class Daf : public PercussionInstrument {
        public:
            Daf() : PercussionInstrument("Daf") {
                recipe.fixed_pitch_hz = 200.0; recipe.noise_mix = 0.35; recipe.filter_cutoff = 3200.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.35, 0.0, 0.06};
            }
        };

        class Bodhran : public PercussionInstrument {
        public:
            Bodhran() : PercussionInstrument("Bodhran") {
                recipe.fixed_pitch_hz = 110.0; recipe.noise_mix = 0.12; recipe.filter_cutoff = 1600.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.4, 0.0, 0.06};
            }
        };

        class Tumba : public PercussionInstrument {
        public:
            Tumba() : PercussionInstrument("Tumba") {
                recipe.fixed_pitch_hz = 150.0; recipe.noise_mix = 0.08; recipe.filter_cutoff = 1500.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.45, 0.0, 0.06};
            }
        };

        class Dhol : public PercussionInstrument {
        public:
            Dhol() : PercussionInstrument("Dhol") {
                recipe.fixed_pitch_hz = 90.0; recipe.noise_mix = 0.15; recipe.filter_cutoff = 2000.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.35, 0.0, 0.06};
            }
        };

        class Dholak : public PercussionInstrument {
        public:
            Dholak() : PercussionInstrument("Dholak") {
                recipe.fixed_pitch_hz = 140.0; recipe.noise_mix = 0.15; recipe.filter_cutoff = 2200.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.3, 0.0, 0.06};
            }
        };

        class Mridangam : public PercussionInstrument {
        public:
            Mridangam() : PercussionInstrument("Mridangam") {
                recipe.fixed_pitch_hz = 165.0; recipe.noise_mix = 0.06; recipe.filter_cutoff = 2600.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.5, 0.0, 0.06};
            }
        };

        class Ghatam : public PercussionInstrument {
        public:
            Ghatam() : PercussionInstrument("Ghatam") {
                recipe.fixed_pitch_hz = 400.0; recipe.noise_mix = 0.05; recipe.filter_cutoff = 3500.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.22, 0.0, 0.06};
            }
        };

        class Kanjira : public PercussionInstrument {
        public:
            Kanjira() : PercussionInstrument("Kanjira") {
                recipe.fixed_pitch_hz = 330.0; recipe.noise_mix = 0.25; recipe.filter_cutoff = 3600.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.25, 0.0, 0.06};
            }
        };

        class SlitDrum : public PercussionInstrument {
        public:
            SlitDrum() : PercussionInstrument("Slit Drum") {
                recipe.fixed_pitch_hz = 330.0; recipe.noise_mix = 0.03; recipe.filter_cutoff = 1800.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.5, 0.0, 0.06};
            }
        };

        class SleighBells : public PercussionInstrument {
        public:
            SleighBells() : PercussionInstrument("Sleigh Bells") {
                recipe.fixed_pitch_hz = 4200.0; recipe.noise_mix = 0.9; recipe.filter_cutoff = 11000.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.002, 0.0, 0.6, 0.0, 0.2};
            }
        };

    }
}
