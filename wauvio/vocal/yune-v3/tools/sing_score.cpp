// sing_score: render a text score (or a MIDI file) with the V3 singer.
// usage: sing_score score.txt|song.mid out.wav [--voice id|file.wvc] [--dump timeline.csv] [--check]
//
// Score commands (one per line, # comments):
//   voice yune|path.wvc            (first line: which character sings; default yune)
//   tempo BPM                        rest TICKS
//   style NAME VALUE                 any Style field (see voice.hpp); "style devoicing off"
//   persona NAME [AMOUNT]            persona_blend A B T
//   persona_lane NAME t:v;t:v;...    lane NAME t:v;t:v;...    (absolute seconds; NAME "pitch" = semitones)
//   seed N    humanize [TIMING PITCH LENGTH BIAS TAKE]    auto_expression [on|off]
//   natural [AMOUNT [TAKE]]          V3: a Singer starts at the voice's own natural amount (singer-sized for Yune) and
//                                    auto_expression on; `natural 0` + `auto_expression off` gives a perfectly even delivery
//   arc KIND T0 T1 [DEPTH]           phrase_arc over seconds
//   note MIDI TICKS LYRIC [VEL] [extras]       lyric may be {k a n a} phoneme input
//   event KIND MIDI SECONDS [INTENSITY] [VARIANT]
//   say "TEXT" SECONDS [KIND] [MIDI] [accent=N]   speech with intonation (TEXT without spaces);
//                                    accent=N: Tokyo pitch accent (0 heiban, 1 atamadaka, n = falls after mora n)
//   accent_height ST                 L-H distance for say(..., accent=N), default 3 semitones
//   parse particles on|off           read a trailing は/へ of a lyric as わ/え ("は^" / "へ^" mark one anywhere)
//   double N                         (V3) add N doubled takes of the lead, detuned and spread in stereo
//   harmony SEMITONES [PAN [GAIN]]   (V3) add a transposed copy of the lead as a harmony part
//   chain natural|pop|intimate|hall  (V3) finish with the wauvio::fx vocal chain (stereo output)
// note extras: vib=D,R,DELAY porta=S novib stac whisper hum orn=NAME[,DEPTH] persona=NAME[,AMOUNT]
//              lane=NAME@t:v;t:v  (seconds from note start)  NAME=VALUE (any Style field)
#include "../../v3_fx.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

using namespace wauvio;
using namespace wauvio::vocal::v3;

static Curve parse_curve(const std::string& s) {
    Curve c; std::istringstream is(s); std::string tok;
    while (std::getline(is, tok, ';')) { double t, v; if (std::sscanf(tok.c_str(), "%lf:%lf", &t, &v) == 2) c.push_back({ t, v }); }
    return c;
}
static std::vector<std::string> split(const std::string& s, char d) { std::vector<std::string> o; std::istringstream is(s); std::string t; while (std::getline(is, t, d)) o.push_back(t); return o; }

int main(int argc, char** argv) {
    if (argc < 3) { std::fprintf(stderr, "usage: %s score.txt|song.mid out.wav [--dump timeline.csv] [--check]\n", argv[0]); return 1; }
    const std::string in = argv[1], out = argv[2]; std::string dump, start_voice; bool check = false;
    for (int i = 3; i < argc; ++i) { if (!std::strcmp(argv[i], "--dump") && i + 1 < argc) dump = argv[++i]; else if (!std::strcmp(argv[i], "--voice") && i + 1 < argc) start_voice = argv[++i]; else if (!std::strcmp(argv[i], "--check")) check = true; }
    Singer s;
    int n_double = 0; std::string chain;
    struct Harm { int st = 0; float pan = 0.f, gain = 0.7f; };
    std::vector<Harm> harmonies;
    if (!start_voice.empty()) { VoiceProfile p; std::string e; if (!make_voice(start_voice, p, &e)) { std::fprintf(stderr, "voice '%s': %s\n", start_voice.c_str(), e.c_str()); return 1; } s.set_profile(p); }
    if (in.size() > 4 && (in.substr(in.size() - 4) == ".mid" || (in.size() > 5 && in.substr(in.size() - 5) == ".midi"))) {
        size_t n = s.load_midi_file(in);
        std::printf("loaded %zu notes from MIDI\n", n);
    } else {
        std::ifstream f(in); if (!f) { std::fprintf(stderr, "cannot open %s\n", in.c_str()); return 1; }
        std::string line; int ln = 0;
        auto num = [&](const std::string& t, float& v) { try { v = std::stof(t); return true; } catch (...) { std::fprintf(stderr, "line %d: bad number '%s'\n", ln, t.c_str()); return false; } };
        while (std::getline(f, line)) {
            ++ln; auto h = line.find('#'); if (h != std::string::npos) line.resize(h);
            std::istringstream is(line); std::string cmd; if (!(is >> cmd)) continue;
            if (cmd == "voice") { std::string v; is >> v; VoiceProfile p; std::string e; if (!make_voice(v, p, &e)) std::fprintf(stderr, "line %d: voice '%s': %s\n", ln, v.c_str(), e.c_str()); else s.set_profile(p); }
            else if (cmd == "tempo") { double b = 120; is >> b; s.set_tempo(b); }
            else if (cmd == "rest") { int t = 0; is >> t; s.rest(t); }
            else if (cmd == "seed") { unsigned v = 0; is >> v; s.seed(v); }
            else if (cmd == "style") {
                std::string k, vs; is >> k >> vs;
                if (k == "devoicing") { s.style().devoicing = (vs != "0" && vs != "false" && vs != "off"); continue; }
                float v = 0.f; if (!num(vs, v)) continue;
                if (float Style::* m = style_field(k)) s.style().*m = v; else std::fprintf(stderr, "line %d: unknown style '%s'\n", ln, k.c_str());
            }
            else if (cmd == "persona") { std::string n; float a = 1.f; is >> n; std::string as; if (is >> as) num(as, a);
                if (!s.personas().find(n)) std::fprintf(stderr, "line %d: unknown persona '%s'\n", ln, n.c_str()); else s.persona(n, a); }
            else if (cmd == "persona_blend") { std::string a, b; float t = 0.5f; is >> a >> b; std::string ts; is >> ts; num(ts, t); s.persona_blend(a, b, t); }
            else if (cmd == "persona_lane" || cmd == "lane") { std::string n, c; is >> n >> c; if (cmd == "lane") s.lane(n, parse_curve(c)); else s.persona_lane(n, parse_curve(c)); }
            else if (cmd == "humanize") { double t = 0.012, p = 0.05, l = 0.03, b = 0.0; unsigned take = 1; is >> t >> p >> l >> b >> take; s.humanize(t, p, l, b, take); }
            else if (cmd == "auto_expression") { std::string v; is >> v; s.auto_expression(!(v == "off" || v == "0" || v == "false")); }
            else if (cmd == "natural") { float amt = -1.f; unsigned take = 1; std::string t; if (is >> t) { num(t, amt); is >> take; } s.natural(amt, take); }
            else if (cmd == "arc") { std::string k; double t0 = 0, t1 = 0; float d = 1.f; is >> k >> t0 >> t1; std::string ds; if (is >> ds) num(ds, d); s.phrase_arc(k, t0, t1, d); }
            else if (cmd == "event") { std::string k, var; int m = 64; double secs = 1.0; float it = 0.6f; is >> k >> m >> secs; std::string x; if (is >> x) num(x, it); is >> var; s.event(k, m, secs, it, var); }
            else if (cmd == "say") {
                std::string txt, kind = "statement"; double secs = 1.0; int m = 64, acc = -1; is >> txt >> secs;
                std::string x; while (is >> x) {
                    if (x.rfind("accent=", 0) == 0) acc = std::atoi(x.c_str() + 7);
                    else if (std::isdigit((unsigned char)x[0]) || x[0] == '-') m = std::atoi(x.c_str());
                    else kind = x;
                }
                s.say(txt, secs, kind, m, 0.7f, acc);
            }
            else if (cmd == "accent_height") { float v = 3.f; std::string t; is >> t; if (num(t, v)) s.accent_height(v); }
            else if (cmd == "parse") { std::string k, v; is >> k >> v; const bool on = (v != "0" && v != "off" && v != "false");
                if (k == "particles") s.parse_options().particles = on; else if (k == "cute") s.parse_options().cute = on; else std::fprintf(stderr, "line %d: unknown parse option '%s'\n", ln, k.c_str()); }
            else if (cmd == "double") { is >> n_double; }
            else if (cmd == "harmony") { Harm h; is >> h.st; std::string a; if (is >> a) num(a, h.pan); if (is >> a) num(a, h.gain); harmonies.push_back(h); }
            else if (cmd == "chain") { is >> chain; }
            else if (cmd == "note") {
                int m = 60, t = 480; std::string lyr; float vel = 0.75f; is >> m >> t >> lyr;
                if (!lyr.empty() && lyr[0] == '{' && lyr.find('}') == std::string::npos) { std::string x; while (is >> x) { lyr += " " + x; if (x.find('}') != std::string::npos) break; } }
                std::vector<std::string> extra; std::string x; while (is >> x) extra.push_back(x);
                size_t first_extra = 0; if (!extra.empty() && (std::isdigit((unsigned char)extra[0][0]) || extra[0][0] == '.')) { vel = std::stof(extra[0]); first_extra = 1; }
                NoteRef r = s.note(m, t, lyr, vel);
                for (size_t i = first_extra; i < extra.size(); ++i) {
                    const std::string& e = extra[i];
                    if (e.rfind("vib=", 0) == 0) { float d = 0, rt = -1, dl = -1; std::sscanf(e.c_str() + 4, "%f,%f,%f", &d, &rt, &dl); r.vibrato(d, rt, dl); }
                    else if (e.rfind("porta=", 0) == 0) r.portamento(std::stof(e.substr(6)));
                    else if (e == "novib") r.no_vibrato();
                    else if (e == "stac") r.articulation(wauvio::music::Articulation::Staccato);
                    else if (e == "whisper") r.articulation(wauvio::music::Articulation::Whisper);
                    else if (e == "hum") r.articulation(wauvio::music::Articulation::Humming);
                    else if (e.rfind("orn=", 0) == 0) { auto p = split(e.substr(4), ','); r.ornament(p[0], p.size() > 1 ? std::stof(p[1]) : 1.f); }
                    else if (e.rfind("persona=", 0) == 0) { auto p = split(e.substr(8), ','); r.persona(p[0], p.size() > 1 ? std::stof(p[1]) : 1.f); }
                    else if (e.rfind("lane=", 0) == 0) { auto at = e.find('@'); if (at != std::string::npos) r.lane(e.substr(5, at - 5), parse_curve(e.substr(at + 1))); }
                    else if (auto eq = e.find('='); eq != std::string::npos) {
                        float Style::* fm = style_field(e.substr(0, eq)); float v = 0.f;
                        if (fm && num(e.substr(eq + 1), v)) r.style().*fm = v; else if (!fm) std::fprintf(stderr, "line %d: unknown note option '%s'\n", ln, e.c_str());
                    }
                }
            } else std::fprintf(stderr, "line %d: unknown command '%s'\n", ln, cmd.c_str());
        }
    }
    for (const std::string& d : s.diagnostics()) std::fprintf(stderr, "warning: %s\n", d.c_str());
    if (check) return s.diagnostics().empty() ? 0 : 2;
    if (!dump.empty()) {                                   // what the engine will actually sing
        std::ofstream o(dump); o << "start,dur,midi,vel,lyric_morae,event\n";
        for (const SynthNote& n : s.notes()) { o << n.start << ',' << n.dur << ',' << n.midi << ',' << n.vel << ',' << n.morae.size() << ',' << n.event << '\n'; }
    }
    if (n_double > 0 || !harmonies.empty() || !chain.empty()) {            // stereo path
        StereoBuffer st;
        if (n_double > 0 || !harmonies.empty()) {
            Ensemble e; e.add(s, "lead");
            if (n_double > 0) e.add_double(s, n_double);
            for (const Harm& h : harmonies) e.add_harmony(s, h.st, "", h.pan, h.gain);
            st = e.render_stereo();
        } else st = pan_mono(s.render(), 0.0f);
        if (!chain.empty()) { try { st = process_vocal(std::move(st), chain); } catch (const std::exception& ex) { std::fprintf(stderr, "%s\n", ex.what()); return 1; } }
        float pk = 0.f; for (size_t i = 0; i < st.size(); ++i) pk = std::max(pk, std::max(std::fabs(st.L[i]), std::fabs(st.R[i])));
        if (pk > 1e-6f) for (size_t i = 0; i < st.size(); ++i) { st.L[i] *= 0.9f / pk; st.R[i] *= 0.9f / pk; }
        save_wav_stereo(st, out);
        std::printf("wrote %s (%.1f s, stereo, %zu notes)\n", out.c_str(), double(st.size()) / global_config().sample_rate, s.size());
        return 0;
    }
    Buffer b = s.render(); normalize(b, 0.9f); save_wav(b, out);
    std::printf("wrote %s (%.1f s, %zu notes)\n", out.c_str(), double(b.size()) / global_config().sample_rate, s.size());
    return 0;
}
