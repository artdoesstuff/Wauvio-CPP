// render_instruments: writes one audition WAV per instrument (and per drum kit) plus index.csv, for listening one by one.
//
//   g++ -std=c++17 -O2 -I<repo root> wauvio/ext/tools/render_instruments.cpp -o render_instruments
//   ./render_instruments [--out DIR] [--only TEXT] [--normalize] [--split] [--list] [--no-kits]
//
//   --out DIR      output folder (default: instrument_audition). Files land in DIR/<family>/NNN_<Name>.wav, kits in DIR/_kits/.
//   --only TEXT    only instruments whose family, class or name contains TEXT (case-insensitive), e.g. --only brass, --only horn
//   --normalize    scale each file to a -1 dBFS peak. Default is OFF so loudness differences between instruments stay audible.
//   --split        also write every segment as its own file, DIR/<family>/NNN_<Name>/<label>.wav
//   --list         print the instruments that would be rendered and exit
//   --no-kits      skip the drum kits
//
// Each instrument file is a fixed sequence (dry, no effects), 0.35 s apart:
//   pitched:   C3 mf | C4 mf | C5 mf | C4 pp | C4 ff | C-major scale (legato-ish) | one note per supported articulation
//   unpitched: hit pp | hit mf | hit ff | one hit per supported articulation
// index.csv lists every file with objective measurements on the C4 mf note (levels, pitch error in cents, attack time capped at 300 ms, spectral
// centroid, how abruptly it ends) and a FLAGS column for things that are technically wrong (CLIP, DC, SILENT, PITCH, CUTOFF).
// segments.csv gives the start time and label of every segment inside every file.
//
// instrument_catalog.hpp is generated: run `python3 gen_catalog.py` (same folder) after adding or removing an instrument class.
#include "../../ext.hpp"
#include "instrument_catalog.hpp"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

using namespace wauvio;
namespace fs = std::filesystem;

namespace {
    const double GAP = 0.35;

    struct Options { std::string out = "instrument_audition", only; bool normalize = false, split = false, list = false, kits = true; };
    struct Segment { std::string label; double start = 0, dur = 0; };
    struct Metrics {
        double peak_db = -200, rms_db = -200, c4_peak_db = -200, c4_rms_db = -200, f0_err = NAN, attack_ms = NAN, centroid = NAN, end_db = NAN, dc = 0;
        size_t clipped = 0; std::string flags;
    };

    std::string lower(std::string s) { for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return s; }
    std::string file_safe(const std::string& s) {
        std::string o; for (char c : s) o += (std::isalnum(static_cast<unsigned char>(c)) ? c : '_');
        while (o.size() > 1 && o.back() == '_') o.pop_back(); return o;
    }
    double to_db(double x) { return x > 1e-10 ? 20.0 * std::log10(x) : -200.0; }
    Buffer mono(const StereoBuffer& b) { Buffer m(b.L.size()); for (size_t i = 0; i < m.size(); ++i) m[i] = 0.5f * (b.L[i] + b.R[i]); return m; }

    // Overlap-add src into dst starting at `offset`, growing dst as needed.
    void add_at(StereoBuffer& dst, const StereoBuffer& src, size_t offset) {
        const size_t need = offset + src.L.size();
        if (dst.L.size() < need) { dst.L.resize(need, 0.0f); dst.R.resize(need, 0.0f); }
        for (size_t i = 0; i < src.L.size(); ++i) { dst.L[offset + i] += src.L[i]; dst.R[offset + i] += src.R[i]; }
    }

    void fft(std::vector<std::complex<double>>& a) {
        const size_t n = a.size();
        for (size_t i = 1, j = 0; i < n; ++i) { size_t bit = n >> 1; for (; j & bit; bit >>= 1) j ^= bit; j ^= bit; if (i < j) std::swap(a[i], a[j]); }
        for (size_t len = 2; len <= n; len <<= 1) {
            const double ang = -2.0 * M_PI / static_cast<double>(len); const std::complex<double> wl(std::cos(ang), std::sin(ang));
            for (size_t i = 0; i < n; i += len) { std::complex<double> w(1); for (size_t k = 0; k < len / 2; ++k) {
                auto u = a[i + k], v = a[i + k + len / 2] * w; a[i + k] = u + v; a[i + k + len / 2] = u - v; w *= wl; } }
        }
    }
    double spectral_centroid(const Buffer& x, size_t from, int sr) {
        const size_t N = 8192; if (from + N > x.size()) return NAN;
        std::vector<std::complex<double>> a(N);
        for (size_t i = 0; i < N; ++i) a[i] = x[from + i] * (0.5 - 0.5 * std::cos(2.0 * M_PI * static_cast<double>(i) / static_cast<double>(N - 1)));
        fft(a); double num = 0, den = 0;
        for (size_t k = 1; k < N / 2; ++k) { const double m = std::abs(a[k]); num += m * (static_cast<double>(k) * sr / N); den += m; }
        return den > 1e-9 ? num / den : NAN;
    }
    // Pitch error of the strongest spectral peak within +-600 cents of the expected pitch (so inharmonic bars and bells, whose
    // overtones fool autocorrelation, are judged by their fundamental). NaN if the window holds no real peak.
    double pitch_error_cents(const Buffer& x, size_t from, size_t len, double expected_hz, int sr) {
        const size_t N = 32768; if (from + len > x.size() || len < 2048) return NAN;
        std::vector<std::complex<double>> a(N, 0.0);
        for (size_t i = 0; i < len; ++i) a[i] = x[from + i] * (0.5 - 0.5 * std::cos(2.0 * M_PI * static_cast<double>(i) / static_cast<double>(len - 1)));
        fft(a);
        const double bin = static_cast<double>(sr) / N;
        const size_t lo = static_cast<size_t>(expected_hz * std::pow(2.0, -0.5) / bin), hi = static_cast<size_t>(expected_hz * std::pow(2.0, 0.5) / bin) + 1;
        size_t best = lo; double bm = 0, total = 0;
        for (size_t k = lo; k <= hi; ++k) { const double m = std::abs(a[k]); total += m; if (m > bm) { bm = m; best = k; } }
        if (best <= lo || best >= hi || bm < 3.0 * total / static_cast<double>(hi - lo + 1)) return NAN;
        const double l = std::log(std::abs(a[best - 1]) + 1e-12), c = std::log(bm + 1e-12), r = std::log(std::abs(a[best + 1]) + 1e-12);
        const double den = l - 2 * c + r, frac = std::fabs(den) > 1e-12 ? 0.5 * (l - r) / den : 0.0;
        return 1200.0 * std::log2(((static_cast<double>(best) + frac) * bin) / expected_hz);
    }

    Metrics analyse(const StereoBuffer& whole, const StereoBuffer* c4, bool pitched, int sr) {
        Metrics m; double peak = 0, sum = 0, sumsq = 0; const size_t n = whole.L.size();
        for (size_t i = 0; i < n; ++i) for (float v : { whole.L[i], whole.R[i] }) {
            peak = std::max(peak, static_cast<double>(std::fabs(v))); sum += v; sumsq += static_cast<double>(v) * v; if (std::fabs(v) >= 0.999f) ++m.clipped; }
        m.peak_db = to_db(peak); m.rms_db = to_db(std::sqrt(sumsq / std::max<size_t>(1, 2 * n))); m.dc = sum / static_cast<double>(std::max<size_t>(1, 2 * n));
        if (c4 && !c4->L.empty()) {
            const Buffer x = mono(*c4); double p = 0, s2 = 0; for (float v : x) { p = std::max(p, static_cast<double>(std::fabs(v))); s2 += static_cast<double>(v) * v; }
            m.c4_peak_db = to_db(p); m.c4_rms_db = to_db(std::sqrt(s2 / static_cast<double>(x.size())));
            const size_t hop = static_cast<size_t>(0.002 * sr);                                  // 2 ms peak envelope
            std::vector<double> env; for (size_t i = 0; i < x.size(); i += hop) { double e = 0; for (size_t k = i; k < std::min(x.size(), i + hop); ++k) e = std::max(e, static_cast<double>(std::fabs(x[k]))); env.push_back(e); }
            // attack: 10% -> 90% of the loudest level in the first 300 ms (later swells from vibrato/beating must not count);
            // reported as 300 when the note is still rising at 300 ms
            const size_t win = std::min(env.size(), static_cast<size_t>(0.3 * sr) / hop + 1);
            double ref = 0; for (size_t i = 0; i < win; ++i) ref = std::max(ref, env[i]);
            double t10 = -1, t90 = -1; for (size_t i = 0; i < win; ++i) { if (t10 < 0 && env[i] >= 0.1 * ref) t10 = static_cast<double>(i); if (t90 < 0 && env[i] >= 0.9 * ref) { t90 = static_cast<double>(i); break; } }
            if (t10 >= 0 && t90 >= 0) m.attack_ms = std::min(300.0, (t90 - t10) * hop * 1000.0 / sr);
            const size_t tail = std::min(x.size(), static_cast<size_t>(0.001 * sr)); double te = 0; for (size_t i = x.size() - tail; i < x.size(); ++i) te += static_cast<double>(x[i]) * x[i];
            m.end_db = to_db(std::sqrt(te / static_cast<double>(std::max<size_t>(1, tail)))) - m.c4_peak_db;   // level of the last 1 ms vs the note's peak
            m.centroid = spectral_centroid(x, static_cast<size_t>(0.25 * sr), sr);
            if (pitched) m.f0_err = pitch_error_cents(x, static_cast<size_t>(0.2 * sr), static_cast<size_t>(0.6 * sr), wauvio::theory::midi_to_freq(60), sr);
        }
        if (m.clipped > 0) m.flags += "CLIP ";
        if (std::fabs(m.dc) > 0.005) m.flags += "DC ";
        if (m.peak_db < -60.0) m.flags += "SILENT ";
        if (pitched && !std::isnan(m.f0_err) && std::fabs(m.f0_err) > 25.0) m.flags += "PITCH ";
        if (!std::isnan(m.end_db) && m.end_db > -30.0) m.flags += "CUTOFF ";
        if (!m.flags.empty()) m.flags.pop_back();
        return m;
    }

    struct Sink {
        const Options& o; int index = 0; std::ofstream csv, seg;
        explicit Sink(const Options& opt) : o(opt) {}

        void write(const StereoBuffer& buf_in, const fs::path& path, bool normalize) {
            StereoBuffer buf = buf_in;
            if (normalize) { float p = 0; for (size_t i = 0; i < buf.L.size(); ++i) p = std::max({ p, std::fabs(buf.L[i]), std::fabs(buf.R[i]) });
                if (p > 1e-6f) { const float g = 0.891f / p; for (auto& v : buf.L) v *= g; for (auto& v : buf.R) v *= g; } }
            wauvio::save_wav_stereo(buf, path.string());
        }

        void instrument(const std::string& family, const std::string& cls, const std::string& base, std::shared_ptr<music::Instrument> inst) {
            const std::string hay = lower(family + " " + cls + " " + inst->name);
            if (!o.only.empty() && hay.find(lower(o.only)) == std::string::npos) return;
            ++index;
            char idx[16]; std::snprintf(idx, sizeof idx, "%03d", index);
            const std::string stem = std::string(idx) + "_" + file_safe(inst->name.empty() ? cls : inst->name);
            if (o.list) { std::printf("%s  %-14s %-28s (%s)\n", idx, family.c_str(), inst->name.c_str(), cls.c_str()); return; }
            const int sr = global_config().sample_rate;
            const bool pitched = base != "PercussionInstrument";
            StereoBuffer track; std::vector<Segment> segs; size_t cursor = 0; const StereoBuffer* c4 = nullptr; StereoBuffer c4_copy;
            auto put = [&](const std::string& label, const StereoBuffer& b) {
                segs.push_back({ label, static_cast<double>(cursor) / sr, static_cast<double>(b.L.size()) / sr });
                add_at(track, b, cursor); cursor += b.L.size() + static_cast<size_t>(GAP * sr);
            };
            using music::Dynamics; using music::Articulation;
            auto note = [&](int midi, double dur, Dynamics d, Articulation a = Articulation::Sustain) { return inst->play(midi, dur, d, a).audio; };
            if (pitched) {
                put("C3 mf", note(48, 1.5, Dynamics::mf));
                c4_copy = note(60, 1.5, Dynamics::mf); put("C4 mf", c4_copy); c4 = &c4_copy;
                put("C5 mf", note(72, 1.5, Dynamics::mf));
                put("C4 pp", note(60, 1.2, Dynamics::pp));
                put("C4 ff", note(60, 1.2, Dynamics::ff));
                StereoBuffer scale; const int steps[] = { 0, 2, 4, 5, 7, 9, 11, 12 }; size_t off = 0, hop = static_cast<size_t>(0.28 * sr);
                for (int s : steps) { add_at(scale, note(60 + s, 0.34, Dynamics::mf), off); off += hop; }
                put("scale", scale);
            } else {
                c4_copy = note(60, 1.0, Dynamics::mf); c4 = &c4_copy;
                put("hit pp", note(60, 1.0, Dynamics::pp)); put("hit mf", c4_copy); put("hit ff", note(60, 1.0, Dynamics::ff));
            }
            int arts = 0;
            for (Articulation a : inst->supported_articulations()) {
                if (a == Articulation::Sustain || arts >= 8) continue;
                put(std::string("art ") + music::to_string(a), note(60, 0.8, Dynamics::mf, a)); ++arts;
            }
            const Metrics m = analyse(track, c4, pitched, sr);
            const fs::path dir = fs::path(o.out) / family; fs::create_directories(dir);
            const fs::path file = dir / (stem + ".wav"); write(track, file, o.normalize);
            if (o.split) {
                const fs::path sd = dir / stem; fs::create_directories(sd);
                for (const Segment& s : segs) { StereoBuffer part; const size_t a = static_cast<size_t>(s.start * sr), n = static_cast<size_t>(s.dur * sr);
                    for (size_t i = 0; i < n && a + i < track.L.size(); ++i) { part.L.push_back(track.L[a + i]); part.R.push_back(track.R[a + i]); }
                    write(part, sd / (file_safe(s.label) + ".wav"), o.normalize); }
            }
            auto num = [](double v, int p) { char b[32]; if (std::isnan(v)) return std::string(""); std::snprintf(b, sizeof b, "%.*f", p, v); return std::string(b); };
            csv << index << ',' << family << ",\"" << inst->name << "\"," << cls << ',' << (fs::path(family) / (stem + ".wav")).generic_string() << ','
                << num(static_cast<double>(track.L.size()) / sr, 2) << ',' << num(m.peak_db, 1) << ',' << num(m.rms_db, 1) << ',' << m.clipped << ',' << num(m.dc, 4) << ','
                << num(m.c4_peak_db, 1) << ',' << num(m.c4_rms_db, 1) << ',' << num(m.f0_err, 0) << ',' << num(m.attack_ms, 0) << ',' << num(m.centroid, 0) << ','
                << num(m.end_db, 0) << ',' << m.flags << '\n';
            for (const Segment& s : segs) seg << (fs::path(family) / (stem + ".wav")).generic_string() << ',' << s.label << ',' << num(s.start, 2) << ',' << num(s.dur, 2) << '\n';
            std::printf("%s  %-14s %-28s %s\n", idx, family.c_str(), inst->name.c_str(), m.flags.c_str());
        }

        void kit(const std::string& cls, std::shared_ptr<music::DrumKit> k) {
            if (!o.kits) return;
            if (!o.only.empty() && lower(cls + " " + k->name + " kits").find(lower(o.only)) == std::string::npos) return;
            if (o.list) { std::printf("kit  %-28s (%s)\n", k->name.c_str(), cls.c_str()); return; }
            const int sr = global_config().sample_rate; StereoBuffer track; size_t cursor = 0;
            std::vector<int> notes = k->mapped_notes(); std::sort(notes.begin(), notes.end());
            for (int n : notes) { StereoBuffer b = k->play(n, music::Dynamics::mf, 0.8).audio; add_at(track, b, cursor); cursor += b.L.size() + static_cast<size_t>(GAP * sr); }
            const fs::path dir = fs::path(o.out) / "_kits"; fs::create_directories(dir);
            write(track, dir / (file_safe(k->name) + ".wav"), o.normalize);
            std::printf("kit  %-28s %zu pieces, in note order\n", k->name.c_str(), notes.size());
        }
    };

    template <class T> void visit(Sink& s, const char* family, const char* cls, const char* base) {
        if constexpr (std::is_base_of<music::Instrument, T>::value && std::is_default_constructible<T>::value && !std::is_abstract<T>::value)
            s.instrument(family, cls, base, std::make_shared<T>());
        else if constexpr (std::is_base_of<music::DrumKit, T>::value && std::is_default_constructible<T>::value && !std::is_abstract<T>::value)
            s.kit(cls, std::make_shared<T>());
    }
}

int main(int argc, char** argv) {
    Options o;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--out" && i + 1 < argc) o.out = argv[++i]; else if (a == "--only" && i + 1 < argc) o.only = argv[++i];
        else if (a == "--normalize") o.normalize = true; else if (a == "--split") o.split = true; else if (a == "--list") o.list = true; else if (a == "--no-kits") o.kits = false;
        else { std::fprintf(stderr, "unknown option %s (see the comment at the top of render_instruments.cpp)\n", a.c_str()); return 2; }
    }
    Sink s(o);
    if (!o.list) {
        fs::create_directories(o.out);
        s.csv.open(fs::path(o.out) / "index.csv");  s.seg.open(fs::path(o.out) / "segments.csv");
        s.csv << "index,family,name,class,file,seconds,peak_dBFS,rms_dBFS,clipped,dc,c4_peak_dBFS,c4_rms_dBFS,c4_f0_err_cents,c4_attack_ms,c4_centroid_hz,c4_end_dB,flags\n";
        s.seg << "file,label,start_s,dur_s\n";
    }
#define WAUVIO_VISIT(family, cls, base) visit<instruments::cls>(s, #family, #cls, #base);
    WAUVIO_CLASS_CATALOG(WAUVIO_VISIT)
#undef WAUVIO_VISIT
    if (!o.list) std::printf("\n%d instruments -> %s (index.csv has the measurements)\n", s.index, o.out.c_str());
    return 0;
}
