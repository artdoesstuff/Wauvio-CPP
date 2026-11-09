// =============================================================================
//  wauvio_sfx_tool - discover, audition and export the wauvio_sfx preset catalog
//
//    g++ -std=c++17 -O2 -I<repo root> wauvio_sfx/tools/wauvio_sfx_tool.cpp -o wauvio_sfx_tool
//
//    wauvio_sfx_tool list [--category C] [--search TEXT]
//    wauvio_sfx_tool categories
//    wauvio_sfx_tool search TEXT
//    wauvio_sfx_tool info ID
//    wauvio_sfx_tool render ID            [options]
//    wauvio_sfx_tool render-category CAT  [options]
//    wauvio_sfx_tool render-all           [options]
//    wauvio_sfx_tool catalog              (prints the catalog as Markdown)
//
//  options:  --out DIR (default sfx_out)   --seed N   --sr HZ   --mono   --peak DB
//            --duration SEC | --beats N    --tempo BPM
//            --set NAME=VALUE (repeatable; shortcuts: --intensity --brightness --pitch --tail --width)
//  Rendering writes DIR/<category>/<id>.wav and DIR/manifest.json.
//  Exit status: 0 on success, 1 if any render failed, 2 on a usage error.
// =============================================================================
#include "../../wauvio_sfx.hpp"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <set>

using namespace wauvio::sfx;
namespace fs = std::filesystem;

namespace {
    const char* kToolVersion = "1.0";

    struct Options {
        std::string out = "sfx_out", category, search;
        bool has_seed = false; uint64_t seed = 0; int sr = 44100; bool mono = false; double peak_db = -1.0;
        bool has_duration = false; double duration = 0.0; bool has_beats = false; double beats = 0.0; bool has_tempo = false; double tempo = 120.0;
        std::vector<std::pair<std::string, double>> sets;
    };

    [[noreturn]] void usage_error(const std::string& msg) { std::fprintf(stderr, "error: %s\n(run without arguments for usage)\n", msg.c_str()); std::exit(2); }

    void print_usage() {
        std::printf("wauvio_sfx_tool %s - %zu presets in %zu categories\n\n"
                    "  list [--category C] [--search TEXT]   list presets\n"
                    "  categories                            list categories with counts\n"
                    "  search TEXT                           search ids, names, tags and descriptions\n"
                    "  info ID                               description, parameters and defaults of one preset\n"
                    "  render ID                             render one preset\n"
                    "  render-category CAT                   render every preset of a category\n"
                    "  render-all                            render the whole catalog\n"
                    "  catalog                               print the catalog as Markdown\n\n"
                    "options: --out DIR  --seed N  --sr HZ  --mono  --peak DB  --duration SEC  --beats N  --tempo BPM\n"
                    "         --set NAME=VALUE (repeatable)  --intensity X  --brightness X  --pitch X  --tail X  --width X\n",
                    kToolVersion, builtin_registry().size(), all_categories().size());
    }

    double parse_number(const std::string& s, const char* what) {
        char* end = nullptr; const double v = std::strtod(s.c_str(), &end);
        if (s.empty() || end == s.c_str() || *end != '\0' || !std::isfinite(v)) usage_error(std::string("invalid number for ") + what + ": '" + s + "'");
        return v;
    }

    Options parse_options(const std::vector<std::string>& a, size_t from) {
        Options o;
        for (size_t i = from; i < a.size(); ++i) {
            const std::string& k = a[i];
            auto next = [&](const char* what) -> const std::string& { if (i + 1 >= a.size()) usage_error(std::string("missing value after ") + what); return a[++i]; };
            if (k == "--out") o.out = next("--out");
            else if (k == "--seed") { const double v = parse_number(next("--seed"), "--seed"); if (v < 0) usage_error("--seed must be >= 0"); o.has_seed = true; o.seed = static_cast<uint64_t>(v); }
            else if (k == "--sr") o.sr = static_cast<int>(parse_number(next("--sr"), "--sr"));
            else if (k == "--mono") o.mono = true;
            else if (k == "--peak") o.peak_db = parse_number(next("--peak"), "--peak");
            else if (k == "--duration") { o.has_duration = true; o.duration = parse_number(next("--duration"), "--duration"); }
            else if (k == "--beats") { o.has_beats = true; o.beats = parse_number(next("--beats"), "--beats"); }
            else if (k == "--tempo") { o.has_tempo = true; o.tempo = parse_number(next("--tempo"), "--tempo"); }
            else if (k == "--category") o.category = next("--category");
            else if (k == "--search") o.search = next("--search");
            else if (k == "--set") { const std::string kv = next("--set"); const size_t eq = kv.find('='); if (eq == std::string::npos) usage_error("--set expects NAME=VALUE"); o.sets.emplace_back(kv.substr(0, eq), parse_number(kv.substr(eq + 1), "--set")); }
            else if (k == "--intensity" || k == "--brightness" || k == "--pitch" || k == "--tail" || k == "--width") o.sets.emplace_back(k.substr(2), parse_number(next(k.c_str()), k.c_str()));
            else usage_error("unknown option '" + k + "'");
        }
        return o;
    }

    std::string json_escape(const std::string& s) {
        std::string o; for (char ch : s) { switch (ch) { case '"': o += "\\\""; break; case '\\': o += "\\\\"; break; case '\n': o += "\\n"; break; case '\t': o += "\\t"; break;
            default: if (static_cast<unsigned char>(ch) < 0x20) { char b[8]; std::snprintf(b, sizeof b, "\\u%04x", ch); o += b; } else o += ch; } } return o;
    }
    std::string num(double v) { if (!std::isfinite(v)) return "null"; char b[64]; std::snprintf(b, sizeof b, "%.6g", v); return b; }

    struct Entry {
        const Preset* preset = nullptr; std::string file, error; bool ok = false; uint64_t seed = 0;
        std::vector<std::pair<std::string, double>> params; std::vector<std::string> not_applicable;
        double duration = 0, tempo = 120; size_t frames = 0; Stats stats;
    };

    /// Applies the options to an Effect. Parameters the preset does not have are reported (single render: error; batch: noted in the manifest).
    void configure(Effect& e, const Options& o, Entry& entry, bool strict) {
        if (o.has_seed) e.seed(o.seed);
        if (o.has_tempo) e.tempo(o.tempo);
        if (o.has_duration) e.set("duration", o.duration);
        if (o.has_beats) e.duration_beats(o.beats);
        for (auto& kv : o.sets) {
            if (!e.has(kv.first)) { if (strict) throw Error("preset '" + e.preset().id + "' has no parameter '" + kv.first + "' (valid: " + e.params().names() + ")"); entry.not_applicable.push_back(kv.first); continue; }
            e.set(kv.first, kv.second);
        }
    }

    Entry render_one(const Preset& p, const Options& o, bool strict, std::set<std::string>& used_paths) {
        Entry en; en.preset = &p;
        try {
            Effect e(p); configure(e, o, en, strict);
            RenderSettings st; st.sample_rate = o.sr; st.channels = o.mono ? 1 : 2; st.peak_db = o.peak_db;
            const Rendered r = e.render(st);
            fs::path dir = fs::path(o.out) / category_id(p.category); fs::create_directories(dir);
            std::string name = p.id; fs::path path = dir / (name + ".wav");
            for (int n = 2; used_paths.count(path.string()); ++n) path = dir / (name + "_" + std::to_string(n) + ".wav");     // never overwrite another entry of this run
            used_paths.insert(path.string()); r.save(path.string());
            if (!fs::exists(path) || fs::file_size(path) < 44) throw Error("WAV file was not written: " + path.string());
            en.file = (fs::path(category_id(p.category)) / path.filename()).generic_string(); en.ok = true; en.seed = e.seed(); en.duration = e.duration(); en.tempo = e.tempo(); en.frames = r.frames(); en.stats = r.stats();
            for (auto& s : e.params().specs()) en.params.emplace_back(s.name, e.get(s.name));
        } catch (const std::exception& ex) { en.ok = false; en.error = ex.what(); }
        return en;
    }

    void write_manifest(const std::vector<Entry>& es, const Options& o) {
        fs::create_directories(o.out); std::ofstream f((fs::path(o.out) / "manifest.json").string());
        if (!f) throw Error("cannot write manifest.json in " + o.out);
        size_t failed = 0; for (auto& e : es) failed += e.ok ? 0 : 1;
        f << "{\n  \"tool\": \"wauvio_sfx_tool " << kToolVersion << "\",\n  \"settings\": { \"sample_rate\": " << o.sr << ", \"channels\": " << (o.mono ? 1 : 2) << ", \"peak_db\": " << num(o.peak_db)
          << ", \"seed_override\": " << (o.has_seed ? std::to_string(o.seed) : "null") << ", \"tempo_override\": " << (o.has_tempo ? num(o.tempo) : "null") << " },\n"
          << "  \"count\": " << es.size() << ", \"failed\": " << failed << ",\n  \"entries\": [\n";
        for (size_t i = 0; i < es.size(); ++i) {
            const Entry& e = es[i]; const Preset& p = *e.preset;
            f << "    { \"id\": \"" << json_escape(p.id) << "\", \"name\": \"" << json_escape(p.name) << "\", \"category\": \"" << category_id(p.category) << "\", \"behavior\": \"" << behavior_name(p.behavior)
              << "\", \"reversed\": " << (p.reversed ? "true" : "false") << ", \"status\": \"" << (e.ok ? "ok" : "failed") << "\"";
            if (!e.ok) f << ", \"error\": \"" << json_escape(e.error) << "\" }";
            else {
                f << ", \"file\": \"" << json_escape(e.file) << "\", \"seed\": " << e.seed << ", \"tempo\": " << num(e.tempo) << ", \"duration_s\": " << num(e.duration) << ", \"frames\": " << e.frames
                  << ", \"seconds\": " << num(e.stats.seconds) << ", \"peak_db\": " << num(e.stats.peak_db()) << ", \"rms_db\": " << num(e.stats.rms_db()) << ", \"dc\": " << num(e.stats.dc)
                  << ", \"clip_ratio\": " << num(e.stats.clip_ratio) << ", \"params\": {";
                for (size_t k = 0; k < e.params.size(); ++k) f << (k ? ", " : " ") << "\"" << e.params[k].first << "\": " << num(e.params[k].second);
                f << " }";
                if (!e.not_applicable.empty()) { f << ", \"ignored_options\": ["; for (size_t k = 0; k < e.not_applicable.size(); ++k) f << (k ? ", " : "") << "\"" << e.not_applicable[k] << "\""; f << "]"; }
                f << " }";
            }
            f << (i + 1 < es.size() ? ",\n" : "\n");
        }
        f << "  ]\n}\n";
    }

    int render_many(const std::vector<const Preset*>& ps, const Options& o, bool strict) {
        std::vector<Entry> es; std::set<std::string> used; size_t failed = 0, i = 0;
        for (const Preset* p : ps) {
            Entry e = render_one(*p, o, strict, used); ++i;
            if (e.ok) std::printf("[%3zu/%zu] ok      %-38s %6.2f s  peak %6.2f dB  rms %6.1f dB  -> %s\n", i, ps.size(), p->id.c_str(), e.stats.seconds, e.stats.peak_db(), e.stats.rms_db(), e.file.c_str());
            else { ++failed; std::printf("[%3zu/%zu] FAILED  %-38s %s\n", i, ps.size(), p->id.c_str(), e.error.c_str()); }
            es.push_back(std::move(e));
        }
        write_manifest(es, o);
        std::printf("\n%zu rendered, %zu failed. Manifest: %s\n", es.size() - failed, failed, (fs::path(o.out) / "manifest.json").string().c_str());
        if (failed) { std::printf("Failed presets:\n"); for (auto& e : es) if (!e.ok) std::printf("  %s: %s\n", e.preset->id.c_str(), e.error.c_str()); }
        return failed ? 1 : 0;
    }

    void print_row(const Preset& p) { std::printf("  %-38s %-12s %-9s %5.1f s  %s\n", p.id.c_str(), category_id(p.category), behavior_name(p.behavior), p.default_duration(), p.name.c_str()); }

    Category need_category(const std::string& t) { Category c; if (!parse_category(t, c)) { std::string v; for (Category x : all_categories()) v += std::string(v.empty() ? "" : ", ") + category_id(x); usage_error("unknown category '" + t + "' (valid: " + v + ")"); } return c; }
}

int main(int argc, char** argv) {
    std::vector<std::string> a(argv + 1, argv + argc);
    if (a.empty() || a[0] == "help" || a[0] == "--help" || a[0] == "-h") { print_usage(); return a.empty() ? 2 : 0; }
    const std::string cmd = a[0]; const Registry& R = builtin_registry();
    try {
        if (cmd == "categories") {
            for (Category c : all_categories()) std::printf("  %-13s %2zu presets  %s\n", category_id(c), R.by_category(c).size(), category_name(c));
            return 0;
        }
        if (cmd == "list") {
            const Options o = parse_options(a, 1); std::vector<const Preset*> v;
            if (!o.category.empty()) v = R.by_category(need_category(o.category)); else for (auto& p : R.all()) v.push_back(&p);
            if (!o.search.empty()) { auto hits = R.search(o.search); std::set<const Preset*> keep(hits.begin(), hits.end()); v.erase(std::remove_if(v.begin(), v.end(), [&](const Preset* p) { return !keep.count(p); }), v.end()); }
            for (auto* p : v) print_row(*p);
            std::printf("%zu presets\n", v.size()); return 0;
        }
        if (cmd == "search") { if (a.size() < 2) usage_error("search needs a text"); const auto v = R.search(a[1]); for (auto* p : v) print_row(*p); std::printf("%zu presets\n", v.size()); return v.empty() ? 1 : 0; }
        if (cmd == "info") {
            if (a.size() < 2) { usage_error("info needs a preset id"); }
            const Preset* p = R.find(a[1]);
            if (!p) { try { make(a[1]); } catch (const Error& e) { std::fprintf(stderr, "error: %s\n", e.what()); } return 1; }
            std::printf("%s  (%s)\n  category : %s\n  behavior : %s%s\n  duration : %.2f s   tail: %.2f s   default seed: %llu\n  tags     : ", p->id.c_str(), p->name.c_str(), category_name(p->category), behavior_name(p->behavior), p->reversed ? ", reversed offline" : "", p->default_duration(), p->default_tail(), static_cast<unsigned long long>(p->default_seed));
            for (size_t i = 0; i < p->tags.size(); ++i) std::printf("%s%s", i ? ", " : "", p->tags[i].c_str());
            std::printf("\n  %s\n\n  %-12s %9s %9s %9s  %-10s %s\n", p->description.c_str(), "parameter", "default", "min", "max", "unit", "meaning");
            for (auto& s : p->params) std::printf("  %-12s %9g %9g %9g  %-10s %s\n", s.name.c_str(), s.def, s.min, s.max, s.unit.c_str(), s.description.c_str());
            return 0;
        }
        if (cmd == "render") {
            if (a.size() < 2) { usage_error("render needs a preset id"); }
            const Preset* p = R.find(a[1]);
            if (!p) { try { make(a[1]); } catch (const Error& e) { std::fprintf(stderr, "error: %s\n", e.what()); } return 1; }
            return render_many({ p }, parse_options(a, 2), true);
        }
        if (cmd == "render-category") {
            if (a.size() < 2) { usage_error("render-category needs a category"); }
            return render_many(R.by_category(need_category(a[1])), parse_options(a, 2), false);
        }
        if (cmd == "render-all") { std::vector<const Preset*> v; for (auto& p : R.all()) v.push_back(&p); return render_many(v, parse_options(a, 1), false); }
        if (cmd == "catalog") {
            std::printf("# wauvio_sfx preset catalog\n\n%zu presets in %zu categories. Generated by `wauvio_sfx_tool catalog`; ids are stable.\n\n", R.size(), all_categories().size());
            for (Category c : all_categories()) {
                std::printf("## %s (`%s`) - %zu presets\n\n| id | name | behavior | duration | tags | description |\n|---|---|---|---|---|---|\n", category_name(c), category_id(c), R.by_category(c).size());
                for (const Preset* p : R.by_category(c)) { std::string tags; for (size_t i = 0; i < p->tags.size(); ++i) tags += (i ? ", " : "") + p->tags[i];
                    std::printf("| `%s` | %s | %s%s | %.4g s%s | %s | %s |\n", p->id.c_str(), p->name.c_str(), behavior_name(p->behavior), p->reversed ? ", reversed" : "", p->default_duration(), p->default_tail() > 0 ? (" + " + std::to_string(static_cast<int>(p->default_tail())) + " s tail").c_str() : "", tags.c_str(), p->description.c_str()); }
                std::printf("\n");
            }
            return 0;
        }
        usage_error("unknown command '" + cmd + "'");
    } catch (const std::exception& e) { std::fprintf(stderr, "error: %s\n", e.what()); return 1; }
}
