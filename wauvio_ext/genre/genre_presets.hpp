#pragma once

#include "genre_types.hpp"

namespace wauvio {
    namespace genre {
        namespace presets {

            inline GenrePresetPtr classical() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Classical";
                p->description = "Expressive human timing, legato phrasing, hall reverb.";

                RoleProfile d;
                d.velocity_variance   = 0.05;
                d.timing_humanize_sec = 0.006;
                d.accent_pattern       = {1.0, 0.72, 0.86, 0.72};
                d.force_articulation  = true;
                d.articulation         = audio::Articulation::Legato;
                d.fallback_gate_ratio  = 0.98;
                d.want_reverb          = true;
                d.reverb_room = 0.62f; d.reverb_wet = 0.28f; d.reverb_damping = 0.4f;
                p->default_profile = d;
                return p;
            }

            inline GenrePresetPtr jazz() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Jazz";
                p->description = "Swung eighths, backbeat accents, loose human timing.";

                RoleProfile d;
                d.swing_amount         = 0.58;
                d.timing_humanize_sec  = 0.012;
                d.velocity_variance    = 0.10;
                d.accent_pattern        = {0.88, 1.0, 0.82, 1.0};
                d.want_reverb           = true;
                d.reverb_room = 0.4f; d.reverb_wet = 0.16f;
                p->default_profile = d;

                RoleProfile perc = d;
                perc.thin_probability = 0.10;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr rock() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Rock";
                p->description = "Straight rhythm, punchy backbeat accents, driven guitars.";

                RoleProfile d;
                d.timing_humanize_sec = 0.004;
                d.accent_pattern        = {0.88, 1.0, 0.9, 1.0};
                d.force_articulation   = true;
                d.articulation          = audio::Articulation::Accent;
                d.fallback_gate_ratio   = 0.9;
                d.want_distortion       = true;
                d.distortion_drive      = 2.2f;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.articulation = audio::Articulation::Marcato;
                bass.want_distortion = false;
                p->set_bass(bass);

                return p;
            }

            inline GenrePresetPtr metal() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Metal";
                p->description = "Tight/mechanical timing, heavy distortion, tightened low end.";
                p->tempo_scale = 1.05;

                RoleProfile d;
                d.timing_humanize_sec  = 0.0015;
                d.quantize_grid_fraction = 0.25;
                d.accent_pattern         = {0.82, 1.0, 0.85, 1.0};
                d.force_articulation    = true;
                d.articulation           = audio::Articulation::Accent;
                d.fallback_gate_ratio    = 0.85;
                d.want_distortion        = true;
                d.distortion_drive       = 3.4f;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.filter_cutoff_mult = 0.82;
                bass.want_distortion    = false;
                p->set_bass(bass);

                RoleProfile perc = d;
                perc.want_distortion = false;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr house() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "House";
                p->description = "Quantized four-on-the-floor groove, bright low end, light shuffle.";
                p->tempo_scale = 1.04;

                RoleProfile d;
                d.quantize_grid_fraction = 0.25;
                d.swing_amount           = 0.08;
                d.filter_cutoff_mult     = 1.15;
                d.want_delay             = true;
                d.delay_time_ms = 220.0; d.delay_feedback = 0.22f; d.delay_wet = 0.16f;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.filter_cutoff_mult = 1.3;
                bass.want_delay = false;
                p->set_bass(bass);

                RoleProfile perc = d;
                perc.swing_amount = 0.0;
                perc.want_delay = false;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr techno() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Techno";
                p->description = "Perfectly quantized, mechanical groove, driving and dark.";
                p->tempo_scale = 1.08;

                RoleProfile d;
                d.quantize_grid_fraction = 0.25;
                d.swing_amount           = 0.0;
                d.timing_humanize_sec    = 0.0;
                d.filter_cutoff_mult     = 0.9;
                d.accent_pattern          = {1.0, 0.8, 1.0, 0.8};
                p->default_profile = d;

                RoleProfile bass = d;
                bass.filter_cutoff_mult = 0.75;
                p->set_bass(bass);

                RoleProfile perc = d;
                perc.thin_probability = 0.05;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr drum_and_bass() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Drum & Bass";
                p->description = "Fast tempo, sub-register bass, shuffled breakbeat feel.";
                p->tempo_scale = 1.35;

                RoleProfile d;
                d.timing_humanize_sec = 0.003;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.register_shift_semitones = -12;
                bass.filter_cutoff_mult       = 0.7;
                bass.detune_cents_delta       = -2.0;
                p->set_bass(bass);

                RoleProfile perc = d;
                perc.swing_amount     = 0.18;
                perc.thin_probability = 0.08;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr chiptune() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Chiptune";
                p->description = "Bright, quantized, dry 8-bit-style character.";

                RoleProfile d;
                d.register_shift_semitones = 12;
                d.quantize_grid_fraction   = 0.25;
                d.timing_humanize_sec      = 0.0;
                d.filter_cutoff_mult       = 1.2;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.register_shift_semitones = 0;
                bass.filter_cutoff_mult       = 1.0;
                p->set_bass(bass);

                RoleProfile perc = d;
                perc.register_shift_semitones = 0;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr ambient() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Ambient";
                p->description = "Sparse, slow, wide, and reverberant.";
                p->tempo_scale = 0.85;

                RoleProfile d;
                d.thin_probability      = 0.22;
                d.timing_humanize_sec   = 0.02;
                d.force_articulation    = true;
                d.articulation           = audio::Articulation::Legato;
                d.fallback_gate_ratio    = 1.0;
                d.stereo_width_override = 0.6;
                d.want_reverb            = true;
                d.reverb_room = 0.88f; d.reverb_wet = 0.5f; d.reverb_damping = 0.3f;
                d.want_delay              = true;
                d.delay_time_ms = 560.0; d.delay_feedback = 0.4f; d.delay_wet = 0.32f;
                p->default_profile = d;
                return p;
            }

            inline GenrePresetPtr touhou() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Touhou";
                p->description = "Bright, fast, driving fusion feel (melodic+rock elements).";
                p->tempo_scale = 1.12;

                RoleProfile d;
                d.quantize_grid_fraction = 0.25;
                d.timing_humanize_sec    = 0.002;
                d.filter_cutoff_mult     = 1.25;
                d.velocity_scale         = 1.0;
                d.accent_pattern          = {1.0, 0.85, 0.95, 0.85};
                d.want_distortion         = true;
                d.distortion_drive        = 1.6f;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.filter_cutoff_mult = 1.0;
                bass.want_distortion = true;
                bass.distortion_drive = 1.4f;
                p->set_bass(bass);

                return p;
            }

            inline GenrePresetPtr funk() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Funk";
                p->description = "Syncopated, dry, ghost-note-heavy groove that hits hard on beat one.";

                RoleProfile d;
                d.swing_amount        = 0.18;
                d.thin_probability    = 0.12;
                d.velocity_variance   = 0.08;
                d.accent_pattern       = {1.0, 0.55, 0.68, 0.55};
                p->default_profile = d;

                RoleProfile bass = d;
                bass.thin_probability = 0.05;
                p->set_bass(bass);

                RoleProfile perc = d;
                perc.thin_probability = 0.18;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr blues() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Blues";
                p->description = "Loose 12-bar-style shuffle feel, warm light overdrive, relaxed tempo.";
                p->tempo_scale = 0.95;

                RoleProfile d;
                d.swing_amount         = 0.45;
                d.timing_humanize_sec  = 0.015;
                d.velocity_variance    = 0.12;
                d.force_articulation   = true;
                d.articulation          = audio::Articulation::Legato;
                d.fallback_gate_ratio   = 1.0;
                d.want_distortion       = true;
                d.distortion_drive      = 1.35f;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.want_distortion = false;
                p->set_bass(bass);

                return p;
            }

            inline GenrePresetPtr reggae() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Reggae";
                p->description = "Off-beat skank emphasis and a bass that drags behind the beat.";
                p->tempo_scale = 0.92;

                RoleProfile d;
                d.downbeat_deemphasis = 0.45;
                d.timing_humanize_sec = 0.006;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.timing_offset_sec = 0.018;
                bass.downbeat_deemphasis = 0.0;
                p->set_bass(bass);

                RoleProfile perc = d;
                perc.downbeat_deemphasis = 0.55;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr trance() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Trance";
                p->description = "Uplifting, quantized, bright and wide, with slow filter movement.";
                p->tempo_scale = 1.06;

                RoleProfile d;
                d.quantize_grid_fraction = 0.25;
                d.filter_cutoff_mult     = 1.3;
                d.wobble_amount          = 0.25;
                d.stereo_width_override  = 0.5;
                d.accent_pattern          = {1.0, 0.75, 0.9, 0.75};
                d.want_delay              = true;
                d.delay_time_ms = 300.0; d.delay_feedback = 0.35f; d.delay_wet = 0.28f;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.wobble_amount = 0.0;
                bass.filter_cutoff_mult = 0.95;
                p->set_bass(bass);

                return p;
            }

            inline GenrePresetPtr dubstep() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Dubstep";
                p->description = "Heavy sub-bass wobble, sparse/spacious percussion, slower half-time feel.";
                p->tempo_scale = 0.78;

                RoleProfile d;
                d.thin_probability = 0.1;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.register_shift_semitones = -12;
                bass.filter_cutoff_mult       = 0.55;
                bass.wobble_amount            = 0.85;
                bass.want_distortion          = true;
                bass.distortion_drive         = 2.6f;
                p->set_bass(bass);

                RoleProfile perc = d;
                perc.thin_probability = 0.22;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr bossa_nova() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Bossa Nova";
                p->description = "Gentle syncopation, soft dynamics, nylon-guitar-and-hand-percussion feel.";
                p->tempo_scale = 0.97;

                RoleProfile d;
                d.swing_amount        = 0.22;
                d.timing_humanize_sec = 0.008;
                d.velocity_variance   = 0.06;
                d.accent_pattern       = {0.92, 0.78, 0.96, 0.78};
                p->default_profile = d;

                RoleProfile perc = d;
                perc.thin_probability = 0.06;
                p->set_percussion(perc);

                return p;
            }

            inline GenrePresetPtr hardcore() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Hardcore";
                p->description = "Very fast, perfectly mechanical, distorted electronic aggression.";
                p->tempo_scale = 1.45;

                RoleProfile d;
                d.quantize_grid_fraction = 0.25;
                d.timing_humanize_sec    = 0.0;
                d.accent_pattern          = {1.0, 0.7, 0.85, 0.7};
                d.want_distortion         = true;
                d.distortion_drive        = 3.6f;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.filter_cutoff_mult = 0.7;
                bass.distortion_drive = 3.0f;
                p->set_bass(bass);

                return p;
            }

            inline GenrePresetPtr speedcore() {
                auto p = std::make_shared<GenrePreset>();
                p->name = "Speedcore";
                p->description = "Extreme tempo, hyper-mechanical quantization, maximal distortion.";
                p->tempo_scale = 2.1;

                RoleProfile d;
                d.quantize_grid_fraction = 0.125;
                d.timing_humanize_sec    = 0.0;
                d.accent_pattern          = {1.0, 0.65, 0.8, 0.65};
                d.want_distortion         = true;
                d.distortion_drive        = 5.0f;
                p->default_profile = d;

                RoleProfile bass = d;
                bass.filter_cutoff_mult = 0.6;
                bass.distortion_drive = 4.2f;
                p->set_bass(bass);

                return p;
            }

        }
    }
}
