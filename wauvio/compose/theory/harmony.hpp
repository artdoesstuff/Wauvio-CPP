#pragma once

#include "../core/compose_common.hpp"

namespace wauvio {
    namespace compose {

        enum class ChordQuality {
            Major,
            Minor,
            Diminished,
            Augmented,
            Sus2,
            Sus4,
            Major7,
            Minor7,
            Dominant7,
            HalfDim7,
            Dim7,
            MinMaj7,
            Add9,
            Maj9,
            Min9,
            Dom9,
            Power5
        };

        inline const std::vector<int>& chord_intervals(ChordQuality q) {
            static const std::vector<int> maj   { 0, 4, 7 };
            static const std::vector<int> min   { 0, 3, 7 };
            static const std::vector<int> dim   { 0, 3, 6 };
            static const std::vector<int> aug   { 0, 4, 8 };
            static const std::vector<int> sus2  { 0, 2, 7 };
            static const std::vector<int> sus4  { 0, 5, 7 };
            static const std::vector<int> maj7  { 0, 4, 7, 11 };
            static const std::vector<int> min7  { 0, 3, 7, 10 };
            static const std::vector<int> dom7  { 0, 4, 7, 10 };
            static const std::vector<int> hdim7 { 0, 3, 6, 10 };
            static const std::vector<int> dim7  { 0, 3, 6, 9 };
            static const std::vector<int> mmaj7 { 0, 3, 7, 11 };
            static const std::vector<int> add9  { 0, 4, 7, 14 };
            static const std::vector<int> maj9  { 0, 4, 7, 11, 14 };
            static const std::vector<int> min9  { 0, 3, 7, 10, 14 };
            static const std::vector<int> dom9  { 0, 4, 7, 10, 14 };
            static const std::vector<int> pow5  { 0, 7 };
            switch (q) {
                case ChordQuality::Major:      return maj;
                case ChordQuality::Minor:      return min;
                case ChordQuality::Diminished: return dim;
                case ChordQuality::Augmented:  return aug;
                case ChordQuality::Sus2:       return sus2;
                case ChordQuality::Sus4:       return sus4;
                case ChordQuality::Major7:     return maj7;
                case ChordQuality::Minor7:     return min7;
                case ChordQuality::Dominant7:  return dom7;
                case ChordQuality::HalfDim7:   return hdim7;
                case ChordQuality::Dim7:       return dim7;
                case ChordQuality::MinMaj7:    return mmaj7;
                case ChordQuality::Add9:       return add9;
                case ChordQuality::Maj9:       return maj9;
                case ChordQuality::Min9:       return min9;
                case ChordQuality::Dom9:       return dom9;
                case ChordQuality::Power5:     return pow5;
            }
            return maj;
        }

        inline const char* quality_suffix(ChordQuality q) noexcept {
            switch (q) {
                case ChordQuality::Major:      return "";
                case ChordQuality::Minor:      return "m";
                case ChordQuality::Diminished: return "dim";
                case ChordQuality::Augmented:  return "aug";
                case ChordQuality::Sus2:       return "sus2";
                case ChordQuality::Sus4:       return "sus4";
                case ChordQuality::Major7:     return "maj7";
                case ChordQuality::Minor7:     return "m7";
                case ChordQuality::Dominant7:  return "7";
                case ChordQuality::HalfDim7:   return "m7b5";
                case ChordQuality::Dim7:       return "dim7";
                case ChordQuality::MinMaj7:    return "mMaj7";
                case ChordQuality::Add9:       return "add9";
                case ChordQuality::Maj9:       return "maj9";
                case ChordQuality::Min9:       return "m9";
                case ChordQuality::Dom9:       return "9";
                case ChordQuality::Power5:     return "5";
            }
            return "";
        }

        struct RomanChord {
            int          degree         = 0;
            int          accidental     = 0;
            ChordQuality quality        = ChordQuality::Major;
            int          inversion      = 0;
            double       duration_beats = 4.0;
            int          secondary_of   = -1;

            std::string to_string() const {
                static const char* upper[7] = { "I","II","III","IV","V","VI","VII" };
                static const char* lower[7] = { "i","ii","iii","iv","v","vi","vii" };
                const int d = ((degree % 7) + 7) % 7;
                const bool min_like = (quality == ChordQuality::Minor || quality == ChordQuality::Minor7 ||
                                       quality == ChordQuality::Min9  || quality == ChordQuality::MinMaj7 ||
                                       quality == ChordQuality::Diminished || quality == ChordQuality::Dim7 ||
                                       quality == ChordQuality::HalfDim7);
                std::string s;
                if (accidental < 0) s += std::string(static_cast<size_t>(-accidental), 'b');
                if (accidental > 0) s += std::string(static_cast<size_t>(accidental), '#');
                s += min_like ? lower[d] : upper[d];
                switch (quality) {
                    case ChordQuality::Major:      break;
                    case ChordQuality::Minor:      break;
                    case ChordQuality::Diminished: s += "dim"; break;
                    case ChordQuality::Augmented:  s += "aug"; break;
                    case ChordQuality::Sus2:       s += "sus2"; break;
                    case ChordQuality::Sus4:       s += "sus4"; break;
                    case ChordQuality::Major7:     s += "maj7"; break;
                    case ChordQuality::Minor7:     s += "7"; break;
                    case ChordQuality::Dominant7:  s += "7"; break;
                    case ChordQuality::HalfDim7:   s += "m7b5"; break;
                    case ChordQuality::Dim7:       s += "dim7"; break;
                    case ChordQuality::MinMaj7:    s += "maj7"; break;
                    case ChordQuality::Add9:       s += "add9"; break;
                    case ChordQuality::Maj9:       s += "maj9"; break;
                    case ChordQuality::Min9:       s += "9"; break;
                    case ChordQuality::Dom9:       s += "9"; break;
                    case ChordQuality::Power5:     s += "5"; break;
                }
                if (secondary_of >= 0) {
                    const int sd = ((secondary_of % 7) + 7) % 7;
                    s += "/";
                    s += upper[sd];
                }
                if (inversion > 0) s += "^" + std::to_string(inversion);
                return s;
            }
        };

        namespace detail {
            inline bool parse_roman_numeral_head(const std::string& s, size_t& pos, int& degree, bool& is_upper) {
                struct Entry { const char* rn; int deg; };
                static const Entry table[] = {
                    { "vii", 6 }, { "iii", 2 }, { "vi", 5 }, { "iv", 3 }, { "ii", 1 }, { "v", 4 }, { "i", 0 }
                };
                std::string low;
                for (size_t i = pos; i < s.size() && i < pos + 3; ++i)
                    low.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(s[i]))));
                for (const auto& e : table) {
                    size_t len = std::char_traits<char>::length(e.rn);
                    if (low.compare(0, len, e.rn) == 0) {
                        degree = e.deg;
                        is_upper = std::isupper(static_cast<unsigned char>(s[pos])) != 0;
                        pos += len;
                        return true;
                    }
                }
                return false;
            }
        }

        inline bool parse_roman_chord(const std::string& token, RomanChord& out, double default_beats = 4.0) {
            if (token.empty()) return false;
            std::string core = token;
            out = RomanChord{};
            out.duration_beats = default_beats;

            if (auto col = core.find(':'); col != std::string::npos) {
                try {
                    double b = std::stod(core.substr(col + 1));
                    if (b <= 0.0 || !std::isfinite(b)) return false;
                    out.duration_beats = b;
                } catch (...) { return false; }
                core = core.substr(0, col);
            }
            if (auto inv = core.find('^'); inv != std::string::npos) {
                try { out.inversion = std::max(0, std::min(3, std::stoi(core.substr(inv + 1)))); }
                catch (...) { return false; }
                core = core.substr(0, inv);
            }
            if (auto sl = core.find('/'); sl != std::string::npos) {
                std::string tgt = core.substr(sl + 1);
                size_t tp = 0; int sd = 0; bool su = false;
                if (!detail::parse_roman_numeral_head(tgt, tp, sd, su) || tp != tgt.size()) return false;
                out.secondary_of = sd;
                core = core.substr(0, sl);
            }

            size_t pos = 0;
            while (pos < core.size() && (core[pos] == 'b' || core[pos] == '#')) {
                out.accidental += (core[pos] == '#') ? 1 : -1;
                ++pos;
            }
            bool is_upper = true;
            if (!detail::parse_roman_numeral_head(core, pos, out.degree, is_upper)) return false;

            std::string suf = core.substr(pos);
            if (suf.empty()) {
                out.quality = is_upper ? ChordQuality::Major : ChordQuality::Minor;
                return true;
            }
            if (suf == "maj7" || suf == "M7")             { out.quality = is_upper ? ChordQuality::Major7 : ChordQuality::MinMaj7; return true; }
            if (suf == "maj9" || suf == "M9")             { out.quality = ChordQuality::Maj9; return true; }
            if (suf == "m7b5" || suf == "hdim7")          { out.quality = ChordQuality::HalfDim7; return true; }
            if (suf == "dim7" || suf == "o7" || suf == "°7") { out.quality = ChordQuality::Dim7; return true; }
            if (suf == "dim"  || suf == "o"  || suf == "°")  { out.quality = ChordQuality::Diminished; return true; }
            if (suf == "aug"  || suf == "+")              { out.quality = ChordQuality::Augmented; return true; }
            if (suf == "sus2")                            { out.quality = ChordQuality::Sus2; return true; }
            if (suf == "sus4" || suf == "sus")            { out.quality = ChordQuality::Sus4; return true; }
            if (suf == "m7")                              { out.quality = ChordQuality::Minor7; return true; }
            if (suf == "m9")                              { out.quality = ChordQuality::Min9; return true; }
            if (suf == "m")                               { out.quality = ChordQuality::Minor; return true; }
            if (suf == "7")                               { out.quality = is_upper ? ChordQuality::Dominant7 : ChordQuality::Minor7; return true; }
            if (suf == "9")                               { out.quality = is_upper ? ChordQuality::Dom9 : ChordQuality::Min9; return true; }
            if (suf == "add9")                            { out.quality = ChordQuality::Add9; return true; }
            if (suf == "5")                               { out.quality = ChordQuality::Power5; return true; }
            return false;
        }

        struct ChordSymbol {
            int          root_pc        = 0;
            ChordQuality quality        = ChordQuality::Major;
            int          bass_pc        = 0;
            double       duration_beats = 4.0;

            std::vector<int> pitch_classes() const {
                std::vector<int> pcs;
                for (int iv : chord_intervals(quality)) {
                    int pc = wrap_pc(root_pc + iv);
                    if (std::find(pcs.begin(), pcs.end(), pc) == pcs.end()) pcs.push_back(pc);
                }
                return pcs;
            }

            std::vector<int> tones_in_range(int low_midi, int high_midi) const {
                std::vector<int> out;
                auto pcs = pitch_classes();
                for (int m = low_midi; m <= high_midi; ++m) {
                    int pc = wrap_pc(m);
                    if (std::find(pcs.begin(), pcs.end(), pc) != pcs.end()) out.push_back(m);
                }
                return out;
            }

            int nearest_chord_tone(int midi_note) const {
                auto pcs = pitch_classes();
                int best = midi_note, best_d = 999;
                for (int d = 0; d <= 12; ++d) {
                    for (int cand : { midi_note + d, midi_note - d }) {
                        if (cand < 0 || cand > 127) continue;
                        if (std::find(pcs.begin(), pcs.end(), wrap_pc(cand)) != pcs.end()) {
                            if (d < best_d) { best_d = d; best = cand; }
                        }
                    }
                    if (best_d <= d) break;
                }
                return best;
            }

            std::string name() const {
                std::string s = std::string(pitch_class_name(root_pc)) + quality_suffix(quality);
                if (wrap_pc(bass_pc) != wrap_pc(root_pc))
                    s += "/" + std::string(pitch_class_name(bass_pc));
                return s;
            }
        };

        inline ChordSymbol resolve_chord(const RomanChord& rc, const Key& key) {
            ChordSymbol cs;
            cs.quality = rc.quality;
            cs.duration_beats = rc.duration_beats;
            if (rc.secondary_of >= 0) {
                const int target_pc = key.degree_pc(rc.secondary_of);
                static const int maj_iv[7] = { 0, 2, 4, 5, 7, 9, 11 };
                const int d = ((rc.degree % 7) + 7) % 7;
                cs.root_pc = wrap_pc(target_pc + maj_iv[d] + rc.accidental);
            } else if (rc.accidental != 0) {
                static const int maj_iv[7] = { 0, 2, 4, 5, 7, 9, 11 };
                const int d = ((rc.degree % 7) + 7) % 7;
                cs.root_pc = wrap_pc(key.root_pc() + maj_iv[d] + rc.accidental);
            } else {
                cs.root_pc = key.degree_pc(rc.degree, 0);
            }
            const auto& ivs = chord_intervals(cs.quality);
            const size_t inv_idx = std::min<size_t>(static_cast<size_t>(std::max(0, rc.inversion)), ivs.size() - 1);
            cs.bass_pc = wrap_pc(cs.root_pc + ivs[inv_idx]);
            return cs;
        }

        inline std::vector<int> voice_lead(const std::vector<int>& prev_voicing, const ChordSymbol& target,
                                           int low_midi = 48, int high_midi = 72, int num_voices = 4)
        {
            const auto& ivs = chord_intervals(target.quality);
            std::vector<int> core_pcs;
            for (int iv : ivs) core_pcs.push_back(wrap_pc(target.root_pc + iv));
            const int V = std::max(2, std::min(num_voices, 5));

            if (prev_voicing.empty()) {
                const int mid = (low_midi + high_midi) / 2;
                int base = mid - 6;
                while (wrap_pc(base) != wrap_pc(target.bass_pc)) ++base;
                std::vector<int> out;
                out.push_back(std::clamp(base, low_midi, high_midi));
                int cur = out.back();
                for (int v = 1; v < V; ++v) {
                    int want_pc = core_pcs[static_cast<size_t>(v) % core_pcs.size()];
                    int nxt = cur + 1;
                    while (wrap_pc(nxt) != want_pc) ++nxt;
                    if (nxt > high_midi) nxt -= 12;
                    out.push_back(std::clamp(nxt, low_midi, high_midi));
                    cur = out.back();
                }
                std::sort(out.begin(), out.end());
                return out;
            }

            std::vector<int> best;
            double best_cost = 1e18;
            const auto pool = target.tones_in_range(low_midi, high_midi);
            if (pool.size() < 2) return prev_voicing;

            for (size_t start_i = 0; start_i < pool.size(); ++start_i) {
                std::vector<int> cand;
                cand.push_back(pool[start_i]);
                for (size_t j = start_i + 1; j < pool.size() && static_cast<int>(cand.size()) < V; ++j) {
                    if (pool[j] - cand.back() >= 2) cand.push_back(pool[j]);
                }
                while (static_cast<int>(cand.size()) < V) {
                    int n = cand.back() + 12;
                    if (n > high_midi) n = cand.front();
                    cand.push_back(n);
                    std::sort(cand.begin(), cand.end());
                }
                if (cand.back() - cand.front() > 19) continue;

                double cost = 0.0;
                const size_t n_cmp = std::min(prev_voicing.size(), cand.size());
                for (size_t k = 0; k < n_cmp; ++k) {
                    double d = std::abs(cand[k] - prev_voicing[k]);
                    cost += d * d * (k == 0 ? 1.3 : 1.0);
                }
                bool has_3rd = false, has_7th = false;
                for (int m : cand) {
                    int rel = wrap_pc(m - target.root_pc);
                    if (rel == 3 || rel == 4) has_3rd = true;
                    if (rel == 10 || rel == 11) has_7th = true;
                }
                if (!has_3rd && ivs.size() >= 2) cost += 25.0;
                if (ivs.size() >= 4 && !has_7th) cost += 12.0;
                if (wrap_pc(cand.front()) != wrap_pc(target.bass_pc)) cost += 6.0;

                if (cost < best_cost) { best_cost = cost; best = std::move(cand); }
            }
            return best.empty() ? prev_voicing : best;
        }

        inline std::vector<std::vector<int>> voice_lead_progression(const std::vector<ChordSymbol>& chords,
                                                                    int low_midi = 48, int high_midi = 72,
                                                                    int num_voices = 4)
        {
            std::vector<std::vector<int>> out;
            std::vector<int> prev;
            for (const auto& c : chords) {
                prev = voice_lead(prev, c, low_midi, high_midi, num_voices);
                out.push_back(prev);
            }
            return out;
        }

    }
}
