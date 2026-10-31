#pragma once
// Optional "recording" stage: proximity warmth, de-esser, air band, mouth/lip noise.
// Not part of the voice (Singer::render never calls it); apply to the rendered Buffer.
#include "../../wauvio.hpp"
#include <cmath>
#include <cstdint>
#include <vector>

namespace wauvio {
    namespace vocal {
        struct MicModel {
            float proximity   = 0.f;   // 0..1: low-shelf warmth (+0..5 dB below ~200 Hz), as when singing close
            float air         = 0.f;   // 0..1: high shelf above ~9 kHz (+0..4 dB)
            float de_ess      = 0.f;   // 0..1: dynamic cut of 5-9 kHz when sibilants spike
            float mouth_noise = 0.f;   // 0..1: tiny deterministic lip/tongue clicks in quiet passages
            uint32_t seed = 7;
        };

        namespace mic_detail {
            struct Bq { double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
                double run(double x) { const double y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; } };
            inline Bq shelf(bool high, double fc, double gain_db, double sr) {          // RBJ shelf, S = 1
                const double A = std::pow(10.0, gain_db / 40.0), w = 2 * PI * fc / sr, cs = std::cos(w), sn = std::sin(w);
                const double al = sn / 2.0 * std::sqrt(2.0), sq = 2 * std::sqrt(A) * al;
                double b0, b1, b2, a0, a1, a2;
                if (!high) { b0 = A * ((A + 1) - (A - 1) * cs + sq); b1 = 2 * A * ((A - 1) - (A + 1) * cs); b2 = A * ((A + 1) - (A - 1) * cs - sq);
                             a0 = (A + 1) + (A - 1) * cs + sq; a1 = -2 * ((A - 1) + (A + 1) * cs); a2 = (A + 1) + (A - 1) * cs - sq; }
                else       { b0 = A * ((A + 1) + (A - 1) * cs + sq); b1 = -2 * A * ((A - 1) + (A + 1) * cs); b2 = A * ((A + 1) + (A - 1) * cs - sq);
                             a0 = (A + 1) - (A - 1) * cs + sq; a1 = 2 * ((A - 1) - (A + 1) * cs); a2 = (A + 1) - (A - 1) * cs - sq; }
                Bq q; q.b0 = b0 / a0; q.b1 = b1 / a0; q.b2 = b2 / a0; q.a1 = a1 / a0; q.a2 = a2 / a0; return q;
            }
            inline Bq bandpass(double fc, double Q, double sr) {
                const double w = 2 * PI * fc / sr, al = std::sin(w) / (2 * Q), a0 = 1 + al;
                Bq q; q.b0 = al / a0; q.b1 = 0; q.b2 = -al / a0; q.a1 = -2 * std::cos(w) / a0; q.a2 = (1 - al) / a0; return q;
            }
        }

        inline void apply_mic(Buffer& b, int sr, const MicModel& m) {
            using namespace mic_detail;
            if (b.empty()) return;
            if (m.proximity <= 0.f && m.air <= 0.f && m.de_ess <= 0.f && m.mouth_noise <= 0.f) return;
            Bq lo = shelf(false, 200.0, 5.0 * m.proximity, sr), hi = shelf(true, 9000.0, 4.0 * m.air, sr);
            Bq sib = bandpass(7000.0, 1.2, sr);
            double env = 0.0, gain = 1.0; uint32_t rng = m.seed * 2654435761u + 1u; size_t quiet = 0;
            const double ea = std::exp(-1.0 / (0.002 * sr)), er = std::exp(-1.0 / (0.030 * sr)), ga = std::exp(-1.0 / (0.001 * sr)), gr = std::exp(-1.0 / (0.040 * sr));
            double slow = 0.0; const double es = std::exp(-1.0 / (0.050 * sr));
            for (size_t i = 0; i < b.size(); ++i) {
                double x = b[i];
                if (m.proximity > 0.f) x = lo.run(x);
                if (m.air > 0.f) x = hi.run(x);
                if (m.de_ess > 0.f) {                                   // duck only the 5-9 kHz part when it dominates
                    const double s = sib.run(x), a = std::fabs(s);
                    env = a > env ? ea * env + (1 - ea) * a : er * env + (1 - er) * a;
                    slow = es * slow + (1 - es) * std::fabs(x);
                    const double ratio = env / (slow + 1e-6), thr = 0.55;
                    const double target = ratio > thr ? 1.0 - double(m.de_ess) * std::min(1.0, (ratio - thr) / 0.6) * 0.7 : 1.0;
                    gain = target < gain ? ga * gain + (1 - ga) * target : gr * gain + (1 - gr) * target;
                    x = x - s * (1.0 - gain);
                }
                if (m.mouth_noise > 0.f) {
                    const double lvl = std::fabs(x);
                    quiet = lvl < 0.01 ? quiet + 1 : 0;
                    rng = rng * 1664525u + 1013904223u;
                    if (quiet > static_cast<size_t>(0.02 * sr) && (rng >> 8) % static_cast<uint32_t>(sr * 3) == 0) {   // ~1 click per 3 s of quiet
                        const int len = static_cast<int>(0.004 * sr);
                        for (int k = 0; k < len && i + k < b.size(); ++k) {
                            rng = rng * 1664525u + 1013904223u;
                            const double nz = (double((rng >> 8) & 0xFFFF) / 32768.0 - 1.0);
                            b[i + k] += static_cast<float>(0.004 * double(m.mouth_noise) * nz * std::exp(-6.0 * k / len));
                        }
                    }
                }
                b[i] = static_cast<float>(x);
            }
        }
    }
}
