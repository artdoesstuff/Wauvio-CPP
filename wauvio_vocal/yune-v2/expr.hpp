#pragma once

#include "voice.hpp"

#include <array>
#include <string>

namespace wauvio {
    namespace vocal {

        // Render-level expression channels. They are additive around a neutral zero, so an
        // all-zero track is a no-op (the V1 render path is taken untouched).
        enum class Ch : int {
            Volume, Brightness, Breath, Tension, FormantShift, MouthOpen,
            Growl, Tremor, Fry, Air, Register, RegAuto, Natural, N
        };
        constexpr int N_CH = static_cast<int>(Ch::N);

        // An automation lane: a Curve on one channel. Note lanes use seconds from the note
        // start; Singer-wide lanes use absolute seconds.
        struct Lane { Ch ch = Ch::Volume; Curve curve; };

        inline bool channel_from_name(const std::string& n, Ch& out) {
            struct E { const char* name; Ch ch; };
            static const E tab[] = {
                {"volume", Ch::Volume}, {"dynamics", Ch::Volume}, {"brightness", Ch::Brightness},
                {"breathiness", Ch::Breath}, {"breath", Ch::Breath}, {"tension", Ch::Tension},
                {"formant", Ch::FormantShift}, {"gender", Ch::FormantShift}, {"formant_shift", Ch::FormantShift},
                {"mouth", Ch::MouthOpen}, {"mouth_open", Ch::MouthOpen}, {"growl", Ch::Growl},
                {"tremor", Ch::Tremor}, {"fry", Ch::Fry}, {"air", Ch::Air},
                {"register", Ch::Register}, {"reg", Ch::Register}, {"reg_auto", Ch::RegAuto}, {"natural", Ch::Natural}, {"human", Ch::Natural} };
            for (const E& e : tab) if (n == e.name) { out = e.ch; return true; }
            return false;
        }

        struct Expr {
            std::array<float, N_CH> v{};
            float tremor_rate = 9.f;
            float operator[](Ch c) const { return v[static_cast<size_t>(c)]; }
        };

        class ExprTrack {
        public:
            std::vector<Expr> data;     // one sample per ms (PITCH_RATE)
            bool any = false;

            Expr at(double t) const {
                if (data.empty()) return Expr{};
                double x = t * 1000.0;
                if (x <= 0) return data.front();
                size_t i = static_cast<size_t>(x);
                if (i + 1 >= data.size()) return data.back();
                const float f = static_cast<float>(x - static_cast<double>(i));
                Expr o;
                for (int c = 0; c < N_CH; ++c)
                    o.v[static_cast<size_t>(c)] = data[i].v[static_cast<size_t>(c)] +
                        (data[i + 1].v[static_cast<size_t>(c)] - data[i].v[static_cast<size_t>(c)]) * f;
                o.tremor_rate = data[i].tremor_rate + (data[i + 1].tremor_rate - data[i].tremor_rate) * f;
                return o;
            }
        };

        struct ExprNote {
            double start = 0.0, end = 0.0;
            Style style;
            std::vector<Lane> lanes;
        };

        inline double expr_smooth(double x) { x = x < 0 ? 0 : (x > 1 ? 1 : x); return x * x * (3 - 2 * x); }

        // Builds the expression track from per-note Style fields, per-note lanes and global lanes.
        inline ExprTrack build_expr_track(const std::vector<ExprNote>& notes, const std::vector<Lane>& global,
                                          double total_seconds)
        {
            ExprTrack tr;
            // anything to do?
            bool need = !global.empty();
            for (const ExprNote& n : notes) {
                const Style& s = n.style;
                if (!n.lanes.empty() || s.formant_shift != 0.f || s.mouth_open != 0.f || s.growl != 0.f || s.tremor != 0.f ||
                    s.fry != 0.f || s.air != 0.f || s.reg != 0.f || s.reg_auto != 0.f || s.glottal_onset != 0.f || s.natural != 0.f) { need = true; break; }
            }
            if (!need) return tr;
            const size_t n = static_cast<size_t>(total_seconds * 1000.0) + 2;
            tr.data.assign(n, Expr{});
            std::vector<double> rate_num(n, 0.0), rate_den(n, 0.0);

            auto add = [&](size_t k, Ch c, float v) { tr.data[k].v[static_cast<size_t>(c)] += v; };

            for (size_t i = 0; i < notes.size(); ++i) {
                const ExprNote& nt = notes[i];
                const Style& s = nt.style;
                auto connected = [&](size_t a, size_t b) { return std::fabs(notes[b].start - notes[a].end) < 0.03; };
                double r0, r1, f0, f1;
                if (i > 0 && connected(i - 1, i)) { const double b = 0.5 * (nt.start + notes[i - 1].end); r0 = b - 0.012; r1 = b + 0.012; }
                else { r0 = nt.start - 0.02; r1 = nt.start; }
                if (i + 1 < notes.size() && connected(i, i + 1)) { const double b = 0.5 * (nt.end + notes[i + 1].start); f0 = b - 0.012; f1 = b + 0.012; }
                else { f0 = nt.end; f1 = nt.end + 0.04; }
                const size_t k0 = static_cast<size_t>(std::max(0.0, r0) * 1000.0);
                const size_t k1 = std::min(n - 1, static_cast<size_t>(std::max(0.0, f1) * 1000.0) + 1);
                for (size_t k = k0; k <= k1; ++k) {
                    const double t = static_cast<double>(k) / 1000.0;
                    const double rise = r1 > r0 ? expr_smooth((t - r0) / (r1 - r0)) : (t >= r1 ? 1.0 : 0.0);
                    const double fall = f1 > f0 ? 1.0 - expr_smooth((t - f0) / (f1 - f0)) : (t <= f0 ? 1.0 : 0.0);
                    const float w = static_cast<float>(rise * fall);
                    if (w <= 0.f) continue;
                    add(k, Ch::FormantShift, w * s.formant_shift);
                    add(k, Ch::MouthOpen, w * s.mouth_open);
                    add(k, Ch::Growl, w * s.growl);
                    add(k, Ch::Tremor, w * s.tremor);
                    add(k, Ch::Fry, w * s.fry);
                    add(k, Ch::Air, w * s.air);
                    add(k, Ch::Register, w * s.reg);
                    add(k, Ch::RegAuto, w * s.reg_auto);
                    add(k, Ch::Natural, w * s.natural);
                    if (s.tremor > 0.f) { rate_num[k] += static_cast<double>(w * s.tremor * s.tremor_rate); rate_den[k] += static_cast<double>(w * s.tremor); }
                    if (s.glottal_onset > 0.f) {            // brief creak at the start of the note
                        const double u = (t - nt.start) / 0.045;
                        if (u >= 0.0 && u < 3.0) add(k, Ch::Fry, w * s.glottal_onset * 0.8f * static_cast<float>(std::exp(-u * 1.6)));
                    }
                    for (const Lane& ln : nt.lanes) add(k, ln.ch, w * static_cast<float>(curve_eval(ln.curve, t - nt.start, 0.0)));
                }
            }
            for (const Lane& ln : global)
                for (size_t k = 0; k < n; ++k) add(k, ln.ch, static_cast<float>(curve_eval(ln.curve, static_cast<double>(k) / 1000.0, 0.0)));
            for (size_t k = 0; k < n; ++k) {
                tr.data[k].tremor_rate = rate_den[k] > 1e-6 ? static_cast<float>(rate_num[k] / rate_den[k]) : 9.f;
                for (int c = 0; c < N_CH; ++c) if (tr.data[k].v[static_cast<size_t>(c)] != 0.f) { tr.any = true; break; }
            }
            return tr;
        }

    }
}
