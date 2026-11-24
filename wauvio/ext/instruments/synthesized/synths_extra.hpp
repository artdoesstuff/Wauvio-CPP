#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class SupersawLead : public SynthInstrument {
        public:
            SupersawLead() : SynthInstrument("Supersaw Lead") {
                recipe.osc2_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.9; recipe.detune_cents = 24.0;
                recipe.stereo_width = 0.6; recipe.filter_cutoff = 6500.0; recipe.filter_env_amount = 800.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.005, 0.0, 0.1, 0.85, 0.2};
            }
        };

        class SawLead : public SynthInstrument {
        public:
            SawLead() : SynthInstrument("Saw Lead") {
                recipe.osc_mix = 0.15; recipe.detune_cents = 7.0; recipe.filter_cutoff = 5500.0; recipe.vibrato_depth_cents = 6.0; recipe.vibrato_delay_sec = 0.2;
            }
        };

        class SquareLead : public SynthInstrument {
        public:
            SquareLead() : SynthInstrument("Square Lead") {
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc_mix = 0.1; recipe.filter_cutoff = 4800.0;
            }
        };

        class SineLead : public SynthInstrument {
        public:
            SineLead() : SynthInstrument("Sine Lead") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine; recipe.osc_mix = 0.0; recipe.filter_cutoff = 9000.0; recipe.vibrato_depth_cents = 12.0; recipe.vibrato_delay_sec = 0.2;
            }
        };

        class SyncLead : public SynthInstrument {
        public:
            SyncLead() : SynthInstrument("Sync Lead") {
                recipe.use_fm = true; recipe.fm_ratio = 2.5; recipe.fm_index = 4.0; recipe.osc_mix = 0.8; recipe.filter_cutoff = 7000.0;
            }
        };

        class AcidBass : public SynthInstrument {
        public:
            AcidBass() : SynthInstrument("Acid Bass") {
                recipe.osc_mix = 0.0; recipe.filter_cutoff = 700.0; recipe.filter_q = 6.0; recipe.filter_env_amount = 3600.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.16, 0.35, 0.05};
                recipe.gain = 0.75;
            }
        };

        class SubBass : public SynthInstrument {
        public:
            SubBass() : SynthInstrument("Sub Bass") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine; recipe.osc_mix = 0.0; recipe.filter_cutoff = 300.0; recipe.stereo_width = 0.02; recipe.filter_env_amount = 0.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.01, 0.0, 0.05, 1.0, 0.12};
                recipe.gain = 0.45;
            }
        };

        class ReeseBass : public SynthInstrument {
        public:
            ReeseBass() : SynthInstrument("Reese Bass") {
                recipe.osc2_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.9; recipe.detune_cents = 22.0;
                recipe.filter_cutoff = 1200.0; recipe.filter_q = 1.2; recipe.filter_env_amount = 0.0; recipe.stereo_width = 0.35;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.01, 0.0, 0.1, 0.9, 0.2};
            }
        };

        class WarmPad : public SynthInstrument {
        public:
            WarmPad() : SynthInstrument("Warm Pad") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle; recipe.osc_mix = 0.3; recipe.detune_cents = 12.0; recipe.filter_cutoff = 1800.0; recipe.filter_env_amount = 0.0; recipe.stereo_width = 0.55;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.9, 0.0, 0.5, 0.8, 1.4};
            }
        };

        class GlassPad : public SynthInstrument {
        public:
            GlassPad() : SynthInstrument("Glass Pad") {
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine; recipe.osc2_shape = wauvio::osc::WaveShape::Sine; recipe.osc2_ratio = 4.0; recipe.osc_mix = 0.5; recipe.filter_cutoff = 7000.0; recipe.filter_env_amount = 0.0;
                recipe.vibrato_rate_hz = 4.5; recipe.vibrato_depth_cents = 4.0; recipe.stereo_width = 0.5;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.7, 0.0, 0.4, 0.8, 1.6};
            }
        };

        class SweepPad : public SynthInstrument {
        public:
            SweepPad() : SynthInstrument("Sweep Pad") {
                recipe.osc2_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.9; recipe.detune_cents = 14.0; recipe.filter_cutoff = 600.0; recipe.filter_env_amount = 4500.0; recipe.stereo_width = 0.5;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 1.4, 0.0, 0.5, 0.8, 1.4};
                recipe.gain = 1.6;
            }
        };

        class PolySynth : public SynthInstrument {
        public:
            PolySynth() : SynthInstrument("Poly Synth") {
                recipe.osc2_shape = wauvio::osc::WaveShape::BL_Square; recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.5; recipe.detune_cents = 9.0; recipe.stereo_width = 0.5; recipe.filter_cutoff = 3600.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.01, 0.0, 0.3, 0.7, 0.35};
            }
        };

        class SynthStab : public SynthInstrument {
        public:
            SynthStab() : SynthInstrument("Synth Stab") {
                recipe.osc2_shape = wauvio::osc::WaveShape::BL_Sawtooth; recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.9; recipe.detune_cents = 16.0; recipe.filter_cutoff = 5000.0; recipe.filter_env_amount = 3000.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.12, 0.0, 0.08};
            }
        };

    }
}
