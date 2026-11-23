#pragma once
#include "loudness.hpp"
#include "../effects/eq.hpp"
#include "../effects/stereo.hpp"
#include <cctype>

namespace wauvio {
    namespace fx {
        enum class MasterPreset { Clean, Warm, Club, Loud, Gentle, LoFi };

        struct MasterSettings {
            MasterPreset preset = MasterPreset::Clean;
            double target_lufs  = -14.0;    ///< integrated loudness to reach
            double ceiling_dbtp = -1.0;     ///< true-peak ceiling
            double glue         = 1.0;      ///< 0 = no bus compression .. 1 = preset amount .. 2 = twice as much
            double saturation   = 1.0;      ///< 0 = off .. 1 = preset amount .. 2 = more
            double width        = 1.0;      ///< multiplies the preset's stereo width
            double tilt_db      = 0.0;      ///< + = brighter, - = darker (shelves around 300 Hz / 4 kHz)
            bool   bass_mono    = true;     ///< keep the lows centred (mono below ~120 Hz)

            /// Preset defaults, including a sensible loudness target for the style.
            static MasterSettings for_preset(MasterPreset p) {
                MasterSettings s; s.preset = p;
                switch (p) {
                    case MasterPreset::Clean:  s.target_lufs = -14.0; break;
                    case MasterPreset::Warm:   s.target_lufs = -14.0; break;
                    case MasterPreset::Club:   s.target_lufs =  -9.0; break;
                    case MasterPreset::Loud:   s.target_lufs =  -8.0; break;
                    case MasterPreset::Gentle: s.target_lufs = -16.0; break;
                    case MasterPreset::LoFi:   s.target_lufs = -16.0; s.bass_mono = false; break;
                }
                return s;
            }
        };

        struct MasterReport {
            LoudnessReport before, after;
            double gain_db = 0.0;              ///< input gain pushed into the limiter
            double limiter_reduction_db = 0.0; ///< deepest limiter gain reduction
            bool   target_reached = false;     ///< within 0.5 LU of the target
            int    iterations = 0;
        };

        /// Everything up to (not including) the loudness limiter.
        inline Chain build_master_chain(const MasterSettings& s) {
            Chain c;
            const double glue = std::max(0.0, s.glue), sat = std::max(0.0, s.saturation);
            const MasterPreset p = s.preset;

            {   auto& eq = c.add<EQ>();
                eq.highpass(p == MasterPreset::LoFi ? 60.0 : 22.0, 12);
                switch (p) {
                    case MasterPreset::Warm:   eq.low_shelf(110, 1.5).high_shelf(9000, -1.0); break;
                    case MasterPreset::Club:   eq.low_shelf(70, 2.0).peak(3000, 1.0, 0.8).high_shelf(10000, 1.5); break;
                    case MasterPreset::Loud:   eq.low_shelf(90, 1.5).peak(2500, 1.0, 0.8).high_shelf(9000, 1.0); break;
                    case MasterPreset::Gentle: eq.high_shelf(8000, 0.5); break;
                    case MasterPreset::LoFi:   eq.lowpass(9500, 12).peak(1200, 2.0, 0.7); break;
                    case MasterPreset::Clean:  break;
                }
                if (std::fabs(s.tilt_db) > 1e-6) eq.low_shelf(300, -0.5 * s.tilt_db).high_shelf(4000, 0.5 * s.tilt_db);
            }
            if (sat > 0.0 && (p == MasterPreset::Warm || p == MasterPreset::Club || p == MasterPreset::Loud || p == MasterPreset::LoFi)) {
                auto& st = c.add<Saturator>();
                st.mode = (p == MasterPreset::Warm || p == MasterPreset::LoFi) ? Saturator::Mode::Tape : Saturator::Mode::Soft;
                st.drive_db = (p == MasterPreset::Loud ? 4.0 : p == MasterPreset::LoFi ? 8.0 : 3.0) * sat;
                st.mix = (p == MasterPreset::Warm) ? 0.5 : 0.6; st.oversample = 2;
                st.output_db = -std::min(3.0, st.drive_db * 0.4);
            }
            if (p == MasterPreset::Club || p == MasterPreset::Loud) {
                auto& mb = c.add<MultibandCompressor>();
                const double k = (p == MasterPreset::Loud) ? 1.0 : 0.8;
                mb.low.threshold_db = -20; mb.low.ratio = 1.0 + 2.0 * k; mb.low.attack_ms = 25; mb.low.release_ms = 160;
                mb.mid.threshold_db = -20; mb.mid.ratio = 1.0 + 1.2 * k;
                mb.high.threshold_db = -24; mb.high.ratio = 1.0 + 1.0 * k;
                mb.mix = 1.0;
            }
            if (glue > 0.0) {
                auto& comp = c.add<Compressor>();
                const double base = (p == MasterPreset::Gentle) ? 1.4 : (p == MasterPreset::Loud || p == MasterPreset::Club) ? 2.2 : 1.7;
                comp.ratio = 1.0 + (base - 1.0) * glue;
                comp.threshold_db = (p == MasterPreset::Gentle) ? -16.0 : -18.0;
                comp.attack_ms  = (p == MasterPreset::Club || p == MasterPreset::Loud) ? 12.0 : 30.0;
                comp.release_ms = (p == MasterPreset::Club || p == MasterPreset::Loud) ? 90.0 : 200.0;
                comp.knee_db = 8.0; comp.detector = Compressor::Detect::RMS; comp.sidechain_hpf_hz = 90.0;
            }
            {   auto& im = c.add<StereoImager>();
                const double base_w = (p == MasterPreset::Club) ? 1.12 : (p == MasterPreset::Gentle) ? 1.0 : (p == MasterPreset::LoFi) ? 0.85 : 1.05;
                im.width = base_w * std::max(0.0, s.width);
                im.mono_below_hz = s.bass_mono ? 120.0 : 0.0;
            }
            return c;
        }

        /// Master `buf` in place to the loudness target (buffer length is unchanged).
        inline MasterReport master(StereoBuffer& buf, int sr, const MasterSettings& s) {
            if (sr <= 0) sr = global_config().sample_rate;
            MasterReport rep; rep.before = measure_loudness(buf, sr);
            if (buf.empty()) return rep;

            Chain pre = build_master_chain(s);
            pre.process(buf, sr, false);
            const StereoBuffer pre_buf = buf;                       // limiter iterations start from here

            LoudnessReport m0 = measure_loudness(pre_buf, sr);
            if (!m0.valid) { rep.after = m0; return rep; }
            double gain = std::max(-60.0, std::min(60.0, s.target_lufs - m0.integrated_lufs));
            double best_err = 1e9; StereoBuffer best = pre_buf; double best_gain = gain, best_gr = 0.0; LoudnessReport best_rep = m0;
            double ceiling = s.ceiling_dbtp;                       // tightened if the accurate meter finds an overshoot
            for (int it = 0; it < 6; ++it) {
                StereoBuffer work = pre_buf;
                Chain lim; auto& l = lim.add<Limiter>();
                l.input_gain_db = gain; l.ceiling_db = ceiling; l.true_peak = true; l.release_ms = 100.0; l.lookahead_ms = 5.0;
                lim.process(work, sr, false);
                const LoudnessReport m = measure_loudness(work, sr);
                const bool over = m.true_peak_db > s.ceiling_dbtp + 0.02;
                const double err = std::fabs(m.integrated_lufs - s.target_lufs);
                rep.iterations = it + 1;
                // prefer results that respect the ceiling; among those, the closest to the target
                const double score = err + (over ? 100.0 : 0.0);
                if (score < best_err) { best_err = score; best = std::move(work); best_gain = gain; best_gr = l.gain_reduction_db(); best_rep = m; }
                if (over) ceiling -= (m.true_peak_db - s.ceiling_dbtp) + 0.02;
                if (!over && err < 0.15) break;
                if (err >= 0.15) gain = std::max(-60.0, std::min(60.0, gain + (s.target_lufs - m.integrated_lufs)));
            }
            buf = std::move(best);
            rep.after = best_rep; rep.gain_db = best_gain; rep.limiter_reduction_db = best_gr;
            rep.target_reached = best_err < 0.5;       // (a ceiling violation adds 100 to the score)
            return rep;
        }

        /// Starting-point mastering for a genre name (case-, space- and punctuation-insensitive:
        /// "Drum & Bass", "drum_and_bass", "dnb" all work). Unknown names -> Clean at -14 LUFS.
        inline MasterSettings master_for_genre(const std::string& genre) {
            std::string k;
            for (char ch : genre) if (std::isalnum(static_cast<unsigned char>(ch))) k.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
            auto S = [](MasterPreset p, double lufs) { MasterSettings s = MasterSettings::for_preset(p); s.target_lufs = lufs; return s; };
            if (k == "classical")  { auto s = S(MasterPreset::Gentle, -18.0); s.glue = 0.5; return s; }
            if (k == "ambient")    { auto s = S(MasterPreset::Gentle, -18.0); s.glue = 0.6; return s; }
            if (k == "jazz")       return S(MasterPreset::Warm, -16.0);
            if (k == "bossanova")  return S(MasterPreset::Warm, -16.0);
            if (k == "blues")      return S(MasterPreset::Warm, -15.0);
            if (k == "reggae")     return S(MasterPreset::Warm, -14.0);
            if (k == "funk")       return S(MasterPreset::Warm, -13.0);
            if (k == "chiptune")   return S(MasterPreset::Clean, -12.0);
            if (k == "rock")       return S(MasterPreset::Loud, -11.0);
            if (k == "touhou")     return S(MasterPreset::Loud, -10.0);
            if (k == "metal")      return S(MasterPreset::Loud, -9.0);
            if (k == "house" || k == "trance")            return S(MasterPreset::Club, -9.0);
            if (k == "techno" || k == "dubstep")          return S(MasterPreset::Club, -8.0);
            if (k == "drumbass" || k == "drumandbass" || k == "dnb") return S(MasterPreset::Club, -8.0);
            if (k == "hardcore" || k == "speedcore")      return S(MasterPreset::Loud, -7.0);
            return MasterSettings::for_preset(MasterPreset::Clean);
        }
    }
}
