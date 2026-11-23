#pragma once

// Optional: a ready-made vocal effect chain built from wauvio::fx. Include wauvio/vocal/v3_fx.hpp (which pulls in this
// header, wauvio/fx.hpp and the V3 vocal engine). Kept separate so the vocal engine itself does not depend on wauvio::fx.

#include "singer.hpp"
#include "mic.hpp"
#include "../../fx.hpp"

#include <string>
#include <vector>

namespace wauvio {
    namespace vocal {
        namespace v3 {

            inline std::vector<std::string> vocal_chain_presets() { return { "natural", "pop", "intimate", "hall" }; }

            /// V3: EQ -> compressor -> (delay) -> reverb -> limiter, voiced for a sung lead.
            ///   natural  : transparent polish, a little room
            ///   pop      : tighter and brighter, dotted-eighth slap, bigger reverb
            ///   intimate : close, warm, nearly dry
            ///   hall     : gentle compression, long hall
            /// bpm > 0 syncs the delay (pop) to the song tempo. Unknown preset names throw std::invalid_argument.
            inline fx::Chain make_vocal_chain(const std::string& preset = "natural", double bpm = 0.0) {
                using namespace fx;
                Chain c;
                struct P { double hpf, mud_db, pres_db, air_db, thr, ratio, att, rel, makeup, rv_decay, rv_pre, rv_size, rv_mix, dly_mix; };
                P p;
                if (preset == "natural")       p = { 70, -1.5, 1.5, 1.5, -22, 2.5, 12, 120, 3, 1.4, 18, 0.8, 0.12, 0.00 };
                else if (preset == "pop")      p = { 90, -2.0, 2.5, 2.5, -26, 4.0,  6,  90, 5, 1.9, 22, 1.0, 0.18, 0.10 };
                else if (preset == "intimate") p = { 60, -0.5, 1.0, 1.0, -24, 2.0, 15, 140, 2, 0.7, 10, 0.5, 0.06, 0.00 };
                else if (preset == "hall")     p = { 80, -1.0, 1.0, 1.5, -20, 2.0, 14, 160, 2, 3.2, 30, 1.6, 0.28, 0.00 };
                else throw std::invalid_argument("unknown vocal chain preset: " + preset);

                auto& eq = c.add<EQ>();
                eq.highpass(p.hpf, 12).peak(300.0, p.mud_db, 1.0).peak(3800.0, p.pres_db, 0.9).high_shelf(10500.0, p.air_db);
                if (preset == "intimate") eq.low_shelf(200.0, 2.0);

                auto& comp = c.add<Compressor>();
                comp.threshold_db = p.thr; comp.ratio = p.ratio; comp.attack_ms = p.att; comp.release_ms = p.rel;
                comp.knee_db = 8.0; comp.makeup_db = p.makeup; comp.detector = Compressor::Detect::RMS;

                if (p.dly_mix > 0.0) {
                    auto& d = c.add<StereoDelay>();
                    if (bpm > 0.0) { d.sync_bpm = bpm; d.sync_beats_left = div::dotted(div::eighth); } else d.time_ms = 320.0;
                    d.feedback = 0.28; d.highcut_hz = 4500.0; d.lowcut_hz = 300.0; d.mix = p.dly_mix;
                }
                auto& rv = c.add<FdnReverb>();
                rv.decay_s = p.rv_decay; rv.predelay_ms = p.rv_pre; rv.size = p.rv_size; rv.mix = p.rv_mix;
                rv.damping_hz = 6000.0; rv.lowcut_hz = 150.0;

                auto& lim = c.add<Limiter>();
                lim.ceiling_db = -1.0;
                return c;
            }

            /// One call: (optional mic model) -> chain, from a mono voice render to a finished stereo signal.
            inline StereoBuffer process_vocal(Buffer dry, const std::string& preset = "natural", const MicModel* mic = nullptr,
                                              int sample_rate = 0, double bpm = 0.0)
            {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                if (mic) apply_mic(dry, sample_rate, *mic);
                StereoBuffer s = pan_mono(dry, 0.0f, sample_rate);
                make_vocal_chain(preset, bpm).process(s, sample_rate);
                return s;
            }
            /// Same for an already-stereo signal (for example Ensemble::render_stereo()).
            inline StereoBuffer process_vocal(StereoBuffer s, const std::string& preset = "natural", int sample_rate = 0, double bpm = 0.0) {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                make_vocal_chain(preset, bpm).process(s, sample_rate);
                return s;
            }

        }
    }
}
