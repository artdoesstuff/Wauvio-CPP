#pragma once

#include "synth.hpp"
#include "../wauvio_midi/core/midi_types.hpp"
#include "../wauvio_midi/core/midi_parser.hpp"
#include "../wauvio_midi/core/tempo_map.hpp"

#include <map>
#include <string>

namespace wauvio {
    namespace vocal {
        class Singer;

        // Returned by Singer::note(); chain setters to tweak that one note.
        // Valid only until the next call that adds a note to the same Singer.
        class NoteRef {
        public:
            explicit NoteRef(SynthNote& n) : n_(n) {}
            NoteRef& velocity(float v)             { n_.vel = v; return *this; }
            NoteRef& articulation(audio::Articulation a) { n_.art = a; return *this; }
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
        private:
            SynthNote& n_;
        };

        class Singer {
        public:
            Singer() = default;

            Singer& set_voice(const VoiceModel& v) { voice_ = v; return *this; }
            const VoiceModel& voice() const { return voice_; }
            Singer& set_tempo(double bpm, int ticks_per_quarter = 480) { bpm_ = bpm; ppq_ = ticks_per_quarter; return *this; }
            Style& style() { return style_; }                 // defaults applied to notes added afterwards
            const Style& style() const { return style_; }
            ParseOptions& parse_options() { return popt_; }
            Singer& phrase_pitch_curve(const Curve& semitones_vs_seconds) { phrase_curve_ = semitones_vs_seconds; return *this; }
            Singer& seed(uint32_t s) { seed_ = s; return *this; }

            NoteRef note(int midi, int ticks, const std::string& lyric, float velocity = 0.75f) {
                return note_sec(midi, ticks_to_sec(ticks), lyric, velocity);
            }
            Singer& rest(int ticks) { cursor_ += ticks_to_sec(ticks); return *this; }

            NoteRef note_sec(int midi, double seconds, const std::string& lyric, float velocity = 0.75f) {
                return note_at(cursor_, midi, seconds, lyric, velocity);
            }
            Singer& rest_sec(double seconds) { cursor_ += seconds; return *this; }
            // Absolute placement; the cursor moves to the end of the note.
            NoteRef note_at(double start_sec, int midi, double seconds, const std::string& lyric, float velocity = 0.75f) {
                SynthNote n;
                n.start = start_sec; n.dur = seconds; n.midi = midi; n.vel = velocity; n.style = style_;
                n.morae = parse_lyric(lyric, popt_);
                notes_.push_back(std::move(n));
                cursor_ = start_sec + seconds;
                return NoteRef(notes_.back());
            }

            void clear() { notes_.clear(); cursor_ = 0.0; phrase_curve_.clear(); }
            size_t size() const { return notes_.size(); }
            const std::vector<SynthNote>& notes() const { return notes_; }
            double cursor() const { return cursor_; }
            double ticks_to_sec(int ticks) const { return static_cast<double>(ticks) / ppq_ * 60.0 / bpm_; }

            Buffer render(int sample_rate = 0) const {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                std::vector<SynthNote> ns = notes_;
                std::stable_sort(ns.begin(), ns.end(), [](const SynthNote& a, const SynthNote& b) { return a.start < b.start; });
                if (ns.empty()) return Buffer();

                std::vector<Ev> evs = build_events(ns);
                Articulator art(voice_, sample_rate);
                double end_time = 0.0;
                KeyTrack kt = art.build(evs, end_time);

                std::vector<PitchNote> pn;
                for (size_t i = 0; i < ns.size(); ++i) {
                    PitchNote p;
                    p.start = ns[i].start; p.end = ns[i].start + ns[i].dur; p.midi = ns[i].midi;
                    p.style = ns[i].style; p.bend = ns[i].bend; p.glide_from = ns[i].glide_from; p.index = static_cast<int>(i);
                    p.connected = i > 0 && (ns[i].start - (ns[i - 1].start + ns[i - 1].dur)) < 0.012 && ns[i].start >= ns[i - 1].start;
                    p.followed_by_gap = (i + 1 >= ns.size()) || (ns[i + 1].start - p.end) > 0.03;
                    pn.push_back(p);
                }
                const double total = end_time + 0.5;
                PitchTrack pt = build_pitch_track(pn, total, voice_, phrase_curve_);
                VoiceRenderer vr(voice_, sample_rate);
                return vr.render(kt, pt, total, seed_);
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
            VoiceModel voice_;
            Style style_;
            ParseOptions popt_;
            std::vector<SynthNote> notes_;
            Curve phrase_curve_;
            double cursor_ = 0.0, bpm_ = 120.0;
            int ppq_ = 480;
            uint32_t seed_ = 0xA17E5u;
        };

        // Drop-in for Arrangement / MidiMusic parts: renders ONE note with a fixed lyric
        // (no coarticulation with neighbouring notes; use Singer for phrases).
        class SingerInstrument : public audio::Instrument {
        public:
            std::string lyric = "あ";
            Style style;
            VoiceModel voice_model;

            SingerInstrument() { name = "Japanese Female Singer"; }
            explicit SingerInstrument(std::string lyr) : lyric(std::move(lyr)) { name = "Japanese Female Singer"; }

            StereoBuffer render_note(const audio::Note& n, int sample_rate = 0) const override {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                if (n.is_rest()) return make_stereo(n.duration, sample_rate);
                Singer s; s.set_voice(voice_model); s.style() = style;
                s.style().volume *= static_cast<float>(n.expression);
                auto ref = s.note_sec(n.midi_note, n.duration, lyric, static_cast<float>(audio::dynamics_to_velocity(n.dynamics)));
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
            std::vector<audio::Articulation> supported_articulations() const override {
                return { audio::Articulation::Sustain, audio::Articulation::Legato, audio::Articulation::Staccato,
                         audio::Articulation::Whisper, audio::Articulation::Breath, audio::Articulation::Humming };
            }
        };

    }
}
