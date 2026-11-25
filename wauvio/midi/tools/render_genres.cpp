// render_genres: renders one MIDI file in every genre preset (or a chosen few), so the presets can be compared by ear.
//
//   g++ -std=c++17 -O2 -I<repo root> wauvio/midi/tools/render_genres.cpp -o render_genres
//   ./render_genres song.mid [--out DIR] [--only TEXT] [--strength S] [--no-instruments] [--seed N] [--list]
//
//   --out DIR          output folder (default: genre_audition). Files: DIR/<family>/<Genre>.wav, plus DIR/original.wav (no genre)
//   --only TEXT        only genres whose name, alias or family contains TEXT (case-insensitive), e.g. --only metal, --only latin
//   --strength S       apply each genre at strength S (0..1; default 1). 0.5 = half-way between the original and the genre
//   --no-instruments   keep the song's own instruments and drum kit (default: the genre may swap them, as setGenre allows)
//   --seed N           seed for the genre's humanize/variance randomness (default 1)
//   --list             print the genres that would be rendered and exit
#include "../../midi.hpp"
#include "../../ext.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <string>

using namespace wauvio;
namespace fs = std::filesystem;
namespace MG = wauvio::midi::genre;
namespace MT = wauvio::midi::track;

static std::string lower(std::string s) { for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return s; }
static std::string file_safe(const std::string& s) { std::string o; for (char c : s) o += (std::isalnum(static_cast<unsigned char>(c)) ? c : '_'); return o; }

int main(int argc, char** argv) {
    if (argc < 2) { std::fprintf(stderr, "usage: render_genres song.mid [--out DIR] [--only TEXT] [--strength S] [--no-instruments] [--seed N] [--list]\n"); return 2; }
    std::string midi_path = argv[1], out = "genre_audition", only; double strength = 1.0; bool instruments = true, list = false; unsigned long long seed = 1;
    for (int i = 2; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--out" && i + 1 < argc) out = argv[++i]; else if (a == "--only" && i + 1 < argc) only = argv[++i];
        else if (a == "--strength" && i + 1 < argc) strength = std::atof(argv[++i]); else if (a == "--seed" && i + 1 < argc) seed = std::strtoull(argv[++i], nullptr, 10);
        else if (a == "--no-instruments") instruments = false; else if (a == "--list") list = true;
        else { std::fprintf(stderr, "unknown option %s\n", a.c_str()); return 2; }
    }
    if (!list) fs::create_directories(out);
    MG::GenreApplyOptions opts; opts.allow_instrumentation = instruments; opts.seed = seed;
    if (!list) { MT::MidiMusic m = MT::load_midi(midi_path); render(m, (fs::path(out) / "original.wav").string()); std::printf("original\n"); }
    int n = 0;
    for (const auto& base : wauvio::music::genre::all()) {
        std::string hay = lower(base->name + " " + base->family);
        for (const auto& al : base->aliases) hay += " " + lower(al);
        if (!only.empty() && hay.find(lower(only)) == std::string::npos) continue;
        ++n;
        if (list) { std::printf("%-22s %-20s %3.0f-%3.0f bpm  %s\n", base->name.c_str(), base->family.c_str(), base->bpm_min, base->bpm_max, base->description.c_str()); continue; }
        MT::MidiMusic m = MT::load_midi(midi_path);
        m.setGenre(MG::with_strength(MG::find(base->name), strength), opts);
        const fs::path dir = fs::path(out) / file_safe(base->family); fs::create_directories(dir);
        render(m, (dir / (file_safe(base->name) + ".wav")).string());
        std::printf("%-22s -> %s\n", base->name.c_str(), (dir / (file_safe(base->name) + ".wav")).string().c_str());
    }
    if (!list) std::printf("\n%d genres rendered to %s\n", n, out.c_str());
    return 0;
}
