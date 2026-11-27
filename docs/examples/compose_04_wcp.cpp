// Bring your own harmony and motifs with a .wcp (Wauvio Compose Preset) text file.
#include "wauvio/compose.hpp"
#include <fstream>
#include <iostream>
using namespace wauvio::compose;

int main() {
    // A .wcp file is plain text. Write one (or edit it by hand). '#' starts a comment.
    std::ofstream("my_pack.wcp") << R"(# My pack
name My Pack
author me
key E minor
bpm 96
meter 4/4

progression "Verse"
  tags minor, moody
  chords | i:4 | VI:4 | III:4 | VII:4 |     # roman numerals; :N is the length in beats
end

motif "Hook"
  tags hook
  notes 0:0.75 2:0.75 c2:0.5 4:1.0@f r:0.5 3:0.5     # scale step:beats[@dynamic]; cN = Nth chord tone; r = rest
end
)";

    WcpBank bank; std::string err;
    if (!load_wcp("my_pack.wcp", bank, &err)) { std::cout << "bad file: " << err << "\n"; return 1; }
    std::cout << "loaded " << bank.progressions.size() << " progression(s), " << bank.motifs.size() << " motif(s)\n";

    ComposedSong song = Composer()
        .key(bank.key.root, bank.key.scale).tempo(bank.bpm).mood(Mood::Melancholic)
        .use_wcp(bank, "Verse", "Hook")          // use the pack's progression and motif
        .seed(3)
        .compose();
    song.save("compose_04_wcp.wav");

    // Shortcut: one call from a file.
    // compose_from_wcp("my_pack.wcp", "Verse", "Hook", Mood::Melancholic, 3).save("shortcut.wav");
}
