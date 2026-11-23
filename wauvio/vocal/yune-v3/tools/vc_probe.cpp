// vc_probe: how far the vowel's F2 has moved toward a following consonant's locus 8 ms and 3 ms before the vowel
// ends (acoustic offset = amplitude falling through 50%), relative to the vowel midpoint. Reads the control track.
// Usage: vc_probe [voice-id | file.wvc]   (compare with the speech numbers in RESEARCH_NOTES.md)
#include "../../v3.hpp"
#include <cmath>
#include <cstdio>
using namespace wauvio; using namespace wauvio::vocal::v3;
int main(int argc, char** argv) {
    VoiceProfile vp; std::string err, which = argc > 1 ? argv[1] : "yune";
    if (is_voicebank_path(which)) { if (!load_profile(which, vp, &err)) { std::fprintf(stderr, "%s\n", err.c_str()); return 1; } }
    else if (!make_voice(which, vp, &err)) { std::fprintf(stderr, "%s\n", err.c_str()); return 1; }
    const char* cs[] = { "p", "t", "k" }; const char* names[] = { "labial", "alveolar", "velar" }; const char* vs[] = { "a", "i", "u", "e", "o" };
    std::printf("F2 / vowel-mid F2 at 8 ms and 3 ms before the acoustic vowel end\n");
    for (int c = 0; c < 3; ++c) for (int v = 0; v < 5; ++v) {
        Singer s(vp); s.natural(0.f); s.auto_expression(false); s.style().vibrato_depth = 0.f; s.style().coarticulation = 0.f;
        s.rest_sec(0.5); s.note_sec(60, 1.0, std::string("{") + vs[v] + " " + cs[c] + " a}");
        KeyTrack kt = s.key_track(); const float a0 = kt.at(0.75)[AMP]; double toff = 0.9;
        for (double t = 0.80; t < 1.05; t += 0.0002) if (kt.at(t)[AMP] < 0.5f * a0) { toff = t; break; }
        const double mid = kt.at(0.75)[F2];
        std::printf("%-8s after %s: %.2f  %.2f\n", names[c], vs[v], kt.at(toff - 0.008)[F2] / mid, kt.at(toff - 0.003)[F2] / mid);
    }
    return 0;
}
