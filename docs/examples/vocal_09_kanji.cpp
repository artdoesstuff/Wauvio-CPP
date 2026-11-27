// Kanji in lyrics. They are converted to kana first, in this order of priority:
//   1. ruby written in the lyric:  漢字《かんじ》   (always wins)
//   2. the dictionary (a built-in list of common song vocabulary, plus your own entries)
//   3. otherwise the kanji is skipped and reported by diagnostics()
#include "wauvio/vocal/v3.hpp"
#include <cstdio>
using namespace wauvio::vocal::v3;

int main() {
    Singer s;
    s.set_tempo(90);

    s.kanji_dictionary().add("星空", "ほしぞら");      // teach it a word it does not know (reading in hiragana)
    // s.load_kanji_dictionary("my_words.txt");       // or a file with one "kanji reading" per line

    s.note(67, 480, "夢");                // from the built-in dictionary: ゆめ
    s.note(69, 480, "空");                // そら
    s.note(71, 960, "星空");              // from the entry we just added
    s.note(72, 480, "今日《きょう》");     // ruby: you choose the reading, overriding the dictionary
    s.note(74, 960, "鬱");                // unknown: skipped, and reported below

    for (const std::string& problem : s.diagnostics()) std::printf("diagnostic: %s\n", problem.c_str());
    s.save("vocal_09_kanji.wav");
}
