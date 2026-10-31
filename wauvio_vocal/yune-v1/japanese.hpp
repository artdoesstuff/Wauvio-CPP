#pragma once

#include <cctype>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <string>
#include <vector>

namespace wauvio {
    namespace vocal {
        enum class Vowel : uint8_t { A = 0, I = 1, U = 2, E = 3, O = 4 };

        // Underlying (phonological) consonant of a mora: the kana row.
        enum class Cons : uint8_t {
            None, K, G, S, Z, T, D, N, H, P, B, M, R, Y, W
        };

        struct Mora {
            Cons  c        = Cons::None;
            Vowel v        = Vowel::A;
            bool  palatal  = false;   // yo-on: きゃ しゅ ちょ ...
            bool  geminate = false;   // っ
            bool  nasal    = false;   // ん (moraic nasal)
            bool  longv    = false;   // ー or merged long vowel: continue previous vowel
            // Foreign-sound overrides (ふぁ てぃ つぁ ...): force a particular surface
            // consonant in place of the one implied by (c, v).
            enum class Force : uint8_t { None, F, TI, DI, TS, J, CH, SH } force = Force::None;
            std::string text;         // source text of the mora (UTF-8), for diagnostics
        };

        // Realised consonant classes (what the synthesizer actually builds).
        enum class Surf : uint8_t {
            None,
            K, G,            // velar stops (palatalised before i / yo-on)
            T, D,            // alveolar stops
            P, B,            // labial stops
            TS, CH, J,       // affricates: ts, tɕ, dʑ
            S, SH, Z,        // fricatives: s, ɕ, z
            H, HY, FU,       // h, ç, ɸ
            N, NY, M,        // nasals
            R,               // alveolar tap ɾ
            Y, W,            // glides j, ɰᵝ
            GEM,             // sokuon
            MORAIC_N         // ん
        };

        inline bool is_voiced(Surf s) {
            switch (s) {
                case Surf::G: case Surf::D: case Surf::B: case Surf::J: case Surf::Z:
                case Surf::N: case Surf::NY: case Surf::M: case Surf::R:
                case Surf::Y: case Surf::W: case Surf::MORAIC_N: return true;
                default: return false;
            }
        }
        inline bool is_obstruent_unvoiced(Surf s) {
            switch (s) {
                case Surf::K: case Surf::T: case Surf::P: case Surf::TS: case Surf::CH:
                case Surf::S: case Surf::SH: case Surf::H: case Surf::HY: case Surf::FU: return true;
                default: return false;
            }
        }

        struct SurfaceMora {
            Surf surf    = Surf::None;
            bool palatal = false;   // palatalised release / j-glide between C and V
        };

        // Resolve the allophone of a mora (independent of neighbours).
        inline SurfaceMora surface(const Mora& m) {
            SurfaceMora s;
            if (m.geminate) { s.surf = Surf::GEM; return s; }
            if (m.nasal)    { s.surf = Surf::MORAIC_N; return s; }
            if (m.longv)    { s.surf = Surf::None; return s; }
            const bool pal = m.palatal || m.v == Vowel::I;
            s.palatal = m.palatal;
            switch (m.force) {
                case Mora::Force::F:  s.surf = Surf::FU; return s;
                case Mora::Force::TI: s.surf = Surf::T;  s.palatal = false; return s;
                case Mora::Force::DI: s.surf = Surf::D;  s.palatal = false; return s;
                case Mora::Force::TS: s.surf = Surf::TS; return s;
                case Mora::Force::J:  s.surf = Surf::J;  return s;
                case Mora::Force::CH: s.surf = Surf::CH; return s;
                case Mora::Force::SH: s.surf = Surf::SH; return s;
                default: break;
            }
            switch (m.c) {
                case Cons::None: s.surf = Surf::None; break;
                case Cons::K: s.surf = Surf::K; break;
                case Cons::G: s.surf = Surf::G; break;
                case Cons::S: s.surf = pal ? Surf::SH : Surf::S; break;
                case Cons::Z: s.surf = pal ? Surf::J  : Surf::Z; break;
                case Cons::T:
                    if (pal) s.surf = Surf::CH;
                    else if (m.v == Vowel::U) s.surf = Surf::TS;
                    else s.surf = Surf::T;
                    break;
                case Cons::D:
                    if (m.palatal || m.v == Vowel::I) s.surf = Surf::J;      // ぢ, ぢゃ = ji
                    else if (m.v == Vowel::U) s.surf = Surf::Z;              // づ = zu
                    else s.surf = Surf::D;
                    break;
                case Cons::N: s.surf = pal ? Surf::NY : Surf::N; break;
                case Cons::H:
                    if (pal) s.surf = Surf::HY;
                    else if (m.v == Vowel::U) s.surf = Surf::FU;
                    else s.surf = Surf::H;
                    break;
                case Cons::P: s.surf = Surf::P; break;
                case Cons::B: s.surf = Surf::B; break;
                case Cons::M: s.surf = Surf::M; break;
                case Cons::R: s.surf = Surf::R; break;
                case Cons::Y: s.surf = Surf::Y; break;
                case Cons::W: s.surf = Surf::W; break;
            }
            // Inherently palatal affricates/fricatives need no extra j-glide of their own
            // unless a yo-on was actually written.
            return s;
        }

        namespace jp_detail {

            inline bool next_cp(const std::string& s, size_t& i, uint32_t& cp) {
                if (i >= s.size()) return false;
                const unsigned char c = static_cast<unsigned char>(s[i]);
                if (c < 0x80) { cp = c; i += 1; return true; }
                int n = (c >= 0xF0) ? 3 : (c >= 0xE0) ? 2 : (c >= 0xC0) ? 1 : 0;
                if (n == 0 || i + static_cast<size_t>(n) >= s.size()) {
                    cp = 0xFFFD; i += 1; return true;
                }
                cp = (n == 1) ? (c & 0x1Fu) : (n == 2) ? (c & 0x0Fu) : (c & 0x07u);
                for (int k = 1; k <= n; ++k) {
                    const unsigned char cc = static_cast<unsigned char>(s[i + static_cast<size_t>(k)]);
                    if ((cc & 0xC0) != 0x80) { cp = 0xFFFD; i += 1; return true; }
                    cp = (cp << 6) | (cc & 0x3Fu);
                }
                i += static_cast<size_t>(n) + 1;
                return true;
            }

            inline std::string cp_to_utf8(uint32_t cp) {
                std::string o;
                if (cp < 0x80) o += static_cast<char>(cp);
                else if (cp < 0x800) { o += static_cast<char>(0xC0 | (cp >> 6)); o += static_cast<char>(0x80 | (cp & 0x3F)); }
                else { o += static_cast<char>(0xE0 | (cp >> 12)); o += static_cast<char>(0x80 | ((cp >> 6) & 0x3F)); o += static_cast<char>(0x80 | (cp & 0x3F)); }
                return o;
            }

            inline bool valid_utf8(const std::string& s) {
                size_t i = 0; uint32_t cp;
                while (i < s.size()) { next_cp(s, i, cp); if (cp == 0xFFFD) return false; }
                return true;
            }

            // Katakana -> hiragana (ァ..ヶ -> ぁ..ゖ); ー stays.
            inline uint32_t to_hira(uint32_t cp) {
                if (cp >= 0x30A1 && cp <= 0x30F6) return cp - 0x60;
                return cp;
            }

            struct Row { Cons c; bool pal; };

            // Basic consonant-vowel table keyed by hiragana code point -> (consonant, vowel).
            struct Cv { Cons c; Vowel v; };

            inline const std::map<uint32_t, Cv>& cv_table() {
                static const std::map<uint32_t, Cv> t = [] {
                    std::map<uint32_t, Cv> m;
                    auto row = [&](Cons c, std::initializer_list<uint32_t> cps) {
                        // order: a i u e o
                        Vowel vs[5] = { Vowel::A, Vowel::I, Vowel::U, Vowel::E, Vowel::O };
                        int k = 0;
                        for (uint32_t cp : cps) { if (cp) m[cp] = Cv{ c, vs[k] }; ++k; }
                    };
                    row(Cons::None, { U'あ', U'い', U'う', U'え', U'お' });
                    row(Cons::K, { U'か', U'き', U'く', U'け', U'こ' });
                    row(Cons::G, { U'が', U'ぎ', U'ぐ', U'げ', U'ご' });
                    row(Cons::S, { U'さ', U'し', U'す', U'せ', U'そ' });
                    row(Cons::Z, { U'ざ', U'じ', U'ず', U'ぜ', U'ぞ' });
                    row(Cons::T, { U'た', U'ち', U'つ', U'て', U'と' });
                    row(Cons::D, { U'だ', U'ぢ', U'づ', U'で', U'ど' });
                    row(Cons::N, { U'な', U'に', U'ぬ', U'ね', U'の' });
                    row(Cons::H, { U'は', U'ひ', U'ふ', U'へ', U'ほ' });
                    row(Cons::B, { U'ば', U'び', U'ぶ', U'べ', U'ぼ' });
                    row(Cons::P, { U'ぱ', U'ぴ', U'ぷ', U'ぺ', U'ぽ' });
                    row(Cons::M, { U'ま', U'み', U'む', U'め', U'も' });
                    row(Cons::Y, { U'や', 0, U'ゆ', 0, U'よ' });
                    row(Cons::R, { U'ら', U'り', U'る', U'れ', U'ろ' });
                    row(Cons::W, { U'わ', 0, 0, 0, 0 });
                    m[U'を'] = Cv{ Cons::None, Vowel::O };   // を is /o/ in singing
                    // Small vowel kana used alone behave as plain vowels.
                    m[U'ぁ'] = Cv{ Cons::None, Vowel::A }; m[U'ぃ'] = Cv{ Cons::None, Vowel::I };
                    m[U'ぅ'] = Cv{ Cons::None, Vowel::U }; m[U'ぇ'] = Cv{ Cons::None, Vowel::E };
                    m[U'ぉ'] = Cv{ Cons::None, Vowel::O };
                    m[U'ゎ'] = Cv{ Cons::W, Vowel::A };
                    m[U'ゔ'] = Cv{ Cons::B, Vowel::U };      // ヴ approximated by voiced labial
                    return m;
                }();
                return t;
            }

            inline bool is_small_ya(uint32_t cp, Vowel& v) {
                if (cp == U'ゃ') { v = Vowel::A; return true; }
                if (cp == U'ゅ') { v = Vowel::U; return true; }
                if (cp == U'ょ') { v = Vowel::O; return true; }
                return false;
            }
            inline bool is_small_vowel(uint32_t cp, Vowel& v) {
                switch (cp) {
                    case U'ぁ': v = Vowel::A; return true; case U'ぃ': v = Vowel::I; return true;
                    case U'ぅ': v = Vowel::U; return true; case U'ぇ': v = Vowel::E; return true;
                    case U'ぉ': v = Vowel::O; return true;
                }
                return false;
            }

        }

        struct ParseOptions {
            // "おう" / "えい" / "ああ" ... inside ONE lyric string become a single long
            // vowel (ō, ē, aa) instead of two separate morae.
            bool merge_long_vowels = true;
        };

        // Kana (hiragana or katakana) -> morae. Whitespace, '|', ',' and '/' are ignored.
        inline std::vector<Mora> parse_kana(const std::string& text, const ParseOptions& opt = {}) {
            using namespace jp_detail;
            std::vector<uint32_t> cps;
            for (size_t i = 0; i < text.size();) {
                uint32_t cp; next_cp(text, i, cp);
                if (cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r' || cp == '|' || cp == ',' ||
                    cp == '/' || cp == 0x3000 || cp == 0x3001 || cp == 0x3002) continue;
                cps.push_back(cp == 0x30FC ? cp : to_hira(cp));
            }
            std::vector<Mora> out;
            for (size_t i = 0; i < cps.size(); ++i) {
                uint32_t cp = cps[i];
                Mora m;
                m.text = cp_to_utf8(cp);
                if (cp == 0x30FC || cp == U'〜' || cp == U'~' || cp == U'-') {
                    m.longv = true;
                    if (!out.empty()) m.v = out.back().v;
                    out.push_back(m); continue;
                }
                if (cp == U'っ') { m.geminate = true; out.push_back(m); continue; }
                if (cp == U'ん') { m.nasal = true; out.push_back(m); continue; }
                auto it = cv_table().find(cp);
                if (it == cv_table().end()) continue;           // unknown symbol: skip
                m.c = it->second.c; m.v = it->second.v;

                // Look ahead for small kana
                if (i + 1 < cps.size()) {
                    Vowel sv;
                    if (is_small_ya(cps[i + 1], sv)) {
                        // y-glide only makes sense after a consonant row (i-row kana usually)
                        m.v = sv; m.palatal = true; m.text += cp_to_utf8(cps[i + 1]);
                        if (m.c == Cons::None || m.c == Cons::Y) { m.c = Cons::Y; m.palatal = false; }
                        ++i;
                    } else if (is_small_vowel(cps[i + 1], sv)) {
                        // Foreign sounds: ふぁ ふぃ ふぇ ふぉ / てぃ でぃ / うぃ うぇ うぉ / しぇ じぇ ちぇ / つぁ つぃ つぇ つぉ
                        bool handled = true;
                        Mora::Force f = Mora::Force::None;
                        if (cp == U'ふ') f = Mora::Force::F;
                        else if (cp == U'て' && sv == Vowel::I) f = Mora::Force::TI;
                        else if (cp == U'で' && sv == Vowel::I) f = Mora::Force::DI;
                        else if (cp == U'つ') f = Mora::Force::TS;
                        else if (cp == U'し' && sv == Vowel::E) f = Mora::Force::SH;
                        else if (cp == U'じ' && sv == Vowel::E) f = Mora::Force::J;
                        else if (cp == U'ち' && sv == Vowel::E) f = Mora::Force::CH;
                        else if (cp == U'う') { m.c = Cons::W; f = Mora::Force::None; }
                        else handled = false;
                        if (handled) {
                            m.v = sv; m.force = f; m.text += cp_to_utf8(cps[i + 1]);
                            if (cp == U'う') m.c = Cons::W;
                            ++i;
                        }
                    }
                }
                // Long-vowel merging inside one string
                if (opt.merge_long_vowels && !out.empty() && m.c == Cons::None && !m.palatal) {
                    const Mora& p = out.back();
                    if (!p.geminate && !p.nasal) {
                        bool merge = (m.v == p.v) ||
                                     (p.v == Vowel::O && m.v == Vowel::U && cp == U'う') ||
                                     (p.v == Vowel::E && m.v == Vowel::I && cp == U'い') ||
                                     (p.v == Vowel::U && m.v == Vowel::U);
                        if (merge && (cp == U'う' || cp == U'い' || cp == U'あ' || cp == U'え' || cp == U'お')) {
                            Mora lm; lm.longv = true; lm.v = p.v; lm.text = m.text;
                            out.push_back(lm); continue;
                        }
                    }
                }
                out.push_back(m);
            }
            return out;
        }

        // Hepburn/Kunrei-style romaji -> morae.  "ka ki kya shi chi tsu fu n xtu(=っ) -"
        // Doubled consonants (kka, ssa, tta) produce a geminate; "n" before a vowel/y
        // must be written "n'" or "nn".
        inline std::vector<Mora> parse_romaji(const std::string& text, const ParseOptions& opt = {}) {
            using namespace jp_detail;
            std::string s;
            for (char ch : text) s += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            std::vector<Mora> out;
            auto vowel_of = [](char ch, Vowel& v) {
                switch (ch) { case 'a': v = Vowel::A; return true; case 'i': v = Vowel::I; return true;
                    case 'u': v = Vowel::U; return true; case 'e': v = Vowel::E; return true;
                    case 'o': v = Vowel::O; return true; }
                return false;
            };
            size_t i = 0;
            while (i < s.size()) {
                char ch = s[i];
                if (ch == ' ' || ch == '|' || ch == ',' || ch == '/' || ch == '\t' || ch == '\'') { ++i; continue; }
                Mora m;
                if (ch == '-' || ch == '~') { m.longv = true; if (!out.empty()) m.v = out.back().v; m.text = "-"; out.push_back(m); ++i; continue; }
                if (s.compare(i, 3, "xtu") == 0 || s.compare(i, 3, "ltu") == 0) { m.geminate = true; m.text = "xtu"; out.push_back(m); i += 3; continue; }
                if (ch == 'q' ) { m.geminate = true; m.text = "q"; out.push_back(m); ++i; continue; }
                Vowel v;
                if (vowel_of(ch, v)) {
                    m.v = v; m.text = std::string(1, ch);
                    // long vowel merge
                    if (opt.merge_long_vowels && !out.empty() && !out.back().geminate && !out.back().nasal) {
                        const Mora& p = out.back();
                        if (p.v == v) { Mora lm; lm.longv = true; lm.v = v; lm.text = m.text; out.push_back(lm); ++i; continue; }
                    }
                    out.push_back(m); ++i; continue;
                }
                // n handling
                if (ch == 'n') {
                    Vowel nv;
                    bool next_vowel = (i + 1 < s.size()) && (vowel_of(s[i + 1], nv) || s[i + 1] == 'y');
                    if (!next_vowel) {
                        if (i + 1 < s.size() && s[i + 1] == 'n' ) { m.nasal = true; m.text = "nn"; out.push_back(m); i += 2; continue; }
                        m.nasal = true; m.text = "n"; out.push_back(m); ++i; continue;
                    }
                }
                // Gemination by doubled consonant
                if (i + 1 < s.size() && s[i] == s[i + 1] && std::string("kstpgzdbhfjcr").find(ch) != std::string::npos) {
                    Mora g; g.geminate = true; g.text = std::string(1, ch); out.push_back(g); ++i; continue;
                }
                // Consonant clusters
                struct Ent { const char* rom; Cons c; bool pal; Mora::Force f; };
                static const Ent ents[] = {
                    {"sh", Cons::S, true, Mora::Force::SH}, {"ch", Cons::T, true, Mora::Force::CH},
                    {"ts", Cons::T, false, Mora::Force::TS}, {"fy", Cons::H, true, Mora::Force::F},
                    {"ky", Cons::K, true, Mora::Force::None}, {"gy", Cons::G, true, Mora::Force::None},
                    {"ny", Cons::N, true, Mora::Force::None}, {"hy", Cons::H, true, Mora::Force::None},
                    {"by", Cons::B, true, Mora::Force::None}, {"py", Cons::P, true, Mora::Force::None},
                    {"my", Cons::M, true, Mora::Force::None}, {"ry", Cons::R, true, Mora::Force::None},
                    {"sy", Cons::S, true, Mora::Force::SH}, {"zy", Cons::Z, true, Mora::Force::J},
                    {"jy", Cons::Z, true, Mora::Force::J}, {"ty", Cons::T, true, Mora::Force::CH},
                    {"dy", Cons::D, true, Mora::Force::J}, {"j", Cons::Z, true, Mora::Force::J},
                    {"k", Cons::K, false, Mora::Force::None}, {"g", Cons::G, false, Mora::Force::None},
                    {"s", Cons::S, false, Mora::Force::None}, {"z", Cons::Z, false, Mora::Force::None},
                    {"t", Cons::T, false, Mora::Force::None}, {"d", Cons::D, false, Mora::Force::None},
                    {"n", Cons::N, false, Mora::Force::None}, {"h", Cons::H, false, Mora::Force::None},
                    {"f", Cons::H, false, Mora::Force::F},    {"b", Cons::B, false, Mora::Force::None},
                    {"p", Cons::P, false, Mora::Force::None}, {"m", Cons::M, false, Mora::Force::None},
                    {"r", Cons::R, false, Mora::Force::None}, {"y", Cons::Y, false, Mora::Force::None},
                    {"w", Cons::W, false, Mora::Force::None},
                };
                bool done = false;
                for (const Ent& e : ents) {
                    size_t L = std::char_traits<char>::length(e.rom);
                    if (s.compare(i, L, e.rom) != 0) continue;
                    size_t j = i + L;
                    Vowel ev;
                    if (j < s.size() && vowel_of(s[j], ev)) {
                        m.c = e.c; m.palatal = e.pal && L == 2 && e.rom[1] == 'y';
                        m.v = ev; m.force = e.f; m.text = s.substr(i, L + 1);
                        if (std::string(e.rom) == "sh" || std::string(e.rom) == "ch" || std::string(e.rom) == "j") {
                            m.palatal = (ev != Vowel::I && ev != Vowel::E) ? true : false;
                            if (ev == Vowel::E) m.palatal = false;
                        }
                        if (std::string(e.rom) == "ts") m.palatal = false;
                        if (std::string(e.rom) == "f") m.palatal = false;
                        if (m.c == Cons::Y) m.palatal = false;
                        // "shi" "chi" "ji": palatal handled by surface()
                        if (m.c == Cons::S && m.force == Mora::Force::SH && !m.palatal) m.force = Mora::Force::None;
                        if (m.c == Cons::T && m.force == Mora::Force::CH && !m.palatal && ev == Vowel::I) m.force = Mora::Force::None;
                        out.push_back(m); i = j + 1; done = true; break;
                    }
                }
                if (!done) ++i;      // skip unknown
            }
            return out;
        }

        // Parse a lyric string: UTF-8 kana first; if it is plain ASCII, treat as romaji.
        inline std::vector<Mora> parse_lyric(const std::string& text, const ParseOptions& opt = {}) {
            bool ascii = true;
            for (unsigned char c : text) if (c >= 0x80) { ascii = false; break; }
            return ascii ? parse_romaji(text, opt) : parse_kana(text, opt);
        }

        // Minimal Shift-JIS kana decoder for MIDI karaoke files (kana, ー, ASCII only).
        // Returns UTF-8, or empty if the bytes are not decodable as UTF-8/ASCII/SJIS-kana.
        inline std::string decode_lyric_bytes(const std::vector<uint8_t>& b) {
            std::string raw(b.begin(), b.end());
            if (jp_detail::valid_utf8(raw)) return raw;
            std::string out;
            for (size_t i = 0; i < b.size(); ++i) {
                uint8_t c = b[i];
                if (c < 0x80) { out += static_cast<char>(c); continue; }
                if (i + 1 >= b.size()) return std::string();
                uint8_t d = b[++i];
                uint32_t cp = 0;
                if (c == 0x82 && d >= 0x9F && d <= 0xF1) cp = 0x3041 + (d - 0x9F);
                else if (c == 0x83 && d >= 0x40 && d <= 0x96 && d != 0x7F) cp = 0x30A1 + (d - 0x40) - (d > 0x7F ? 1 : 0);
                else if (c == 0x81 && d == 0x5B) cp = 0x30FC;
                else if (c == 0x81 && (d == 0x41 || d == 0x42 || d == 0x40)) cp = ' ';
                else return std::string();
                out += jp_detail::cp_to_utf8(cp);
            }
            return out;
        }

    }
}