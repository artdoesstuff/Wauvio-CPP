#pragma once

#include "../progression/progressions.hpp"
#include "../motif/motifs.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace wauvio {
    namespace compose {

        inline constexpr const char* WCP_EXT = ".wcp";

        inline bool is_wcp_path(const std::string& path) {
            const std::string ext = WCP_EXT;
            return path.size() > ext.size() && path.compare(path.size() - ext.size(), ext.size(), ext) == 0;
        }

        struct WcpBank {
            std::string name;
            std::string author;
            Key         key   { PitchClass::C, ScaleType::Major };
            double      bpm   = 120.0;
            Meter       meter { 4, 4 };
            std::vector<ChordProgression> progressions;
            std::vector<Motif>            motifs;

            const ChordProgression* find_progression(const std::string& target) const {
                for (const auto& p : progressions) if (p.name == target) return &p;
                return nullptr;
            }
            const Motif* find_motif(const std::string& target) const {
                for (const auto& m : motifs) if (m.name == target) return &m;
                return nullptr;
            }
        };

        namespace wcp_detail {
            inline std::string trim(const std::string& s) {
                size_t a = s.find_first_not_of(" \t\r\n");
                if (a == std::string::npos) return {};
                size_t b = s.find_last_not_of(" \t\r\n");
                return s.substr(a, b - a + 1);
            }

            inline std::string unquote(const std::string& s) {
                std::string t = trim(s);
                if (t.size() >= 2 && ((t.front() == '"' && t.back() == '"') || (t.front() == '\'' && t.back() == '\'')))
                    return t.substr(1, t.size() - 2);
                return t;
            }

            inline std::vector<std::string> split_ws(const std::string& s) {
                std::vector<std::string> out;
                std::istringstream is(s);
                std::string tok;
                while (is >> tok) out.push_back(tok);
                return out;
            }

            inline std::vector<std::string> split_csv(const std::string& s) {
                std::vector<std::string> out;
                std::istringstream is(s);
                std::string item;
                while (std::getline(is, item, ',')) {
                    item = trim(item);
                    if (!item.empty()) out.push_back(item);
                }
                return out;
            }

            inline bool parse_dyn(const std::string& s, wauvio::music::Dynamics& out) {
                using D = wauvio::music::Dynamics;
                if (s == "ppp") { out = D::ppp; return true; }
                if (s == "pp")  { out = D::pp;  return true; }
                if (s == "p")   { out = D::p;   return true; }
                if (s == "mp")  { out = D::mp;  return true; }
                if (s == "mf")  { out = D::mf;  return true; }
                if (s == "f")   { out = D::f;   return true; }
                if (s == "ff")  { out = D::ff;  return true; }
                if (s == "fff") { out = D::fff; return true; }
                return false;
            }

            inline const char* dyn_str(wauvio::music::Dynamics d) {
                using D = wauvio::music::Dynamics;
                switch (d) {
                    case D::ppp: return "ppp"; case D::pp: return "pp"; case D::p: return "p";
                    case D::mp:  return "mp";  case D::mf: return "mf"; case D::f: return "f";
                    case D::ff:  return "ff";  case D::fff: return "fff";
                }
                return "mf";
            }
        }

        inline bool wcp_from_text(const std::string& text, WcpBank& out, std::string* err = nullptr) {
            using namespace wcp_detail;
            out = WcpBank{};

            enum class Block { None, Progression, Motif };
            Block block = Block::None;
            ChordProgression cur_prog;
            Motif cur_motif;
            std::vector<std::string> m_steps, m_durs, m_dyns;

            std::istringstream in(text);
            std::string line;
            int ln = 0;

            auto fail = [&](const std::string& msg) -> bool {
                if (err) *err = "wcp line " + std::to_string(ln) + ": " + msg;
                return false;
            };

            auto flush_motif = [&]() -> bool {
                if (!cur_motif.notes.empty()) {
                    out.motifs.push_back(std::move(cur_motif));
                    cur_motif = Motif{};
                    m_steps.clear(); m_durs.clear(); m_dyns.clear();
                    return true;
                }
                if (m_steps.empty()) return fail("motif '" + cur_motif.name + "' has no notes/steps");
                for (size_t i = 0; i < m_steps.size(); ++i) {
                    MotifNote mn;
                    std::string st = m_steps[i];
                    if (st == "r" || st == "R" || st == ".") {
                        mn.is_rest = true;
                    } else if ((st[0] == 'c' || st[0] == 'C') && st.size() > 1) {
                        mn.basis = StepBasis::ChordTone;
                        try { mn.step = std::stoi(st.substr(1)); } catch (...) { return fail("bad chord-tone step '" + st + "'"); }
                    } else {
                        mn.basis = StepBasis::ScaleStep;
                        try { mn.step = std::stoi(st); } catch (...) { return fail("bad scale step '" + st + "'"); }
                    }
                    if (i < m_durs.size()) {
                        try { mn.duration_beats = std::stod(m_durs[i]); } catch (...) { return fail("bad duration '" + m_durs[i] + "'"); }
                        if (mn.duration_beats <= 0.0) return fail("duration must be > 0");
                    } else if (!m_durs.empty()) {
                        try { mn.duration_beats = std::stod(m_durs.back()); } catch (...) { mn.duration_beats = 0.5; }
                    }
                    if (i < m_dyns.size()) {
                        if (!parse_dyn(m_dyns[i], mn.dynamics)) return fail("unknown dynamic '" + m_dyns[i] + "'");
                    }
                    cur_motif.notes.push_back(mn);
                }
                out.motifs.push_back(std::move(cur_motif));
                cur_motif = Motif{};
                m_steps.clear(); m_durs.clear(); m_dyns.clear();
                return true;
            };

            while (std::getline(in, line)) {
                ++ln;
                if (ln == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
                if (auto h = line.find('#'); h != std::string::npos) line.resize(h);
                line = trim(line);
                if (line.empty()) continue;

                std::istringstream is(line);
                std::string cmd;
                is >> cmd;
                std::string rest;
                std::getline(is, rest);
                rest = trim(rest);

                if (block == Block::None) {
                    if (cmd == "name")        out.name = unquote(rest);
                    else if (cmd == "author") out.author = unquote(rest);
                    else if (cmd == "bpm") {
                        try { out.bpm = std::stod(rest); } catch (...) { return fail("invalid bpm '" + rest + "'"); }
                        if (out.bpm <= 10.0 || out.bpm > 400.0) return fail("bpm out of range (10..400)");
                    } else if (cmd == "key") {
                        auto toks = split_ws(rest);
                        if (toks.empty() || !parse_pitch_class(toks[0], out.key.root))
                            return fail("invalid key root in '" + rest + "'");
                        if (toks.size() >= 2 && !parse_scale_type(toks[1], out.key.scale))
                            return fail("invalid scale type '" + toks[1] + "'");
                    } else if (cmd == "meter") {
                        int num = 4, den = 4;
                        if (std::sscanf(rest.c_str(), "%d/%d", &num, &den) != 2 || num <= 0 || den <= 0)
                            return fail("meter must be N/D (e.g. 4/4)");
                        out.meter = Meter{ num, den };
                    } else if (cmd == "progression") {
                        if (rest.empty()) return fail("progression needs a name");
                        block = Block::Progression;
                        cur_prog = ChordProgression{};
                        cur_prog.name = unquote(rest);
                    } else if (cmd == "motif") {
                        if (rest.empty()) return fail("motif needs a name");
                        block = Block::Motif;
                        cur_motif = Motif{};
                        cur_motif.name = unquote(rest);
                        m_steps.clear(); m_durs.clear(); m_dyns.clear();
                    } else {
                        return fail("unknown top-level key '" + cmd + "'");
                    }
                } else if (block == Block::Progression) {
                    if (cmd == "end") {
                        if (cur_prog.chords.empty()) return fail("progression '" + cur_prog.name + "' has no chords");
                        out.progressions.push_back(std::move(cur_prog));
                        block = Block::None;
                    } else if (cmd == "description" || cmd == "desc") {
                        cur_prog.description = unquote(rest);
                    } else if (cmd == "tags") {
                        cur_prog.tags = split_csv(rest);
                    } else if (cmd == "minor") {
                        cur_prog.minor_bias = (rest == "1" || rest == "true" || rest == "yes");
                    } else if (cmd == "chords") {
                        auto toks = split_ws(rest);
                        const double def_beats = out.meter.bar_beats();
                        for (const auto& t : toks) {
                            if (t == "|") continue;
                            RomanChord rc;
                            if (!parse_roman_chord(t, rc, def_beats))
                                return fail("invalid roman chord token '" + t + "'");
                            cur_prog.chords.push_back(rc);
                        }
                    } else {
                        return fail("unknown progression directive '" + cmd + "'");
                    }
                } else if (block == Block::Motif) {
                    if (cmd == "end") {
                        if (!flush_motif()) return false;
                        block = Block::None;
                    } else if (cmd == "description" || cmd == "desc") {
                        cur_motif.description = unquote(rest);
                    } else if (cmd == "tags") {
                        cur_motif.tags = split_csv(rest);
                    } else if (cmd == "steps") {
                        m_steps = split_ws(rest);
                    } else if (cmd == "dur" || cmd == "durations") {
                        m_durs = split_ws(rest);
                    } else if (cmd == "dyn" || cmd == "dynamics") {
                        m_dyns = split_ws(rest);
                    } else if (cmd == "notes") {
                        for (const auto& tok : split_ws(rest)) {
                            MotifNote mn;
                            std::string core = tok;
                            if (auto at = core.find('@'); at != std::string::npos) {
                                if (!parse_dyn(core.substr(at + 1), mn.dynamics))
                                    return fail("unknown dynamic in '" + tok + "'");
                                core = core.substr(0, at);
                            }
                            std::string st = core;
                            if (auto col = core.find(':'); col != std::string::npos) {
                                st = core.substr(0, col);
                                try { mn.duration_beats = std::stod(core.substr(col + 1)); }
                                catch (...) { return fail("invalid duration in '" + tok + "'"); }
                                if (mn.duration_beats <= 0.0) return fail("duration must be > 0 in '" + tok + "'");
                            }
                            if (st == "r" || st == "R" || st == ".") {
                                mn.is_rest = true;
                            } else if ((st[0] == 'c' || st[0] == 'C') && st.size() > 1) {
                                mn.basis = StepBasis::ChordTone;
                                try { mn.step = std::stoi(st.substr(1)); } catch (...) { return fail("invalid step in '" + tok + "'"); }
                            } else {
                                mn.basis = StepBasis::ScaleStep;
                                try { mn.step = std::stoi(st); } catch (...) { return fail("invalid step in '" + tok + "'"); }
                            }
                            cur_motif.notes.push_back(mn);
                        }
                    } else {
                        return fail("unknown motif directive '" + cmd + "'");
                    }
                }
            }

            if (block != Block::None) return fail("unclosed block (missing 'end')");
            return true;
        }

        inline std::string wcp_to_text(const WcpBank& bank) {
            std::ostringstream o;
            o << "# Wauvio Compose Preset (.wcp)\n";
            if (!bank.name.empty())   o << "name " << bank.name << "\n";
            if (!bank.author.empty()) o << "author " << bank.author << "\n";
            o << "key " << bank.key.name() << "\n";
            o << "bpm " << bank.bpm << "\n";
            o << "meter " << bank.meter.beats_per_bar << "/" << bank.meter.beat_unit << "\n";

            for (const auto& p : bank.progressions) {
                o << "\nprogression \"" << p.name << "\"\n";
                if (!p.description.empty()) o << "  description \"" << p.description << "\"\n";
                if (!p.tags.empty()) {
                    o << "  tags ";
                    for (size_t i = 0; i < p.tags.size(); ++i) o << (i ? ", " : "") << p.tags[i];
                    o << "\n";
                }
                if (p.minor_bias) o << "  minor true\n";
                o << "  chords";
                for (const auto& c : p.chords) o << " " << c.to_string() << ":" << c.duration_beats;
                o << "\nend\n";
            }

            for (const auto& m : bank.motifs) {
                o << "\nmotif \"" << m.name << "\"\n";
                if (!m.description.empty()) o << "  description \"" << m.description << "\"\n";
                if (!m.tags.empty()) {
                    o << "  tags ";
                    for (size_t i = 0; i < m.tags.size(); ++i) o << (i ? ", " : "") << m.tags[i];
                    o << "\n";
                }
                o << "  notes";
                for (const auto& n : m.notes) {
                    o << " ";
                    if (n.is_rest) o << "r";
                    else if (n.basis == StepBasis::ChordTone) o << "c" << n.step;
                    else o << n.step;
                    o << ":" << n.duration_beats;
                    if (n.dynamics != wauvio::music::Dynamics::mf) o << "@" << wcp_detail::dyn_str(n.dynamics);
                }
                o << "\nend\n";
            }
            return o.str();
        }

        inline bool load_wcp(const std::string& path, WcpBank& out, std::string* err = nullptr) {
            std::ifstream f(path, std::ios::binary);
            if (!f) { if (err) *err = "cannot open .wcp file: " + path; return false; }
            std::ostringstream ss;
            ss << f.rdbuf();
            return wcp_from_text(ss.str(), out, err);
        }

        inline bool save_wcp(const WcpBank& bank, const std::string& path) {
            std::ofstream f(path, std::ios::binary);
            if (!f) return false;
            f << wcp_to_text(bank);
            return static_cast<bool>(f);
        }

    }
}
