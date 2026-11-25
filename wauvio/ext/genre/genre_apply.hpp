#pragma once

#include "genre_types.hpp"
#include "../core/model_instruments.hpp"

#include <cstdint>

namespace wauvio {
    namespace music {
        namespace genre {

            // Feel that depends on tempo (swing, quantize, groove, humanize, gate, downbeat weight, velocity groove and compression).
            // A Melody has no tempo of its own, so this runs only when apply_to_melody is given a bpm. Notes keep their order and the
            // melody keeps its total length: each note now lasts from its new onset to the next one.
            inline void apply_timing_to_melody(wauvio::music::Melody& melody, const RoleProfile& prof, double bpm, uint64_t seed)
            {
                const bool timing = prof.swing_amount > 0.0 || prof.timing_humanize_sec > 0.0 || prof.quantize_grid_fraction > 0.0 ||
                                    prof.timing_offset_sec != 0.0 || !prof.groove_timing.empty() || prof.gate_ratio < 1.0;
                const bool velocity = !prof.groove_velocity.empty() || prof.velocity_compress > 0.0 || prof.downbeat_deemphasis > 0.0;
                if (bpm <= 0.0 || melody.empty() || (!timing && !velocity)) return;
                const double beat = 60.0 / bpm, step = beat / 4.0;
                const size_t n = melody.size();
                std::vector<double> t0(n), t1(n);
                double t = 0.0;
                for (size_t i = 0; i < n; ++i) { t0[i] = t; t += melody[i].duration; }
                const double total = t;
                const int sub = std::max(2, prof.swing_subdivision);
                const double pair = beat * 2.0 / sub;

                for (size_t i = 0; i < n; ++i) {
                    wauvio::music::Note& nt = melody[i];
                    double tt = t0[i];
                    if (prof.quantize_grid_fraction > 0.0) { const double grid = beat * prof.quantize_grid_fraction; tt = std::round(tt / grid) * grid; }
                    bool off = false;
                    if (prof.swing_amount > 0.0 || prof.downbeat_deemphasis > 0.0) {
                        double pos = std::fmod(tt, pair); if (pos < 0.0) pos += pair;
                        off = pos > pair * 0.25 && pos < pair * 0.75;
                    }
                    const double tt_grid = tt;
                    if (prof.swing_amount > 0.0 && off) tt += (pair / 2.0) * prof.swing_amount * 0.66;
                    long gi = 0;
                    if (!prof.groove_timing.empty() || !prof.groove_velocity.empty()) gi = std::llround(tt_grid / step);
                    if (!prof.groove_timing.empty()) {
                        long k = gi % static_cast<long>(prof.groove_timing.size()); if (k < 0) k += static_cast<long>(prof.groove_timing.size());
                        tt += prof.groove_timing[static_cast<size_t>(k)] * step;
                    }
                    tt += prof.timing_offset_sec;
                    if (prof.timing_humanize_sec > 0.0 && !nt.is_rest())
                        tt += detail::signed_unit(detail::note_hash(seed, t0[i], nt.midi_note, i, 4)) * prof.timing_humanize_sec;
                    t1[i] = std::max(0.0, tt);

                    if (!nt.is_rest() && velocity) {
                        double mult = 1.0;
                        if (prof.downbeat_deemphasis > 0.0 && !off) mult *= (1.0 - prof.downbeat_deemphasis);
                        if (!prof.groove_velocity.empty()) {
                            long k = gi % static_cast<long>(prof.groove_velocity.size()); if (k < 0) k += static_cast<long>(prof.groove_velocity.size());
                            mult *= prof.groove_velocity[static_cast<size_t>(k)];
                        }
                        double e = nt.expression * mult;
                        if (prof.velocity_compress > 0.0) e = 0.85 + (e - 0.85) * (1.0 - prof.velocity_compress);
                        nt.expression = detail::clampd(e, prof.velocity_floor, 1.0);
                    }
                }
                for (size_t i = 1; i < n; ++i) t1[i] = std::max(t1[i], t1[i - 1] + 0.001);

                wauvio::music::Melody out; out.reserve(n + n / 2);
                for (size_t i = 0; i < n; ++i) {
                    wauvio::music::Note nt = melody[i];
                    const double next = (i + 1 < n) ? t1[i + 1] : std::max(total, t1[i] + 0.01);
                    const double d = std::max(0.005, next - t1[i]);
                    if (!nt.is_rest() && prof.gate_ratio < 1.0 && d * (1.0 - prof.gate_ratio) > 0.005) {
                        nt.duration = std::max(0.005, d * prof.gate_ratio); out.push_back(nt);
                        out.push_back(wauvio::music::rest(d - nt.duration));
                    } else { nt.duration = d; out.push_back(nt); }
                }
                melody.swap(out);
            }

            // bpm > 0 also applies the tempo-dependent feel above; bpm = 0 (the default) keeps the original behaviour.
            inline void apply_to_melody(wauvio::music::Melody& melody, const GenrePreset& preset, Role role,
                                         const wauvio::music::Instrument* instrument = nullptr, uint64_t seed = 1, double bpm = 0.0)
            {
                const RoleProfile& prof = preset.profile_for(role);

                for (size_t i = 0; i < melody.size(); ++i) {
                    wauvio::music::Note& n = melody[i];
                    if (n.is_rest()) continue;

                    if (prof.thin_probability > 0.0 && n.dynamics < wauvio::music::Dynamics::f) {
                        double r = detail::unit(detail::note_hash(seed, static_cast<double>(i), n.midi_note, i, 1));
                        if (r < prof.thin_probability) {
                            n = wauvio::music::rest(n.duration);
                            continue;
                        }
                    }

                    if (prof.register_shift_semitones != 0)
                        n.midi_note = detail::clampi(n.midi_note + prof.register_shift_semitones, 0, 127);

                    if (prof.force_articulation && (!instrument || instrument->supports(prof.articulation)))
                        n.articulation = prof.articulation;

                    double mult = prof.velocity_scale;
                    if (!prof.accent_pattern.empty()) {
                        mult *= detail::accent_multiplier(prof.accent_pattern, static_cast<int>(i));
                    }
                    if (prof.velocity_variance > 0.0) {
                        double r = detail::signed_unit(detail::note_hash(seed, static_cast<double>(i), n.midi_note, i, 2));
                        mult *= (1.0 + r * prof.velocity_variance);
                    }
                    double new_expr = n.expression * mult;
                    n.expression = detail::clampd(new_expr, prof.velocity_floor, 1.0);
                }
                apply_timing_to_melody(melody, prof, bpm, seed);
            }

            struct InstrumentStyleBaseline {
                float gain = 1.0f, pan = 0.0f;
                bool  reverb_on = false;     wauvio::fx::basic::Reverb       reverb_fx;
                bool  chorus_on = false;     wauvio::fx::basic::Chorus       chorus_fx;
                bool  delay_on = false;      wauvio::fx::basic::DelayLine    delay_fx;
                bool  eq_on = false;         wauvio::filter::ParametricEQ eq_fx;
                bool  distortion_on = false; float distortion_drive = 2.0f;

                bool             has_recipe = false;
                wauvio::music::TimbreRecipe recipe;
            };

            inline InstrumentStyleBaseline capture_style_baseline(const wauvio::music::Instrument& instrument) {
                InstrumentStyleBaseline b;
                b.gain = instrument.gain; b.pan = instrument.pan;
                b.reverb_on = instrument.reverb_on; b.reverb_fx = instrument.reverb_fx;
                b.chorus_on = instrument.chorus_on; b.chorus_fx = instrument.chorus_fx;
                b.delay_on = instrument.delay_on; b.delay_fx = instrument.delay_fx;
                b.eq_on = instrument.eq_on; b.eq_fx = instrument.eq_fx;
                b.distortion_on = instrument.distortion_on; b.distortion_drive = instrument.distortion_drive;
                if (auto* modeled = dynamic_cast<const wauvio::music::ModeledInstrument*>(&instrument)) {
                    b.has_recipe = true;
                    b.recipe = modeled->recipe;
                }
                return b;
            }

            inline void restore_style_baseline(wauvio::music::Instrument& instrument, const InstrumentStyleBaseline& b) {
                instrument.gain = b.gain; instrument.pan = b.pan;
                instrument.reverb_on = b.reverb_on; instrument.reverb_fx = b.reverb_fx;
                instrument.chorus_on = b.chorus_on; instrument.chorus_fx = b.chorus_fx;
                instrument.delay_on = b.delay_on; instrument.delay_fx = b.delay_fx;
                instrument.eq_on = b.eq_on; instrument.eq_fx = b.eq_fx;
                instrument.distortion_on = b.distortion_on; instrument.distortion_drive = b.distortion_drive;
                if (b.has_recipe) {
                    if (auto* modeled = dynamic_cast<wauvio::music::ModeledInstrument*>(&instrument))
                        modeled->recipe = b.recipe;
                }
            }

            inline Family instrument_family(const wauvio::music::Instrument& in) {
                using namespace wauvio::music;
                if (dynamic_cast<const GuitarInstrument*>(&in)) return Family::Guitar;
                if (dynamic_cast<const KeyboardInstrument*>(&in)) return Family::Keys;
                if (dynamic_cast<const BowedStringInstrument*>(&in)) return Family::Strings;
                if (dynamic_cast<const WoodwindInstrument*>(&in) || dynamic_cast<const SaxophoneInstrument*>(&in)) return Family::Winds;
                if (dynamic_cast<const BrassInstrument*>(&in)) return Family::Brass;
                if (dynamic_cast<const SynthInstrument*>(&in)) return Family::Synth;
                if (dynamic_cast<const MalletInstrument*>(&in)) return Family::Mallet;
                if (dynamic_cast<const VocalInstrument*>(&in)) return Family::Vocal;
                if (dynamic_cast<const WorldInstrument*>(&in)) return Family::World;
                return Family::Any;
            }

            inline void style_instrument(wauvio::music::Instrument& instrument, const GenrePreset& preset, Role role) {
                const RoleProfile& prof = preset.profile_for(role);

                if (auto* modeled = dynamic_cast<wauvio::music::ModeledInstrument*>(&instrument)) {
                    auto& recipe = modeled->recipe;
                    recipe.filter_cutoff = std::max(60.0, recipe.filter_cutoff * prof.filter_cutoff_mult);
                    recipe.detune_cents += prof.detune_cents_delta;
                    recipe.noise_mix = detail::clampd(recipe.noise_mix + prof.noise_mix_delta, 0.0, 1.0);
                    if (prof.stereo_width_override >= 0.0) recipe.stereo_width = prof.stereo_width_override;

                    if (prof.wobble_amount > 0.0) {
                        wauvio::music::ModRoute route;
                        route.source = wauvio::music::ModSource::LFO2;
                        route.target = wauvio::music::ModTarget::FilterCutoff;
                        route.amount = prof.wobble_amount * 3500.0;
                        recipe.modulation.push_back(route);
                    }
                }

                if (prof.want_reverb && !instrument.reverb_on)
                    instrument.reverb(prof.reverb_room, prof.reverb_wet, prof.reverb_damping);
                if (prof.want_chorus && !instrument.chorus_on)
                    instrument.chorus(prof.chorus_rate_hz, prof.chorus_depth_ms, prof.chorus_wet);
                if (prof.want_delay && !instrument.delay_on)
                    instrument.delay(prof.delay_time_ms, prof.delay_feedback, prof.delay_wet);
                if (prof.want_distortion && !instrument.distortion_on &&
                    (prof.distortion_family == Family::Any || prof.distortion_family == instrument_family(instrument)))
                    instrument.distortion(prof.distortion_drive);
            }

        }
    }
}
