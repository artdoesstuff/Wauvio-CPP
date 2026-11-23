#pragma once

#include "synth.hpp"
#include "persona.hpp"
#include "events.hpp"
#include "expression.hpp"
#include "voices/voices.hpp"
#include "../../midi/core.hpp"

#include <deque>
#include <functional>
#include <map>
#include <string>

namespace wauvio {
    namespace vocal {
        namespace v3 {
            class Singer;

            // Returned by Singer::note(); chain setters to tweak that one note.
            // V3: stays valid for the life of the Singer (notes live in a std::deque, so adding more notes
            // never moves earlier ones); it is invalidated only by Singer::clear() or destroying the Singer.
            class NoteRef {
            public:
                NoteRef(SynthNote& n, const PersonaLibrary* lib) : n_(n), lib_(lib) {}
                NoteRef& velocity(float v)             { n_.vel = v; return *this; }
                NoteRef& articulation(wauvio::music::Articulation a) { n_.art = a; return *this; }
                NoteRef& vibrato(float depth_st, float rate_hz = -1.f, float delay_s = -1.f, float fade_s = -1.f) {
                    n_.style.vibrato_depth = depth_st; n_.style.vibrato_rate = rate_hz;
                    n_.style.vibrato_delay = delay_s;  n_.style.vibrato_fade = fade_s; return *this;
                }
                NoteRef& no_vibrato()                  { n_.style.vibrato_depth = 0.f; return *this; }
                NoteRef& portamento(float seconds)     { n_.style.portamento = seconds; return *this; }
                NoteRef& glide_from(double midi)       { n_.glide_from = midi; return *this; }
                NoteRef& bend(const Curve& semitones_vs_seconds) { n_.bend = semitones_vs_seconds; return *this; }
                NoteRef& overshoot(float o)            { n_.style.overshoot = o; return *this; }
                NoteRef& scoop(float semitones)        { n_.style.scoop = semitones; return *this; }
                Style&   style()                       { return n_.style; }

                // V2: expression and automation. Names: volume, brightness, breathiness, tension, formant,
                // mouth, growl, tremor, fry, air, register, plus "pitch" (semitones). Curve time = seconds
                // from the note start.
                NoteRef& lane(const std::string& name, const Curve& c) {
                    if (name == "pitch") { n_.bend = curve_add(n_.bend, c); return *this; }
                    Ch ch; if (channel_from_name(name, ch)) n_.lanes.push_back({ ch, c });
                    return *this;
                }
                // Persona for this note only (moves its look toward the persona by `amount`).
                NoteRef& persona(const std::string& name, float amount = 1.f) { if (lib_) apply_persona(*lib_, n_.style, name, amount); return *this; }
                // Morph this note's look toward a persona over time. amount: 0..1 vs seconds from note start.
                // Only channel-capable fields move inside a note (see persona_morph_lanes).
                NoteRef& persona_morph(const std::string& name, const Curve& amount) {
                    if (const Persona* p = lib_ ? lib_->find(name) : nullptr)
                        for (Lane& l : persona_morph_lanes(n_.style, *p, amount, std::max(n_.dur, amount.empty() ? 0.0 : amount.back().first)))
                            n_.lanes.push_back(std::move(l));
                    return *this;
                }
                NoteRef& growl(float v)         { n_.style.growl = v; return *this; }
                NoteRef& tremor(float v, float rate_hz = -1.f) { n_.style.tremor = v; if (rate_hz > 0) n_.style.tremor_rate = rate_hz; return *this; }
                NoteRef& fry(float v)           { n_.style.fry = v; return *this; }
                NoteRef& breathy(float v)       { n_.style.breathiness = v; return *this; }
                NoteRef& falsetto(float v)      { n_.style.reg = v; return *this; }
                NoteRef& formant_shift(float st){ n_.style.formant_shift = st; return *this; }
                NoteRef& mouth_open(float v)    { n_.style.mouth_open = v; return *this; }
                NoteRef& hard_onset(float v)    { n_.style.glottal_onset = v; return *this; }
                // Pitch ornament: fall, doit, shakuri, scoop_in, kobushi, turn, mordent, trill. depth scales size.
                NoteRef& ornament(const std::string& name, float depth = 1.f) {
                    Curve c = make_ornament(name, n_.dur, depth);
                    if (!c.empty()) n_.bend = curve_add(n_.bend, c);
                    return *this;
                }
            private:
                SynthNote& n_;
                const PersonaLibrary* lib_;
            };

            class Singer {
            public:
                Singer() { set_profile(voice_yune()); }          // Yune is the default voice
                explicit Singer(const VoiceProfile& p) { set_profile(p); }

                // Switch character: model, default look, lyric defaults and personas all come from the profile.
                // Call before adding notes (the default look applies to notes added afterwards).
                Singer& set_profile(const VoiceProfile& p) {
                    profile_ = p; voice_ = p.model; style_ = p.style; popt_ = p.parse;
                    // V3: the voice's own natural() strength (singer-sized imperfection for Yune) plus rule-based expression.
                    // Singer::natural(0) and Singer::auto_expression(false) switch it off again.
                    natural();
                    auto_.on = true;
                    return *this;
                }
                const VoiceProfile& profile() const { return profile_; }
                const PersonaLibrary& personas() const { return profile_.personas; }
                // Engine-level override of the tract model only (keeps the profile's personas and look).
                Singer& set_voice(const VoiceModel& v) { voice_ = v; return *this; }
                const VoiceModel& voice() const { return voice_; }
                Singer& set_tempo(double bpm, int ticks_per_quarter = 480) { bpm_ = bpm; ppq_ = ticks_per_quarter; return *this; }
                Style& style() { return style_; }                 // defaults applied to notes added afterwards
                const Style& style() const { return style_; }
                ParseOptions& parse_options() { return popt_; }
                Singer& phrase_pitch_curve(const Curve& semitones_vs_seconds) { phrase_curve_ = semitones_vs_seconds; return *this; }
                Singer& seed(uint32_t s) { seed_ = s; return *this; }
                Singer& accent_height(float semitones) { accent_height_ = semitones; return *this; }   // V3: L-H distance used by say(..., accent)
                // V2 personas. These move the Singer's default look for notes added afterwards.
                Singer& persona(const std::string& name, float amount = 1.f) { apply_persona(profile_.personas, style_, name, amount); return *this; }
                Singer& persona_blend(const std::string& a, const std::string& b, float t, float amount = 1.f) { apply_persona_blend(profile_.personas, style_, a, b, t, amount); return *this; }
                // Persona automation over the whole song: amount (0..1) vs absolute seconds, relative to the
                // current default look. Drive verse/chorus changes without re-annotating notes.
                Singer& persona_lane(const std::string& name, const Curve& amount) {
                    if (const Persona* p = profile_.personas.find(name); p && !amount.empty())
                        for (Lane& l : persona_morph_lanes(style_, *p, amount, amount.back().first, amount.front().first)) lanes_.push_back(std::move(l));
                    return *this;
                }
                // V2: small deterministic imperfections in timing, length and tuning. Different seeds = different takes.
                Singer& humanize(double timing_s = 0.012, double pitch_st = 0.05, double length = 0.03, double bias_s = 0.0, uint32_t take = 1) {
                    human_.on = true; human_.timing = timing_s; human_.pitch = pitch_st; human_.length = length; human_.bias = bias_s; human_.seed = take; return *this;
                }
                // V2: one call for a more living voice. amount 0..1 sets the `natural` voice control (jitter, wander, irregular
                // vibrato) AND per-note human imperfections (timing, tuning, entry error, length, phrase breath pressure).
                // `take` picks a different performance. amount 0 turns it off.
                Singer& natural(float amount = -1.f, uint32_t take = 1) {      // amount < 0: the voice's recommended strength
                    if (amount < 0.f) amount = profile_.natural_amount;
                    const double a = std::max(0.0, std::min(1.0, double(amount)));
                    style_.natural = static_cast<float>(a);
                    if (a <= 0.0) { human_ = Humanize(); return *this; }
                    // V3: timing, tuning and length are sized from a real singer (JSUT-song, measured at about 23 cents of pitch spread
                    // and 30 ms of timing at amount 0.3, the default of the Yune voice), so amount 0.3 = 60 ms / 0.9 / 3%.
                    // They grow linearly up to amount 0.6 (twice the singer) and stay there; tone, entry error and phrase pressure keep scaling to 1.
                    const double h = std::min(a, 0.6);
                    human_.on = true; human_.timing = 0.2 * h; human_.pitch = 3.0 * h; human_.length = 0.1 * h; human_.bias = 0.0;
                    human_.entry = 0.25 * a; human_.tone = a; human_.pressure = a; human_.seed = take; seed_ = 0xA17E5u + take * 7919u; return *this;
                }
                // V2: rule-based expression (vibrato shape, phrase-end, dynamics). Only fills fields left at defaults.
                Singer& auto_expression(bool on = true, double phrase_gap = 0.30) { auto_.on = on; auto_.phrase_gap = phrase_gap; return *this; }
                AutoExpr& auto_expression_options() { return auto_; }
                // V2: volume shape over [t0,t1] seconds: "arch", "crescendo", "decrescendo", "swell".
                Singer& phrase_arc(const std::string& kind, double t0, double t1, float depth = 1.f) {
                    if (t1 <= t0) return *this;
                    Curve c; for (int i = 0; i <= 16; ++i) { const double u = i / 16.0; c.push_back({ t0 + u * (t1 - t0), depth * phrase_arc_shape(kind, u) }); }
                    c.insert(c.begin(), { std::max(0.0, t0 - 0.001), 0.0 }); c.push_back({ t1 + 0.001, 0.0 });
                    lanes_.push_back({ Ch::Volume, c }); return *this;
                }
                // V2 speech: one utterance at `base_midi`. kind: statement, question, flat, trail, emphatic, ne, yo, ka, no.
                // Intonation moves pitch only; vibrato and scoop are off, morae are weighted by type.
                // V3: `accent` = Tokyo pitch-accent nucleus (0 heiban, 1 atamadaka, n = fall after mora n; -1 = none).
                // With an accent the sentence contour is reduced to 60% and the word-level H/L pattern rides on it.
                NoteRef say(const std::string& text, double seconds, const std::string& kind = "statement", int base_midi = 64, float velocity = 0.7f, int accent = -1) {
                    NoteRef r = note_sec(base_midi, seconds, text, velocity);
                    const double k = accent >= 0 ? 0.6 : 1.0;
                    Curve c; for (int i = 0; i <= 12; ++i) { const double u = i / 12.0; c.push_back({ u * seconds, k * speech_contour(kind, u) }); }
                    if (accent >= 0) c = curve_add(c, accent_contour(notes_.back().morae, seconds, accent, accent_height_, 0.0f, 1.f));
                    Style& st = r.style(); st.vibrato_depth = 0.f; st.scoop = 0.f; st.exit_fall = 0.f; st.mora_weighting = 1.f; st.portamento = 0.02f; st.medial_vot = 1.f;
                    r.bend(c); return r;
                }
                // Problems worth knowing about before rendering (empty = none).
                std::vector<std::string> diagnostics() const {
                    std::vector<std::string> d; int i = 0;
                    for (const SynthNote& n : notes_) {
                        ++i; const std::string at = "note " + std::to_string(i) + " @" + std::to_string(n.start).substr(0, 5) + "s: ";
                        if (!n.event.empty()) {
                            EventSpec sp;
                            if (!parse_event_tag(n.event, sp)) d.push_back(at + "unknown event tag " + n.event);
                            else for (const std::string& m : diagnose_event(sp)) d.push_back(at + m + " in " + n.event);   // V3: kind, variant and parameter checks
                            continue;
                        }
                        if (n.midi < voice_.hard_low || n.midi > voice_.hard_high) d.push_back(at + "pitch outside the voice's hard range");
                        else if (n.midi < voice_.range_low || n.midi > voice_.range_high) d.push_back(at + "pitch outside the comfortable range");
                        if (n.dur <= 0.0) d.push_back(at + "zero or negative length");
                        else if (!n.morae.empty() && n.dur / double(n.morae.size()) < 0.045) d.push_back(at + "too many morae for its length (" + std::to_string(n.morae.size()) + " in " + std::to_string(n.dur).substr(0, 5) + "s)");
                        if (n.morae.empty() && n.event.empty()) d.push_back(at + "no lyric");
                    }
                    return d;
                }
                // V2: Singer-wide automation lane (absolute seconds). See NoteRef::lane for names.
                Singer& lane(const std::string& name, const Curve& c) {
                    if (name == "pitch") { phrase_curve_ = curve_add(phrase_curve_, c); return *this; }
                    Ch ch; if (channel_from_name(name, ch)) lanes_.push_back({ ch, c });
                    return *this;
                }

                NoteRef note(int midi, int ticks, const std::string& lyric, float velocity = 0.75f) {
                    return note_sec(midi, ticks_to_sec(ticks), lyric, velocity);
                }
                Singer& rest(int ticks) { cursor_ += ticks_to_sec(ticks); return *this; }

                NoteRef note_sec(int midi, double seconds, const std::string& lyric, float velocity = 0.75f) {
                    return note_at(cursor_, midi, seconds, lyric, velocity);
                }
                Singer& rest_sec(double seconds) { cursor_ += seconds; return *this; }
                // V2 events at the cursor: laugh, giggle, sob, hum, shout, kyaa and
                // interjections (ah, eh, un, fuun, uun, eeto, ano, waa, hya, hee, eee, maa). Same as the lyric
                // tag, e.g. note_sec(69, 1.2, "<laugh>"). `variant` and extra "key=value" go in the tag.
                NoteRef event(const std::string& kind, int midi, double seconds, float intensity = 0.6f, const std::string& variant = "") {
                    std::string tag = "<" + kind + (variant.empty() ? "" : ":" + variant) + ",intensity=" + std::to_string(intensity) + ">";
                    return note_sec(midi, seconds, tag);
                }
                // Absolute placement; the cursor moves to the end of the note.
                NoteRef note_at(double start_sec, int midi, double seconds, const std::string& lyric, float velocity = 0.75f) {
                    SynthNote n;
                    n.start = start_sec; n.dur = seconds; n.midi = midi; n.vel = velocity; n.style = style_;
                    if (is_event_tag(lyric)) n.event = lyric;           // expanded at render time (see events.hpp)
                    else n.morae = parse_lyric(lyric, popt_);
                    notes_.push_back(std::move(n));
                    cursor_ = start_sec + seconds;
                    return NoteRef(notes_.back(), &profile_.personas);
                }

                void clear() { notes_.clear(); cursor_ = 0.0; phrase_curve_.clear(); lanes_.clear(); }
                size_t size() const { return notes_.size(); }
                const std::deque<SynthNote>& notes() const { return notes_; }
                double cursor() const { return cursor_; }
                double ticks_to_sec(int ticks) const { return static_cast<double>(ticks) / ppq_ * 60.0 / bpm_; }

                // Debug/listening hook: when set, every finished render is passed to it.
                using TapFn = std::function<void(const Buffer&, int)>;
                static TapFn& render_tap() { static TapFn f; return f; }

                Buffer render(int sample_rate = 0) const {
                    Buffer r = render_impl(sample_rate);
                    if (render_tap()) render_tap()(r, sample_rate > 0 ? sample_rate : global_config().sample_rate);
                    return r;
                }

                Buffer render_impl(int sample_rate) const {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    if (notes_.empty()) return Buffer();
                    VoiceStream st = stream(sample_rate);
                    return st.read_all();
                }

                /// V3: on-demand rendering. The score is analysed once when the stream is created; audio is then pulled
                /// in chunks of any size with VoiceStream::read(). The samples are identical to render(), however the
                /// reads are chunked. (Notes added after the stream was created are not heard by it.)
                VoiceStream stream(int sample_rate = 0) const {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    std::vector<SynthNote> ns = prepare_notes();
                    if (ns.empty()) return VoiceStream(voice_, sample_rate, KeyTrack(), PitchTrack(), ExprTrack(), false, 0.0, seed_);

                    std::vector<Ev> evs = build_events(ns);
                    Articulator art(voice_, sample_rate, seed_);
                    double end_time = 0.0;
                    KeyTrack kt = art.build(evs, end_time);

                    std::vector<PitchNote> pn;
                    for (size_t i = 0; i < ns.size(); ++i) {
                        PitchNote p;
                        p.start = ns[i].start; p.end = ns[i].start + ns[i].dur; p.midi = ns[i].midi;
                        p.style = ns[i].style; p.bend = ns[i].bend; p.glide_from = ns[i].glide_from; p.index = static_cast<int>(i); p.vel = ns[i].vel;
                        p.connected = i > 0 && (ns[i].start - (ns[i - 1].start + ns[i - 1].dur)) < 0.012 && ns[i].start >= ns[i - 1].start;
                        p.followed_by_gap = (i + 1 >= ns.size()) || (ns[i + 1].start - p.end) > 0.03;
                        pn.push_back(p);
                    }
                    const double total = end_time + 0.5;
                    PitchTrack pt = build_pitch_track(pn, total, voice_, phrase_curve_, seed_);
                    std::vector<ExprNote> en;
                    for (const SynthNote& n : ns) { ExprNote e; e.start = n.start; e.end = n.start + n.dur; e.style = n.style; e.lanes = n.lanes; en.push_back(std::move(e)); }
                    ExprTrack ex = build_expr_track(en, lanes_, total);
                    return VoiceStream(voice_, sample_rate, std::move(kt), std::move(pt), std::move(ex), true, total, seed_);
                }

                /// V3: the articulation control track (formants, bandwidths, noise, amplitude per key-frame) exactly as the
                /// synthesiser will receive it, before pitch tracking and expression. For inspection and tooling
                /// (tools/locus_probe.cpp measures consonant loci from it).
                KeyTrack key_track(int sample_rate = 0) const {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    std::vector<SynthNote> ns = prepare_notes();
                    if (ns.empty()) return KeyTrack();
                    std::vector<Ev> evs = build_events(ns);
                    Articulator art(voice_, sample_rate, seed_);
                    double end_time = 0.0;
                    return art.build(evs, end_time);
                }

                /// V3: a copy with every note moved by `semitones` (for harmonies). Personas, look and lanes are kept.
                Singer transposed(int semitones) const {
                    Singer c(*this);
                    for (SynthNote& n : c.notes_) { n.midi += semitones; if (n.glide_from >= 0.0) n.glide_from += semitones; }
                    return c;
                }
                /// V3: a copy that performs the same notes as a different take: its own timing/pitch imperfection and
                /// voice wobble (`natural_amount`), optionally detuned by a constant number of cents.
                Singer doubled(uint32_t take, float natural_amount = 0.3f, double detune_cents = 0.0) const {
                    Singer c(*this);
                    c.natural(natural_amount, take);
                    for (SynthNote& n : c.notes_) n.style.natural = natural_amount;
                    if (detune_cents != 0.0) c.phrase_curve_ = curve_add(c.phrase_curve_, Curve{ { 0.0, detune_cents / 100.0 }, { 1.0e5, detune_cents / 100.0 } });
                    return c;
                }

                StereoBuffer render_stereo(int sample_rate = 0) const {
                    Buffer m = render(sample_rate);
                    return pan_mono(m, 0.0f, sample_rate > 0 ? sample_rate : global_config().sample_rate);
                }

                void save(const std::string& path, int sample_rate = 0) const {
                    Buffer b = render(sample_rate);
                    normalize(b, 0.9f);
                    save_wav(b, path, sample_rate);
                }

                // Reads notes (and Lyric/Text meta events) from one track of a parsed MIDI
                // file. track < 0 picks the first track that contains lyrics. Notes without a
                // lyric continue the previous vowel (melisma). Lyrics may be UTF-8 or
                // Shift-JIS kana. Returns the number of notes added.
                size_t load_midi_track(const midi::ParsedMidiFile& f, int track = -1) {
                    using namespace midi;
                    auto has_lyrics = [&](const ParsedTrack& t) {
                        for (auto& e : t.events) if (e.type == MidiEventType::Meta && e.meta_type == 0x05) return true;
                        return false;
                    };
                    if (track < 0) {
                        for (size_t i = 0; i < f.tracks.size(); ++i) if (has_lyrics(f.tracks[i])) { track = static_cast<int>(i); break; }
                        if (track < 0) {   // fall back: first track with notes
                            for (size_t i = 0; i < f.tracks.size() && track < 0; ++i)
                                for (auto& e : f.tracks[i].events) if (e.type == MidiEventType::NoteOn) { track = static_cast<int>(i); break; }
                        }
                    }
                    if (track < 0 || static_cast<size_t>(track) >= f.tracks.size()) return 0;

                    TempoMap tm;
                    for (auto& tr : f.tracks) for (auto& e : tr.events)
                        if (e.type == MidiEventType::Meta && e.meta_type == 0x51 && e.meta_data.size() == 3) {
                            uint32_t mpq = (uint32_t(e.meta_data[0]) << 16) | (uint32_t(e.meta_data[1]) << 8) | e.meta_data[2];
                            if (mpq > 0) tm.add_tempo_event(e.abs_tick, mpq);
                        }
                    if (f.smpte_timing) tm.finalize_smpte(f.smpte_fps, f.smpte_ticks_per_frame); else tm.finalize(f.division);

                    const ParsedTrack& tr = f.tracks[static_cast<size_t>(track)];
                    bool use_lyric_meta = has_lyrics(tr);
                    std::vector<std::pair<uint64_t, std::string>> lyrics;
                    for (auto& e : tr.events) {
                        if (e.type != MidiEventType::Meta) continue;
                        if (e.meta_type == 0x05 || (!use_lyric_meta && e.meta_type == 0x01)) {
                            std::string s = decode_lyric_bytes(e.meta_data);
                            if (!s.empty() && s[0] == '@') continue;            // karaoke header lines
                            lyrics.emplace_back(e.abs_tick, s);
                        }
                    }
                    const uint64_t tol = f.division / 8u;
                    struct Held { uint64_t tick; uint8_t vel; };
                    std::map<int, Held> held;
                    size_t li = 0, added = 0;
                    struct Raw { uint64_t on, off; int key; uint8_t vel; };
                    std::vector<Raw> raws;
                    for (auto& e : tr.events) {
                        if (e.type == MidiEventType::NoteOn) held[e.data1] = { e.abs_tick, e.data2 };
                        else if (e.type == MidiEventType::NoteOff) {
                            auto it = held.find(e.data1);
                            if (it != held.end()) { raws.push_back({ it->second.tick, e.abs_tick, e.data1, it->second.vel }); held.erase(it); }
                        }
                    }
                    std::stable_sort(raws.begin(), raws.end(), [](const Raw& a, const Raw& b) { return a.on < b.on; });
                    // monophonic: if notes overlap, keep the highest started later one by trimming
                    for (size_t i = 0; i < raws.size(); ++i) {
                        const Raw& r = raws[i];
                        uint64_t off = r.off;
                        if (i + 1 < raws.size() && raws[i + 1].on < off) off = raws[i + 1].on;
                        if (off <= r.on) continue;
                        std::string lyr;
                        while (li < lyrics.size() && lyrics[li].first <= r.on + tol) {
                            if (!lyrics[li].second.empty()) lyr = lyrics[li].second;
                            ++li;
                        }
                        double t0 = tm.tick_to_seconds(r.on), t1 = tm.tick_to_seconds(off);
                        note_at(t0, r.key, t1 - t0, lyr, static_cast<float>(r.vel) / 127.f);
                        ++added;
                    }
                    return added;
                }

                size_t load_midi_file(const std::string& path, int track = -1) {
                    return load_midi_track(midi::parse_midi_file(path), track);
                }

            private:
                // sort, then expand event tags into ordinary notes (per-event deterministic seeds)
                std::vector<SynthNote> prepare_notes() const {
                    std::vector<SynthNote> ns(notes_.begin(), notes_.end());
                    std::stable_sort(ns.begin(), ns.end(), [](const SynthNote& a, const SynthNote& b) { return a.start < b.start; });
                    apply_humanize(ns, human_);
                    apply_pressure(ns, human_);
                    std::vector<SynthNote> out;
                    for (size_t i = 0; i < ns.size(); ++i) {
                        if (ns[i].event.empty()) { out.push_back(std::move(ns[i])); continue; }
                        EventSpec sp;
                        if (!parse_event_tag(ns[i].event, sp)) continue;
                        for (SynthNote& e : expand_event(ns[i], sp, seed_ ^ (static_cast<uint32_t>(i + 1) * 2654435761u))) out.push_back(std::move(e));
                    }
                    std::stable_sort(out.begin(), out.end(), [](const SynthNote& a, const SynthNote& b) { return a.start < b.start; });
                    apply_auto_expression(out, auto_, voice_);
                    for (SynthNote& n : out) {                       // grit: roughness that comes in only when pushed (velocity > 0.65)
                        if (n.kind == NoteKind::Normal && n.event.empty() && n.style.grit > 0.f && n.vel > 0.65f) {
                            const double push = std::min(1.0, (double(n.vel) - 0.65) / 0.30) * double(n.style.grit);
                            n.lanes.push_back({ Ch::Growl, Curve{ { 0.0, 0.0 }, { std::min(0.06, 0.3 * n.dur), 0.8 * push }, { n.dur, 0.8 * push } } });   // eases in
                            n.lanes.push_back({ Ch::Fry, Curve{ { 0.0, 0.0 }, { n.dur, 0.12 * push } } });
                        }
                    }
                    for (SynthNote& n : out) {                       // final っ: creak and cut (glottal stop)
                        if (n.kind == NoteKind::Normal && n.style.glottal_final > 0.f && !n.morae.empty() && n.morae.back().geminate) {
                            // vowel ends where the っ share begins (exactly the boundary build_events uses)
                            const std::vector<double> cum = mora_cum(n.morae, n.style.mora_weighting);
                            const double ve = n.dur * cum[n.morae.size() - 1], L = std::min(0.14, 0.6 * ve), g = double(n.style.glottal_final);
                            n.lanes.push_back({ Ch::Fry, { { 0.0, 0.0 }, { ve - L, 0.0 }, { ve - 0.3 * L, g }, { ve + 0.05, g } } });
                            n.lanes.push_back({ Ch::Volume, { { 0.0, 0.0 }, { ve - L, 0.0 }, { ve, -0.5 * g } } });
                        }
                    }
                    return out;
                }

                VoiceProfile profile_;
                VoiceModel voice_;
                Style style_;
                ParseOptions popt_;
                std::deque<SynthNote> notes_;
                Curve phrase_curve_;
                std::vector<Lane> lanes_;
                Humanize human_; AutoExpr auto_;
                double cursor_ = 0.0, bpm_ = 120.0;
                int ppq_ = 480;
                uint32_t seed_ = 0xA17E5u;
                float accent_height_ = 3.0f;
            };

            // Drop-in for Arrangement / MidiMusic parts: renders ONE note with a fixed lyric
            // (no coarticulation with neighbouring notes; use Singer for phrases).
            class SingerInstrument : public wauvio::music::Instrument {
            public:
                std::string lyric = "あ";
                Style style;
                VoiceProfile profile = voice_yune();   // which character sings (default Yune)

                SingerInstrument() { name = "Japanese Female Singer"; }
                explicit SingerInstrument(std::string lyr) : lyric(std::move(lyr)) { name = "Japanese Female Singer"; }

                StereoBuffer render_note(const wauvio::music::Note& n, int sample_rate = 0) const override {
                    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                    if (n.is_rest()) return make_stereo(n.duration, sample_rate);
                    Singer s(profile); s.style() = style;
                    s.style().volume *= static_cast<float>(n.expression);
                    auto ref = s.note_sec(n.midi_note, n.duration, lyric, static_cast<float>(wauvio::music::dynamics_to_velocity(n.dynamics)));
                    ref.articulation(n.articulation);
                    if (n.glide_from_midi >= 0) { ref.glide_from(n.glide_from_midi); ref.portamento(static_cast<float>(n.glide_time)); }
                    Curve c;
                    for (auto& p : n.pitch_bend_curve) c.push_back(p);
                    if (c.empty() && n.pitch_bend_semitones != 0.0) c.push_back({ 0.0, n.pitch_bend_semitones });
                    if (!c.empty()) ref.bend(c);
                    Buffer m = s.render(sample_rate);
                    m.resize(static_cast<size_t>((n.duration + 0.4) * sample_rate), 0.f);
                    return pan_mono(m, static_cast<float>(n.pan), sample_rate);
                }
                std::vector<wauvio::music::Articulation> supported_articulations() const override {
                    return { wauvio::music::Articulation::Sustain, wauvio::music::Articulation::Legato, wauvio::music::Articulation::Staccato,
                             wauvio::music::Articulation::Whisper, wauvio::music::Articulation::Breath, wauvio::music::Articulation::Humming };
                }
            };

        }
    }
}
