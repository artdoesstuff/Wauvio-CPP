// probe_strings: measures the targets the bowed-string model is tuned against, per instrument, and writes strings_probe.csv.
//
//   g++ -std=c++17 -O2 -I<repo root> wauvio/ext/tools/probe_strings.cpp -o probe_strings
//   ./probe_strings [--out FILE] [--only TEXT]
//
// Only instruments whose recipe.bowed.kind != None are measured. Spectral measurements are taken on a
// copy of the instrument with vibrato and random jitter switched off, so harmonics are sharp lines and the inter-harmonic floor is
// breath noise rather than vibrato sidebands.
//   att_pp/mf/ff/stac_ms  10%->90% rise of the 2 ms RMS envelope inside the first 300 ms (C4)
//   H1..H8_db             harmonic levels at C4 mf relative to the strongest harmonic
//   even_odd_db           mean of H2,H4,H6 minus mean of H1,H3,H5 (negative = odd-weighted)
//   cent_pp/mf/ff_hz      spectral centroid, C4 steady state (0.4-1.2 s)
//   cent_c3/c5_hz         same at C3 and C5 (mf), for register-dependent brightness
//   cent_early_hz         centroid of 5-45 ms of C4 mf; compare with cent_mf_hz (high partials should arrive later for brass/reeds)
//   floor_db              energy between harmonics relative to total, C4 mf (breath noise level, approximate)
//   rms_dbfs              C4 mf RMS of the full instrument (vibrato on)
//   hcent_*_hz            centroid of the harmonic peaks only (breath noise excluded); pp/mf/ff at C4, C3 and C5 at mf
//   loud_pp_db/loud_ff_db RMS of C4 pp / ff relative to C4 mf (vibrato off)
//   pizz_t20_ms           pizzicato (C4 mf) time for the 5 ms energy envelope to fall 20 dB below its early peak (energy, so 20 dB = 1%)
//   pizz_h1/h5_t20_ms     time for harmonic 1 / harmonic 5 to fall 20 dB (high partials should die first)
//   pizz_hcent_*          harmonic centroid at 10-110 ms and 500-600 ms of the pizzicato note
// Sections are measured as a single voice (ensemble off) so spectra are clean; use the audit/WAVs for ensemble behaviour.
#include "../../ext.hpp"
#include "instrument_catalog.hpp"
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
using namespace wauvio;

namespace {
    std::string lower(std::string s) { for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return s; }
    void fft(std::vector<std::complex<double>>& a) {
        const size_t n = a.size();
        for (size_t i = 1, j = 0; i < n; ++i) { size_t bit = n >> 1; for (; j & bit; bit >>= 1) j ^= bit; j ^= bit; if (i < j) std::swap(a[i], a[j]); }
        for (size_t len = 2; len <= n; len <<= 1) {
            const double ang = -2.0 * M_PI / static_cast<double>(len); const std::complex<double> wl(std::cos(ang), std::sin(ang));
            for (size_t i = 0; i < n; i += len) { std::complex<double> w(1); for (size_t k = 0; k < len / 2; ++k) {
                auto u = a[i + k], v = a[i + k + len / 2] * w; a[i + k] = u + v; a[i + k + len / 2] = u - v; w *= wl; } }
        }
    }
    std::vector<double> power_spectrum(const std::vector<float>& x, size_t from, size_t len, size_t N) {
        std::vector<std::complex<double>> a(N, 0.0);
        for (size_t i = 0; i < len && from + i < x.size(); ++i) a[i] = x[from + i] * (0.5 - 0.5 * std::cos(2.0 * M_PI * static_cast<double>(i) / static_cast<double>(len - 1)));
        fft(a); std::vector<double> p(N / 2); for (size_t k = 0; k < N / 2; ++k) p[k] = std::norm(a[k]); return p;
    }
    double centroid(const std::vector<float>& x, size_t from, size_t len, int sr) {
        size_t N = 16384; while (N < len) N <<= 1; auto p = power_spectrum(x, from, len, N); double num = 0, den = 0;
        for (size_t k = 1; k < p.size(); ++k) { const double m = std::sqrt(p[k]); num += m * (static_cast<double>(k) * sr / N); den += m; }
        return den > 1e-12 ? num / den : NAN;
    }
    std::vector<float> mono(const StereoBuffer& b) { std::vector<float> m(b.L.size()); for (size_t i = 0; i < m.size(); ++i) m[i] = 0.5f * (b.L[i] + b.R[i]); return m; }
    double attack_ms(const std::vector<float>& x, int sr) {
        const size_t hop = static_cast<size_t>(0.001 * sr), win = static_cast<size_t>(0.002 * sr), lim = std::min(x.size(), static_cast<size_t>(0.3 * sr));
        std::vector<double> env; for (size_t i = 0; i + win < lim; i += hop) { double s = 0; for (size_t k = 0; k < win; ++k) s += static_cast<double>(x[i + k]) * x[i + k]; env.push_back(std::sqrt(s / win)); }
        double ref = 0; for (double e : env) ref = std::max(ref, e);
        double t10 = -1, t90 = -1; for (size_t i = 0; i < env.size(); ++i) { if (t10 < 0 && env[i] >= 0.1 * ref) t10 = static_cast<double>(i); if (t90 < 0 && env[i] >= 0.9 * ref) { t90 = static_cast<double>(i); break; } }
        return (t10 >= 0 && t90 >= 0) ? (t90 - t10) * hop * 1000.0 / sr : NAN;
    }
    // centroid of the harmonic peaks only (amplitude-weighted), so breath noise does not inflate it
    double harm_centroid(const std::vector<float>& x, size_t from, size_t len, double f0, int sr) {
        const size_t N = 65536; auto P = power_spectrum(x, from, len, N); const double bin = static_cast<double>(sr) / N; double num = 0, den = 0;
        for (int h = 1; h <= 60; ++h) { const double fc = h * f0; if (fc > 0.45 * sr) break;
            const size_t lo = static_cast<size_t>(std::max(1.0, (fc - 0.12 * f0) / bin)), hi = static_cast<size_t>((fc + 0.12 * f0) / bin) + 1; double pk = 0;
            for (size_t k = lo; k <= hi && k < P.size(); ++k) pk = std::max(pk, P[k]);
            num += fc * std::sqrt(pk); den += std::sqrt(pk); }
        return den > 0 ? num / den : NAN;
    }
    double rms_db_of(const std::vector<float>& x) { double s = 0; for (float v : x) s += static_cast<double>(v) * v; return 10.0 * std::log10(std::max(1e-20, s / std::max<size_t>(1, x.size()))); }
    std::string num(double v, int p = 0) { char b[32]; if (std::isnan(v)) return ""; std::snprintf(b, sizeof b, "%.*f", p, v); return b; }
    double db(double x) { return 10.0 * std::log10(std::max(1e-30, x)); }

    struct Out { std::ofstream f; std::string only; int count = 0; };

    template <class T> void visit(Out& o, const char* family, const char* cls, const char*) {
        if constexpr (std::is_base_of<music::ModeledInstrument, T>::value && std::is_default_constructible<T>::value && !std::is_abstract<T>::value) {
            auto probe = std::make_shared<T>();
            if (probe->recipe.bowed.kind == music::TimbreRecipe::Bowed::Kind::None) return;
            const std::string hay = lower(std::string(family) + " " + cls + " " + probe->name);
            if (!o.only.empty() && hay.find(lower(o.only)) == std::string::npos) return;
            const int sr = global_config().sample_rate;
            using music::Dynamics; using music::Articulation;
            auto real = std::make_shared<T>();
            auto render = [&](T& inst, int midi, double dur, Dynamics d, Articulation a = Articulation::Sustain) { return mono(inst.play(midi, dur, d, a).audio); };
            const auto c4mf = render(*real, 60, 1.5, Dynamics::mf);
            double sumsq = 0; for (float v : c4mf) sumsq += static_cast<double>(v) * v;
            const double rms_db = 10.0 * std::log10(std::max(1e-20, sumsq / c4mf.size()));
            const double a_pp = attack_ms(render(*real, 60, 1.2, Dynamics::pp), sr), a_mf = attack_ms(c4mf, sr), a_ff = attack_ms(render(*real, 60, 1.2, Dynamics::ff), sr);
            double a_st = NAN;
            if (real->supports(Articulation::Staccato)) a_st = attack_ms(render(*real, 60, 0.8, Dynamics::mf, Articulation::Staccato), sr);

            auto clean = std::make_shared<T>();
            auto& w = clean->recipe.bowed; w.vib_depth_cents = 0; w.jitter_cents = 0; w.amp_jitter = 0; w.vib_am = 0; w.scoop_cents = 0; w.ensemble_voices = 1; w.ensemble_drift_cents = 0;
            const size_t s0 = static_cast<size_t>(0.4 * sr), L = static_cast<size_t>(0.8 * sr);
            const auto cl_mf = render(*clean, 60, 1.5, Dynamics::mf), cl_pp = render(*clean, 60, 1.5, Dynamics::pp), cl_ff = render(*clean, 60, 1.5, Dynamics::ff);
            const auto cl_c3 = render(*clean, 48, 1.5, Dynamics::mf), cl_c5 = render(*clean, 72, 1.5, Dynamics::mf);
            const double f0 = theory::midi_to_freq(60);
            const size_t N = 65536; auto P = power_spectrum(cl_mf, s0, L, N); const double bin = static_cast<double>(sr) / N;
            double H[9] = {}, hmax = 1e-30, total = 0, harm = 0;
            for (size_t k = 1; k < P.size(); ++k) total += P[k];
            std::vector<char> near(P.size(), 0);
            for (int h = 1; h <= 40; ++h) { const double fc = h * f0; if (fc > 0.45 * sr) break;
                const size_t lo = static_cast<size_t>(std::max(1.0, (fc - 0.12 * f0) / bin)), hi = static_cast<size_t>((fc + 0.12 * f0) / bin) + 1; double pk = 0;
                for (size_t k = lo; k <= hi && k < P.size(); ++k) { pk = std::max(pk, P[k]); near[k] = 1; harm += P[k]; }
                if (h <= 8) { H[h] = pk; hmax = std::max(hmax, pk); } }
            double floor_e = 0; for (size_t k = 1; k < P.size(); ++k) if (!near[k]) floor_e += P[k];
            const double eo = db(((H[2] + H[4] + H[6]) / 3.0) / ((H[1] + H[3] + H[5]) / 3.0));
            const size_t e0 = static_cast<size_t>(0.005 * sr), eL = static_cast<size_t>(0.040 * sr);
            double pz_t20 = NAN, pz_cent_early = NAN, pz_cent_late = NAN, pz_h1_t20 = NAN, pz_h5_t20 = NAN;
            if (real->supports(Articulation::Pizzicato)) {
                auto pz = render(*clean, 60, 2.0, Dynamics::mf, Articulation::Pizzicato);
                const size_t hop = static_cast<size_t>(0.005 * sr); double e0 = 0; std::vector<double> env;
                for (size_t i = 0; i + hop < pz.size(); i += hop) { double q = 0; for (size_t k = 0; k < hop; ++k) q += static_cast<double>(pz[i + k]) * pz[i + k]; env.push_back(q); }
                for (size_t i = 0; i < std::min<size_t>(env.size(), 8); ++i) e0 = std::max(e0, env[i]);
                for (size_t i = 0; i < env.size(); ++i) if (env[i] < e0 * 0.01) { pz_t20 = i * 5.0; break; }
                pz_cent_early = harm_centroid(pz, static_cast<size_t>(0.01 * sr), static_cast<size_t>(0.1 * sr), f0, sr);
                pz_cent_late = harm_centroid(pz, static_cast<size_t>(0.5 * sr), static_cast<size_t>(0.1 * sr), f0, sr);
                auto hdec = [&](int h) { // time for harmonic h to fall 20 dB from its level at 20-60 ms
                    const double fc = h * f0; double ref = 0, tt = NAN;
                    for (double t0 = 0.02; t0 < 1.9; t0 += 0.02) {
                        const size_t a0 = static_cast<size_t>(t0 * sr), L2 = static_cast<size_t>(0.04 * sr);
                        auto P2 = power_spectrum(pz, a0, L2, 16384); const double bn = static_cast<double>(sr) / 16384; double pk = 0;
                        for (size_t k = static_cast<size_t>((fc - 0.1 * f0) / bn); k <= static_cast<size_t>((fc + 0.1 * f0) / bn); ++k) pk = std::max(pk, P2[k]);
                        if (t0 < 0.03) ref = pk; else if (pk < ref * 0.01) { tt = (t0 - 0.02) * 1000.0; break; }
                    }
                    return tt; };
                pz_h1_t20 = hdec(1); pz_h5_t20 = hdec(5);
            }
            o.f << o.count + 1 << ',' << family << ",\"" << probe->name << "\"," << cls << ',' << num(a_pp) << ',' << num(a_mf) << ',' << num(a_ff) << ',' << num(a_st);
            for (int h = 1; h <= 8; ++h) o.f << ',' << num(db(H[h] / hmax), 1);
            o.f << ',' << num(eo, 1) << ',' << num(centroid(cl_pp, s0, L, sr)) << ',' << num(centroid(cl_mf, s0, L, sr)) << ',' << num(centroid(cl_ff, s0, L, sr))
                << ',' << num(centroid(cl_c3, s0, L, sr)) << ',' << num(centroid(cl_c5, s0, L, sr)) << ',' << num(centroid(cl_mf, e0, eL, sr))
                << ',' << num(db(floor_e / std::max(1e-30, total)), 1) << ',' << num(rms_db, 1)
                << ',' << num(harm_centroid(cl_pp, s0, L, f0, sr)) << ',' << num(harm_centroid(cl_mf, s0, L, f0, sr)) << ',' << num(harm_centroid(cl_ff, s0, L, f0, sr))
                << ',' << num(harm_centroid(cl_c3, s0, L, theory::midi_to_freq(48), sr)) << ',' << num(harm_centroid(cl_c5, s0, L, theory::midi_to_freq(72), sr))
                << ',' << num(rms_db_of(cl_pp) - rms_db_of(cl_mf), 1) << ',' << num(rms_db_of(cl_ff) - rms_db_of(cl_mf), 1)
                << ',' << num(pz_t20) << ',' << num(pz_h1_t20) << ',' << num(pz_h5_t20) << ',' << num(pz_cent_early) << ',' << num(pz_cent_late) << '\n';
            ++o.count;
        }
    }
}

int main(int argc, char** argv) {
    Out o; std::string path = "strings_probe.csv";
    for (int i = 1; i < argc; ++i) { const std::string a = argv[i]; if (a == "--out" && i + 1 < argc) path = argv[++i]; else if (a == "--only" && i + 1 < argc) o.only = argv[++i]; }
    o.f.open(path);
    o.f << "index,family,name,class,att_pp_ms,att_mf_ms,att_ff_ms,att_stac_ms,H1_db,H2_db,H3_db,H4_db,H5_db,H6_db,H7_db,H8_db,even_odd_db,"
           "cent_pp_hz,cent_mf_hz,cent_ff_hz,cent_c3_hz,cent_c5_hz,cent_early_hz,floor_db,rms_dbfs,hcent_pp_hz,hcent_mf_hz,hcent_ff_hz,hcent_c3_hz,hcent_c5_hz,loud_pp_db,loud_ff_db,pizz_t20_ms,pizz_h1_t20_ms,pizz_h5_t20_ms,pizz_hcent_early_hz,pizz_hcent_late_hz\n";
#define WAUVIO_VISIT(family, cls, base) visit<instruments::cls>(o, #family, #cls, #base);
    WAUVIO_CLASS_CATALOG(WAUVIO_VISIT)
#undef WAUVIO_VISIT
    std::printf("%d instruments measured -> %s\n", o.count, path.c_str());
    return 0;
}
