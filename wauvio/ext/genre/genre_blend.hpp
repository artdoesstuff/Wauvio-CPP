#pragma once
#include "genre_types.hpp"

#include <cmath>
#include <string>

namespace wauvio {
    namespace music {
        namespace genre {

            namespace detail {
                inline double lerp(double a, double b, double t) { return a + (b - a) * t; }
                inline double glerp(double a, double b, double t) { return std::exp(lerp(std::log(std::max(1e-6, a)), std::log(std::max(1e-6, b)), t)); }
                template <class T> inline const T& pick(const T& a, const T& b, double t) { return t >= 0.5 ? b : a; }

                inline std::vector<double> blend_vec(const std::vector<double>& a, const std::vector<double>& b, double t, double neutral) {
                    if (a.empty() && b.empty()) return {};
                    const std::vector<double>& ref = a.empty() ? b : a;
                    if (!a.empty() && !b.empty() && a.size() != b.size()) return t >= 0.5 ? b : a;
                    std::vector<double> out(ref.size());
                    for (size_t i = 0; i < ref.size(); ++i)
                        out[i] = lerp(a.empty() ? neutral : a[i], b.empty() ? neutral : b[i], t);
                    return out;
                }
            }

            inline RoleProfile blend(const RoleProfile& a, const RoleProfile& b, double t) {
                using detail::lerp;
                t = std::max(0.0, std::min(1.0, t));
                RoleProfile r;
                r.velocity_scale = lerp(a.velocity_scale, b.velocity_scale, t);
                r.velocity_floor = lerp(a.velocity_floor, b.velocity_floor, t);
                r.velocity_variance = lerp(a.velocity_variance, b.velocity_variance, t);
                r.accent_pattern = detail::blend_vec(a.accent_pattern, b.accent_pattern, t, 1.0);
                r.swing_amount = lerp(a.swing_amount, b.swing_amount, t);
                r.timing_humanize_sec = lerp(a.timing_humanize_sec, b.timing_humanize_sec, t);
                r.quantize_grid_fraction = detail::pick(a.quantize_grid_fraction, b.quantize_grid_fraction, t) > 0.0
                    ? (a.quantize_grid_fraction > 0.0 && b.quantize_grid_fraction > 0.0 ? lerp(a.quantize_grid_fraction, b.quantize_grid_fraction, t) : detail::pick(a.quantize_grid_fraction, b.quantize_grid_fraction, t))
                    : 0.0;
                r.timing_offset_sec = lerp(a.timing_offset_sec, b.timing_offset_sec, t);
                r.downbeat_deemphasis = lerp(a.downbeat_deemphasis, b.downbeat_deemphasis, t);
                r.thin_probability = lerp(a.thin_probability, b.thin_probability, t);
                r.register_shift_semitones = static_cast<int>(std::lround(lerp(a.register_shift_semitones, b.register_shift_semitones, t)));
                r.force_articulation = detail::pick(a.force_articulation, b.force_articulation, t);
                r.articulation = detail::pick(a.articulation, b.articulation, t);
                r.fallback_gate_ratio = lerp(a.fallback_gate_ratio, b.fallback_gate_ratio, t);
                r.filter_cutoff_mult = detail::glerp(a.filter_cutoff_mult, b.filter_cutoff_mult, t);
                r.detune_cents_delta = lerp(a.detune_cents_delta, b.detune_cents_delta, t);
                if (a.stereo_width_override >= 0.0 && b.stereo_width_override >= 0.0) r.stereo_width_override = lerp(a.stereo_width_override, b.stereo_width_override, t);
                else r.stereo_width_override = detail::pick(a.stereo_width_override, b.stereo_width_override, t);
                r.noise_mix_delta = lerp(a.noise_mix_delta, b.noise_mix_delta, t);
                r.wobble_amount = lerp(a.wobble_amount, b.wobble_amount, t);

                // An effect only one side asks for fades in with the blend weight instead of switching on at full strength.
                const double wa = 1.0 - t, wb = t;
                auto fx = [&](bool aw, bool bw, auto&& set_both, auto&& set_a, auto&& set_b, bool& want) {
                    if (aw && bw) { want = true; set_both(); }
                    else if (aw && wa > 0.02) { want = true; set_a(wa); }
                    else if (bw && wb > 0.02) { want = true; set_b(wb); }
                };
                fx(a.want_reverb, b.want_reverb,
                   [&] { r.reverb_room = (float)lerp(a.reverb_room, b.reverb_room, t); r.reverb_wet = (float)lerp(a.reverb_wet, b.reverb_wet, t); r.reverb_damping = (float)lerp(a.reverb_damping, b.reverb_damping, t); },
                   [&](double w) { r.reverb_room = a.reverb_room; r.reverb_wet = a.reverb_wet * (float)w; r.reverb_damping = a.reverb_damping; },
                   [&](double w) { r.reverb_room = b.reverb_room; r.reverb_wet = b.reverb_wet * (float)w; r.reverb_damping = b.reverb_damping; }, r.want_reverb);
                fx(a.want_chorus, b.want_chorus,
                   [&] { r.chorus_rate_hz = lerp(a.chorus_rate_hz, b.chorus_rate_hz, t); r.chorus_depth_ms = lerp(a.chorus_depth_ms, b.chorus_depth_ms, t); r.chorus_wet = (float)lerp(a.chorus_wet, b.chorus_wet, t); },
                   [&](double w) { r.chorus_rate_hz = a.chorus_rate_hz; r.chorus_depth_ms = a.chorus_depth_ms; r.chorus_wet = a.chorus_wet * (float)w; },
                   [&](double w) { r.chorus_rate_hz = b.chorus_rate_hz; r.chorus_depth_ms = b.chorus_depth_ms; r.chorus_wet = b.chorus_wet * (float)w; }, r.want_chorus);
                fx(a.want_delay, b.want_delay,
                   [&] { r.delay_time_ms = lerp(a.delay_time_ms, b.delay_time_ms, t); r.delay_feedback = (float)lerp(a.delay_feedback, b.delay_feedback, t); r.delay_wet = (float)lerp(a.delay_wet, b.delay_wet, t); },
                   [&](double w) { r.delay_time_ms = a.delay_time_ms; r.delay_feedback = a.delay_feedback; r.delay_wet = a.delay_wet * (float)w; },
                   [&](double w) { r.delay_time_ms = b.delay_time_ms; r.delay_feedback = b.delay_feedback; r.delay_wet = b.delay_wet * (float)w; }, r.want_delay);
                fx(a.want_distortion, b.want_distortion,
                   [&] { r.distortion_drive = (float)lerp(a.distortion_drive, b.distortion_drive, t); },
                   [&](double w) { r.distortion_drive = 1.0f + (a.distortion_drive - 1.0f) * (float)w; },
                   [&](double w) { r.distortion_drive = 1.0f + (b.distortion_drive - 1.0f) * (float)w; }, r.want_distortion);

                r.swing_subdivision = detail::pick(a.swing_subdivision, b.swing_subdivision, t);
                r.groove_timing = detail::blend_vec(a.groove_timing, b.groove_timing, t, 0.0);
                r.groove_velocity = detail::blend_vec(a.groove_velocity, b.groove_velocity, t, 1.0);
                r.velocity_compress = lerp(a.velocity_compress, b.velocity_compress, t);
                r.gate_ratio = lerp(a.gate_ratio, b.gate_ratio, t);
                r.distortion_family = (a.want_distortion && b.want_distortion) ? detail::pick(a.distortion_family, b.distortion_family, t)
                                      : (a.want_distortion ? a.distortion_family : b.distortion_family);
                return r;
            }

            // The "no genre" preset: every profile at its defaults.
            inline GenrePresetPtr neutral() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Neutral"; p->description = "No genre processing."; p->family = "Neutral";
                return p;
            }

            // t = 0 gives `a`, t = 1 gives `b`; in between every number is interpolated and effects fade in/out.
            inline GenrePresetPtr blend(const GenrePreset& a, const GenrePreset& b, double t) {
                t = std::max(0.0, std::min(1.0, t));
                auto p = std::make_shared<GenrePreset>();
                p->name = t <= 0.0 ? a.name : (t >= 1.0 ? b.name : a.name + " / " + b.name);
                p->description = t >= 0.5 ? b.description : a.description;
                p->family = t >= 0.5 ? b.family : a.family;
                p->aliases = t >= 0.5 ? b.aliases : a.aliases;
                p->bpm_min = detail::lerp(a.bpm_min, b.bpm_min, t); p->bpm_max = detail::lerp(a.bpm_max, b.bpm_max, t);
                p->tempo_scale = detail::glerp(a.tempo_scale, b.tempo_scale, t);
                p->default_profile = blend(a.default_profile, b.default_profile, t);
                p->bass_profile = blend(a.profile_for(Role::Bass), b.profile_for(Role::Bass), t);
                p->has_bass_profile = a.has_bass_profile || b.has_bass_profile;
                p->percussion_profile = blend(a.profile_for(Role::Percussion), b.profile_for(Role::Percussion), t);
                p->has_percussion_profile = a.has_percussion_profile || b.has_percussion_profile;
                return p;
            }

            // How much of a genre to apply: 1 = the preset itself, 0 = none, 0.5 = halfway between neutral and the preset.
            inline GenrePresetPtr scaled(const GenrePresetPtr& preset, double strength) {
                if (!preset || strength >= 1.0) return preset;
                auto n = neutral(); auto p = std::const_pointer_cast<GenrePreset>(blend(*n, *preset, strength));
                p->name = preset->name + " (" + std::to_string(static_cast<int>(std::lround(std::max(0.0, strength) * 100.0))) + "%)";
                p->family = preset->family; p->aliases = preset->aliases; p->description = preset->description;
                return p;
            }

        }
    }
}
