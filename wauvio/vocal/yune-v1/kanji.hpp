#pragma once
// Kanji in lyrics. The kana parser only knows sounds, so kanji are first turned into hiragana here and the result goes
// through the normal parser. Three ways to say how a kanji is read, in this order:
//
//   1. Ruby, written in the lyric:   漢字《かんじ》     (the reading belongs to the whole kanji run before 《)
//                                    ｜漢字《かんじ》    (｜ marks where the base starts; also works for kana bases)
//      Always wins. Use it for anything the dictionary reads wrongly (行って, 今日, 人気 ...).
//   2. The dictionary (longest match). Entries may carry okurigana, so 食べる / 出す / 行って can be listed as words,
//      and a plain stem entry (食 -> た) lets 食べ / 食べた / 食べない work with the kana left as written.
//      The built-in list is small and favours the readings that are common in song lyrics; add your own with
//      Singer::kanji_dictionary().add("漢字", "かんじ") or load_kanji_dictionary("my.txt") (one "kanji reading" per line).
//   3. Nothing: a kanji without a reading is skipped and reported by Singer::diagnostics(). Compounds that were only
//      readable piece by piece (三人 -> さん + ひと) are reported too, because that is usually the wrong reading.
//
// Not covered: choosing a reading from context (this is a dictionary, not a morphological analyser) and Shift-JIS kanji in
// MIDI lyric events (use UTF-8 lyrics for kanji).
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace wauvio {
    namespace vocal {
        namespace v1 {

            namespace kanji_detail {
                inline bool next_cp(const std::string& s, size_t& i, uint32_t& cp) {
                    if (i >= s.size()) return false;
                    const unsigned char c = static_cast<unsigned char>(s[i]);
                    if (c < 0x80) { cp = c; ++i; return true; }
                    const int n = (c >= 0xF0) ? 3 : (c >= 0xE0) ? 2 : (c >= 0xC0) ? 1 : 0;
                    if (n == 0 || i + static_cast<size_t>(n) >= s.size()) { cp = 0xFFFD; ++i; return true; }
                    cp = (n == 1) ? (c & 0x1Fu) : (n == 2) ? (c & 0x0Fu) : (c & 0x07u);
                    for (int k = 1; k <= n; ++k) {
                        const unsigned char cc = static_cast<unsigned char>(s[i + static_cast<size_t>(k)]);
                        if ((cc & 0xC0) != 0x80) { cp = 0xFFFD; ++i; return true; }
                        cp = (cp << 6) | (cc & 0x3Fu);
                    }
                    i += static_cast<size_t>(n) + 1;
                    return true;
                }
                inline std::string put_cp(uint32_t cp) {
                    std::string o;
                    if (cp < 0x80) o += static_cast<char>(cp);
                    else if (cp < 0x800) { o += static_cast<char>(0xC0 | (cp >> 6)); o += static_cast<char>(0x80 | (cp & 0x3F)); }
                    else if (cp < 0x10000) { o += static_cast<char>(0xE0 | (cp >> 12)); o += static_cast<char>(0x80 | ((cp >> 6) & 0x3F)); o += static_cast<char>(0x80 | (cp & 0x3F)); }
                    else { o += static_cast<char>(0xF0 | (cp >> 18)); o += static_cast<char>(0x80 | ((cp >> 12) & 0x3F)); o += static_cast<char>(0x80 | ((cp >> 6) & 0x3F)); o += static_cast<char>(0x80 | (cp & 0x3F)); }
                    return o;
                }
                inline std::vector<uint32_t> decode(const std::string& s) { std::vector<uint32_t> v; size_t i = 0; uint32_t cp; while (next_cp(s, i, cp)) v.push_back(cp); return v; }
                inline std::string encode(const std::vector<uint32_t>& v, size_t a, size_t b) { std::string o; for (size_t i = a; i < b; ++i) o += put_cp(v[i]); return o; }
                // CJK ideographs plus the iteration mark 々
                inline bool is_kanji(uint32_t cp) {
                    return (cp >= 0x4E00 && cp <= 0x9FFF) || (cp >= 0x3400 && cp <= 0x4DBF) || (cp >= 0xF900 && cp <= 0xFAFF) ||
                           (cp >= 0x20000 && cp <= 0x2FA1F) || cp == 0x3005;
                }
                inline uint32_t to_hira(uint32_t cp) { return (cp >= 0x30A1 && cp <= 0x30F6) ? cp - 0x60 : cp; }
                inline std::string hira_text(const std::vector<uint32_t>& v, size_t a, size_t b) {
                    std::string o; for (size_t i = a; i < b; ++i) o += put_cp(to_hira(v[i])); return o;
                }
                constexpr uint32_t RUBY_OPEN = 0x300A, RUBY_CLOSE = 0x300B, RUBY_BASE = 0xFF5C;   // 《 》 ｜
            }

            inline bool has_kanji(const std::string& text) {
                size_t i = 0; uint32_t cp;
                while (kanji_detail::next_cp(text, i, cp)) if (kanji_detail::is_kanji(cp)) return true;
                return false;
            }

            // True if the text has kanji or ruby marks (《 ｜), i.e. kanji_to_kana has something to do.
            inline bool needs_kanji_pass(const std::string& text) {
                size_t i = 0; uint32_t cp;
                while (kanji_detail::next_cp(text, i, cp))
                    if (kanji_detail::is_kanji(cp) || cp == kanji_detail::RUBY_OPEN || cp == kanji_detail::RUBY_BASE) return true;
                return false;
            }

            class KanjiDictionary {
            public:
                KanjiDictionary() = default;
                // The built-in list (shared, built on first use) and a private copy of it to add to.
                static const KanjiDictionary& builtin();
                static KanjiDictionary with_builtin() { return builtin(); }

                // Adds or replaces one entry. The reading may be hiragana or katakana. Returns false if either side is empty.
                bool add(const std::string& key, const std::string& reading) {
                    using namespace kanji_detail;
                    const std::vector<uint32_t> k = decode(key), r = decode(reading);
                    if (k.empty() || r.empty()) return false;
                    map_[key] = hira_text(r, 0, r.size());
                    if (k.size() > max_len_) max_len_ = k.size();
                    return true;
                }
                // One entry per line: `key reading` (space or tab). Blank lines and lines starting with # are ignored.
                // Valid lines are loaded even if others fail; returns false (with *err naming the first bad line) if any failed.
                bool load_text(const std::string& text, std::string* err = nullptr) {
                    std::istringstream in(text); std::string line; int n = 0; bool ok = true;
                    while (std::getline(in, line)) {
                        ++n;
                        if (n == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
                        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) line.pop_back();
                        const size_t a = line.find_first_not_of(" \t");
                        if (a == std::string::npos || line[a] == '#') continue;
                        const size_t sp = line.find_first_of(" \t", a);
                        const size_t b = sp == std::string::npos ? std::string::npos : line.find_first_not_of(" \t", sp);
                        if (b == std::string::npos || !add(line.substr(a, sp - a), line.substr(b))) {
                            if (ok && err) *err = "line " + std::to_string(n) + ": expected `kanji reading`";
                            ok = false;
                        }
                    }
                    return ok;
                }
                bool load_file(const std::string& path, std::string* err = nullptr) {
                    std::ifstream f(path, std::ios::binary);
                    if (!f) { if (err) *err = "cannot open " + path; return false; }
                    std::ostringstream ss; ss << f.rdbuf();
                    return load_text(ss.str(), err);
                }
                size_t size() const { return map_.size(); }
                const std::string* find(const std::string& key) const { auto it = map_.find(key); return it == map_.end() ? nullptr : &it->second; }

                // Longest entry that starts at cps[i]. On success: *len = codepoints used, *reading = its hiragana.
                bool longest(const std::vector<uint32_t>& cps, size_t i, size_t* len, const std::string** reading) const {
                    size_t l = std::min(max_len_, cps.size() - i);
                    for (; l >= 1; --l) {
                        auto it = map_.find(kanji_detail::encode(cps, i, i + l));
                        if (it != map_.end()) { *len = l; *reading = &it->second; return true; }
                    }
                    return false;
                }
            private:
                std::unordered_map<std::string, std::string> map_;
                size_t max_len_ = 1;
            };

            // Kanji -> hiragana. Text without kanji is returned unchanged. `notes` (optional) receives short messages for
            // kanji without a reading and for compounds read piece by piece.
            inline std::string kanji_to_kana(const std::string& text, const KanjiDictionary& dict, std::vector<std::string>* notes = nullptr) {
                using namespace kanji_detail;
                if (!needs_kanji_pass(text)) return text;
                const std::vector<uint32_t> cps = decode(text);
                const size_t n = cps.size();
                std::string out, last;           // `last`: reading of the previous single kanji, for 々
                auto find_close = [&](size_t from) { for (size_t k = from + 1; k < n; ++k) if (cps[k] == RUBY_CLOSE) return k; return n; };
                size_t i = 0;
                while (i < n) {
                    const uint32_t cp = cps[i];
                    if (cp == RUBY_BASE) {                                   // ｜base《reading》
                        size_t k = i + 1; while (k < n && cps[k] != RUBY_OPEN && cps[k] != RUBY_BASE) ++k;
                        if (k < n && cps[k] == RUBY_OPEN) { const size_t c = find_close(k); if (c < n) { last = hira_text(cps, k + 1, c); out += last; i = c + 1; continue; } }
                        ++i; continue;                                       // stray marker
                    }
                    if (!is_kanji(cp)) { out += put_cp(cp); ++i; continue; }
                    size_t r = i; while (r < n && is_kanji(cps[r])) ++r;     // the kanji run
                    if (r < n && cps[r] == RUBY_OPEN) {                      // 漢字《かんじ》
                        const size_t c = find_close(r);
                        if (c < n) { last = hira_text(cps, r + 1, c); out += last; i = c + 1; continue; }
                    }
                    size_t pos = i; int pieces = 0; bool any_single = false;
                    while (pos < r) {
                        if (cps[pos] == 0x3005) {                            // 々 repeats the previous kanji's reading
                            if (!last.empty()) out += last; else if (notes) notes->push_back("no reading for 々 here");
                            ++pos; continue;
                        }
                        size_t len = 0; const std::string* rd = nullptr;
                        if (dict.longest(cps, pos, &len, &rd)) {
                            out += *rd; ++pieces; if (len == 1) { any_single = true; last = *rd; } else last.clear();
                            pos += len;
                        } else {
                            if (notes) notes->push_back("no reading for " + put_cp(cps[pos]) + "  (add it to the dictionary or write " + put_cp(cps[pos]) + "《よみ》)");
                            last.clear(); ++pos;
                        }
                    }
                    if (notes && pieces >= 2 && any_single) {
                        const std::string word = encode(cps, i, std::min(r, pos));
                        notes->push_back(word + " was read piece by piece; check it or add the word to the dictionary / write " + word + "《よみ》");
                    }
                    i = std::max(pos, r);
                }
                return out;
            }

            namespace kanji_detail {
                // Built-in readings: the common lyric vocabulary, plus kun stems that take okurigana (思 -> おも keeps 思い / 思う).
                // A stem used by itself next to another kanji is read piece by piece and reported (see above).
                inline const char* seed_people() { return R"KJ(
私 わたし
私達 わたしたち
僕 ぼく
僕達 ぼくたち
君 きみ
君達 きみたち
俺 おれ
貴方 あなた
彼 かれ
彼女 かのじょ
誰 だれ
何 なに
何度 なんど
何回 なんかい
人 ひと
人々 ひとびと
皆 みんな
皆さん みなさん
母 はは
父 ちち
親 おや
子 こ
子供 こども
大人 おとな
友 とも
友達 ともだち
仲間 なかま
少女 しょうじょ
少年 しょうねん
男 おとこ
女 おんな
先生 せんせい
自分 じぶん
一人 ひとり
二人 ふたり
達 たち
手 て
足 あし
目 め
口 くち
耳 みみ
顔 かお
髪 かみ
瞳 ひとみ
背中 せなか
胸 むね
指 ゆび
肩 かた
腕 うで
頬 ほお
体 からだ
)KJ"; }
                inline const char* seed_nature() { return R"KJ(
空 そら
海 うみ
山 やま
川 かわ
星 ほし
月 つき
太陽 たいよう
雲 くも
雨 あめ
雪 ゆき
風 かぜ
花 はな
桜 さくら
森 もり
夜 よる
朝 あさ
昼 ひる
夕方 ゆうがた
夕焼け ゆうやけ
朝日 あさひ
夕日 ゆうひ
日 ひ
日々 ひび
光 ひかり
影 かげ
闇 やみ
水 みず
火 ひ
土 つち
木 き
虹 にじ
波 なみ
嵐 あらし
春 はる
夏 なつ
秋 あき
冬 ふゆ
季節 きせつ
町 まち
街 まち
道 みち
家 いえ
部屋 へや
窓 まど
扉 とびら
橋 はし
坂 さか
丘 おか
島 しま
庭 にわ
野原 のはら
景色 けしき
地球 ちきゅう
宇宙 うちゅう
旅 たび
旅立 たびだ
音 おと
音楽 おんがく
言葉 ことば
物語 ものがたり
詩 し
)KJ"; }
                inline const char* seed_abstract() { return R"KJ(
愛 あい
恋 こい
夢 ゆめ
夢中 むちゅう
心 こころ
声 こえ
歌 うた
涙 なみだ
笑顔 えがお
泣き顔 なきがお
命 いのち
世界 せかい
未来 みらい
過去 かこ
現在 げんざい
今 いま
時 とき
時々 ときどき
時間 じかん
瞬間 しゅんかん
一瞬 いっしゅん
永遠 えいえん
記憶 きおく
約束 やくそく
思い出 おもいで
幸せ しあわせ
悲しみ かなしみ
楽しみ たのしみ
孤独 こどく
運命 うんめい
奇跡 きせき
勇気 ゆうき
希望 きぼう
絶望 ぜつぼう
人生 じんせい
生活 せいかつ
自由 じゆう
平和 へいわ
本当 ほんとう
全部 ぜんぶ
全て すべて
一番 いちばん
最初 さいしょ
最後 さいご
最高 さいこう
必要 ひつよう
普通 ふつう
特別 とくべつ
大切 たいせつ
大丈夫 だいじょうぶ
大好き だいすき
無理 むり
無限 むげん
色々 いろいろ
様々 さまざま
一緒 いっしょ
一生 いっしょう
毎日 まいにち
今日 きょう
明日 あした
昨日 きのう
今夜 こんや
今朝 けさ
今年 ことし
来年 らいねん
去年 きょねん
毎年 まいとし
此処 ここ
其処 そこ
一つ ひとつ
二つ ふたつ
三つ みっつ
四つ よっつ
五つ いつつ
六つ むっつ
七つ ななつ
八つ やっつ
九つ ここのつ
一 いち
二 に
三 さん
四 よん
五 ご
六 ろく
七 なな
八 はち
九 きゅう
十 じゅう
百 ひゃく
千 せん
万 まん
零 れい
)KJ"; }
                inline const char* seed_stems() { return R"KJ(
言 い
行 い
行って いって
行った いった
行う おこなう
来る くる
来て きて
来た きた
来ない こない
来い こい
見 み
聞 き
知 し
気 き
気持ち きもち
生き いき
生まれ うまれ
死 し
立 た
歩 ある
走 はし
飛 と
泳 およ
咲 さ
散 ち
落 お
消 き
消す けす
消し けし
続 つづ
届 とど
繋 つな
守 まも
探 さが
待 ま
会 あ
出 で
出す だす
出し だし
入 はい
入れ いれ
帰 かえ
戻 もど
向 む
進 すす
変 か
終 お
始 はじ
開 ひら
閉 と
話 はな
笑 わら
泣 な
抱 だ
握 にぎ
触 ふ
感 かん
信 しん
願 ねが
祈 いの
叶 かな
忘 わす
覚 おぼ
目覚め めざめ
失 うしな
壊 こわ
乗 の
作 つく
使 つか
持 も
取 と
渡 わた
送 おく
受 う
思 おも
想 おも
重 おも
大 おお
小 ちい
高 たか
低 ひく
長 なが
短 みじか
広 ひろ
深 ふか
遠 とお
近 ちか
早 はや
遅 おそ
新 あたら
古 ふる
白 しろ
黒 くろ
赤 あか
青 あお
緑 みどり
黄色 きいろ
暗 くら
明 あか
冷 つめ
熱 あつ
軽 かる
若 わか
無 な
同 おな
好 す
嫌 きら
強 つよ
弱 よわ
美 うつく
痛 いた
苦 くる
怖 こわ
寂 さび
優 やさ
悲 かな
嬉 うれ
楽 たの
幸 しあわ
)KJ"; }
            }

            inline const KanjiDictionary& KanjiDictionary::builtin() {
                static const KanjiDictionary d = [] {
                    KanjiDictionary k;
                    k.load_text(kanji_detail::seed_stems());      // stems first, so the word entries below win
                    k.load_text(kanji_detail::seed_people());
                    k.load_text(kanji_detail::seed_nature());
                    k.load_text(kanji_detail::seed_abstract());
                    return k;
                }();
                return d;
            }

        }
    }
}
