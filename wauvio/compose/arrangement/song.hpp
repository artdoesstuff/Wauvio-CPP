#pragma once

#include "../melody/melody_gen.hpp"
#include "../wcp/wcp_format.hpp"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace wauvio {
    namespace compose {

        class DrumKitInstrument : public wauvio::music::Instrument {
        public:
            std::shared_ptr<wauvio::music::DrumKit> kit;

            explicit DrumKitInstrument(std::shared_ptr<wauvio::music::DrumKit> k = nullptr) : kit(std::move(k)) {
                name = kit ? kit->name : "DrumKit";
            }

            StereoBuffer render_note(const wauvio::music::Note& note, int sample_rate = 0) const override {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                if (note.is_rest() || note.duration <= 0.0 || !kit)
                    return make_stereo(std::max(0.0, note.duration), sample_rate);
                const double hit_dur = std::max(0.35, note.duration);
                StereoBuffer audio = kit->play(note.midi_note, note.dynamics, hit_dur, sample_rate).audio;
                if (note.expression != 1.0)
                    apply_gain(audio, static_cast<float>(std::clamp(note.expression, 0.0, 2.0)));
                return audio;
            }
        };

        enum class AccompanimentPattern {
            Pad,
            BlockChords,
            ArpUp,
            ArpDown,
            ArpBounce,
            Alberti,
            PulseEighths,
            OffbeatSkank
        };

        enum class BassPattern {
            RootWhole,
            RootFifth,
            OctaveBounce,
            DrivingEighths,
            Walking,
            Syncopated
        };

        enum class DrumPattern {
            Auto,
            None,
            PopRock,
            FourOnFloor,
            HalfTime,
            Breakbeat,
            Waltz,
            JazzSwing,
            Bossa,
            LoFi
        };

        enum class Mood { Bright, Melancholic, Epic, Chill, Energetic, Jazzy, Ambient, Retro };

        enum class SectionRole { Intro, Verse, PreChorus, Chorus, Bridge, Breakdown, Outro };

        inline const char* section_role_name(SectionRole r) noexcept {
            switch (r) {
                case SectionRole::Intro:     return "Intro";
                case SectionRole::Verse:     return "Verse";
                case SectionRole::PreChorus: return "Pre-Chorus";
                case SectionRole::Chorus:    return "Chorus";
                case SectionRole::Bridge:    return "Bridge";
                case SectionRole::Breakdown: return "Breakdown";
                case SectionRole::Outro:     return "Outro";
            }
            return "Section";
        }

        namespace pattern_detail {

            inline wauvio::music::Melody generate_arp_track(const std::vector<ChordSymbol>& chords,
                                                            AccompanimentPattern pat,
                                                            double bpm,
                                                            int low_midi, int high_midi,
                                                            wauvio::music::Dynamics dyn)
            {
                using namespace wauvio::music;
                Melody out;
                const double sec_per_beat = 60.0 / std::max(20.0, bpm);
                std::vector<int> prev_v;

                for (const auto& ch : chords) {
                    prev_v = voice_lead(prev_v, ch, low_midi, high_midi, 4);
                    if (prev_v.empty()) { out.push_back(rest(ch.duration_beats * sec_per_beat)); continue; }

                    if (pat == AccompanimentPattern::OffbeatSkank) {
                        double rem = ch.duration_beats;
                        while (rem > 1e-6) {
                            double r1 = std::min(0.5, rem);
                            out.push_back(rest(r1 * sec_per_beat));
                            rem -= r1;
                            if (rem <= 1e-6) break;
                            double h1 = std::min(0.5, rem);
                            out.push_back(Note(prev_v.back(), h1 * sec_per_beat, dyn, Articulation::Staccato));
                            rem -= h1;
                        }
                        continue;
                    }

                    const double step_b = 0.5;
                    const int steps = std::max(1, static_cast<int>(std::round(ch.duration_beats / step_b)));
                    const double actual_sec = (ch.duration_beats / steps) * sec_per_beat;
                    const size_t nv = prev_v.size();

                    for (int s = 0; s < steps; ++s) {
                        int note_midi = prev_v[0];
                        switch (pat) {
                            case AccompanimentPattern::ArpUp:
                                note_midi = prev_v[static_cast<size_t>(s) % nv];
                                break;
                            case AccompanimentPattern::ArpDown:
                                note_midi = prev_v[nv - 1 - (static_cast<size_t>(s) % nv)];
                                break;
                            case AccompanimentPattern::ArpBounce: {
                                size_t cyc = (nv > 1) ? (2 * nv - 2) : 1;
                                size_t p = static_cast<size_t>(s) % cyc;
                                size_t idx = (p < nv) ? p : (cyc - p);
                                note_midi = prev_v[idx];
                                break;
                            }
                            case AccompanimentPattern::Alberti: {
                                static const size_t alb[4] = { 0, 2, 1, 2 };
                                note_midi = prev_v[std::min(nv - 1, alb[static_cast<size_t>(s) % 4])];
                                break;
                            }
                            case AccompanimentPattern::PulseEighths:
                            default:
                                note_midi = prev_v[static_cast<size_t>(s % 2 == 0 ? 0 : std::min<size_t>(1, nv - 1))];
                                break;
                        }
                        out.push_back(Note(note_midi, actual_sec, dyn, Articulation::Sustain));
                    }
                }
                return out;
            }

            inline wauvio::music::Melody generate_bass_track(const Key& key,
                                                             const std::vector<ChordSymbol>& chords,
                                                             BassPattern pat,
                                                             double bpm,
                                                             int low_midi, int high_midi,
                                                             wauvio::music::Dynamics dyn,
                                                             Rng& rng)
            {
                using namespace wauvio::music;
                Melody out;
                const double sec_per_beat = 60.0 / std::max(20.0, bpm);

                auto bass_in_range = [&](int pc) {
                    int m = low_midi;
                    while (wrap_pc(m) != wrap_pc(pc)) ++m;
                    if (m > high_midi) m -= 12;
                    return std::clamp(m, 24, 60);
                };

                for (size_t ci = 0; ci < chords.size(); ++ci) {
                    const ChordSymbol& ch = chords[ci];
                    const int root = bass_in_range(ch.bass_pc);
                    int fifth = root + 7;
                    if (fifth > high_midi) fifth -= 12;
                    int oct = (root + 12 <= high_midi + 5) ? root + 12 : root;

                    switch (pat) {
                        case BassPattern::RootWhole:
                            out.push_back(Note(root, ch.duration_beats * sec_per_beat, dyn));
                            break;
                        case BassPattern::RootFifth: {
                            double rem = ch.duration_beats;
                            bool flip = false;
                            while (rem > 1e-6) {
                                double d = std::min(2.0, rem);
                                out.push_back(Note(flip ? fifth : root, d * sec_per_beat, dyn));
                                flip = !flip; rem -= d;
                            }
                            break;
                        }
                        case BassPattern::OctaveBounce: {
                            int steps = std::max(1, static_cast<int>(std::round(ch.duration_beats / 0.5)));
                            double ds = (ch.duration_beats / steps) * sec_per_beat;
                            for (int s = 0; s < steps; ++s)
                                out.push_back(Note((s % 2 == 0) ? root : oct, ds, (s % 2 == 0) ? dyn : Dynamics::mp, Articulation::Staccato));
                            break;
                        }
                        case BassPattern::DrivingEighths: {
                            int steps = std::max(1, static_cast<int>(std::round(ch.duration_beats / 0.5)));
                            double ds = (ch.duration_beats / steps) * sec_per_beat;
                            for (int s = 0; s < steps; ++s)
                                out.push_back(Note(root, ds, (s % 2 == 0) ? dyn : Dynamics::mp));
                            break;
                        }
                        case BassPattern::Walking: {
                            int steps = std::max(1, static_cast<int>(std::round(ch.duration_beats)));
                            double ds = (ch.duration_beats / steps) * sec_per_beat;
                            const int next_root = bass_in_range(chords[(ci + 1) % chords.size()].bass_pc);
                            for (int s = 0; s < steps; ++s) {
                                int m = root;
                                if (s == 1) m = bass_in_range(ch.root_pc + chord_intervals(ch.quality)[std::min<size_t>(1, chord_intervals(ch.quality).size() - 1)]);
                                else if (s == 2) m = fifth;
                                else if (s >= 3) m = next_root + (rng.chance(0.5) ? -1 : 1);
                                if (s < 3) m = key.snap_to_scale(m);
                                out.push_back(Note(std::clamp(m, low_midi, high_midi + 5), ds, dyn));
                            }
                            break;
                        }
                        case BassPattern::Syncopated: {
                            double rem = ch.duration_beats;
                            const double cell[3] = { 1.5, 1.5, 1.0 };
                            int k = 0;
                            while (rem > 1e-6) {
                                double d = std::min(cell[k % 3], rem);
                                int m = (k % 3 == 0) ? root : (k % 3 == 1 ? fifth : oct);
                                out.push_back(Note(m, d * sec_per_beat, (k % 3 == 0) ? dyn : Dynamics::mp));
                                rem -= d; ++k;
                            }
                            break;
                        }
                    }
                }
                return out;
            }

            struct DrumTracks {
                wauvio::music::Melody kick, snare, hat, crash;
            };

            inline DrumTracks generate_drums(DrumPattern pat, int bars, int beats_per_bar, double bpm, double energy, Rng& rng) {
                using namespace wauvio::music;
                DrumTracks dt;
                if (pat == DrumPattern::None || bars <= 0) return dt;

                const double sec_per_beat = 60.0 / std::max(20.0, bpm);
                const double sub = 0.25;
                const int steps_per_bar = std::max(4, beats_per_bar * 4);
                const double step_sec = sub * sec_per_beat;

                auto push_step = [&](Melody& track, bool hit, int midi, Dynamics d) {
                    if (hit) track.push_back(Note(midi, step_sec, d));
                    else     track.push_back(rest(step_sec));
                };

                for (int b = 0; b < bars; ++b) {
                    const bool is_fill_bar = (b + 1 == bars && bars >= 2 && energy >= 0.45);
                    for (int s = 0; s < steps_per_bar; ++s) {
                        bool k = false, sn = false, hh = false, cr = false;
                        bool open_hh = false;
                        Dynamics kd = Dynamics::f, sd = Dynamics::f, hd = Dynamics::mp;

                        switch (pat) {
                            case DrumPattern::FourOnFloor:
                                k = (s % 4 == 0);
                                sn = (s == 4 || s == 12);
                                hh = (s % 2 == 0) || (energy >= 0.65);
                                open_hh = (s % 4 == 2);
                                hd = (s % 4 == 2) ? Dynamics::mf : Dynamics::p;
                                break;
                            case DrumPattern::PopRock:
                                k = (s == 0 || s == 6 || s == 8 || (energy > 0.6 && s == 10));
                                sn = (s == 4 || s == 12);
                                hh = (s % 2 == 0);
                                if (is_fill_bar && s >= steps_per_bar - 4) { sn = (s % 2 == 0) || s == steps_per_bar - 1; }
                                break;
                            case DrumPattern::HalfTime:
                                k = (s == 0 || s == 10);
                                sn = (s == 8);
                                hh = (s % 2 == 0) || (energy > 0.7 && s >= 12);
                                break;
                            case DrumPattern::Breakbeat:
                                k = (s == 0 || s == 6 || s == 10);
                                sn = (s == 4 || s == 12 || (s == 7 && rng.chance(0.5)) || (s == 15 && rng.chance(0.4)));
                                hh = (s % 2 == 0);
                                break;
                            case DrumPattern::Waltz:
                                k = (s == 0);
                                sn = (s == 4 || s == 8);
                                hh = (s % 4 == 0);
                                sd = Dynamics::mp;
                                break;
                            case DrumPattern::JazzSwing:
                                k = (s == 0 || (rng.chance(0.25) && s == 10));
                                sn = (s == 4 || s == 12) && rng.chance(0.6);
                                hh = (s % 4 == 0 || s == 4 || s == 7 || s == 8 || s == 12 || s == 15);
                                sd = Dynamics::mp;
                                break;
                            case DrumPattern::Bossa:
                                k = (s == 0 || s == 3 || s == 4 || s == 7 || s == 8 || s == 11 || s == 12 || s == 15) && (s % 4 == 0 || s % 4 == 3);
                                sn = (s == 0 || s == 3 || s == 6 || s == 10 || s == 12);
                                hh = (s % 2 == 0);
                                sd = Dynamics::mp;
                                break;
                            case DrumPattern::LoFi:
                                k = (s == 0 || s == 7 || s == 10);
                                sn = (s == 4 || s == 12);
                                hh = (s % 2 == 0);
                                hd = Dynamics::p;
                                break;
                            default:
                                break;
                        }
                        if (b == 0 && s == 0 && energy >= 0.65) cr = true;

                        push_step(dt.kick,  k,  36, kd);
                        push_step(dt.snare, sn, (pat == DrumPattern::Bossa) ? 37 : 38, sd);
                        push_step(dt.hat,   hh, open_hh ? 46 : 42, hd);
                        push_step(dt.crash, cr, 49, Dynamics::mf);
                    }
                }
                return dt;
            }

        }

        struct SectionSpec {
            SectionRole      role           = SectionRole::Verse;
            std::string      name;
            int              bars           = 4;
            double           energy         = 0.6;
            ChordProgression progression;
            Motif            motif;
            bool             enable_melody  = true;
            bool             enable_chords  = true;
            bool             enable_bass    = true;
            bool             enable_drums   = true;
            bool             enable_counter = false;
        };

        struct ComposeConfig {
            Key         key            { PitchClass::C, ScaleType::Major };
            double      bpm            = 120.0;
            Meter       meter          { 4, 4 };
            Mood        mood           = Mood::Bright;
            std::string genre;
            double      genre_strength = 0.7;
            int         quick_bars     = 0;
            uint64_t    seed           = 42;

            ChordProgression progression;
            Motif            motif;
            std::vector<SectionSpec> sections;

            AccompanimentPattern accompaniment = AccompanimentPattern::ArpBounce;
            BassPattern          bass_pattern  = BassPattern::RootFifth;
            DrumPattern          drum_pattern  = DrumPattern::Auto;

            wauvio::music::InstrumentPtr             lead_instrument;
            wauvio::music::InstrumentPtr             chord_instrument;
            wauvio::music::InstrumentPtr             bass_instrument;
            wauvio::music::InstrumentPtr             counter_instrument;
            std::shared_ptr<wauvio::music::DrumKit>  drum_kit;
        };

        struct ComposedSection {
            SectionSpec spec;
            double start_sec    = 0.0;
            double duration_sec = 0.0;
            std::vector<ChordSymbol> chords;
            wauvio::music::Melody melody;
            wauvio::music::Melody bass;
            wauvio::music::Melody accompaniment;
            wauvio::music::Melody counter;
        };

        struct ComposedSong {
            ComposeConfig config;
            std::vector<ComposedSection> sections;
            wauvio::music::Arrangement arrangement;

            std::vector<wauvio::music::InstrumentPtr> owned_instruments;
            std::shared_ptr<wauvio::music::DrumKit>   owned_kit;

            double total_duration_seconds() const noexcept {
                double t = 0.0;
                for (const auto& s : sections) t = std::max(t, s.start_sec + s.duration_sec);
                return t;
            }

            StereoBuffer render(int sample_rate = 0, const RenderOptions& opts = RenderOptions()) const {
                RenderOptions ro = opts;
                if (sample_rate > 0) ro.sample_rate = sample_rate;
                return wauvio::music::render_to_buffer(arrangement, ro);
            }

            void save(const std::string& wav_path, const RenderOptions& opts = RenderOptions()) const {
                wauvio::render(arrangement, wav_path, opts);
            }

            std::string summary() const {
                std::ostringstream o;
                o << "Key: " << config.key.name()
                  << " | Tempo: " << config.bpm << " BPM"
                  << " | Meter: " << config.meter.beats_per_bar << "/" << config.meter.beat_unit
                  << " | Seed: " << config.seed << "\n";
                for (size_t i = 0; i < sections.size(); ++i) {
                    const auto& sec = sections[i];
                    o << "  [" << (i + 1) << "] " << (sec.spec.name.empty() ? section_role_name(sec.spec.role) : sec.spec.name)
                      << " (" << sec.spec.bars << " bars, " << sec.start_sec << "s): ";
                    for (size_t c = 0; c < sec.chords.size(); ++c) {
                        o << (c ? " -> " : "") << sec.chords[c].name();
                    }
                    o << "\n";
                }
                return o.str();
            }
        };

        namespace compose_detail {

            inline void pick_palette_for_mood(Mood mood,
                                              wauvio::music::InstrumentPtr& lead,
                                              wauvio::music::InstrumentPtr& chords,
                                              wauvio::music::InstrumentPtr& bass,
                                              wauvio::music::InstrumentPtr& counter,
                                              std::shared_ptr<wauvio::music::DrumKit>& kit)
            {
                using namespace wauvio::instruments;
                switch (mood) {
                    case Mood::Bright:
                        lead    = lead    ? lead    : std::make_shared<AcousticGrandPiano>();
                        chords  = chords  ? chords  : std::make_shared<ElectricPiano>();
                        bass    = bass    ? bass    : std::make_shared<BassGuitar>();
                        counter = counter ? counter : std::make_shared<StringEnsemble>();
                        kit     = kit     ? kit     : std::make_shared<StudioKit>();
                        break;
                    case Mood::Melancholic:
                        lead    = lead    ? lead    : std::make_shared<FeltPiano>();
                        chords  = chords  ? chords  : std::make_shared<ChamberStrings>();
                        bass    = bass    ? bass    : std::make_shared<FretlessBass>();
                        counter = counter ? counter : std::make_shared<SoloCello>();
                        kit     = kit     ? kit     : std::make_shared<LoFiKit>();
                        break;
                    case Mood::Epic:
                        lead    = lead    ? lead    : std::make_shared<FrenchHorn>();
                        chords  = chords  ? chords  : std::make_shared<FullStringSection>();
                        bass    = bass    ? bass    : std::make_shared<LowBrassSection>();
                        counter = counter ? counter : std::make_shared<SoloViolin>();
                        kit     = kit     ? kit     : std::make_shared<OrchestralPercussionKit>();
                        break;
                    case Mood::Chill:
                        lead    = lead    ? lead    : std::make_shared<RhodesMarkI>();
                        chords  = chords  ? chords  : std::make_shared<WarmPad>();
                        bass    = bass    ? bass    : std::make_shared<SubBass>();
                        counter = counter ? counter : std::make_shared<Flute>();
                        kit     = kit     ? kit     : std::make_shared<LoFiKit>();
                        break;
                    case Mood::Energetic:
                        lead    = lead    ? lead    : std::make_shared<SynthLead>();
                        chords  = chords  ? chords  : std::make_shared<PolySynth>();
                        bass    = bass    ? bass    : std::make_shared<SynthBass>();
                        counter = counter ? counter : std::make_shared<ArpeggioSynth>();
                        kit     = kit     ? kit     : std::make_shared<Kit909>();
                        break;
                    case Mood::Jazzy:
                        lead    = lead    ? lead    : std::make_shared<TenorSaxophone>();
                        chords  = chords  ? chords  : std::make_shared<RhodesMarkII>();
                        bass    = bass    ? bass    : std::make_shared<UprightBassPluck>();
                        counter = counter ? counter : std::make_shared<Vibraphone>();
                        kit     = kit     ? kit     : std::make_shared<JazzKit>();
                        break;
                    case Mood::Ambient:
                        lead    = lead    ? lead    : std::make_shared<Celesta>();
                        chords  = chords  ? chords  : std::make_shared<GlassPad>();
                        bass    = bass    ? bass    : std::make_shared<SubBass>();
                        counter = counter ? counter : std::make_shared<Harp>();
                        kit     = kit     ? kit     : std::make_shared<LoFiKit>();
                        break;
                    case Mood::Retro:
                        lead    = lead    ? lead    : std::make_shared<ChiptuneLead>();
                        chords  = chords  ? chords  : std::make_shared<SquareLead>();
                        bass    = bass    ? bass    : std::make_shared<ChiptuneBass>();
                        counter = counter ? counter : std::make_shared<Pluck>();
                        kit     = kit     ? kit     : std::make_shared<Kit808>();
                        break;
                }
            }

            inline DrumPattern default_drums_for_mood(Mood mood, int beats_per_bar) noexcept {
                if (beats_per_bar == 3) return DrumPattern::Waltz;
                switch (mood) {
                    case Mood::Bright:      return DrumPattern::PopRock;
                    case Mood::Melancholic: return DrumPattern::HalfTime;
                    case Mood::Epic:        return DrumPattern::HalfTime;
                    case Mood::Chill:       return DrumPattern::LoFi;
                    case Mood::Energetic:   return DrumPattern::FourOnFloor;
                    case Mood::Jazzy:       return DrumPattern::JazzSwing;
                    case Mood::Ambient:     return DrumPattern::None;
                    case Mood::Retro:       return DrumPattern::PopRock;
                }
                return DrumPattern::PopRock;
            }

            inline std::vector<SectionSpec> default_sections(int quick_bars) {
                if (quick_bars > 0) {
                    SectionSpec s;
                    s.role = SectionRole::Chorus; s.name = "Draft"; s.bars = quick_bars; s.energy = 0.75;
                    return { s };
                }
                SectionSpec intro   { SectionRole::Intro,     "Intro",     4, 0.35 };
                intro.enable_melody = false; intro.enable_drums = false;
                SectionSpec verse   { SectionRole::Verse,     "Verse 1",   4, 0.55 };
                SectionSpec chorus  { SectionRole::Chorus,    "Chorus 1",  4, 0.85 };
                chorus.enable_counter = true;
                SectionSpec verse2  { SectionRole::Verse,     "Verse 2",   4, 0.65 };
                SectionSpec chorus2 { SectionRole::Chorus,    "Chorus 2",  4, 0.90 };
                chorus2.enable_counter = true;
                SectionSpec outro   { SectionRole::Outro,     "Outro",     4, 0.30 };
                outro.enable_drums = false; outro.enable_bass = false;
                return { intro, verse, chorus, verse2, chorus2, outro };
            }

        }

        class Composer {
        public:
            Composer() = default;
            explicit Composer(ComposeConfig cfg) : cfg_(std::move(cfg)) {}

            Composer& key(PitchClass root, ScaleType scale = ScaleType::Major) { cfg_.key = Key(root, scale); return *this; }
            Composer& tempo(double bpm) { cfg_.bpm = bpm; return *this; }
            Composer& meter(int beats_per_bar, int beat_unit = 4) { cfg_.meter = Meter{ beats_per_bar, beat_unit }; return *this; }
            Composer& mood(Mood m) { cfg_.mood = m; return *this; }
            Composer& genre(const std::string& g, double strength = 0.7) { cfg_.genre = g; cfg_.genre_strength = strength; return *this; }
            Composer& seed(uint64_t s) { cfg_.seed = s; return *this; }
            Composer& quick_draft(int bars = 8) { cfg_.quick_bars = bars; return *this; }
            Composer& progression(ChordProgression p) { cfg_.progression = std::move(p); return *this; }
            Composer& progression(const std::string& name) {
                if (const auto* p = find_progression(name)) cfg_.progression = *p;
                else throw ComposeError("Composer: unknown progression '" + name + "'");
                return *this;
            }
            Composer& motif(Motif m) { cfg_.motif = std::move(m); return *this; }
            Composer& motif(const std::string& name) {
                if (const auto* m = find_motif(name)) cfg_.motif = *m;
                else throw ComposeError("Composer: unknown motif '" + name + "'");
                return *this;
            }
            Composer& accompaniment(AccompanimentPattern p) { cfg_.accompaniment = p; return *this; }
            Composer& bass_pattern(BassPattern p) { cfg_.bass_pattern = p; return *this; }
            Composer& drum_pattern(DrumPattern p) { cfg_.drum_pattern = p; return *this; }
            Composer& add_section(SectionSpec s) { cfg_.sections.push_back(std::move(s)); return *this; }
            Composer& clear_sections() { cfg_.sections.clear(); return *this; }

            Composer& lead_instrument(wauvio::music::InstrumentPtr i)    { cfg_.lead_instrument = std::move(i); return *this; }
            Composer& chord_instrument(wauvio::music::InstrumentPtr i)   { cfg_.chord_instrument = std::move(i); return *this; }
            Composer& bass_instrument(wauvio::music::InstrumentPtr i)    { cfg_.bass_instrument = std::move(i); return *this; }
            Composer& counter_instrument(wauvio::music::InstrumentPtr i) { cfg_.counter_instrument = std::move(i); return *this; }
            Composer& drum_kit(std::shared_ptr<wauvio::music::DrumKit> k){ cfg_.drum_kit = std::move(k); return *this; }

            Composer& use_wcp(const WcpBank& bank, const std::string& prog_name = "", const std::string& motif_name = "") {
                cfg_.key = bank.key;
                cfg_.bpm = bank.bpm;
                cfg_.meter = bank.meter;
                if (!prog_name.empty()) {
                    const auto* p = bank.find_progression(prog_name);
                    if (!p) throw ComposeError("WcpBank has no progression '" + prog_name + "'");
                    cfg_.progression = *p;
                } else if (!bank.progressions.empty()) {
                    cfg_.progression = bank.progressions.front();
                }
                if (!motif_name.empty()) {
                    const auto* m = bank.find_motif(motif_name);
                    if (!m) throw ComposeError("WcpBank has no motif '" + motif_name + "'");
                    cfg_.motif = *m;
                } else if (!bank.motifs.empty()) {
                    cfg_.motif = bank.motifs.front();
                }
                return *this;
            }

            ComposedSong compose() const {
                ComposedSong song;
                song.config = cfg_;
                Rng rng(cfg_.seed);

                auto lead_in    = cfg_.lead_instrument;
                auto chord_in   = cfg_.chord_instrument;
                auto bass_in    = cfg_.bass_instrument;
                auto counter_in = cfg_.counter_instrument;
                auto kit        = cfg_.drum_kit;
                compose_detail::pick_palette_for_mood(cfg_.mood, lead_in, chord_in, bass_in, counter_in, kit);

                auto drum_in = std::make_shared<DrumKitInstrument>(kit);
                song.owned_instruments = { lead_in, chord_in, bass_in, counter_in, drum_in };
                song.owned_kit = kit;

                wauvio::music::genre::GenrePresetPtr genre_preset;
                if (!cfg_.genre.empty()) {
                    genre_preset = wauvio::music::genre::find(cfg_.genre);
                    if (genre_preset) {
                        genre_preset = wauvio::music::genre::scaled(genre_preset, cfg_.genre_strength);
                        wauvio::music::genre::style_instrument(*lead_in, *genre_preset, wauvio::music::genre::Role::Melodic);
                        wauvio::music::genre::style_instrument(*chord_in, *genre_preset, wauvio::music::genre::Role::Melodic);
                        wauvio::music::genre::style_instrument(*bass_in, *genre_preset, wauvio::music::genre::Role::Bass);
                    }
                }

                ChordProgression base_prog = cfg_.progression;
                if (base_prog.chords.empty()) {
                    ProgressionOptions po;
                    po.bars = 4;
                    po.beats_per_bar = cfg_.meter.beats_per_bar;
                    po.harmonic_rhythm_beats = cfg_.meter.bar_beats();
                    po.complexity = (cfg_.mood == Mood::Jazzy || cfg_.mood == Mood::Chill) ? 0.70 : 0.35;
                    po.cadence = CadenceType::Authentic;
                    po.seed = cfg_.seed;
                    base_prog = generate_progression(cfg_.key, po);
                }

                Motif base_motif = cfg_.motif;
                if (base_motif.notes.empty()) {
                    MotifOptions mo;
                    mo.length_beats = cfg_.meter.bar_beats();
                    mo.contour = (cfg_.mood == Mood::Melancholic) ? Contour::Descending
                               : (cfg_.mood == Mood::Epic)        ? Contour::Ascending
                               : (cfg_.mood == Mood::Energetic)   ? Contour::Hook : Contour::Arch;
                    mo.density = (cfg_.mood == Mood::Ambient || cfg_.mood == Mood::Chill) ? 0.35 : 0.60;
                    mo.syncopation = (cfg_.mood == Mood::Jazzy || cfg_.mood == Mood::Energetic) ? 0.45 : 0.20;
                    mo.seed = cfg_.seed + 101;
                    base_motif = generate_motif(mo);
                }

                std::vector<SectionSpec> specs = cfg_.sections.empty()
                    ? compose_detail::default_sections(cfg_.quick_bars)
                    : cfg_.sections;

                song.arrangement.tempo = wauvio::music::Tempo(cfg_.bpm);
                song.arrangement.time_signature = wauvio::music::TimeSignature(cfg_.meter.beats_per_bar, cfg_.meter.beat_unit);

                wauvio::music::Track lead_track(*lead_in);       lead_track.name = "Lead";
                wauvio::music::Track chord_track(*chord_in);     chord_track.name = "Chords";
                wauvio::music::Track bass_track(*bass_in);       bass_track.name = "Bass";
                wauvio::music::Track counter_track(*counter_in); counter_track.name = "Counter";
                wauvio::music::Track kick_track(*drum_in);       kick_track.name = "Kick";
                wauvio::music::Track snare_track(*drum_in);      snare_track.name = "Snare";
                wauvio::music::Track hat_track(*drum_in);        hat_track.name = "HiHat";
                wauvio::music::Track crash_track(*drum_in);      crash_track.name = "Crash";

                const double sec_per_beat = 60.0 / std::max(20.0, cfg_.bpm);
                double cursor_sec = 0.0;

                for (size_t si = 0; si < specs.size(); ++si) {
                    const SectionSpec& sp = specs[si];
                    const double target_beats = std::max(1.0, sp.bars * cfg_.meter.bar_beats());
                    const double sec_len = target_beats * sec_per_beat;

                    ChordProgression sec_prog = sp.progression;
                    if (sec_prog.chords.empty()) {
                        if (sp.role == SectionRole::Bridge || sp.role == SectionRole::PreChorus) {
                            ProgressionOptions po;
                            po.bars = sp.bars;
                            po.beats_per_bar = cfg_.meter.beats_per_bar;
                            po.harmonic_rhythm_beats = cfg_.meter.bar_beats();
                            po.complexity = 0.55;
                            po.start_on_tonic = false;
                            po.cadence = CadenceType::Half;
                            po.seed = cfg_.seed + 500 + si * 31;
                            sec_prog = generate_progression(cfg_.key, po);
                        } else {
                            sec_prog = base_prog;
                            std::vector<RomanChord> fitted;
                            double acc = 0.0;
                            size_t ci = 0;
                            while (acc < target_beats - 1e-6 && !sec_prog.chords.empty()) {
                                RomanChord rc = sec_prog.chords[ci % sec_prog.chords.size()];
                                rc.duration_beats = std::min(rc.duration_beats, target_beats - acc);
                                fitted.push_back(rc);
                                acc += rc.duration_beats;
                                ++ci;
                            }
                            sec_prog.chords = std::move(fitted);
                        }
                    }

                    ComposedSection cs;
                    cs.spec = sp;
                    cs.start_sec = cursor_sec;
                    cs.duration_sec = sec_len;
                    cs.chords = sec_prog.resolve(cfg_.key);

                    if (sp.enable_melody) {
                        Motif sec_motif = sp.motif.notes.empty() ? base_motif : sp.motif;
                        if (sp.role == SectionRole::Chorus && sp.motif.notes.empty())
                            sec_motif = transpose(sec_motif, 2);

                        MelodyOptions mo;
                        mo.low_midi = (sp.role == SectionRole::Chorus) ? 64 : 60;
                        mo.high_midi = (sp.role == SectionRole::Chorus) ? 84 : 79;
                        mo.bpm = cfg_.bpm;
                        mo.beats_per_bar = cfg_.meter.beats_per_bar;
                        mo.seed = cfg_.seed + si * 17 + 3;
                        cs.melody = generate_melody(cfg_.key, sec_prog, sec_motif, mo);
                        if (genre_preset)
                            wauvio::music::genre::apply_to_melody(cs.melody, *genre_preset,
                                wauvio::music::genre::Role::Melodic, lead_in.get(), cfg_.seed + si, cfg_.bpm);
                        lead_track.add(cs.melody);
                    } else {
                        lead_track.rest(sec_len);
                    }

                    if (sp.enable_chords) {
                        if (cfg_.accompaniment == AccompanimentPattern::Pad ||
                            cfg_.accompaniment == AccompanimentPattern::BlockChords ||
                            sp.role == SectionRole::Intro || sp.role == SectionRole::Outro)
                        {
                            auto voicings = voice_lead_progression(cs.chords, 48, 72, 4);
                            auto dyn = (sp.energy > 0.7) ? wauvio::music::Dynamics::mf : wauvio::music::Dynamics::mp;
                            for (size_t i = 0; i < cs.chords.size(); ++i) {
                                wauvio::music::Chord c(voicings[i], cs.chords[i].duration_beats * sec_per_beat, dyn);
                                chord_track.add_chord(c);
                            }
                        } else {
                            auto dyn = (sp.energy > 0.7) ? wauvio::music::Dynamics::mf : wauvio::music::Dynamics::mp;
                            cs.accompaniment = pattern_detail::generate_arp_track(
                                cs.chords, cfg_.accompaniment, cfg_.bpm, 52, 76, dyn);
                            if (genre_preset)
                                wauvio::music::genre::apply_to_melody(cs.accompaniment, *genre_preset,
                                    wauvio::music::genre::Role::Melodic, chord_in.get(), cfg_.seed + 200 + si, cfg_.bpm);
                            chord_track.add(cs.accompaniment);
                        }
                    } else {
                        chord_track.rest(sec_len);
                    }

                    if (sp.enable_bass) {
                        BassPattern bp = cfg_.bass_pattern;
                        if (sp.role == SectionRole::Chorus && cfg_.bass_pattern == BassPattern::RootWhole)
                            bp = BassPattern::DrivingEighths;
                        auto dyn = (sp.energy > 0.65) ? wauvio::music::Dynamics::f : wauvio::music::Dynamics::mf;
                        cs.bass = pattern_detail::generate_bass_track(cfg_.key, cs.chords, bp, cfg_.bpm, 33, 52, dyn, rng);
                        if (genre_preset)
                            wauvio::music::genre::apply_to_melody(cs.bass, *genre_preset,
                                wauvio::music::genre::Role::Bass, bass_in.get(), cfg_.seed + 300 + si, cfg_.bpm);
                        bass_track.add(cs.bass);
                    } else {
                        bass_track.rest(sec_len);
                    }

                    if (sp.enable_counter) {
                        cs.counter = generate_countermelody(cfg_.key, sec_prog, cs.melody, cfg_.bpm, 55, 74, cfg_.seed + 400 + si);
                        counter_track.add(cs.counter);
                    } else {
                        counter_track.rest(sec_len);
                    }

                    DrumPattern dp = (cfg_.drum_pattern == DrumPattern::Auto)
                        ? compose_detail::default_drums_for_mood(cfg_.mood, cfg_.meter.beats_per_bar)
                        : cfg_.drum_pattern;
                    if (sp.enable_drums && dp != DrumPattern::None) {
                        auto dts = pattern_detail::generate_drums(dp, sp.bars, cfg_.meter.beats_per_bar, cfg_.bpm, sp.energy, rng);
                        kick_track.add(dts.kick);
                        snare_track.add(dts.snare);
                        hat_track.add(dts.hat);
                        crash_track.add(dts.crash);
                    } else {
                        kick_track.rest(sec_len);
                        snare_track.rest(sec_len);
                        hat_track.rest(sec_len);
                        crash_track.rest(sec_len);
                    }

                    song.sections.push_back(std::move(cs));
                    cursor_sec += sec_len;
                }

                song.arrangement.add_track(lead_track,    0.0, 0.90f);
                song.arrangement.add_track(chord_track,   0.0, 0.55f);
                song.arrangement.add_track(bass_track,    0.0, 0.70f);
                song.arrangement.add_track(counter_track, 0.0, 0.45f);
                song.arrangement.add_track(kick_track,    0.0, 0.75f);
                song.arrangement.add_track(snare_track,   0.0, 0.65f);
                song.arrangement.add_track(hat_track,     0.0, 0.45f);
                song.arrangement.add_track(crash_track,   0.0, 0.50f);
                song.arrangement.master_reverb(0.45f, 0.14f);
                song.arrangement.limiter(0.92f);

                return song;
            }

        private:
            ComposeConfig cfg_;
        };

        inline ComposedSong compose_draft(PitchClass root = PitchClass::C,
                                          ScaleType scale = ScaleType::Major,
                                          double bpm = 120.0,
                                          Mood mood = Mood::Bright,
                                          int bars = 8,
                                          uint64_t seed = 42)
        {
            return Composer().key(root, scale).tempo(bpm).mood(mood).quick_draft(bars).seed(seed).compose();
        }

        inline ComposedSong compose_from_wcp(const std::string& wcp_path,
                                             const std::string& prog_name = "",
                                             const std::string& motif_name = "",
                                             Mood mood = Mood::Bright,
                                             uint64_t seed = 42)
        {
            WcpBank bank;
            std::string err;
            if (!load_wcp(wcp_path, bank, &err)) throw ComposeError(err);
            return Composer().use_wcp(bank, prog_name, motif_name).mood(mood).seed(seed).compose();
        }

    }
}
