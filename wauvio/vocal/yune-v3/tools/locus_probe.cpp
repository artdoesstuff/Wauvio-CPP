// locus_probe: reads the engine's own F2/F3 control trajectory around consonant release and fits locus equations
//   F2_onset = slope * F2_vowel + intercept     (the same quantity the locus-equation literature measures),
// without any acoustic analysis noise. Usage: locus_probe [voice-id | file.wvc] [offset_ms]
//   offset_ms: how long after the release to sample "onset" (default 6 = about the first glottal pulses)
// Prints one line per consonant: slope, intercept, r2, and the F2 onset/vowel values for a i u e o.
#include "../../v3.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace wauvio; using namespace wauvio::vocal::v3;
int main(int argc, char** argv) {
    VoiceProfile vp; std::string err, which = argc > 1 ? argv[1] : "yune";
    if (is_voicebank_path(which)) { if (!load_profile(which, vp, &err)) { std::fprintf(stderr, "%s\n", err.c_str()); return 1; } }
    else if (!make_voice(which, vp, &err)) { std::fprintf(stderr, "%s\n", err.c_str()); return 1; }
    const double off = (argc > 2 ? std::atof(argv[2]) : 6.0) / 1000.0;
    // release = vowel start minus burst + release part of the stop spec (rows: voiced then voiceless)
    struct C { const char* ph; double rel; };
    const C cs[] = { {"b", .017}, {"d", .018}, {"g", .020}, {"p", .020}, {"t", .025}, {"k", .044} };
    const char* vs[] = { "a", "i", "u", "e", "o" };
    for (const C& c : cs) {
        double on[5], on3[5], md[5], md3[5];
        for (int i = 0; i < 5; ++i) {
            Singer s(vp); s.natural(0.f); s.auto_expression(false); s.style().vibrato_depth = 0.f; s.style().coarticulation = 0.f;
            s.rest_sec(0.5); s.note_sec(60, 1.0, std::string("{") + c.ph + " " + vs[i] + "}");
            KeyTrack kt = s.key_track();
            PV a = kt.at(0.5 - c.rel + off), m = kt.at(0.5 + 0.30);
            on[i] = a[F2]; md[i] = m[F2]; on3[i] = a[F3]; md3[i] = m[F3];
        }
        auto fit = [](const double* x, const double* y, double& s, double& c, double& r2) {
            double mx = 0, my = 0; for (int i = 0; i < 5; ++i) { mx += x[i]; my += y[i]; } mx /= 5; my /= 5;
            double sxy = 0, sxx = 0, syy = 0; for (int i = 0; i < 5; ++i) { sxy += (x[i]-mx)*(y[i]-my); sxx += (x[i]-mx)*(x[i]-mx); syy += (y[i]-my)*(y[i]-my); }
            s = sxy / sxx; c = my - s * mx; r2 = sxy * sxy / (sxx * syy);
        };
        double s2, c2, r2, s3, c3, q3; fit(md, on, s2, c2, r2); fit(md3, on3, s3, c3, q3);
        std::printf("%s F2 slope %.2f int %5.0f r2 %.2f | F3 slope %.2f int %5.0f r2 %.2f | F2 on:", c.ph, s2, c2, r2, s3, c3, q3);
        for (int i = 0; i < 5; ++i) std::printf(" %s=%.0f/%.0f", vs[i], on[i], md[i]);
        std::printf(" | F3 on:"); for (int i = 0; i < 5; ++i) std::printf(" %s=%.0f/%.0f", vs[i], on3[i], md3[i]);
        // VOT and prevoicing from the control track, found without assuming timings: the release is the time of the
        // strongest burst noise (FRIC), voicing starts when VOICE passes 0.5. Measured twice: after a rest ("initial")
        // and between vowels ("medial", lyric "a C a"; second mora starts at 1.0 s with equal mora weights).
        auto vot = [&](const std::string& lyric, double t0, double t1, double& vot_ms, bool& prevoiced, float medial_vot = 0.f) {
            Singer s(vp); s.natural(0.f); s.auto_expression(false); s.style().vibrato_depth = 0.f; s.style().coarticulation = 0.f; s.style().medial_vot = medial_vot;
            s.rest_sec(0.5); s.note_sec(60, 1.0, lyric);
            KeyTrack kt = s.key_track(); double tr = t0, best = -1;
            for (double t = t0; t < t1; t += 0.0002) { const float f = kt.at(t)[FRIC]; if (f > best) { best = f; tr = t; } }
            double von = -1; for (double t = tr; t < tr + 0.12; t += 0.0001) if (kt.at(t)[VOICE] >= 0.5f) { von = t; break; }
            prevoiced = kt.at(tr - 0.004)[VOICE] >= 0.5f; vot_ms = (von - tr) * 1000.0;
        };
        double vi, vm; bool pi_, pm_;
        vot(std::string("{") + c.ph + " a}", 0.40, 0.52, vi, pi_);
        vot(std::string("{a ") + c.ph + " a}", 0.90, 1.02, vm, pm_, 1.f);    // medial = speech mode (medial_vot 1)
        std::printf(" | initial %s VOT %.0f ms | medial (speech mode) %s VOT %.0f ms", pi_ ? "prevoiced" : "unvoiced-closure", vi, pm_ ? "prevoiced" : "unvoiced-closure", vm);
        std::printf("\n");
    }
    return 0;
}
