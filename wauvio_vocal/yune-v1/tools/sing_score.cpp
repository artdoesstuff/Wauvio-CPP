#include "wauvio_vocal.hpp"
#include <cstdio>
#include <fstream>
#include <map>
#include <sstream>

using namespace wauvio;
using namespace wauvio::vocal;

static bool set_style(Style& s, const std::string& k, float v) {
    static const std::map<std::string, float Style::*> m = {
        {"brightness",&Style::brightness},{"breathiness",&Style::breathiness},{"tension",&Style::tension},
        {"softness",&Style::softness},{"power",&Style::power},{"resonance",&Style::resonance},{"femininity",&Style::femininity},
        {"vibrato_depth",&Style::vibrato_depth},{"vibrato_rate",&Style::vibrato_rate},{"vibrato_delay",&Style::vibrato_delay},
        {"vibrato_fade",&Style::vibrato_fade},{"portamento",&Style::portamento},{"pitch_response",&Style::pitch_response},
        {"overshoot",&Style::overshoot},{"scoop",&Style::scoop},{"exit_fall",&Style::exit_fall},{"drift",&Style::drift},
        {"consonant_strength",&Style::consonant_strength},{"consonant_duration",&Style::consonant_duration},
        {"vowel_clarity",&Style::vowel_clarity},{"legato",&Style::legato},{"attack_sharpness",&Style::attack_sharpness},
        {"release",&Style::release},{"volume",&Style::volume},{"velocity_response",&Style::velocity_response},
        {"compression",&Style::compression},{"dynamic_timbre",&Style::dynamic_timbre}};
    auto it = m.find(k); if (it == m.end()) return false; s.*(it->second) = v; return true;
}

int main(int argc, char** argv) {
    if (argc < 3) { std::fprintf(stderr, "usage: %s score.txt|song.mid out.wav\n", argv[0]); return 1; }
    const std::string in = argv[1], out = argv[2];
    Singer s;
    if (in.size() > 4 && (in.substr(in.size() - 4) == ".mid" || in.substr(in.size() - 5) == ".midi")) {
        size_t n = s.load_midi_file(in);
        std::printf("loaded %zu notes from MIDI\n", n);
    } else {
        std::ifstream f(in); if (!f) { std::fprintf(stderr, "cannot open %s\n", in.c_str()); return 1; }
        std::string line; int ln = 0;
        while (std::getline(f, line)) {
            ++ln; auto h = line.find('#'); if (h != std::string::npos) line.resize(h);
            std::istringstream is(line); std::string cmd; if (!(is >> cmd)) continue;
            if (cmd == "tempo") { double b; is >> b; s.set_tempo(b); }
            else if (cmd == "rest") { int t; is >> t; s.rest(t); }
            else if (cmd == "style") { std::string k; float v; is >> k >> v; if (!set_style(s.style(), k, v)) std::fprintf(stderr, "line %d: unknown style '%s'\n", ln, k.c_str()); }
            else if (cmd == "note") {
                int m, t; std::string lyr; float vel = 0.75f; is >> m >> t >> lyr;
                std::vector<std::string> extra; std::string x; while (is >> x) extra.push_back(x);
                size_t first_extra = 0; if (!extra.empty() && (std::isdigit((unsigned char)extra[0][0]) || extra[0][0] == '.')) { vel = std::stof(extra[0]); first_extra = 1; }
                NoteRef r = s.note(m, t, lyr, vel);
                for (size_t i = first_extra; i < extra.size(); ++i) {
                    const std::string& e = extra[i];
                    if (e.rfind("vib=", 0) == 0) { float d = 0, rt = -1, dl = -1; std::sscanf(e.c_str() + 4, "%f,%f,%f", &d, &rt, &dl); r.vibrato(d, rt, dl); }
                    else if (e.rfind("porta=", 0) == 0) r.portamento(std::stof(e.substr(6)));
                    else if (e == "novib") r.no_vibrato();
                    else if (e == "stac") r.articulation(audio::Articulation::Staccato);
                    else if (e == "whisper") r.articulation(audio::Articulation::Whisper);
                    else if (e == "hum") r.articulation(audio::Articulation::Humming);
                }
            } else std::fprintf(stderr, "line %d: unknown command '%s'\n", ln, cmd.c_str());
        }
    }
    Buffer b = s.render(); normalize(b, 0.9f); save_wav(b, out);
    std::printf("wrote %s (%.1f s, %zu notes)\n", out.c_str(), double(b.size()) / global_config().sample_rate, s.size());
    return 0;
}
