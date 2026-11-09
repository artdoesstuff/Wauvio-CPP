#pragma once
#include "sfx_recipe.hpp"

namespace wauvio {
    namespace sfx {
        inline Traj random_stairs(Rng& r, int n, double lo, double hi, bool snap = true) {
            n = std::max(n, 2); auto pick = [&] { const double v = r.range(lo, hi); return snap ? std::round(v) : v; };
            Traj t(pick()); for (int i = 1; i < n; ++i) t.to(static_cast<double>(i) / n, pick(), Interp::Step); return t;
        }
        inline Traj env_bell(double center = 0.5, double width = 0.22, int keys = 24, double floor = 0.0008) {
            Traj t(std::max(floor, std::exp(-0.5 * std::pow((0.0 - center) / width, 2.0))));
            for (int i = 1; i <= keys; ++i) { const double u = static_cast<double>(i) / keys; t.to(u, std::max(floor, std::exp(-0.5 * std::pow((u - center) / width, 2.0))), Interp::Linear); }
            return t;
        }
        inline Traj env_swell(double peak_at = 0.85, double start = 0.02, double end = 0.0008) { return Traj(start).to(peak_at, 1.0, Interp::Exp).to(1.0, end, Interp::Exp); }
        inline Traj env_hit(double a = 0.004, double floor = 0.0008) { return Traj::ar(a, 1.0, floor); }

        inline Mod lfo(Mod::Shape s, double rate_hz, double depth, double phase = 0.0) { Mod m; m.shape = s; m.rate_hz = rate_hz; m.depth = depth; m.phase = phase; return m; }
        inline Mod lfo_beats(Mod::Shape s, double beats_per_cycle, double depth, double phase = 0.0) { Mod m; m.shape = s; m.beats = beats_per_cycle; m.depth = depth; m.phase = phase; return m; }

        inline std::vector<double> hashed_pattern(uint64_t seed, int n, double density) {
            std::vector<double> v; for (int i = 0; i < n; ++i) v.push_back(i == 0 || hash01(mix_seed(seed, static_cast<uint64_t>(i))) < density ? 1.0 : 0.0); return v;
        }
    }
}
