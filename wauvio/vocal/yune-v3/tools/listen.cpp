// listen: renders A/B listening demos for every V2 and V3 feature into a folder of WAVs.
// usage: listen [outdir=listen_out] [only-substring]
// Each demo is named NNN_area_name.wav. Sections 1-8 are the V2 demos (V3 defaults now include n_nasalize and
// mora_weighting, so those A/B pairs say which setting they use); section 9 is new in V3.
#include "../../v3_fx.hpp"
#include <cstdio>
#include <filesystem>
#include <functional>
#include <string>

using namespace wauvio; using namespace wauvio::vocal::v3;
static std::string OUT = "listen_out", ONLY; static int N = 0;

static void put(const std::string& name, Buffer b, const MicModel* mic = nullptr) {
    if (!ONLY.empty() && name.find(ONLY) == std::string::npos) return;
    if (mic) apply_mic(b, global_config().sample_rate, *mic);
    normalize(b, 0.9f); char id[16]; std::snprintf(id, sizeof id, "%03d_", ++N);
    save_wav(b, OUT + "/" + id + name + ".wav"); std::printf("%s%s\n", id, name.c_str());
}
static void putS(const std::string& name, StereoBuffer b) {
    if (!ONLY.empty() && name.find(ONLY) == std::string::npos) return;
    float pk = 0.f; for (size_t i = 0; i < b.size(); ++i) pk = std::max(pk, std::max(std::fabs(b.L[i]), std::fabs(b.R[i])));
    if (pk > 1e-6f) for (size_t i = 0; i < b.size(); ++i) { b.L[i] *= 0.9f / pk; b.R[i] *= 0.9f / pk; }
    char id[16]; std::snprintf(id, sizeof id, "%03d_", ++N);
    save_wav_stereo(b, OUT + "/" + id + name + ".wav"); std::printf("%s%s (stereo)\n", id, name.c_str());
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
static Singer phrase_singer(const Setup& f = [](Singer&) {}) {
    Singer s; f(s);
    const char* syl[] = { "さ", "く", "ら", "さ", "く", "ら", "や", "よ" };
    const int pit[] = { 69, 69, 71, 69, 69, 71, 72, 76 };
    const int len[] = { 480, 480, 960, 480, 480, 960, 480, 1920 };
    for (int i = 0; i < 8; ++i) s.note(pit[i], len[i], syl[i]);
    return s;
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
    put("phon_n_closure_only_(n_nasalize=0)", [] { Singer s; s.style().n_nasalize = 0.f; s.note_sec(69, 1.4, "こんいろ"); return s.render(); }());
    put("phon_n_nasalized_(V3_default)", [] { Singer s; s.note_sec(69, 1.4, "こんいろ"); return s.render(); }());
    put("phon_mumble_+1", [] { Singer s; s.style().mumble = 1.f; s.note_sec(69, 2.0, "かたさしすせそ"); return s.render(); }());
    put("phon_crisp_-1", [] { Singer s; s.style().mumble = -1.f; s.note_sec(69, 2.0, "かたさしすせそ"); return s.render(); }());
    put("phon_mumble_0_reference", [] { Singer s; s.note_sec(69, 2.0, "かたさしすせそ"); return s.render(); }());
    put("phon_vv_glide_off", [] { Singer s; s.note_sec(69, 1.6, "あいうえお"); return s.render(); }());
    put("phon_vv_glide_on", [] { Singer s; s.style().vv_glide = 1.f; s.note_sec(69, 1.6, "あいうえお"); return s.render(); }());
    put("phon_transition_snappy_2x", [] { Singer s; s.style().transition_speed = 2.f; s.note_sec(69, 1.6, "かさたなは"); return s.render(); }());
    put("phon_transition_smooth_0.5x", [] { Singer s; s.style().transition_speed = .5f; s.note_sec(69, 1.6, "かさたなは"); return s.render(); }());
    put("phon_mora_weighting_off", [] { Singer s; s.style().mora_weighting = 0.f; s.note_sec(69, 1.4, "かっさんー"); return s.render(); }());
    put("phon_mora_weighting_on_(V3_default)", [] { Singer s; s.note_sec(69, 1.4, "かっさんー"); return s.render(); }());
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

    // 9. V3 -----------------------------------------------------------------------------------------------------
    // 9a. particles: は/へ read as わ/え
    put("v3_particles_off_わたしは_さくらへ", [] { Singer s; s.note_sec(67, 1.6, "わたしは"); s.note_sec(69, 1.4, "さくらへ"); return s.render(); }());
    put("v3_particles_on_わたしは_さくらへ", [] { Singer s; s.parse_options().particles = true; s.note_sec(67, 1.6, "わたしは"); s.note_sec(69, 1.4, "さくらへ"); return s.render(); }());
    put("v3_particles_marked_は^_へ^", [] { Singer s; s.note_sec(67, 1.6, "わたしは^"); s.note_sec(69, 1.4, "さくらへ^"); return s.render(); }());
    // 9b. ヴ is a real [v], compared with ぶ
    put("v3_v_ヴぁヴぃヴヴぇヴぉ", [] { Singer s; for (const char* l : { "ヴぁ", "ヴぃ", "ヴ", "ヴぇ", "ヴぉ" }) s.note_sec(69, 0.5, l); return s.render(); }());
    put("v3_b_ばびぶべぼ_reference", [] { Singer s; for (const char* l : { "ば", "び", "ぶ", "べ", "ぼ" }) s.note_sec(69, 0.5, l); return s.render(); }());
    // 9c. pitch accent: 端 (heiban) / 箸 (atamadaka) / 橋 (nucleus 2), each with the particle が so the fall is audible
    for (int a : { 0, 1, 2 })
        put("v3_accent_はしが_nucleus" + std::to_string(a), [&] { Singer s; s.say("はしが", 0.9, "flat", 62, 0.7f, a); return s.render(); }());
    put("v3_accent_こんにちは_heiban_vs_nucleus3", [] { Singer s; s.say("こんにちは", 1.1, "statement", 62, 0.7f, 0); s.rest_sec(0.4); s.say("こんにちは", 1.1, "statement", 62, 0.7f, 3); return s.render(); }());
    // 9d. vibrato: coupled amplitude, skewed shape, rate drift (V2-like = all three at 0)
    auto vib = [](float am, float shape, float accel) { return [=](Singer& s) { s.style().vib_am = am; s.style().vib_shape = shape; s.style().vib_accel = accel; s.style().vibrato_depth = 0.40f; }; };
    put("v3_vibrato_v2like_am0_shape0_accel0", held(vib(0, 0, 0), "あ", 69, 3.5));
    put("v3_vibrato_default_am.5_shape.4_accel.05", held(vib(0.5f, 0.4f, 0.05f), "あ", 69, 3.5));
    put("v3_vibrato_am_only", held(vib(1.0f, 0, 0), "あ", 69, 3.5));
    put("v3_vibrato_shape_only", held(vib(0, 1.0f, 0), "あ", 69, 3.5));
    put("v3_vibrato_accel_only", held(vib(0, 0, 0.30f), "あ", 69, 3.5));
    put("v3_vibrato_strong_all", held(vib(1.0f, 0.8f, 0.2f), "あ", 69, 3.5));
    // 9e. coarticulation (vowel leans toward the next vowel across a consonant)
    for (float c : { 0.f, 0.6f, 1.f })
        put("v3_coarticulation_" + std::to_string(int(c * 10)) + "_あかいきあおくえ", [&] { Singer s; s.style().coarticulation = c; s.note_sec(64, 3.2, "あかいきあおくえ"); return s.render(); }());
    // 9f. nasal anti-formant and pole (off = V2-style subtractive notch only)
    put("v3_nasal_notch_pole_on_なんまみむ", [] { Singer s; s.note_sec(62, 2.4, "あんなんまみむんー"); return s.render(); }());
    put("v3_nasal_notch_pole_off", [] { Singer s; VoiceModel v = s.voice(); v.nasal_zero_depth = 0.f; v.nasal_pole_gain = 0.f; v.nasal_bw_widen = 0.f; s.set_voice(v); s.note_sec(62, 2.4, "あんなんまみむんー"); return s.render(); }());
    // 9g. source: bright, tense, high notes (this is where band-limiting matters)
    put("v3_source_bright_high_C6_G6_C7", [] { Singer s; s.style().brightness = 1.f; s.style().tension = 1.f; s.style().power = 1.f; s.style().vibrato_depth = 0.f; for (int m : { 84, 91 }) s.note_sec(m, 1.2, "あ"); return s.render(); }());
    // 9h. consonants with locus equations / log formant motion, every voice
    for (const std::string& id : voice_ids()) {
        VoiceProfile p; make_voice(id, p);
        put("v3_consonants_" + id, [&] { Singer s(p); const int b = p.model.range_low + 12; int i = 0; for (const char* l : { "たちつてと", "なにぬねの", "かきくけこ", "ばびぶべぼ", "さしすせそ", "らりるれろ" }) s.note_sec(b + (i++ % 3) * 2, 0.9, l); return s.render(); }());
    }
    // 9i. ensembles
    { Singer lead = phrase_singer([](Singer& s) { s.natural(0.3f, 1); });
      putS("v3_ensemble_1_lead_alone", pan_mono(lead.render(), 0.f));
      { Ensemble e; e.add(lead, "lead"); e.add_double(lead, 2); putS("v3_ensemble_2_lead_plus_2_doubles", e.render_stereo()); }
      { Ensemble e; e.add(lead, "lead"); e.add_harmony(lead, 4, "third", -0.5f, 0.55f); e.add_harmony(lead, -5, "low", 0.5f, 0.55f); putS("v3_ensemble_3_harmony_third_and_low", e.render_stereo()); }
      { Ensemble e; e.add(lead, "lead"); e.add_double(lead, 2); e.add_harmony(lead, 4, "third", -0.5f, 0.55f); e.add_harmony(lead, -5, "low", 0.5f, 0.55f);
        const VoiceProfile kh = voice_koharu(); e.add_harmony(lead, 7, "fifth_koharu", -0.25f, 0.4f, &kh);
        putS("v3_ensemble_4_full_choir_5_voices_3_timbres", e.render_stereo()); }
      { const VoiceProfile mio = voice_mio(), akari = voice_akari();        // the Yune family: Yune leads, Akari a third above, Mio a fourth below
        Ensemble e; e.add(lead, "yune_lead"); e.add_harmony(lead, 4, "akari_third", -0.45f, 0.65f, &akari); e.add_harmony(lead, -5, "mio_low", 0.45f, 0.65f, &mio);
        putS("v3_ensemble_5_trio_yune_akari_mio", e.render_stereo()); }
      for (const std::string& preset : vocal_chain_presets()) putS("v3_chain_" + preset + "_lead", process_vocal(lead.render(), preset, nullptr, 0, 120.0));
      { Ensemble e; e.add(lead, "lead"); e.add_double(lead, 2); e.add_harmony(lead, 4, "third", -0.5f, 0.55f); putS("v3_chain_pop_on_ensemble", process_vocal(e.render_stereo(), "pop", 0, 120.0)); } }
    // 9j. streaming: same audio as render(), pulled in small chunks (writes the streamed result)
    put("v3_stream_in_256_sample_chunks", [] { Singer s = phrase_singer(); VoiceStream st = s.stream(); Buffer o; std::vector<float> c(256); for (;;) { size_t n = st.read(c.data(), c.size()); if (!n) break; o.insert(o.end(), c.begin(), c.begin() + static_cast<std::ptrdiff_t>(n)); } return o; }());


    // 10. human delivery (V3: a Singer now starts at natural = the voice's own strength (singer-sized for Yune), auto_expression on)
    auto hp = [](float nat, bool ae, uint32_t take) { Singer s = phrase_singer([&](Singer& x) { x.natural(nat, take); x.auto_expression(ae); }); return s.render(); };
    put("v3_human_phrase_0_even_(natural_0)", hp(0.f, false, 1));
    put("v3_human_phrase_1_default_singer_sized_(natural_0.3)", phrase_singer().render());
    put("v3_human_phrase_2_twice_singer_(natural_0.6)", hp(0.6f, true, 1));
    put("v3_human_phrase_3_tone_at_full_(natural_1.0)", hp(1.0f, true, 1));
    put("v3_human_phrase_4_three_takes_default_amount", [&] { Buffer o; for (uint32_t t : { 1u, 2u, 3u }) { Buffer b = hp(0.3f, true, t); o.insert(o.end(), b.begin(), b.end()); o.insert(o.end(), 22050, 0.f); } return o; }());
    for (auto cfg : { std::make_pair(0.f, std::string("even")), std::make_pair(0.3f, std::string("default")), std::make_pair(0.6f, std::string("twice_singer")) })
        put("v3_human_eight_identical_notes_" + cfg.second, [&] { Singer s; s.natural(cfg.first, 1); s.auto_expression(cfg.first > 0.f); s.style().vibrato_depth = 0.f;
            for (int i = 0; i < 8; ++i) { s.note(64, 360, "あ"); s.rest(120); } return s.render(); }());
    for (const std::string& id : voice_ids()) {
        VoiceProfile p; make_voice(id, p);
        put("v3_human_voice_" + id + "_even_then_default", [&] { Buffer a = hp(0.f, false, 1); Singer s(p); const char* syl[] = { "さ","く","ら","さ","く","ら","や","よ" }; const int pit[] = { 69,69,71,69,69,71,72,76 }, len[] = { 480,480,960,480,480,960,480,1920 };
            Singer e(p); e.natural(0.f); e.auto_expression(false); for (int i = 0; i < 8; ++i) { e.note(pit[i], len[i], syl[i]); s.note(pit[i], len[i], syl[i]); }
            Buffer x = e.render(), y = s.render(); x.insert(x.end(), 22050, 0.f); x.insert(x.end(), y.begin(), y.end()); (void)a; return x; }());
    }
    std::printf("\n%d files in %s/\n", N, OUT.c_str());
}
