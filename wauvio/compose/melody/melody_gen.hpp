#pragma once

#include "../progression/progressions.hpp"
#include "../motif/motifs.hpp"

namespace wauvio {
    namespace compose {

        enum class PhraseForm { AABA, ABAC, AA_Prime, AABB, Through };

        struct MelodyOptions {
            int        low_midi            = 60;
            int        high_midi           = 81;
            int        base_octave         = 4;
            double     motif_adherence     = 0.75;
            double     chord_tone_strength = 0.85;
            double     passing_tone_prob   = 0.30;
            double     climax_pos          = 0.72;
            bool       resolve_on_tonic    = true;
            PhraseForm form                = PhraseForm::ABAC;
            double     bpm                 = 120.0;
            int        beats_per_bar       = 4;
            uint64_t   seed                = 1;
        };

        namespace melody_detail {

            inline int nearest_chord_tone_in_range(const ChordSymbol& chord, int target_midi, int low_midi, int high_midi) {
                auto pool = chord.tones_in_range(low_midi, high_midi);
                if (pool.empty()) return std::clamp(target_midi, low_midi, high_midi);
                int best = pool[0], best_d = std::abs(pool[0] - target_midi);
                for (int m : pool) {
                    int d = std::abs(m - target_midi);
                    if (d < best_d) { best_d = d; best = m; }
                }
                return best;
            }

            inline int clamp_register(int midi, int low_midi, int high_midi) {
                while (midi < low_midi && midi + 12 <= high_midi) midi += 12;
                while (midi > high_midi && midi - 12 >= low_midi) midi -= 12;
                return std::clamp(midi, low_midi, high_midi);
            }

        }

        inline wauvio::music::Melody realize_motif_span(const Key& key,
                                                         const ChordProgression& prog,
                                                         const Motif& motif,
                                                         double prog_beat_offset,
                                                         double span_beats,
                                                         int anchor_step,
                                                         const MelodyOptions& opts,
                                                         Rng& rng)
        {
            using namespace wauvio::music;
            Melody out;
            if (motif.notes.empty() || span_beats <= 0.0 || prog.chords.empty()) return out;

            const auto resolved = prog.resolve(key);
            const double sec_per_beat = 60.0 / std::max(20.0, opts.bpm);
            double beat = 0.0;
            size_t idx = 0;
            int prev_midi = -1;

            while (beat < span_beats - 1e-6) {
                const MotifNote& mn = motif.notes[idx % motif.notes.size()];
                const double dur_beats = std::min(mn.duration_beats, span_beats - beat);
                const double dur_sec = dur_beats * sec_per_beat;
                const double abs_beat = prog_beat_offset + beat;
                const size_t cidx = prog.chord_index_at_beat(abs_beat);
                const ChordSymbol& chord = resolved[cidx];

                if (mn.is_rest) {
                    out.push_back(rest(dur_sec));
                    beat += dur_beats;
                    ++idx;
                    continue;
                }

                int midi = 60;
                if (mn.basis == StepBasis::ChordTone) {
                    auto pool = chord.tones_in_range(opts.low_midi, opts.high_midi);
                    if (!pool.empty()) {
                        int base_idx = static_cast<int>(pool.size()) / 2 + mn.step;
                        base_idx = std::clamp(base_idx, 0, static_cast<int>(pool.size()) - 1);
                        midi = pool[static_cast<size_t>(base_idx)] + mn.chromatic;
                    } else {
                        midi = key.degree_midi(anchor_step + mn.step, opts.base_octave, mn.chromatic);
                    }
                } else {
                    midi = key.degree_midi(anchor_step + mn.step, opts.base_octave, mn.chromatic);
                }

                midi = melody_detail::clamp_register(midi, opts.low_midi, opts.high_midi);

                const double bar_pos = std::fmod(abs_beat, static_cast<double>(std::max(1, opts.beats_per_bar)));
                const bool strong_beat = (bar_pos < 0.05) || (opts.beats_per_bar == 4 && std::abs(bar_pos - 2.0) < 0.05);
                if (strong_beat && rng.chance(opts.chord_tone_strength)) {
                    midi = melody_detail::nearest_chord_tone_in_range(chord, midi, opts.low_midi, opts.high_midi);
                }

                if (prev_midi >= 0 && std::abs(midi - prev_midi) > 9) {
                    int shifted = midi + (midi > prev_midi ? -12 : 12);
                    if (shifted >= opts.low_midi && shifted <= opts.high_midi) midi = shifted;
                }

                if (prev_midi >= 0 && std::abs(midi - prev_midi) >= 4 && dur_beats >= 1.0 &&
                    rng.chance(opts.passing_tone_prob))
                {
                    int mid_midi = key.snap_to_scale((prev_midi + midi) / 2);
                    double d1 = (dur_beats * 0.5) * sec_per_beat;
                    double d2 = (dur_beats * 0.5) * sec_per_beat;
                    out.push_back(Note(mid_midi, d1, Dynamics::mp, mn.articulation));
                    out.push_back(Note(midi, d2, mn.dynamics, mn.articulation));
                } else {
                    out.push_back(Note(midi, dur_sec, mn.dynamics, mn.articulation));
                }

                prev_midi = midi;
                beat += dur_beats;
                ++idx;
            }
            return out;
        }

        inline wauvio::music::Melody generate_melody(const Key& key,
                                                      const ChordProgression& prog,
                                                      const Motif& seed_motif,
                                                      const MelodyOptions& opts = {})
        {
            using namespace wauvio::music;
            Melody out;
            const double total_beats = prog.total_beats();
            if (total_beats <= 0.0 || prog.chords.empty()) return out;

            Rng rng(opts.seed);
            const double phrase_beats = std::min(total_beats, static_cast<double>(std::max(1, opts.beats_per_bar)) * 2.0);
            const int n_phrases = std::max(1, static_cast<int>(std::ceil(total_beats / phrase_beats - 1e-6)));

            const Motif A = seed_motif;
            const Motif A_prime = vary_motif(A, rng, 1.0 - opts.motif_adherence * 0.5);
            Motif B = transpose(invert(A, 2), 2);
            B = vary_motif(B, rng, 0.5);
            Motif C = generate_motif(MotifOptions{ std::min(4.0, phrase_beats), Contour::Descending, 0.5, 0.2, 0.1, 5, opts.seed + 77 });

            auto motif_for_phrase = [&](int pi) -> const Motif& {
                if (n_phrases == 1) return A;
                switch (opts.form) {
                    case PhraseForm::AABA:     return (pi % 4 == 2) ? B : (pi % 4 == 1 ? A_prime : A);
                    case PhraseForm::ABAC:     return (pi % 4 == 1) ? B : (pi % 4 == 3 ? C : A);
                    case PhraseForm::AA_Prime: return (pi % 2 == 1) ? A_prime : A;
                    case PhraseForm::AABB:     return ((pi / 2) % 2 == 1) ? B : A;
                    case PhraseForm::Through:  return (pi == 0) ? A : (pi == 1 ? A_prime : (pi == 2 ? B : C));
                }
                return A;
            };

            const double sec_per_beat = 60.0 / std::max(20.0, opts.bpm);
            double beat_cursor = 0.0;

            for (int pi = 0; pi < n_phrases && beat_cursor < total_beats - 1e-6; ++pi) {
                const double span = std::min(phrase_beats, total_beats - beat_cursor);
                const double u = static_cast<double>(pi) / static_cast<double>(std::max(1, n_phrases - 1));
                const double lift = 1.0 - std::min(1.0, std::abs(u - opts.climax_pos) * 2.5);
                int anchor = static_cast<int>(std::lround(lift * 3.0));
                if (pi % 4 == 2 && opts.form == PhraseForm::AABA) anchor += 2;

                Melody phrase = realize_motif_span(key, prog, motif_for_phrase(pi), beat_cursor, span, anchor, opts, rng);

                if (pi + 1 == n_phrases && opts.resolve_on_tonic && !phrase.empty()) {
                    for (size_t k = phrase.size(); k-- > 0;) {
                        if (!phrase[k].is_rest()) {
                            const auto resolved = prog.resolve(key);
                            int target_pc = (opts.resolve_on_tonic && prog.chords.back().degree == 0)
                                            ? key.root_pc()
                                            : resolved.back().root_pc;
                            int cand = phrase[k].midi_note;
                            int best = cand, best_d = 99;
                            for (int m = opts.low_midi; m <= opts.high_midi; ++m) {
                                if (wrap_pc(m) == target_pc && std::abs(m - cand) < best_d) {
                                    best_d = std::abs(m - cand); best = m;
                                }
                            }
                            phrase[k].midi_note = best;
                            if (phrase[k].duration >= 0.4 * sec_per_beat)
                                phrase[k].dynamics = Dynamics::mf;
                            break;
                        }
                    }
                }

                out.insert(out.end(), phrase.begin(), phrase.end());
                beat_cursor += span;
            }
            return out;
        }

        inline wauvio::music::Melody generate_countermelody(const Key& key,
                                                             const ChordProgression& prog,
                                                             const wauvio::music::Melody& lead,
                                                             double bpm,
                                                             int low_midi = 55,
                                                             int high_midi = 74,
                                                             uint64_t seed = 99)
        {
            using namespace wauvio::music;
            Melody out;
            if (prog.chords.empty()) return out;
            Rng rng(seed);
            const auto resolved = prog.resolve(key);
            const double sec_per_beat = 60.0 / std::max(20.0, bpm);

            int prev = (low_midi + high_midi) / 2;
            double beat = 0.0;
            size_t lead_idx = 0;
            double lead_t = 0.0;

            for (size_t ci = 0; ci < resolved.size(); ++ci) {
                const ChordSymbol& ch = resolved[ci];
                double rem = ch.duration_beats;
                while (rem > 1e-6) {
                    double step_b = std::min(rem, (ch.duration_beats >= 4.0) ? 2.0 : ch.duration_beats);
                    double cur_sec = beat * sec_per_beat;
                    while (lead_idx + 1 < lead.size() && lead_t + lead[lead_idx].duration <= cur_sec) {
                        lead_t += lead[lead_idx].duration;
                        ++lead_idx;
                    }
                    int lead_midi = (!lead.empty() && !lead[lead_idx].is_rest()) ? lead[lead_idx].midi_note : prev + 5;

                    auto pool = ch.tones_in_range(low_midi, high_midi);
                    int chosen = prev;
                    double best_score = 1e9;
                    for (int m : pool) {
                        if (wrap_pc(m) == wrap_pc(lead_midi)) continue;
                        double score = std::abs(m - prev) * 1.4;
                        if (std::abs(m - lead_midi) < 3) score += 8.0;
                        if (score < best_score) { best_score = score; chosen = m; }
                    }
                    if (rng.chance(0.15)) out.push_back(rest(step_b * sec_per_beat));
                    else out.push_back(Note(chosen, step_b * sec_per_beat, Dynamics::mp, Articulation::Legato));
                    prev = chosen;
                    rem -= step_b;
                    beat += step_b;
                }
            }
            return out;
        }

    }
}
