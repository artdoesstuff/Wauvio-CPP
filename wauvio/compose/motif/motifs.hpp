#pragma once

#include "../core/compose_common.hpp"

namespace wauvio {
    namespace compose {

        enum class StepBasis { ScaleStep, ChordTone };

        struct MotifNote {
            int step                  = 0;
            int chromatic             = 0;
            double duration_beats     = 0.5;
            bool is_rest              = false;
            StepBasis basis           = StepBasis::ScaleStep;
            wauvio::music::Dynamics dynamics         = wauvio::music::Dynamics::mf;
            wauvio::music::Articulation articulation = wauvio::music::Articulation::Sustain;
        };

        struct Motif {
            std::string name;
            std::string description;
            std::vector<std::string> tags;
            std::vector<MotifNote> notes;

            double total_beats() const noexcept {
                double t = 0.0;
                for (const auto& n : notes) t += n.duration_beats;
                return t;
            }
        };

        inline Motif transpose(Motif m, int steps) {
            for (auto& n : m.notes) if (!n.is_rest) n.step += steps;
            return m;
        }

        inline Motif invert(Motif m, int pivot_step = 0) {
            for (auto& n : m.notes) if (!n.is_rest) n.step = 2 * pivot_step - n.step;
            return m;
        }

        inline Motif retrograde(Motif m) {
            std::reverse(m.notes.begin(), m.notes.end());
            return m;
        }

        inline Motif scale_rhythm(Motif m, double factor) {
            if (factor <= 0.0) return m;
            for (auto& n : m.notes) n.duration_beats = std::max(0.0625, n.duration_beats * factor);
            return m;
        }

        inline Motif sequence(const Motif& m, int repetitions, int step_per_repeat) {
            Motif out = m;
            out.notes.clear();
            for (int r = 0; r < repetitions; ++r) {
                for (auto n : m.notes) {
                    if (!n.is_rest) n.step += r * step_per_repeat;
                    out.notes.push_back(n);
                }
            }
            return out;
        }

        inline Motif vary_motif(Motif m, Rng& rng, double amount = 0.35) {
            if (m.notes.empty() || amount <= 0.0) return m;
            std::vector<MotifNote> next;
            for (size_t i = 0; i < m.notes.size(); ++i) {
                MotifNote cur = m.notes[i];
                if (!cur.is_rest && cur.duration_beats >= 1.0 && rng.chance(0.45 * amount)) {
                    MotifNote a = cur, b = cur;
                    a.duration_beats = cur.duration_beats * 0.5;
                    b.duration_beats = cur.duration_beats - a.duration_beats;
                    b.step += rng.chance(0.5) ? 1 : -1;
                    b.basis = StepBasis::ScaleStep;
                    next.push_back(a);
                    next.push_back(b);
                    continue;
                }
                if (!cur.is_rest && i > 0 && rng.chance(0.35 * amount)) {
                    cur.step += rng.chance(0.5) ? 1 : -1;
                }
                next.push_back(cur);
            }
            m.notes = std::move(next);
            return m;
        }

        enum class Contour { Arch, Ascending, Descending, Wave, Pendulum, Hook };

        struct MotifOptions {
            double   length_beats = 4.0;
            Contour  contour      = Contour::Arch;
            double   density      = 0.55;
            double   syncopation  = 0.25;
            double   rest_prob    = 0.12;
            int      range_steps  = 5;
            uint64_t seed         = 1;
        };

        inline Motif generate_motif(const MotifOptions& opts = {}) {
            Rng rng(opts.seed);
            Motif m;
            m.name = "Generated Motif";

            std::vector<double> durs;
            double rem = std::max(0.5, opts.length_beats);
            while (rem > 1e-6) {
                std::vector<double> cand;
                std::vector<double> wt;
                auto push = [&](double d, double w) { if (d <= rem + 1e-6) { cand.push_back(d); wt.push_back(w); } };
                push(0.25, 0.6 * opts.density * opts.density);
                push(0.50, 1.8 * opts.density + 0.4);
                push(0.75, 1.4 * opts.syncopation);
                push(1.00, 1.5 - 0.5 * opts.density);
                push(1.50, 0.8 * opts.syncopation + 0.3);
                push(2.00, 1.2 * (1.0 - opts.density));
                if (cand.empty()) { durs.push_back(rem); break; }
                double chosen = cand[rng.weighted_index(wt)];
                if (rem - chosen < 0.24) chosen = rem;
                durs.push_back(chosen);
                rem -= chosen;
            }

            int prev_step = 0;
            int prev_leap = 0;
            const int span = std::max(2, opts.range_steps);
            for (size_t i = 0; i < durs.size(); ++i) {
                MotifNote mn;
                mn.duration_beats = durs[i];
                const double u = (durs.size() > 1) ? static_cast<double>(i) / static_cast<double>(durs.size() - 1) : 0.0;

                if (i > 0 && i + 1 < durs.size() && durs[i] <= 1.0 && rng.chance(opts.rest_prob)) {
                    mn.is_rest = true;
                    m.notes.push_back(mn);
                    continue;
                }

                if (i == 0) {
                    mn.step = (opts.contour == Contour::Descending) ? span : 0;
                    mn.basis = StepBasis::ChordTone;
                } else if (std::abs(prev_leap) >= 3) {
                    mn.step = prev_step + (prev_leap > 0 ? -1 : 1);
                } else {
                    double target = 0.0;
                    switch (opts.contour) {
                        case Contour::Arch:       target = std::sin(PI * u) * span; break;
                        case Contour::Ascending:  target = u * span; break;
                        case Contour::Descending: target = (1.0 - u) * span; break;
                        case Contour::Wave:       target = (0.5 + 0.5 * std::sin(TWO_PI * u)) * span; break;
                        case Contour::Pendulum:   target = (i % 2 == 0) ? 0.0 : static_cast<double>(span); break;
                        case Contour::Hook:       target = (u < 0.65) ? (u / 0.65) * (span * 0.6) : span * 0.3; break;
                    }
                    int diff = static_cast<int>(std::lround(target)) - prev_step;
                    int move = 0;
                    if (diff > 0)      move = rng.chance(0.75) ? 1 : (rng.chance(0.7) ? 2 : 3);
                    else if (diff < 0) move = rng.chance(0.75) ? -1 : (rng.chance(0.7) ? -2 : -3);
                    else               move = rng.pick(std::vector<int>{ -1, 0, 1 });
                    mn.step = prev_step + move;
                }

                prev_leap = mn.step - prev_step;
                prev_step = mn.step;

                if (i == 0) mn.dynamics = wauvio::music::Dynamics::mf;
                else if (mn.step >= span - 1) mn.dynamics = wauvio::music::Dynamics::f;
                else mn.dynamics = wauvio::music::Dynamics::mf;

                m.notes.push_back(mn);
            }
            return m;
        }

        inline const std::vector<Motif>& builtin_motifs() {
            static const std::vector<Motif> list = [] {
                std::vector<Motif> v;
                auto add = [&](const char* name, const char* desc, std::vector<std::string> tags,
                               std::initializer_list<MotifNote> ns) {
                    Motif m; m.name = name; m.description = desc; m.tags = std::move(tags);
                    m.notes.assign(ns.begin(), ns.end());
                    v.push_back(std::move(m));
                };
                using D = wauvio::music::Dynamics;
                using B = StepBasis;
                add("Rising Triad", "Clean chordal climb with held top note.", { "bright", "anthem", "arpeggio" }, {
                    { 0, 0, 0.5, false, B::ChordTone, D::mp },
                    { 1, 0, 0.5, false, B::ChordTone, D::mf },
                    { 2, 0, 1.0, false, B::ChordTone, D::f  },
                    { 3, 0, 2.0, false, B::ChordTone, D::mf }
                });
                add("Sigh Step", "Gentle appoggiatura-like downward step.", { "ballad", "melancholic", "lyrical" }, {
                    { 3, 0, 1.0, false, B::ScaleStep, D::f  },
                    { 2, 0, 1.0, false, B::ScaleStep, D::mp },
                    { 1, 0, 0.5, false, B::ScaleStep, D::p  },
                    { 0, 0, 1.5, false, B::ChordTone, D::mp }
                });
                add("Syncopated Hook", "Pop/EDM dotted-eighth rhythmic hook.", { "pop", "hook", "syncopated" }, {
                    { 0, 0, 0.75, false, B::ChordTone, D::mf },
                    { 2, 0, 0.75, false, B::ScaleStep, D::f  },
                    { 1, 0, 0.50, false, B::ScaleStep, D::mf },
                    { 0, 0, 0.75, false, B::ChordTone, D::mf },
                    { -1,0, 0.75, false, B::ScaleStep, D::mp },
                    { 0, 0, 0.50, true }
                });
                add("Calling Fifth", "Open heroic leap of a fifth followed by a step.", { "epic", "heroic", "open" }, {
                    { 0, 0, 1.0, false, B::ScaleStep, D::mf },
                    { 4, 0, 1.5, false, B::ScaleStep, D::f  },
                    { 3, 0, 0.5, false, B::ScaleStep, D::mf },
                    { 2, 0, 1.0, false, B::ChordTone, D::mf }
                });
                add("Scalar Arch", "Flowing 8th-note rise and fall.", { "classical", "lyrical", "smooth" }, {
                    { 0, 0, 0.5, false, B::ScaleStep, D::mp },
                    { 1, 0, 0.5, false, B::ScaleStep, D::mf },
                    { 2, 0, 0.5, false, B::ScaleStep, D::mf },
                    { 4, 0, 0.5, false, B::ScaleStep, D::f  },
                    { 3, 0, 0.5, false, B::ScaleStep, D::mf },
                    { 2, 0, 0.5, false, B::ScaleStep, D::mf },
                    { 1, 0, 1.0, false, B::ScaleStep, D::mp }
                });
                add("Ostinato Pulse", "Driving rhythmic cell for energetic verses.", { "driving", "rock", "electronic" }, {
                    { 0, 0, 0.5, false, B::ChordTone, D::f  },
                    { 0, 0, 0.5, false, B::ChordTone, D::mp },
                    { 2, 0, 0.5, false, B::ScaleStep, D::mf },
                    { 0, 0, 0.5, false, B::ChordTone, D::mp },
                    { 3, 0, 0.5, false, B::ScaleStep, D::f  },
                    { 2, 0, 0.5, false, B::ScaleStep, D::mf },
                    { 1, 0, 1.0, false, B::ScaleStep, D::mf }
                });
                add("Pentatonic Bounce", "Playful syncopated cell suited to chiptune/jpop.", { "retro", "jpop", "bright" }, {
                    { 0, 0, 0.5, false, B::ScaleStep, D::mf },
                    { 2, 0, 0.25,false, B::ScaleStep, D::f  },
                    { 4, 0, 0.75,false, B::ScaleStep, D::f  },
                    { 2, 0, 0.5, false, B::ScaleStep, D::mf },
                    { 1, 0, 1.0, false, B::ScaleStep, D::mf },
                    { 0, 0, 1.0, true }
                });
                add("Floating Suspension", "Sparse ambient/chill motif leaving breathing space.", { "ambient", "chill", "lofi" }, {
                    { 2, 0, 1.5, false, B::ScaleStep, D::mp },
                    { 1, 0, 1.5, false, B::ChordTone, D::p  },
                    { 0, 0, 1.0, true }
                });
                return v;
            }();
            return list;
        }

        inline const Motif* find_motif(const std::string& name) {
            auto norm = [](const std::string& s) {
                std::string o;
                for (char c : s) if (std::isalnum(static_cast<unsigned char>(c))) o += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                return o;
            };
            const std::string key = norm(name);
            for (const auto& m : builtin_motifs())
                if (norm(m.name) == key) return &m;
            return nullptr;
        }

    }
}
