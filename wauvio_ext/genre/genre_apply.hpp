#pragma once

#include "genre_types.hpp"
#include "../core/model_instruments.hpp"

#include <cstdint>

namespace wauvio {
    namespace genre {

        inline void apply_to_melody(audio::Melody& melody, const GenrePreset& preset, Role role,
                                     const audio::Instrument* instrument = nullptr, uint64_t seed = 1)
        {
            const RoleProfile& prof = preset.profile_for(role);

            for (size_t i = 0; i < melody.size(); ++i) {
                audio::Note& n = melody[i];
                if (n.is_rest()) continue;

                if (prof.thin_probability > 0.0 && n.dynamics < audio::Dynamics::f) {
                    double r = detail::unit(detail::note_hash(seed, static_cast<double>(i), n.midi_note, i, 1));
                    if (r < prof.thin_probability) {
                        n = audio::rest(n.duration);
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
        }

        struct InstrumentStyleBaseline {
            float gain = 1.0f, pan = 0.0f;
            bool  reverb_on = false;     Reverb       reverb_fx;
            bool  chorus_on = false;     Chorus       chorus_fx;
            bool  delay_on = false;      DelayLine    delay_fx;
            bool  eq_on = false;         ParametricEQ eq_fx;
            bool  distortion_on = false; float distortion_drive = 2.0f;

            bool             has_recipe = false;
            audio::TimbreRecipe recipe;
        };

        inline InstrumentStyleBaseline capture_style_baseline(const audio::Instrument& instrument) {
            InstrumentStyleBaseline b;
            b.gain = instrument.gain; b.pan = instrument.pan;
            b.reverb_on = instrument.reverb_on; b.reverb_fx = instrument.reverb_fx;
            b.chorus_on = instrument.chorus_on; b.chorus_fx = instrument.chorus_fx;
            b.delay_on = instrument.delay_on; b.delay_fx = instrument.delay_fx;
            b.eq_on = instrument.eq_on; b.eq_fx = instrument.eq_fx;
            b.distortion_on = instrument.distortion_on; b.distortion_drive = instrument.distortion_drive;
            if (auto* modeled = dynamic_cast<const audio::ModeledInstrument*>(&instrument)) {
                b.has_recipe = true;
                b.recipe = modeled->recipe;
            }
            return b;
        }

        inline void restore_style_baseline(audio::Instrument& instrument, const InstrumentStyleBaseline& b) {
            instrument.gain = b.gain; instrument.pan = b.pan;
            instrument.reverb_on = b.reverb_on; instrument.reverb_fx = b.reverb_fx;
            instrument.chorus_on = b.chorus_on; instrument.chorus_fx = b.chorus_fx;
            instrument.delay_on = b.delay_on; instrument.delay_fx = b.delay_fx;
            instrument.eq_on = b.eq_on; instrument.eq_fx = b.eq_fx;
            instrument.distortion_on = b.distortion_on; instrument.distortion_drive = b.distortion_drive;
            if (b.has_recipe) {
                if (auto* modeled = dynamic_cast<audio::ModeledInstrument*>(&instrument))
                    modeled->recipe = b.recipe;
            }
        }

        inline void style_instrument(audio::Instrument& instrument, const GenrePreset& preset, Role role) {
            const RoleProfile& prof = preset.profile_for(role);

            if (auto* modeled = dynamic_cast<audio::ModeledInstrument*>(&instrument)) {
                auto& recipe = modeled->recipe;
                recipe.filter_cutoff = std::max(60.0, recipe.filter_cutoff * prof.filter_cutoff_mult);
                recipe.detune_cents += prof.detune_cents_delta;
                recipe.noise_mix = detail::clampd(recipe.noise_mix + prof.noise_mix_delta, 0.0, 1.0);
                if (prof.stereo_width_override >= 0.0) recipe.stereo_width = prof.stereo_width_override;

                if (prof.wobble_amount > 0.0) {
                    audio::ModRoute route;
                    route.source = audio::ModSource::LFO2;
                    route.target = audio::ModTarget::FilterCutoff;
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
            if (prof.want_distortion && !instrument.distortion_on)
                instrument.distortion(prof.distortion_drive);
        }

    }
}
