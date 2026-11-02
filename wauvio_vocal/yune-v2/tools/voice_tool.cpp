// voice_tool: inspect and export voice profiles.
//   voice_tool list                      built-in voices
//   voice_tool export ID out.voice       write a voice as an editable text file
//   voice_tool check file.voice          parse a .voice file (typos are errors) and report
//   voice_tool demo ID|file.voice out.wav   a short phrase in that voice (default look, then 'soft')
#include "../../../wauvio_vocal_v2.hpp"
#include <cstdio>
using namespace wauvio; using namespace wauvio::vocal;
int main(int argc, char** argv) {
    const std::string cmd = argc > 1 ? argv[1] : "";
    if (cmd == "list") { for (auto& id : voice_ids()) { VoiceProfile p; make_voice(id, p); std::printf("%-10s %s - %s\n", id.c_str(), p.display.c_str(), p.notes.c_str()); } return 0; }
    if (cmd == "export" && argc > 3) { VoiceProfile p; std::string e; if (!make_voice(argv[2], p, &e)) { std::fprintf(stderr, "%s\n", e.c_str()); return 1; } return save_profile(p, argv[3]) ? 0 : 1; }
    if (cmd == "check" && argc > 2) { VoiceProfile p = voice_yune(); std::string e; if (!load_profile(argv[2], p, &e)) { std::fprintf(stderr, "%s: %s\n", argv[2], e.c_str()); return 1; }
        std::printf("ok: %s (%s), %zu personas, range %d-%d\n", p.id.c_str(), p.display.c_str(), p.personas.names().size(), p.model.range_low, p.model.range_high); return 0; }
    if (cmd == "demo" && argc > 3) {
        VoiceProfile p; std::string e; if (!make_voice(argv[2], p, &e)) { std::fprintf(stderr, "%s\n", e.c_str()); return 1; }
        Singer s(p); const int b = p.model.range_low + 12; const int iv[] = { 0, 2, 4, 7, 5, 0 }; const char* ly[] = { "さ", "く", "ら", "さ", "く", "ら" };
        for (int i = 0; i < 6; ++i) s.note_sec(b + iv[i], i == 5 ? 1.2 : 0.45, ly[i]);
        s.rest_sec(0.4); s.persona("soft", 0.7f); s.natural();
        for (int i = 0; i < 6; ++i) s.note_sec(b + iv[i], i == 5 ? 1.2 : 0.45, ly[i]);
        for (auto& d : s.diagnostics()) std::fprintf(stderr, "warning: %s\n", d.c_str());
        Buffer buf = s.render(); normalize(buf, 0.9f); save_wav(buf, argv[3]); return 0;
    }
    std::fprintf(stderr, "usage: voice_tool list | export ID out.voice | check file.voice | demo ID|file out.wav\n"); return 1;
}
