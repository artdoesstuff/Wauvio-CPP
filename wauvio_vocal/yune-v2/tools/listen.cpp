// listen: renders A/B listening demos for every V2 feature into a folder of WAVs.
// usage: listen [outdir=listen_out] [only-substring]
// Each demo is named NN_area_name.wav; "_default" files are the V1-identical reference for comparison.
#include "../../../wauvio_vocal_v2.hpp"
#include <cstdio>
#include <filesystem>
#include <functional>
#include <string>

using namespace wauvio; using namespace wauvio::vocal;
static std::string OUT = "listen_out", ONLY; static int N = 0;

static void put(const std::string& name, Buffer b, const MicModel* mic = nullptr) {
    if (!ONLY.empty() && name.find(ONLY) == std::string::npos) return;
    if (mic) apply_mic(b, global_config().sample_rate, *mic);
    normalize(b, 0.9f); char id[8]; std::snprintf(id, sizeof id, "%03d_", ++N);
    save_wav(b, OUT + "/" + id + name + ".wav"); std::printf("%s%s\n", id, name.c_str());
}
using Setup = std::function<void(Singer&)>;
// A short sung phrase: さくら さくら (sa-ku-ra sa-ku-ra), ends on a held note.
static Buffer phrase(const Setup& f = [](Singer&) {}, const char* l = "さくらさくらや") {
    Singer s; f(s);
    const int m[] = { 69, 69, 71, 69, 69, 71, 69, 71, 72, 71, 71, 69, 67, 69 };
    (void)m; // simple melody, one lyric syllable per note
    const char* syl[] = { "さ", "く", "ら", "さ", "く", "ら", "や", "よ" };
    const int pit[] = { 69, 69, 71, 69, 69, 71, 72, 76 };
    const int len[] = { 480, 480, 960, 480, 480, 960, 480, 1920 };
    (void)l;
    for (int i = 0; i < 8; ++i) s.note(pit[i], len[i], syl[i]);
    return s.render();
}
static Buffer held(const Setup& f, const char* lyric = "あ", int midi = 69, double secs = 2.0) { Singer s; f(s); s.note_sec(midi, secs, lyric); return s.render(); }

int main(int argc, char** argv) {
    if (argc > 1) OUT = argv[1]; if (argc > 2) ONLY = argv[2];
    std::filesystem::create_directories(OUT);

    // 1. personas (same phrase, 12 voices) + blends and emotions
    put("persona_default", phrase());
    for (const char* p : { "normal", "soft" })
        put(std::string("persona_") + p, phrase([&](Singer& s) { s.persona(p); }));
    put("persona_half_soft", phrase([](Singer& s) { s.persona("soft", 0.5f); }));
    put("persona_lane_normal_to_soft_over_song", phrase([](Singer& s) { s.persona_lane("soft", { { 0.0, 0.0 }, { 1.5, 0.0 }, { 3.5, 1.0 } }); }));

    // 2. phonation controls on a held あ (default first)
    put("phonation_default", held([](Singer&) {}));
    auto st = [](const char* n, float v) { return [=](Singer& s) { float Style::* f = style_field(n); if (f) s.style().*f = v; }; };
    for (auto [n, v] : std::vector<std::pair<const char*, float>>{ { "fry", .6f }, { "fry", 1.f }, { "growl", .5f }, { "growl", 1.f }, { "tremor", .5f }, { "tremor", 1.f },
         { "air", .6f }, { "reg", 1.f }, { "glottal_onset", 1.f }, { "formant_shift", 2.f }, { "formant_shift", -2.f }, { "mouth_open", .8f }, { "mouth_open", -.8f }, { "breathiness", .5f } })
        put(std::string("phonation_") + n + "_" + std::to_string(int(v * 10)), held(st(n, v)));
    put("phonation_register_sweep_chest_to_falsetto", [] { Singer s; s.note_sec(69, 3.0, "あ"); s.lane("register", { { 0, 0 }, { 3.0, 1.0 } }); return s.render(); }());

    // 3. events: standalone and in a sentence
    for (const char* e : { "laugh", "giggle", "sob", "hum", "shout", "kyaa" })
        put(std::string("event_") + e, [&] { Singer s; s.event(e, 69, 1.4, 0.7f); return s.render(); }());
    for (const char* v : { "ha", "he", "hi", "fu", "kusu" }) put(std::string("event_giggle_") + v, [&] { Singer s; s.event("giggle", 72, 1.2, 0.7f, v); return s.render(); }());
    for (const char* v : { "ha", "he", "hi" }) put(std::string("event_laugh_") + v, [&] { Singer s; s.event("laugh", 69, 1.4, 0.7f, v); return s.render(); }());
    for (const char* k : { "eeto", "fuun", "uun", "ano", "waa", "hya", "hee", "maa", "ah", "un" })
        put(std::string("event_interj_") + k, [&] { Singer s; s.event(k, 67, 0.9, 0.7f); return s.render(); }());
    put("event_in_context_laugh_between_words", [] { Singer s; s.note_sec(69, 0.6, "そ"); s.note_sec(71, 0.6, "う"); s.note_sec(69, 1.4, "<laugh,intensity=0.7>"); s.note_sec(67, 0.8, "ね"); return s.render(); }());
    put("event_in_context_old_fufu", [] { Singer s; s.note_sec(72, 0.25, "ふ"); s.note_sec(72, 0.25, "ふ"); s.note_sec(72, 0.5, "ふ"); return s.render(); }());
    put("event_in_context_new_giggle_fu", [] { Singer s; s.event("giggle", 72, 1.0, 0.7f, "fu"); return s.render(); }());

    // 4. phonemes and transitions
    put("phon_default_ふぁ_とぅ_ヴぁ", [] { Singer s; for (const char* l : { "ふぁ", "とぅ", "ヴぁ", "いぇ", "くぁ", "すぃ" }) s.note_sec(69, 0.45, l); return s.render(); }());
    put("phon_input_{k a n a}", [] { Singer s; s.note_sec(69, 1.0, "{k a n a}"); return s.render(); }());
    put("phon_stronger_consonants_{k*2 a}", [] { Singer s; s.note_sec(69, 0.8, "{k*2 a}"); s.note_sec(69, 0.8, "{k a}"); return s.render(); }());
    put("phon_weights_さ0.2_ん0.8", [] { Singer s; s.note_sec(69, 1.2, "さ[0.2]ん[0.8]"); return s.render(); }());
    put("phon_gagyou_default", [] { Singer s; for (const char* l : { "かがみ", "まがり" }) s.note_sec(69, 1.0, l); return s.render(); }());
    put("phon_gagyou_bidakuon", [] { Singer s; s.style().bidakuon = 1.f; for (const char* l : { "かがみ", "まがり" }) s.note_sec(69, 1.0, l); return s.render(); }());
    put("phon_n_default", [] { Singer s; s.note_sec(69, 1.4, "こんいろ"); return s.render(); }());
    put("phon_n_nasalized", [] { Singer s; s.style().n_nasalize = 1.f; s.note_sec(69, 1.4, "こんいろ"); return s.render(); }());
    put("phon_mumble_+1", [] { Singer s; s.style().mumble = 1.f; s.note_sec(69, 2.0, "かたさしすせそ"); return s.render(); }());
    put("phon_crisp_-1", [] { Singer s; s.style().mumble = -1.f; s.note_sec(69, 2.0, "かたさしすせそ"); return s.render(); }());
    put("phon_mumble_0_reference", [] { Singer s; s.note_sec(69, 2.0, "かたさしすせそ"); return s.render(); }());
    put("phon_vv_glide_off", [] { Singer s; s.note_sec(69, 1.6, "あいうえお"); return s.render(); }());
    put("phon_vv_glide_on", [] { Singer s; s.style().vv_glide = 1.f; s.note_sec(69, 1.6, "あいうえお"); return s.render(); }());
    put("phon_transition_snappy_2x", [] { Singer s; s.style().transition_speed = 2.f; s.note_sec(69, 1.6, "かさたなは"); return s.render(); }());
    put("phon_transition_smooth_0.5x", [] { Singer s; s.style().transition_speed = .5f; s.note_sec(69, 1.6, "かさたなは"); return s.render(); }());
    put("phon_mora_weighting_off", [] { Singer s; s.note_sec(69, 1.4, "かっさんー"); return s.render(); }());
    put("phon_mora_weighting_on", [] { Singer s; s.style().mora_weighting = 1.f; s.note_sec(69, 1.4, "かっさんー"); return s.render(); }());
    put("phon_glottal_final_off", [] { Singer s; s.note_sec(64, 0.6, "あっ"); s.rest_sec(0.3); s.note_sec(64, 0.6, "あっ"); return s.render(); }());
    put("phon_glottal_final_on", [] { Singer s; s.style().glottal_final = 1.f; s.note_sec(64, 0.6, "あっ"); s.rest_sec(0.3); s.note_sec(64, 0.6, "あっ"); return s.render(); }());

    // 5. pitch ornaments
    put("orn_none", held([](Singer&) {}, "あ", 69, 1.2));
    for (const char* o : { "fall", "doit", "shakuri", "scoop_in", "kobushi", "turn", "mordent", "trill" })
        put(std::string("orn_") + o, [&] { Singer s; s.note_sec(69, 1.2, "あ").ornament(o); return s.render(); }());
    put("pitch_loud_sharper_when_loud", [] { Singer s; s.style().pitch_loud = 1.f; s.note_sec(69, 1.0, "あ", 0.3f); s.note_sec(69, 1.0, "あ", 1.0f); return s.render(); }());
    put("pitch_snap_robotic", [] { Singer s; s.style().pitch_snap = 1.f; s.style().portamento = .25f; s.note_sec(60, 1.0, "あ"); s.note_sec(67, 1.0, "あ"); return s.render(); }());

    // 6. shaping
    put("shape_plain", phrase());
    put("shape_humanize_take1", phrase([](Singer& s) { s.humanize(0.015, 0.08, 0.05, 0.0, 1); }));
    put("shape_humanize_take2", phrase([](Singer& s) { s.humanize(0.015, 0.08, 0.05, 0.0, 2); }));
    put("shape_humanize_heavy", phrase([](Singer& s) { s.humanize(0.04, 0.2, 0.12, 0.01, 3); }));
    put("shape_auto_expression", phrase([](Singer& s) { s.auto_expression(); }));
    put("shape_arch", held([](Singer& s) { s.phrase_arc("arch", 0, 3.0); }, "あ", 69, 3.0));
    put("shape_crescendo", held([](Singer& s) { s.phrase_arc("crescendo", 0, 3.0, 1.5f); }, "あ", 69, 3.0));
    put("shape_decrescendo", held([](Singer& s) { s.phrase_arc("decrescendo", 0, 3.0, 1.5f); }, "あ", 69, 3.0));

    // 7. speech mode
    for (const char* k : { "statement", "question", "flat", "trail", "emphatic" })
        put(std::string("speech_そうなんだ_") + k, [&] { Singer s; s.say("そうなんだ", 1.3, k); return s.render(); }());
    for (const char* k : { "ne", "yo", "ka", "no" }) put(std::string("speech_particle_") + k, [&] { Singer s; s.say("いいよね", 1.2, k); return s.render(); }());

    // 8. mic stage
    { MicModel m; m.proximity = 1; m.air = 1; m.de_ess = 1; m.mouth_noise = 1;
      put("mic_dry", phrase()); put("mic_full_chain", phrase(), &m);
      MicModel p; p.proximity = 1; put("mic_proximity_only", phrase(), &p);
      MicModel a; a.air = 1; put("mic_air_only", phrase(), &a); }

    std::printf("\n%d files in %s/\n", N, OUT.c_str());
}
