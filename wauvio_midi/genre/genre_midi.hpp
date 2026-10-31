#pragma once

#include "../track/midi_music.hpp"
#include "../../wauvio_ext/genre/genre_apply.hpp"

#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>

namespace wauvio {
    namespace midi {
        namespace genre {

            struct GenreApplyOptions {
                bool allow_instrumentation      = false;
                bool override_explicit_programs = false;
                uint64_t seed                   = 1;
            };

            class MidiGenrePreset {
            public:
                wauvio::genre::GenrePresetPtr base;
                std::unordered_map<int, InstrumentFactory> melodic_program_overrides;
                std::function<std::shared_ptr<audio::DrumKit>()> percussion_kit;

                explicit MidiGenrePreset(wauvio::genre::GenrePresetPtr b) : base(std::move(b)) {}

                const std::string& name() const noexcept {
                    static const std::string empty;
                    return base ? base->name : empty;
                }
            };

            using MidiGenrePresetPtr = std::shared_ptr<const MidiGenrePreset>;

            inline wauvio::genre::Role role_for_part(const track::MidiPart& part) noexcept {
                if (part.is_percussion) return wauvio::genre::Role::Percussion;
                if (part.gm_program >= 32 && part.gm_program <= 39) return wauvio::genre::Role::Bass;
                return wauvio::genre::Role::Melodic;
            }

            inline double estimate_beat_seconds(const track::MidiMusic& music) noexcept {
                double bpm = music.initial_bpm > 0.0 ? music.initial_bpm : 120.0;
                return 60.0 / bpm;
            }

            inline int estimate_beats_per_bar(const track::MidiMusic& music) noexcept {
                if (!music.time_signatures.empty() && music.time_signatures.front().numerator > 0)
                    return music.time_signatures.front().numerator;
                return 4;
            }

            inline void apply_role_profile(track::MidiPart& part, const wauvio::genre::RoleProfile& prof,
                                            double beat_seconds, int beats_per_bar, uint64_t seed)
            {
                using namespace wauvio::genre::detail;

                for (size_t i = 0; i < part.notes.size(); ++i) {
                    track::ResolvedNote& n = part.notes[i];
                    if (n.dur <= 0.0) continue;

                    const double orig_t = n.t;
                    const int orig_note = n.midi_note;

                    if (prof.thin_probability > 0.0 && n.dyn < audio::Dynamics::f) {
                        double r = unit(note_hash(seed, orig_t, orig_note, i, 1));
                        if (r < prof.thin_probability) { n.dur = 0.0; continue; }
                    }

                    if (prof.register_shift_semitones != 0)
                        n.midi_note = clampi(n.midi_note + prof.register_shift_semitones, 0, 127);

                    bool articulation_supported = false;
                    if (prof.force_articulation) {
                        n.articulation = prof.articulation;
                        articulation_supported = part.instrument && part.instrument->supports(prof.articulation);
                    }
                    if (prof.fallback_gate_ratio < 1.0 && !articulation_supported)
                        n.dur = std::max(0.01, n.dur * prof.fallback_gate_ratio);

                    double t = orig_t;
                    if (prof.quantize_grid_fraction > 0.0 && beat_seconds > 0.0) {
                        double grid = beat_seconds * prof.quantize_grid_fraction;
                        if (grid > 0.0) t = std::round(t / grid) * grid;
                    }
                    bool is_offbeat_eighth = false;
                    if (beat_seconds > 0.0 && (prof.swing_amount > 0.0 || prof.downbeat_deemphasis > 0.0)) {
                        double eighth = beat_seconds / 2.0;
                        double pos = std::fmod(t, beat_seconds);
                        if (pos < 0.0) pos += beat_seconds;
                        is_offbeat_eighth = (pos > eighth * 0.5 && pos < eighth * 1.5);
                    }
                    if (prof.swing_amount > 0.0 && beat_seconds > 0.0 && is_offbeat_eighth)
                        t += (beat_seconds / 2.0) * prof.swing_amount * 0.66;
                    if (prof.timing_offset_sec != 0.0)
                        t += prof.timing_offset_sec;
                    if (prof.timing_humanize_sec > 0.0) {
                        double r = signed_unit(note_hash(seed, orig_t, orig_note, i, 2));
                        t += r * prof.timing_humanize_sec;
                    }
                    n.t = std::max(0.0, t);

                    double mult = prof.velocity_scale;
                    if (!prof.accent_pattern.empty() && beat_seconds > 0.0) {
                        int beat_index = static_cast<int>(std::floor(orig_t / beat_seconds));
                        beat_index = ((beat_index % beats_per_bar) + beats_per_bar) % beats_per_bar;
                        mult *= accent_multiplier(prof.accent_pattern, beat_index);
                    }
                    if (prof.downbeat_deemphasis > 0.0 && !is_offbeat_eighth)
                        mult *= (1.0 - prof.downbeat_deemphasis);
                    if (prof.velocity_variance > 0.0) {
                        double r = signed_unit(note_hash(seed, orig_t, orig_note, i, 3));
                        mult *= (1.0 + r * prof.velocity_variance);
                    }
                    n.expression = clampd(n.expression * mult, prof.velocity_floor, 1.0);
                }
            }

            inline void apply_instrumentation(track::MidiMusic& music, const MidiGenrePreset& preset,
                                               const GenreApplyOptions& opts)
            {
                if (!opts.allow_instrumentation) return;

                for (auto& part : music.parts) {
                    if (part.instrument_pinned) continue;
                    if (part.program_from_midi_file && !opts.override_explicit_programs) continue;

                    if (part.is_percussion) {
                        if (preset.percussion_kit) part.drum_kit = preset.percussion_kit();
                    } else {
                        auto it = preset.melodic_program_overrides.find(part.gm_program);
                        if (it != preset.melodic_program_overrides.end() && it->second)
                            part.instrument = it->second();
                    }
                }
            }

            inline void style_percussion_kit(audio::DrumKit& kit, const wauvio::genre::GenrePreset& preset) {
                for (int note : kit.mapped_notes()) {
                    auto instr = kit.get(note);
                    if (instr) wauvio::genre::style_instrument(*instr, preset, wauvio::genre::Role::Percussion);
                }
            }

        }
    }

    namespace track {

        inline void MidiMusic::setGenre(std::shared_ptr<const midi::genre::MidiGenrePreset> preset) {
            setGenre(std::move(preset), midi::genre::GenreApplyOptions());
        }

        inline void MidiMusic::setGenre(std::shared_ptr<const midi::genre::MidiGenrePreset> preset,
                                         const midi::genre::GenreApplyOptions& options)
        {
            if (!has_pristine_) {
                pristine_parts_ = parts;
                pristine_duration_seconds_ = duration_seconds;

                pristine_has_instrument_style_.assign(parts.size(), false);
                pristine_instrument_style_.assign(parts.size(), genre::InstrumentStyleBaseline());
                pristine_percussion_style_.assign(parts.size(), {});
                for (size_t i = 0; i < parts.size(); ++i) {
                    if (parts[i].instrument) {
                        pristine_has_instrument_style_[i] = true;
                        pristine_instrument_style_[i] = genre::capture_style_baseline(*parts[i].instrument);
                    } else if (parts[i].drum_kit) {
                        for (int note : parts[i].drum_kit->mapped_notes()) {
                            auto voice = parts[i].drum_kit->get(note);
                            if (voice) pristine_percussion_style_[i].emplace_back(note, genre::capture_style_baseline(*voice));
                        }
                    }
                }
                has_pristine_ = true;
            }

            parts = pristine_parts_;
            duration_seconds = pristine_duration_seconds_;
            genre_preset_ = nullptr;
            for (size_t i = 0; i < parts.size(); ++i) {
                if (pristine_has_instrument_style_[i] && parts[i].instrument) {
                    genre::restore_style_baseline(*parts[i].instrument, pristine_instrument_style_[i]);
                } else if (parts[i].drum_kit) {
                    for (auto& [note, baseline] : pristine_percussion_style_[i]) {
                        auto voice = parts[i].drum_kit->get(note);
                        if (voice) genre::restore_style_baseline(*voice, baseline);
                    }
                }
            }

            if (!preset || !preset->base) return;

            const wauvio::genre::GenrePreset& base = *preset->base;
            const double beat_seconds = midi::genre::estimate_beat_seconds(*this);
            const int beats_per_bar = midi::genre::estimate_beats_per_bar(*this);

            for (auto& part : parts) {
                const auto& prof = base.profile_for(midi::genre::role_for_part(part));
                midi::genre::apply_role_profile(part, prof, beat_seconds, beats_per_bar, options.seed);
            }

            midi::genre::apply_instrumentation(*this, *preset, options);

            for (auto& part : parts) {
                if (part.is_percussion) {
                    if (part.drum_kit) midi::genre::style_percussion_kit(*part.drum_kit, base);
                } else if (part.instrument) {
                    wauvio::genre::style_instrument(*part.instrument, base, midi::genre::role_for_part(part));
                }
            }

            if (base.tempo_scale > 0.0 && base.tempo_scale != 1.0) {
                for (auto& part : parts)
                    for (auto& n : part.notes) { n.t /= base.tempo_scale; n.dur /= base.tempo_scale; }
                duration_seconds /= base.tempo_scale;
            }

            genre_preset_ = preset;
        }

        inline void MidiMusic::clearGenre() {
            if (has_pristine_) {
                parts = pristine_parts_;
                duration_seconds = pristine_duration_seconds_;
                for (size_t i = 0; i < parts.size(); ++i) {
                    if (pristine_has_instrument_style_[i] && parts[i].instrument) {
                        genre::restore_style_baseline(*parts[i].instrument, pristine_instrument_style_[i]);
                    } else if (parts[i].drum_kit) {
                        for (auto& [note, baseline] : pristine_percussion_style_[i]) {
                            auto voice = parts[i].drum_kit->get(note);
                            if (voice) genre::restore_style_baseline(*voice, baseline);
                        }
                    }
                }
            }
            genre_preset_ = nullptr;
        }

    }
}