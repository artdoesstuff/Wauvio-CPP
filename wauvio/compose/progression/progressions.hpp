#pragma once

#include "../theory/harmony.hpp"

namespace wauvio {
    namespace compose {

        struct ChordProgression {
            std::string name;
            std::string description;
            std::vector<std::string> tags;
            bool minor_bias = false;
            std::vector<RomanChord> chords;

            double total_beats() const noexcept {
                double t = 0.0;
                for (const auto& c : chords) t += c.duration_beats;
                return t;
            }

            std::vector<ChordSymbol> resolve(const Key& key) const {
                std::vector<ChordSymbol> out;
                out.reserve(chords.size());
                for (const auto& rc : chords) out.push_back(resolve_chord(rc, key));
                return out;
            }

            size_t chord_index_at_beat(double beat) const noexcept {
                if (chords.empty()) return 0;
                const double tot = total_beats();
                if (tot <= 0.0) return 0;
                double b = std::fmod(beat, tot);
                if (b < 0.0) b += tot;
                double acc = 0.0;
                for (size_t i = 0; i < chords.size(); ++i) {
                    acc += chords[i].duration_beats;
                    if (b < acc - 1e-9) return i;
                }
                return chords.size() - 1;
            }
        };

        inline ChordProgression make_progression(std::string name, const std::vector<std::string>& tokens,
                                                 double default_beats = 4.0, bool minor_bias = false,
                                                 std::vector<std::string> tags = {}, std::string desc = "")
        {
            ChordProgression p;
            p.name = std::move(name);
            p.minor_bias = minor_bias;
            p.tags = std::move(tags);
            p.description = std::move(desc);
            for (const auto& tok : tokens) {
                RomanChord rc;
                if (!parse_roman_chord(tok, rc, default_beats))
                    throw ComposeError("make_progression: invalid chord token '" + tok + "'");
                p.chords.push_back(rc);
            }
            return p;
        }

        enum class CadenceType { Authentic, Half, Plagal, Deceptive, Loop };

        struct ProgressionOptions {
            int         bars                  = 4;
            int         beats_per_bar         = 4;
            double      harmonic_rhythm_beats = 4.0;
            double      complexity            = 0.35;
            CadenceType cadence               = CadenceType::Authentic;
            bool        start_on_tonic        = true;
            uint64_t    seed                  = 1;
        };

        inline ChordQuality diatonic_quality(const Key& key, int degree, double complexity, Rng& rng) {
            const auto& iv = scale_intervals(key.scale);
            const int N = static_cast<int>(iv.size());
            if (N < 5) return ChordQuality::Major;

            auto pc_at = [&](int d) {
                int i = ((d % N) + N) % N;
                int oct = (d >= 0) ? (d / N) : -((-d + N - 1) / N);
                return iv[static_cast<size_t>(i)] + oct * 12;
            };
            const int root = pc_at(degree);
            const int third = pc_at(degree + 2) - root;
            const int fifth = pc_at(degree + 4) - root;
            const int seventh = pc_at(degree + 6) - root;

            const bool use_7th = complexity >= 0.4 && rng.chance(std::min(0.9, (complexity - 0.25) * 1.1));
            const bool use_9th = complexity >= 0.72 && rng.chance((complexity - 0.65) * 0.8);

            if (third == 4 && fifth == 7) {
                if (use_9th && seventh == 11) return ChordQuality::Maj9;
                if (use_9th && seventh == 10) return ChordQuality::Dom9;
                if (use_7th && seventh == 11) return ChordQuality::Major7;
                if (use_7th && seventh == 10) return ChordQuality::Dominant7;
                if (complexity >= 0.35 && rng.chance(0.15 * complexity)) return ChordQuality::Add9;
                if (complexity >= 0.25 && rng.chance(0.12 * complexity)) return ChordQuality::Sus4;
                return ChordQuality::Major;
            }
            if (third == 3 && fifth == 7) {
                if (use_9th && seventh == 10) return ChordQuality::Min9;
                if (use_7th && seventh == 10) return ChordQuality::Minor7;
                if (use_7th && seventh == 11) return ChordQuality::MinMaj7;
                return ChordQuality::Minor;
            }
            if (third == 3 && fifth == 6) {
                if (use_7th && seventh == 10) return ChordQuality::HalfDim7;
                if (use_7th && seventh == 9)  return ChordQuality::Dim7;
                return ChordQuality::Diminished;
            }
            if (third == 4 && fifth == 8) return ChordQuality::Augmented;
            return ChordQuality::Major;
        }

        inline ChordProgression generate_progression(const Key& key, const ProgressionOptions& opts = {}) {
            Rng rng(opts.seed);
            const bool minor = is_minor_scale(key.scale);
            const double total_beats = std::max(1.0, opts.bars * static_cast<double>(opts.beats_per_bar));
            const double step = std::max(1.0, opts.harmonic_rhythm_beats);
            const int n_chords = std::max(2, static_cast<int>(std::round(total_beats / step)));

            auto fn_of = [](int deg) -> int {
                switch (((deg % 7) + 7) % 7) {
                    case 0: case 2: case 5: return 0;
                    case 1: case 3:         return 1;
                    default:                return 2;
                }
            };

            std::vector<int> degrees(static_cast<size_t>(n_chords), 0);
            degrees[0] = opts.start_on_tonic ? 0 : rng.pick(std::vector<int>{ 0, 3, 5 });

            for (int i = 1; i < n_chords; ++i) {
                const int prev = degrees[static_cast<size_t>(i - 1)];
                const int fn = fn_of(prev);
                std::vector<int> cands;
                std::vector<double> wts;
                if (fn == 0) {
                    cands = { 3, 1, 4, 5, 2 };
                    wts   = { 3.2, 2.2, 2.4, 1.8, 0.9 };
                } else if (fn == 1) {
                    cands = { 4, 6, 0, 1, 3 };
                    wts   = { 4.2, 0.8, 1.4, 0.7, 0.7 };
                } else {
                    cands = { 0, 5, 3, 2 };
                    wts   = { 4.5, 2.2, 0.9, 0.5 };
                }
                int pick = cands[rng.weighted_index(wts)];
                if (pick == prev && n_chords > 2) pick = (prev + 3) % 7;
                degrees[static_cast<size_t>(i)] = pick;
            }

            if (n_chords >= 2) {
                int& pen = degrees[static_cast<size_t>(n_chords - 2)];
                int& fin = degrees[static_cast<size_t>(n_chords - 1)];
                switch (opts.cadence) {
                    case CadenceType::Authentic:
                        if (n_chords >= 3 && fn_of(pen) == 0) pen = rng.chance(0.55) ? 3 : 1;
                        fin = 4;
                        if (n_chords >= 4) { degrees[static_cast<size_t>(n_chords - 2)] = 4; fin = 0; }
                        break;
                    case CadenceType::Half:
                        fin = 4;
                        break;
                    case CadenceType::Plagal:
                        pen = 3; fin = 0;
                        break;
                    case CadenceType::Deceptive:
                        pen = 4; fin = 5;
                        break;
                    case CadenceType::Loop:
                        if (fin == 0) fin = rng.chance(0.6) ? 4 : 3;
                        break;
                }
            }

            ChordProgression prog;
            prog.name = "Generated Progression";
            prog.minor_bias = minor;
            double rem = total_beats;
            for (int i = 0; i < n_chords; ++i) {
                RomanChord rc;
                rc.degree = degrees[static_cast<size_t>(i)];
                rc.duration_beats = (i + 1 == n_chords) ? rem : std::min(step, rem);
                rem -= rc.duration_beats;
                rc.quality = diatonic_quality(key, rc.degree, opts.complexity, rng);

                if (minor && rc.degree == 4 && (i + 1 == n_chords || degrees[static_cast<size_t>(i + 1)] == 0)) {
                    rc.quality = (opts.complexity >= 0.35) ? ChordQuality::Dominant7 : ChordQuality::Major;
                }
                if (!minor && opts.complexity >= 0.6 && i > 0 && i + 1 < n_chords && rc.degree == 5 && rng.chance(0.25)) {
                    rc.accidental = -1; rc.quality = ChordQuality::Major;
                }
                if (!minor && opts.complexity >= 0.65 && i > 0 && rc.degree == 6 && rng.chance(0.35)) {
                    rc.accidental = -1; rc.quality = ChordQuality::Major;
                }
                if (opts.complexity >= 0.7 && i + 1 < n_chords) {
                    int nxt = degrees[static_cast<size_t>(i + 1)];
                    if ((nxt == 4 || nxt == 5 || nxt == 1) && rc.degree != 0 && rng.chance(0.22)) {
                        rc.degree = 4; rc.secondary_of = nxt; rc.quality = ChordQuality::Dominant7;
                    }
                }
                prog.chords.push_back(rc);
            }
            return prog;
        }

        inline const std::vector<ChordProgression>& builtin_progressions() {
            static const std::vector<ChordProgression> list = {
                make_progression("Pop Axis",       { "I", "V", "vi", "IV" },              4.0, false, { "pop", "bright", "anthem" }, "Classic 4-chord pop progression."),
                make_progression("Sensitive",      { "vi", "IV", "I", "V" },              4.0, false, { "pop", "emotional" },        "Vi-start variation of the axis progression."),
                make_progression("Fifties",        { "I", "vi", "IV", "V" },              4.0, false, { "retro", "doo-wop", "pop" }, "Classic 1950s turnaround."),
                make_progression("Royal Road",     { "IVmaj7", "V7", "iii7", "vi7" },     4.0, false, { "jpop", "anime", "uplifting" }, "Oudou Shinkou (Royal Road) progression."),
                make_progression("Canon",          { "I", "V", "vi", "iii", "IV", "I", "IV", "V" }, 2.0, false, { "classical", "baroque" }, "Pachelbel-style descending bass sequence."),
                make_progression("Jazz ii-V-I",    { "ii7", "V7", "Imaj7:8" },            4.0, false, { "jazz", "smooth" },          "Fundamental jazz cadence."),
                make_progression("Jazz Turnaround",{ "Imaj7", "vi7", "ii7", "V7" },       4.0, false, { "jazz", "swing", "bossa" },  "Rhythm-changes / standard turnaround."),
                make_progression("Neo-Soul",       { "IVmaj7", "iii7", "ii7", "Imaj9" },  4.0, false, { "rnb", "neosoul", "chill" }, "Smooth descending 7th/9th chords."),
                make_progression("Andalusian",     { "i", "bVII", "bVI", "V" },           4.0, true,  { "flamenco", "dramatic", "minor" }, "Descending Phrygian-flavored cadence."),
                make_progression("Epic Minor",     { "i", "bVI", "bIII", "bVII" },        4.0, true,  { "epic", "cinematic", "rock" }, "Wide cinematic minor anthem."),
                make_progression("Dark Tragic",    { "i", "iv", "bVI", "V7" },            4.0, true,  { "minor", "ballad", "dark" }, "Emotional minor progression with harmonic V7."),
                make_progression("Dorian Vamp",    { "i7", "IV7", "i7", "IV7" },          4.0, true,  { "funk", "dorian", "groove" }, "Two-chord funk/fusion vamp."),
                make_progression("Sentimental",    { "I", "I7", "IV", "iv" },             4.0, false, { "ballad", "nostalgic" },     "Major IV resolving through minor iv."),
                make_progression("12-Bar Blues",   { "I7", "IV7", "I7", "I7", "IV7", "IV7", "I7", "I7", "V7", "IV7", "I7", "V7" }, 4.0, false, { "blues", "rock" }, "Standard 12-bar blues."),
                make_progression("Mixolydian Rock",{ "I", "bVII", "IV", "I" },            4.0, false, { "rock", "anthem" },          "Driving rock progression with flat VII."),
                make_progression("Lo-Fi Drift",    { "ii9", "V7", "Imaj9", "vi7" },       4.0, false, { "lofi", "chill", "jazz" },   "Warm extended chords for lo-fi and chill.")
            };
            return list;
        }

        inline const ChordProgression* find_progression(const std::string& name) {
            auto norm = [](const std::string& s) {
                std::string o;
                for (char c : s) if (std::isalnum(static_cast<unsigned char>(c))) o += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                return o;
            };
            const std::string key = norm(name);
            for (const auto& p : builtin_progressions())
                if (norm(p.name) == key) return &p;
            return nullptr;
        }

    }
}
