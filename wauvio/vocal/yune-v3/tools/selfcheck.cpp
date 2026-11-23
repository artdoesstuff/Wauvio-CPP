// selfcheck: objective checks for the V3 engine changes. Exit code 0 = all passed.
// build:  g++ -std=c++17 -O1 -g -fsanitize=address,undefined -pthread selfcheck.cpp -o selfcheck
// (These cover plumbing and rules; sound quality still needs ears - see listen / sing_score.)
#include "../../v3_fx.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>

using namespace wauvio; using namespace wauvio::vocal::v3;
static int failures = 0, checks = 0;
#define CHECK(cond, msg) do { ++checks; if (!(cond)) { ++failures; std::printf("FAIL  %s  (%s:%d)\n", msg, __FILE__, __LINE__); } else std::printf("ok    %s\n", msg); } while (0)

static Singer sample_phrase() {
    Singer s;
    const char* syl[] = { "さ", "く", "ら", "さ", "く", "ら" };
    const int pit[] = { 69, 69, 71, 69, 69, 76 };
    for (int i = 0; i < 6; ++i) s.note_sec(pit[i], i == 5 ? 1.0 : 0.35, syl[i]);
    s.natural(0.5f, 3);
    return s;
}
static bool same(const Buffer& a, const Buffer& b) { return a.size() == b.size() && std::memcmp(a.data(), b.data(), a.size() * sizeof(float)) == 0; }
static bool finite(const Buffer& a) { for (float x : a) if (!std::isfinite(x)) return false; return true; }

int main() {
    // ---- streaming ----
    {
        Singer s = sample_phrase();
        Buffer whole = s.render();
        CHECK(!whole.empty() && finite(whole), "render produces finite audio");
        CHECK(same(whole, s.render()), "render is deterministic (same score, same samples)");
        for (size_t chunk : { size_t(1), size_t(7), size_t(100), size_t(4096) }) {
            VoiceStream st = s.stream();
            Buffer got; std::vector<float> tmp(chunk);
            for (;;) { const size_t n = st.read(tmp.data(), chunk); if (!n) break; got.insert(got.end(), tmp.begin(), tmp.begin() + static_cast<std::ptrdiff_t>(n)); }
            char m[96]; std::snprintf(m, sizeof m, "stream read in chunks of %zu equals render()", chunk);
            CHECK(same(whole, got) && st.finished(), m);
        }
        VoiceStream st = s.stream(); float tmp[64]; st.read(tmp, 64);
        CHECK(st.position() == 64, "stream position counts consumed samples");
    }
    // ---- NoteRef stability (std::deque) ----
    {
        Singer s; NoteRef first = s.note_sec(60, 0.3, "あ");
        for (int i = 0; i < 5000; ++i) s.note_sec(62, 0.1, "い");
        first.velocity(0.123f);
        CHECK(std::fabs(s.notes().front().vel - 0.123f) < 1e-6f, "NoteRef stays valid after thousands of notes are added");
    }
    // ---- parsing ----
    {
        ParseOptions o; o.particles = true;
        auto m = parse_kana("わたしは", o);
        CHECK(m.size() == 4 && m[3].c == Cons::W && m[3].v == Vowel::A, "particles: trailing は is read as わ");
        auto m2 = parse_kana("わたしは", ParseOptions());
        CHECK(m2.size() == 4 && m2[3].c == Cons::H, "particles off: は stays /ha/");
        auto m3 = parse_kana("は^もの");
        CHECK(m3.size() == 3 && m3[0].c == Cons::W && m3[1].c == Cons::M, "'は^' marks a particle in the middle of a string");
        auto m4 = parse_kana("へ^", ParseOptions());
        CHECK(m4.size() == 1 && m4[0].c == Cons::None && m4[0].v == Vowel::E, "'へ^' is read as え");
        auto m5 = parse_kana("はな", o);
        CHECK(m5.size() == 2 && m5[0].c == Cons::H, "particles on: は at the start of a word is untouched");
        auto v1 = parse_kana("ヴぁ"); auto v2 = parse_romaji("va"); auto v3 = parse_lyric("{v a}");
        CHECK(v1.size() == 1 && surface(v1[0]).surf == Surf::V, "ヴぁ -> [v]");
        CHECK(v2.size() == 1 && surface(v2[0]).surf == Surf::V, "romaji 'va' -> [v]");
        CHECK(v3.size() == 1 && surface(v3[0]).surf == Surf::V, "phoneme 'v a' -> [v]");
        auto bu = parse_kana("ぶ");
        CHECK(surface(bu[0]).surf == Surf::B, "ぶ is still [b]");
    }
    // ---- profile text round trip ----
    {
        VoiceProfile p; std::string e;
        CHECK(make_voice("yune", p, &e), "built-in voice loads");
        p.model.loci[2].s2 = 0.3125f; p.model.nasal_zero_q = 3.5f; p.style.vib_am = 0.8f; p.parse.particles = true;
        VoiceProfile q = voice_yune();
        CHECK(profile_from_text(profile_to_text(p), q, &e), ("profile text parses: " + e).c_str());
        CHECK(q.model.loci[2].s2 == 0.3125f && q.model.nasal_zero_q == 3.5f && q.style.vib_am == 0.8f && q.parse.particles,
              "new V3 fields survive a .wvc round trip");
        for (const std::string& id : voice_ids()) { VoiceProfile v; CHECK(make_voice(id, v, &e), ("voice '" + id + "' loads").c_str()); }
    }
    // ---- event diagnostics ----
    {
        Singer s;
        s.note_sec(69, 1.0, "<laugh:ha,intensity=0.7>");
        CHECK(s.diagnostics().empty(), "a valid event tag raises no diagnostics");
        Singer a; a.note_sec(69, 1.0, "<laugh:zzz>");    CHECK(!a.diagnostics().empty(), "unknown laugh variant is reported");
        Singer b; b.note_sec(69, 1.0, "<hum:wat>");      CHECK(!b.diagnostics().empty(), "unknown hum variant is reported");
        Singer c; c.note_sec(69, 1.0, "<blorp>");        CHECK(!c.diagnostics().empty(), "unknown event kind is reported");
        Singer d; d.note_sec(69, 1.0, "<laugh,intnsity=0.5>"); CHECK(!d.diagnostics().empty(), "misspelled parameter is reported");
        Singer f; f.note_sec(69, 1.0, "<laugh,intensity=loud>"); CHECK(!f.diagnostics().empty(), "non-numeric parameter is reported");
        Singer g; g.note_sec(69, 1.0, "<hum:hm?>");      CHECK(g.diagnostics().empty(), "hum variants hm?/mhm/hmm are accepted");
    }
    // ---- accent contour ----
    {
        auto ms = parse_kana("はしがみ");    // 4 morae
        Curve h = accent_contour(ms, 1.0, 0, 3.f, 0.f), a = accent_contour(ms, 1.0, 1, 3.f, 0.f), n3 = accent_contour(ms, 1.0, 3, 3.f, 0.f);
        CHECK(curve_eval(h, 0.05) < curve_eval(h, 0.4) && curve_eval(h, 0.4) == curve_eval(h, 0.9), "heiban: L then H, no fall");
        CHECK(curve_eval(a, 0.05) > curve_eval(a, 0.5), "atamadaka: H then L");
        CHECK(curve_eval(n3, 0.05) < curve_eval(n3, 0.4) && curve_eval(n3, 0.4) > curve_eval(n3, 0.9), "nucleus 3: L H H L");
        Singer s; s.say("こんにちは", 1.2, "statement", 64, 0.7f, 0);
        CHECK(finite(s.render()), "say(..., accent) renders");
    }
    // ---- ensemble / chain ----
    {
        Singer lead = sample_phrase();
        Ensemble e; e.add(lead, "lead"); e.add_double(lead, 2); e.add_harmony(lead, 4, "third", -0.4f, 0.6f);
        StereoBuffer m = e.render_stereo();
        bool ok = m.size() > 0; for (size_t i = 0; i < m.size(); ++i) if (!std::isfinite(m.L[i]) || !std::isfinite(m.R[i])) ok = false;
        CHECK(e.size() == 4 && ok, "ensemble of 4 parts renders finite stereo");
        CHECK(m.size() >= lead.render().size(), "ensemble is at least as long as the lead");
        for (const std::string& p : vocal_chain_presets()) {
            StereoBuffer r = process_vocal(lead.render(), p);
            bool fine = r.size() > 0; for (size_t i = 0; i < r.size(); ++i) if (!std::isfinite(r.L[i])) fine = false;
            CHECK(fine, ("vocal chain '" + p + "' produces finite output").c_str());
        }
        bool threw = false; try { make_vocal_chain("nope"); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw, "unknown chain preset throws");
    }
    // ---- human defaults ----
    {
        auto phrase = [](float nat, bool ae, uint32_t take) { Singer s; s.natural(nat, take); s.auto_expression(ae);
            for (int i = 0; i < 6; ++i) { s.note_sec(64, 0.4, "あ"); }
            return s; };
        Buffer even = phrase(0.f, false, 1).render();
        Singer def; for (int i = 0; i < 6; ++i) def.note_sec(64, 0.4, "あ");
        CHECK(!same(even, def.render()), "a default Singer is no longer perfectly even (natural + auto_expression are on)");
        CHECK(same(def.render(), def.render()), "default (humanised) render is deterministic");
        CHECK(!same(phrase(0.3f, true, 1).render(), phrase(0.3f, true, 2).render()), "different takes give different performances");
        CHECK(same(phrase(0.f, false, 1).render(), phrase(0.f, false, 9).render()), "natural 0 ignores the take number (fully even)");
        CHECK(finite(phrase(1.f, true, 5).render()), "natural 1.0 renders finite audio");
    }
    // ---- consonant timing from the control track ----
    {
        auto track = [](const std::string& lyric, float prevoice) { Singer s; s.natural(0.f); s.auto_expression(false); s.style().vibrato_depth = 0.f; s.style().coarticulation = 0.f;
            s.style().prevoice_initial = prevoice; s.style().medial_vot = 1.f; s.rest_sec(0.5); s.note_sec(60, 1.0, lyric); return s.key_track(); };
        auto vot = [](const KeyTrack& kt, double t0, double t1) { double tr = t0, best = -1; for (double t = t0; t < t1; t += 0.0002) { float f = kt.at(t)[FRIC]; if (f > best) { best = f; tr = t; } }
            for (double t = tr; t < tr + 0.12; t += 0.0001) { if (kt.at(t)[VOICE] >= 0.5f) return (t - tr) * 1000.0; }
            return -1.0; };
        KeyTrack ki = track("{k a}", 0.f), km = track("{a k a}", 0.f), bi = track("{b a}", 0.f), bi1 = track("{b a}", 1.f), bm = track("{a b a}", 0.f);
        CHECK(vot(ki, 0.40, 0.52) > 38.0 && vot(ki, 0.40, 0.52) < 55.0, "initial /k/ VOT is about 46 ms");
        CHECK(vot(km, 0.90, 1.02) < 0.75 * vot(ki, 0.40, 0.52), "medial_vot = 1 (speech mode): medial /k/ VOT is clearly shorter than initial /k/");
        { Singer s0; s0.natural(0.f); s0.auto_expression(false); s0.style().vibrato_depth = 0.f; s0.style().coarticulation = 0.f; s0.rest_sec(0.5); s0.note_sec(60, 1.0, "{a k a}");
          KeyTrack kd = s0.key_track(); CHECK(std::fabs(vot(kd, 0.90, 1.02) - vot(ki, 0.40, 0.52)) < 4.0, "default (medial_vot = 0): a sung /k/ between vowels keeps the full VOT"); }
        CHECK(bi.at(0.47)[VOICE] < 0.5f, "word-initial voiced stop has no prevoicing by default");
        CHECK(bi1.at(0.47)[VOICE] >= 0.5f, "prevoice_initial = 1 restores the voiced closure");
        CHECK(bm.at(0.97)[VOICE] >= 0.5f, "medial voiced stop stays prevoiced (voicing through the closure)");
    }
    // ---- VC transition ----
    {
        auto f2_before_closure = [](float vc) { Singer s; s.natural(0.f); s.auto_expression(false); s.style().vibrato_depth = 0.f; s.style().coarticulation = 0.f; s.style().vc_transition = vc;
            s.rest_sec(0.5); s.note_sec(60, 1.0, "{o k a}"); KeyTrack kt = s.key_track(); const float a0 = kt.at(0.75)[AMP]; double toff = 0.9;
            for (double t = 0.80; t < 1.05; t += 0.0002) if (kt.at(t)[AMP] < 0.5f * a0) { toff = t; break; }
            return kt.at(toff - 0.008)[F2] / kt.at(0.75)[F2]; };
        CHECK(f2_before_closure(0.f) < 1.005f, "vc_transition = 0: F2 has not moved 8 ms before the vowel ends (V2 behaviour)");
        CHECK(f2_before_closure(1.f) > 1.10f, "vc_transition = 1: F2 of /o/ has risen toward /k/ by the end of the vowel");
    }
    // ---- source: no NaN/inf at extreme pitches ----
    for (int midi : { 40, 60, 84, 96 }) {
        Singer s; s.note_sec(midi, 0.5, "あ");
        char m[64]; std::snprintf(m, sizeof m, "finite output at MIDI %d", midi);
        CHECK(finite(s.render()), m);
    }
    std::printf("\n%d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
