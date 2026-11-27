#pragma once

#include "../samples/sampling.hpp"
#include <map>
#include <unordered_map>
#include <cmath>
#include <memory>
#include <utility>

namespace wauvio {
    namespace music {
        struct TimbreRecipe {
            wauvio::osc::WaveShape osc1_shape   = wauvio::osc::WaveShape::BL_Sawtooth;
            wauvio::osc::WaveShape osc2_shape   = wauvio::osc::WaveShape::Sine;
            double    osc2_ratio   = 2.0;
            double    osc_mix      = 0.35;
            double    detune_cents = 4.0;

            bool      use_fm       = false;
            double    fm_ratio     = 2.0;
            double    fm_index     = 2.0;

            double noise_mix   = 0.0;
            double noise_attack_burst = 0.0;
            bool   pink_noise  = true;

            wauvio::env::DAHDSR envelope { 0.0, 0.015, 0.0, 0.12, 0.72, 0.18 };
            bool   percussive = false;

            double filter_cutoff     = 5000.0;
            double filter_q          = 0.8;
            bool   filter_env        = true;
            double filter_env_amount = 0.0;

            double vibrato_rate_hz     = 5.2;
            double vibrato_depth_cents = 0.0;
            double vibrato_delay_sec   = 0.18;
            double pitch_attack_cents  = 0.0;
            double pitch_attack_time   = 0.035;
            double fixed_pitch_hz      = -1.0;
            double pitch_instability   = 0.0;

            double formant1_hz = 0.0, formant1_gain_db = 0.0, formant1_q = 3.0;
            double formant2_hz = 0.0, formant2_gain_db = 0.0, formant2_q = 3.0;

            double stereo_width = 0.18;
            double gain          = 0.9;

            std::map<Articulation, double> art_decay_scale;
            std::map<Articulation, double> art_noise_boost;
            std::map<Articulation, double> art_pitch_shift;
            std::map<Articulation, double> art_filter_scale;

            std::vector<ModRoute> modulation;

            struct Mode { double ratio; double amp; double t60; };
            std::vector<Mode> modes;
            double mode_decay_pitch_exp = 0.6;
            double mode_vel_brightness  = 0.7;

            // Wind-instrument model (air jet, single reed, double reed, lip reed). Opt-in: with kind == None (the default) the
            // legacy subtractive path runs and nothing below is read. See render_wind() for how the fields are used.
            struct Wind {
                enum class Kind { None, AirJet, SingleReed, DoubleReed, LipReed };
                struct Res { double hz = 0.0, gain_db = 0.0, q = 2.0; };
                Kind kind = Kind::None;

                // source spectrum: harmonic k has amplitude k^-tilt * g^(k-1); tilt and g move with blowing pressure (0..1)
                double tilt_soft = 1.0, tilt_loud = 1.0;
                double g_soft = 0.5, g_loud = 0.8, g_pow = 1.0;
                double tilt_per_octave = 0.0;        // added to tilt per octave above C4
                double fund_drop_db = 0.0;           // fundamental weakens by this much from pp to ff (reeds)

                // bore: even harmonics suppressed below even_open_hz (cylinder closed at one end), lattice/bell cutoff, radiation
                double even_low = 1.0, even_open_hz = 1500.0;
                double cutoff_hz = 6000.0, cutoff_order = 2.0, cutoff_loud_gain = 0.0;
                double bell_hz = 0.0, bell_depth = 0.0;
                Res res[4];                          // bore / bell / mouthpiece resonances
                Res mute_res;                        // extra resonance engaged by Muted / PalmMute
                double mute_cutoff_mult = 0.55;

                // breath: band-limited turbulence relative to tone level, plus a short attack chiff
                double breath_db = -40.0, breath_center_hz = 3000.0, breath_center_mult = 0.0, breath_q = 0.8;
                double breath_follow = 0.5, breath_tone_mod = 0.0;
                double chiff_db = -100.0, chiff_ms = 20.0, chiff_center_hz = 3000.0, chiff_q = 0.9;

                // blowing pressure envelope
                double attack_ms = 40.0, attack_shape = 1.0, attack_vel_exp = 0.0;
                double overshoot = 0.0, overshoot_ms = 100.0, release_ms = 80.0;

                // pitch behaviour
                double scoop_cents = 0.0, scoop_ms = 40.0, pressure_pitch_cents = 0.0;
                double vib_rate_hz = 5.2, vib_depth_cents = 0.0, vib_delay_s = 0.3, vib_onset_s = 0.3, vib_am = 0.0, vib_wander = 0.1;
                double jitter_cents = 0.0, amp_jitter = 0.0;

                // sections: several detuned, staggered voices
                int    ensemble_voices = 1;
                double ensemble_cents = 0.0, ensemble_stagger_ms = 0.0;

                double power_comp = 0.5;             // 0 = brightness adds loudness, 1 = loudness held constant as spectrum brightens
                double level = 0.28;                 // overall output level before recipe.gain
            } wind;

            // Bowed-string model (Helmholtz sawtooth, body resonances, bow noise, scrape, sections). Opt-in like `wind`.
            struct Bowed {
                enum class Kind { None, Bowed };
                Kind kind = Kind::None;
                double tilt_soft = 1.3, tilt_loud = 0.9, tilt_per_octave = 0.0;     // harmonic k ~ k^-tilt (ideal Helmholtz = 1)
                double bow_pos = 0.12, bow_tilt_per_beta = 6.0;                      // bow-bridge distance / length; nearer the bridge is brighter
                double cutoff_hz = 9000.0, cutoff_order = 1.5, mute_cutoff_mult = 0.5;
                Wind::Res res[8];                                                    // body modes, bridge hill
                double bow_noise_db = -34.0, bow_noise_center_hz = 3500.0, bow_noise_q = 0.7, bow_noise_follow = 1.0;
                double scrape_db = -11.0, scrape_ms = 30.0, scrape_center_hz = 2500.0;
                double onset_tilt = 1.5, onset_ms = 35.0;                            // extra tilt that decays while the string settles into Helmholtz motion
                double attack_ms = 60.0, attack_shape = 1.0, attack_vel_exp = 0.5, overshoot = 0.05, overshoot_ms = 100.0, release_ms = 120.0;
                double scoop_cents = 6.0, scoop_ms = 25.0;
                double vib_rate_hz = 5.5, vib_depth_cents = 18.0, vib_delay_s = 0.2, vib_onset_s = 0.3, vib_am = 0.0, vib_wander = 0.15;
                double jitter_cents = 1.0, amp_jitter = 0.01;
                double pluck_pos = 0.15, pluck_tau_s = 0.5, pluck_hf_damp = 0.25, pluck_gain = 1.0, pluck_tilt = 1.2;
                double pluck_thump_db = -18.0, pluck_thump_hz = 300.0, pluck_wood_db = -6.0, pluck_wood_hz = 1800.0;
                int    ensemble_voices = 1;
                double ensemble_cents = 0.0, ensemble_stagger_ms = 0.0, ensemble_vib_spread = 0.12, ensemble_drift_cents = 0.0;
                double power_comp = 0.5, level = 0.26;
            } bowed;
        };

        namespace detail_wind {
            struct Biquad {
                double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
                static Biquad bandpass(double f0, double q, double sr) {
                    f0 = std::max(20.0, std::min(f0, 0.45 * sr));
                    const double w = 2.0 * M_PI * f0 / sr, al = std::sin(w) / (2.0 * std::max(0.1, q)), c = std::cos(w), a0 = 1.0 + al;
                    Biquad b; b.b0 = al / a0; b.b1 = 0.0; b.b2 = -al / a0; b.a1 = -2.0 * c / a0; b.a2 = (1.0 - al) / a0;
                    return b;
                }
                double tick(double x) { const double y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
            };
            inline double smooth01(double x) { x = std::max(0.0, std::min(1.0, x)); return x * x * (3.0 - 2.0 * x); }
            inline double lerp(double a, double b, double t) { return a + (b - a) * t; }
            inline double res_mag(double f, const TimbreRecipe::Wind::Res& r) {
                if (r.gain_db == 0.0 || r.hz <= 0.0) return 1.0;
                const double G = std::pow(10.0, r.gain_db / 20.0), x = f / r.hz - r.hz / f;
                return 1.0 + (G - 1.0) / std::sqrt(1.0 + r.q * r.q * x * x);
            }
            // rms-calibration of a band-passed white-noise source so that "breath_db" is relative to a unit-rms tone
            inline double bp_noise_calib(const Biquad& proto, uint32_t seed) {
                Biquad b = proto; wauvio::osc::NoiseGenerator ng(seed); double s2 = 0.0; const int N = 16384;
                for (int i = 0; i < N; ++i) { const double y = b.tick(ng.tick_white()); if (i > 512) s2 += y * y; }
                return 1.0 / std::sqrt(std::max(1e-12, s2 / (N - 512)));
            }

            inline StereoBuffer render(const TimbreRecipe& r, const Note& note, int sample_rate) {
                using W = TimbreRecipe::Wind;
                const W& w = r.wind;
                const double duration = std::max(0.02, note.duration);
                const size_t n = static_cast<size_t>(duration * sample_rate);
                if (n == 0) return StereoBuffer();
                const double sr = static_cast<double>(sample_rate);

                const Articulation art = note.articulation;
                auto get = [](const std::map<Articulation, double>& m, Articulation a, double def) { auto it = m.find(a); return it != m.end() ? it->second : def; };
                double filter_scale = get(r.art_filter_scale, art, 1.0);
                const double pitch_shift_semi = get(r.art_pitch_shift, art, 0.0) + (art == Articulation::Harmonic ? 12.0 : 0.0);
                double att_mult = 1.0, over_add = 0.0, chiff_mult = 1.0, rel_mult = 1.0, tone_scale = 1.0, breath_extra = 0.0, tilt_add = 0.0;
                double gate_len = duration, flutter = 0.0, fall_cents = 0.0;
                bool muted = false;
                switch (art) {
                    case Articulation::Staccato:  att_mult = 0.6; chiff_mult = 1.3; gate_len = std::min(duration, 0.26); rel_mult = 0.35; break;
                    case Articulation::ShortStab: att_mult = 0.5; chiff_mult = 1.3; gate_len = std::min(duration, 0.12); rel_mult = 0.3; break;
                    case Articulation::Marcato:   att_mult = 0.55; over_add = 0.12; chiff_mult = 1.5; break;
                    case Articulation::Legato:    att_mult = 1.7; chiff_mult = 0.15; break;
                    case Articulation::FlutterTongue: flutter = 1.0; breath_extra = 0.04; break;
                    case Articulation::Fall:      fall_cents = -700.0; break;
                    case Articulation::Doit:      fall_cents = 450.0; break;
                    case Articulation::Muted: case Articulation::PalmMute: muted = true; break;
                    case Articulation::Harmonic:  tilt_add = 1.0; break;
                    case Articulation::Breath:    tone_scale = 0.15; breath_extra = 0.35; break;
                    case Articulation::Whisper:   tone_scale = 0.05; breath_extra = 0.55; break;
                    default: break;
                }
                if (art == Articulation::Hard) filter_scale *= 1.3;
                if (art == Articulation::Soft) filter_scale *= 0.7;

                const double base_freq = wauvio::theory::midi_to_freq(note.midi_note) * std::pow(2.0, pitch_shift_semi / 12.0);
                const double velocity = dynamics_to_velocity(note.dynamics) * std::max(0.0, std::min(1.0, note.expression));
                const bool has_glide = (note.glide_from_midi >= 0);
                const double glide_start_cents = has_glide ? 1200.0 * std::log2(wauvio::theory::midi_to_freq(note.glide_from_midi) / std::max(1.0, base_freq)) : 0.0;
                const double glide_time = std::max(0.001, note.glide_time);

                auto eval_mod_sum = [&](ModTarget target, double t) -> double {
                    double sum = 0.0;
                    for (auto& route : r.modulation) {
                        if (route.target != target) continue;
                        double v = 0.0;
                        switch (route.source) {
                            case ModSource::Velocity:    v = velocity; break;
                            case ModSource::ModWheel:    v = note.mod_wheel; break;
                            case ModSource::Aftertouch:  v = note.aftertouch; break;
                            case ModSource::Expression:  v = note.expression; break;
                            case ModSource::KeyPosition: v = note.midi_note / 127.0; break;
                            case ModSource::LFO1:        v = std::sin(TWO_PI * 5.0 * t); break;
                            case ModSource::LFO2:        v = std::sin(TWO_PI * 0.5 * t); break;
                            case ModSource::Envelope1:   v = std::max(0.0, 1.0 - t / duration); break;
                        }
                        sum += v * route.amount;
                    }
                    return sum;
                };

                // ---- spectrum: harmonic amplitudes at fundamental f0 and blowing pressure qb ---------------------------------
                static const int KMAX = 96;
                double logk[KMAX + 2]; for (int k = 1; k <= KMAX + 1; ++k) logk[k] = std::log(static_cast<double>(k));
                const double cutoff_base = w.cutoff_hz * (muted ? w.mute_cutoff_mult : 1.0);
                const double global_fc = std::max(60.0, r.filter_cutoff * filter_scale);   // recipe.filter_cutoff stays a live tone control (genre presets use it)
                const double fund_lin = std::pow(10.0, -w.fund_drop_db / 20.0);
                auto spectrum = [&](double f0, double qb, double* A) -> double {
                    const double qs = std::max(0.0, std::min(1.0, qb));
                    double tilt = lerp(w.tilt_soft, w.tilt_loud, qs) + w.tilt_per_octave * std::log2(std::max(20.0, f0) / 261.6256) + tilt_add;
                    tilt = std::max(0.0, tilt);
                    const double g = std::max(0.01, std::min(1.0, lerp(w.g_soft, w.g_loud, std::pow(qs, w.g_pow))));
                    const double lg = std::log(g);
                    const double fc = std::max(100.0, cutoff_base * (1.0 + w.cutoff_loud_gain * qs));
                    double power = 0.0;
                    const int K = std::max(1, std::min(KMAX, static_cast<int>(0.45 * sr / std::max(20.0, f0))));
                    for (int k = 1; k <= KMAX; ++k) {
                        if (k > K) { A[k] = 0.0; continue; }
                        const double f = k * f0;
                        double a = std::exp(-tilt * logk[k] + (k - 1) * lg);
                        if (k == 1) a *= lerp(1.0, fund_lin, qs);
                        if ((k & 1) == 0 && w.even_low < 1.0) {
                            const double x = smooth01((f / w.even_open_hz - 0.3) / 0.7);
                            a *= w.even_low + (1.0 - w.even_low) * x;
                        }
                        a /= std::sqrt(1.0 + std::pow(f / fc, 2.0 * w.cutoff_order));
                        if (w.bell_depth > 0.0 && w.bell_hz > 0.0) {
                            const double x = f / w.bell_hz, rad = x / std::sqrt(1.0 + x * x);
                            a *= (1.0 - w.bell_depth) + w.bell_depth * rad;
                        }
                        for (int j = 0; j < 4; ++j) a *= res_mag(f, w.res[j]);
                        if (muted) a *= res_mag(f, w.mute_res);
                        a /= std::sqrt(1.0 + std::pow(f / global_fc, 4.0));
                        A[k] = a; power += a * a;
                    }
                    return power;
                };
                double Aref[KMAX + 2];
                const double P_ref = std::max(1e-12, spectrum(base_freq, 0.66, Aref));
                const double norm = 1.0 / std::sqrt(0.5 * P_ref);

                // ---- envelope timing ----------------------------------------------------------------------------------------
                const double rel_s = std::max(0.004, std::min(0.45 * duration, w.release_ms * 0.001 * rel_mult));
                const double gate_end = std::min(gate_len, duration - rel_s);
                const double attack_s = std::max(0.003, std::min(0.6 * std::max(0.01, gate_end),
                                        w.attack_ms * 0.001 * att_mult * std::pow(0.66 / std::max(0.1, velocity), w.attack_vel_exp)));
                const double over = w.overshoot + over_add, over_tau = std::max(0.01, w.overshoot_ms * 0.001);
                const double scoop_s = std::max(0.005, w.scoop_ms * 0.001);

                // ---- voices ---------------------------------------------------------------------------------------------------
                const int nv = std::max(1, w.ensemble_voices);
                Buffer mono(n);
                std::vector<double> voice(n);
                std::fill(mono.begin(), mono.end(), 0.0f);
                for (int v = 0; v < nv; ++v) {
                    const double vpos = (nv == 1) ? 0.0 : (static_cast<double>(v) / (nv - 1) - 0.5);          // -0.5 .. +0.5
                    const double v_detune = vpos * w.ensemble_cents;
                    const uint32_t seed = static_cast<uint32_t>(note.midi_note * 7919u + 17u + static_cast<uint32_t>(v) * 1013u);
                    const double stagger = (nv == 1) ? 0.0 : w.ensemble_stagger_ms * 0.001 * (0.5 + 0.5 * std::sin(1.7 * v + 0.9));
                    wauvio::osc::NoiseGenerator noise(seed), chiff_noise(seed ^ 0x9E3779B9u), jit(seed + 5u), rnd(seed + 11u);

                    const double bc = std::max(w.breath_center_hz, w.breath_center_mult * base_freq);
                    Biquad breath_bp = Biquad::bandpass(bc, w.breath_q, sr);
                    Biquad chiff_bp  = Biquad::bandpass(w.chiff_center_hz, w.chiff_q, sr);
                    const double breath_cal = bp_noise_calib(breath_bp, seed), chiff_cal = bp_noise_calib(chiff_bp, seed ^ 0x9E3779B9u);
                    const double breath_lin = std::pow(10.0, w.breath_db / 20.0) + r.noise_mix * 0.35 + breath_extra;
                    const double chiff_lin  = std::pow(10.0, w.chiff_db / 20.0) * chiff_mult;
                    const double chiff_tau  = std::max(0.002, w.chiff_ms * 0.001 / 2.5);

                    double phase = 0.0, lfo_ph = 2.0 * M_PI * (0.37 * v), rate_walk = 0.0, jn = 0.0, an = 0.0;
                    double Acur[KMAX + 2], Adel[KMAX + 2], Atgt[KMAX + 2];
                    for (int k = 0; k <= KMAX + 1; ++k) { Acur[k] = 0.0; Adel[k] = 0.0; Atgt[k] = 0.0; }
                    const int BLK = 32;
                    bool primed = false;

                    for (size_t i = 0; i < n; ++i) {
                        const double t = static_cast<double>(i) / sr, ton = t - stagger;
                        double a = 0.0;
                        if (ton > 0.0) {
                            a = (ton < attack_s) ? std::pow(ton / attack_s, w.attack_shape) * (1.0 + over)
                                                 : 1.0 + over * std::exp(-(ton - attack_s) / over_tau);
                            if (t > gate_end) { const double rel = (t - gate_end) / rel_s; a *= (rel >= 1.0) ? 0.0 : 0.5 + 0.5 * std::cos(M_PI * rel); }
                        }
                        const double qb = velocity * a;

                        // pitch (cents)
                        double cents = v_detune;
                        if (note.bend_at(t) != 0.0) cents += 100.0 * note.bend_at(t);
                        if (has_glide && t < glide_time) cents += glide_start_cents * (1.0 - t / glide_time);
                        if (ton > 0.0 && ton < scoop_s) cents -= w.scoop_cents * (1.0 - smooth01(ton / scoop_s));
                        cents += w.pressure_pitch_cents * (qb - velocity);
                        const double vib_scale = 1.0 + eval_mod_sum(ModTarget::VibratoDepth, t);
                        double vib_wave = 0.0, vib_ramp = 0.0;
                        if (w.vib_depth_cents > 0.0 && t > w.vib_delay_s) {
                            rate_walk += (rnd.tick_white() - rate_walk) * 0.0002;
                            lfo_ph += TWO_PI * w.vib_rate_hz * (1.0 + w.vib_wander * 3.0 * rate_walk) / sr;
                            vib_ramp = smooth01((t - w.vib_delay_s) / std::max(0.01, w.vib_onset_s));
                            vib_wave = std::sin(lfo_ph);
                            cents += vib_wave * w.vib_depth_cents * vib_ramp * vib_scale;
                        }
                        if (w.jitter_cents > 0.0) { jn += (jit.tick_white() - jn) * 0.002; cents += w.jitter_cents * jn * 55.0; }
                        if (fall_cents != 0.0) { const double s0 = (fall_cents < 0.0) ? 0.55 : 0.62; cents += fall_cents * smooth01((t / duration - s0) / (1.0 - s0 - 0.04)); }
                        cents += eval_mod_sum(ModTarget::Pitch, t) * 100.0;
                        const double f = base_freq * std::pow(2.0, cents / 1200.0);
                        phase += TWO_PI * f / sr; if (phase > TWO_PI) phase -= TWO_PI;

                        // spectrum update per block (amplitudes interpolate linearly inside the block)
                        if (static_cast<int>(i % BLK) == 0) {
                            const double qs = std::max(qb, 0.0);
                            double P = spectrum(f, qs, Atgt);
                            const double comp = std::pow(P_ref / std::max(P, 1e-12), 0.5 * w.power_comp);
                            const double scale = norm * comp;
                            if (!primed) { for (int k = 1; k <= KMAX; ++k) Acur[k] = Atgt[k] * scale; primed = true; }
                            for (int k = 1; k <= KMAX; ++k) { const double nxt = Atgt[k] * scale; Adel[k] = (nxt - Acur[k]) / BLK; Atgt[k] = nxt; }
                        }
                        const double c1 = std::cos(phase), s1 = std::sin(phase);
                        double sk_prev = 0.0, sk = s1, tone = 0.0;
                        for (int k = 1; k <= KMAX; ++k) {
                            tone += Acur[k] * sk;
                            const double sn = 2.0 * c1 * sk - sk_prev; sk_prev = sk; sk = sn;
                            Acur[k] += Adel[k];
                        }

                        // amplitude behaviour: vibrato AM, reed/lip amplitude jitter, flutter-tongue
                        double am = 1.0 + w.vib_am * vib_wave * vib_ramp;
                        if (w.amp_jitter > 0.0) { an += (jit.tick_white() - an) * 0.004; am *= 1.0 + w.amp_jitter * an * 40.0; }
                        if (flutter > 0.0) am *= 1.0 - 0.7 * flutter * (0.5 + 0.5 * std::sin(TWO_PI * 24.0 * t));

                        // breath turbulence follows pressure (and, optionally, is modulated at the tone frequency)
                        double nz = 0.0;
                        if (breath_lin > 0.0 && ton > 0.0) {
                            nz = breath_bp.tick(noise.tick_white()) * breath_cal * breath_lin
                               * std::pow(std::max(qb, 0.02) / 0.66, w.breath_follow) * (1.0 + w.breath_tone_mod * c1);
                        }
                        double ch = 0.0;
                        if (chiff_lin > 0.0 && ton > 0.0) ch = chiff_bp.tick(chiff_noise.tick_white()) * chiff_cal * chiff_lin * std::exp(-ton / chiff_tau);

                        const double mod_volume = std::max(0.0, 1.0 + eval_mod_sum(ModTarget::Volume, t));
                        voice[i] = ((tone * tone_scale * am + nz) * velocity * a + ch * velocity) * mod_volume;
                    }
                    for (size_t i = 0; i < n; ++i) mono[i] += static_cast<float>(voice[i] / std::sqrt(static_cast<double>(nv)));
                }
                apply_gain(mono, static_cast<float>(r.gain * w.level));
                for (auto& v : mono) v = static_cast<float>(std::tanh(static_cast<double>(v)));   // soft ceiling: brassy ff peaks round off instead of hard-clipping
                clamp_buffer(mono);

                StereoBuffer out = pan_mono(mono, 0.0f, sample_rate);
                if (r.stereo_width > 0.0) {
                    StereoBuffer widened = haas_widen(mono, r.stereo_width * 18.0, sample_rate);
                    for (size_t i = 0; i < out.size(); ++i)
                        out.R[i] = out.R[i] * static_cast<float>(1.0 - r.stereo_width) + widened.R[i] * static_cast<float>(r.stereo_width);
                }
                const size_t f = std::min(out.L.size() / 4, static_cast<size_t>(0.006 * sample_rate));
                for (size_t i = 0; i < f; ++i) {
                    const float g = static_cast<float>(0.5 - 0.5 * std::cos(M_PI * static_cast<double>(i) / static_cast<double>(std::max<size_t>(1, f))));
                    out.L[out.L.size() - 1 - i] *= g; out.R[out.R.size() - 1 - i] *= g;
                }
                return out;
            }
        }

        namespace detail_bowed {
            inline StereoBuffer render(const TimbreRecipe& r, const Note& note, int sample_rate) {
                using detail_wind::Biquad; using detail_wind::smooth01; using detail_wind::lerp; using detail_wind::res_mag; using detail_wind::bp_noise_calib;
                const TimbreRecipe::Bowed& b = r.bowed;
                const double duration = std::max(0.02, note.duration);
                const size_t n = static_cast<size_t>(duration * sample_rate);
                if (n == 0) return StereoBuffer();
                const double sr = static_cast<double>(sample_rate);

                const Articulation art = note.articulation;
                auto get = [](const std::map<Articulation, double>& m, Articulation a, double def) { auto it = m.find(a); return it != m.end() ? it->second : def; };
                double filter_scale = get(r.art_filter_scale, art, 1.0);
                const double pitch_shift_semi = get(r.art_pitch_shift, art, 0.0) + (art == Articulation::Harmonic ? 12.0 : 0.0);
                double att_mult = 1.0, scrape_mult = 1.0, noise_mult = 1.0, tilt_add = 0.0, bow_pos = b.bow_pos, gate_len = duration, ring_s = 0.0, rel_mult = 1.0;
                bool pluck = false, wood = false, tremolo = false, muted = false;
                switch (art) {
                    case Articulation::Staccato:  att_mult = 0.6; scrape_mult = 1.5; gate_len = std::min(duration, 0.28); ring_s = 0.12; break;
                    case Articulation::ShortStab: att_mult = 0.5; scrape_mult = 1.6; gate_len = std::min(duration, 0.14); ring_s = 0.10; break;
                    case Articulation::Spiccato:  att_mult = 0.25; scrape_mult = 2.0; gate_len = std::min(duration, 0.10); ring_s = 0.14; break;
                    case Articulation::Tremolo:   tremolo = true; break;
                    case Articulation::Harmonic:  tilt_add = 2.0; noise_mult = 1.3; break;       // flautando natural harmonic: nearly sinusoidal
                    case Articulation::SulPonticello: bow_pos = 0.035; scrape_mult = 1.4; noise_mult = 2.2; break;
                    case Articulation::Pizzicato: pluck = true; break;
                    case Articulation::ColLegno:  pluck = true; wood = true; break;
                    case Articulation::Muted: case Articulation::PalmMute: muted = true; break;
                    case Articulation::Legato:    att_mult = 1.6; scrape_mult = 0.3; break;
                    case Articulation::Whisper: case Articulation::Breath: noise_mult = 4.0; tilt_add = 1.0; break;
                    default: break;
                }
                if (art == Articulation::Hard) filter_scale *= 1.3;
                if (art == Articulation::Soft) filter_scale *= 0.7;

                const double base_freq = wauvio::theory::midi_to_freq(note.midi_note) * std::pow(2.0, pitch_shift_semi / 12.0);
                const double velocity = dynamics_to_velocity(note.dynamics) * std::max(0.0, std::min(1.0, note.expression));
                const bool has_glide = (note.glide_from_midi >= 0);
                const double glide_start_cents = has_glide ? 1200.0 * std::log2(wauvio::theory::midi_to_freq(note.glide_from_midi) / std::max(1.0, base_freq)) : 0.0;
                const double glide_time = std::max(0.001, note.glide_time);

                auto eval_mod_sum = [&](ModTarget target, double t) -> double {
                    double sum = 0.0;
                    for (auto& route : r.modulation) {
                        if (route.target != target) continue;
                        double v = 0.0;
                        switch (route.source) {
                            case ModSource::Velocity:    v = velocity; break;
                            case ModSource::ModWheel:    v = note.mod_wheel; break;
                            case ModSource::Aftertouch:  v = note.aftertouch; break;
                            case ModSource::Expression:  v = note.expression; break;
                            case ModSource::KeyPosition: v = note.midi_note / 127.0; break;
                            case ModSource::LFO1:        v = std::sin(TWO_PI * 5.0 * t); break;
                            case ModSource::LFO2:        v = std::sin(TWO_PI * 0.5 * t); break;
                            case ModSource::Envelope1:   v = std::max(0.0, 1.0 - t / duration); break;
                        }
                        sum += v * route.amount;
                    }
                    return sum;
                };

                static const int KMAX = 96;
                double logk[KMAX + 2]; for (int k = 1; k <= KMAX + 1; ++k) logk[k] = std::log(static_cast<double>(k));
                const double cutoff_base = b.cutoff_hz * (muted ? b.mute_cutoff_mult : 1.0);
                const double global_fc = std::max(60.0, r.filter_cutoff * filter_scale);
                // body: string/bridge roll-off and resonances, evaluated at the partial's instantaneous frequency
                auto body = [&](double f) {
                    double a = 1.0 / std::sqrt(1.0 + std::pow(f / std::max(100.0, cutoff_base), 2.0 * b.cutoff_order));
                    for (int j = 0; j < 8; ++j) a *= res_mag(f, b.res[j]);
                    a /= std::sqrt(1.0 + std::pow(f / global_fc, 4.0));
                    return a;
                };
                // bowed spectrum: Helmholtz sawtooth k^-tilt; bow position, bow pressure (qb) and the onset scrape move the tilt
                auto bowed_spectrum = [&](double f0, double qb, double onset_extra, double* A) -> double {
                    const double qs = std::max(0.0, std::min(1.0, qb));
                    double tilt = lerp(b.tilt_soft, b.tilt_loud, qs) + b.tilt_per_octave * std::log2(std::max(20.0, f0) / 261.6256)
                                + b.bow_tilt_per_beta * (bow_pos - 0.12) + tilt_add + onset_extra;
                    tilt = std::max(0.0, tilt);
                    const int K = std::max(1, std::min(KMAX, static_cast<int>(0.45 * sr / std::max(20.0, f0))));
                    double power = 0.0;
                    for (int k = 1; k <= KMAX; ++k) {
                        if (k > K) { A[k] = 0.0; continue; }
                        const double f = k * f0, a = std::exp(-tilt * logk[k]) * body(f);
                        A[k] = a; power += a * a;
                    }
                    return power;
                };
                // plucked spectrum at t=0: pluck-position comb |sin(k pi x)|, k^-tilt (brighter with velocity), same body
                const double pluck_tilt = b.pluck_tilt + 0.8 * (1.0 - velocity);
                auto pluck_spectrum = [&](double f0, double* A) -> double {
                    const int K = std::max(1, std::min(KMAX, static_cast<int>(0.45 * sr / std::max(20.0, f0))));
                    double power = 0.0;
                    for (int k = 1; k <= KMAX; ++k) {
                        if (k > K) { A[k] = 0.0; continue; }
                        const double f = k * f0, comb = std::fabs(std::sin(M_PI * k * b.pluck_pos));
                        const double a = std::exp(-pluck_tilt * logk[k]) * (0.15 + 0.85 * comb) * body(f);
                        A[k] = a; power += a * a;
                    }
                    return power;
                };
                double Aref[KMAX + 2];
                const double P_ref = std::max(1e-12, pluck ? pluck_spectrum(base_freq, Aref) : bowed_spectrum(base_freq, 0.66, 0.0, Aref));
                const double norm = 1.0 / std::sqrt(0.5 * P_ref);

                const double rel_s = std::max(0.004, std::min(0.45 * duration, b.release_ms * 0.001 * rel_mult));
                const double gate_end = std::min(gate_len, duration - rel_s);
                const double attack_s = std::max(0.002, std::min(0.6 * std::max(0.01, gate_end),
                                        b.attack_ms * 0.001 * att_mult * std::pow(0.66 / std::max(0.1, velocity), b.attack_vel_exp)));
                const double over = b.overshoot, over_tau = std::max(0.01, b.overshoot_ms * 0.001);
                const double scoop_s = std::max(0.005, b.scoop_ms * 0.001);
                const double onset_s = std::max(0.004, b.onset_ms * 0.001);

                const int nv = (pluck) ? std::max(1, b.ensemble_voices) : std::max(1, b.ensemble_voices);
                std::vector<double> Lacc(n, 0.0), Racc(n, 0.0), voice(n);
                for (int v = 0; v < nv; ++v) {
                    const double vpos = (nv == 1) ? 0.0 : (static_cast<double>(v) / (nv - 1) - 0.5);        // -0.5 .. +0.5
                    const double v_detune = vpos * b.ensemble_cents;
                    const uint32_t seed = static_cast<uint32_t>(note.midi_note * 6007u + 29u + static_cast<uint32_t>(v) * 1543u);
                    const double hv = 0.5 + 0.5 * std::sin(2.3 * v + 0.7), hv2 = 0.5 + 0.5 * std::sin(4.1 * v + 1.9);
                    const double stagger = (nv == 1) ? 0.0 : b.ensemble_stagger_ms * 0.001 * hv;
                    const double rate_v = b.vib_rate_hz * (1.0 + b.ensemble_vib_spread * (2.0 * hv2 - 1.0));
                    const double drift_rate = 0.30 + 0.45 * hv2, drift_ph = 6.2831853 * hv;
                    wauvio::osc::NoiseGenerator noise(seed), scrape_noise(seed ^ 0x85EBCA6Bu), jit(seed + 3u), rnd(seed + 13u), thump_noise(seed + 77u);

                    Biquad bow_bp = Biquad::bandpass(b.bow_noise_center_hz, b.bow_noise_q, sr);
                    Biquad scr_bp = Biquad::bandpass(wood ? b.pluck_wood_hz : b.scrape_center_hz, 0.9, sr);
                    Biquad thump_bp = Biquad::bandpass(b.pluck_thump_hz, 1.2, sr);
                    const double bow_cal = bp_noise_calib(bow_bp, seed), scr_cal = bp_noise_calib(scr_bp, seed ^ 0x85EBCA6Bu), thump_cal = bp_noise_calib(thump_bp, seed + 77u);
                    const double bow_lin = (std::pow(10.0, b.bow_noise_db / 20.0) + r.noise_mix * 0.35) * noise_mult;
                    const double scr_lin = std::pow(10.0, (wood ? b.pluck_wood_db : b.scrape_db) / 20.0) * scrape_mult;
                    const double thump_lin = std::pow(10.0, b.pluck_thump_db / 20.0);
                    const double scr_tau = std::max(0.002, (wood ? 8.0 : b.scrape_ms) * 0.001 / 2.5);

                    double phase = 0.0, lfo_ph = 6.2831853 * hv2, rate_walk = 0.0, jn = 0.0, an = 0.0;
                    double Acur[KMAX + 2], Adel[KMAX + 2], Atgt[KMAX + 2];
                    for (int k = 0; k <= KMAX + 1; ++k) { Acur[k] = 0.0; Adel[k] = 0.0; Atgt[k] = 0.0; }
                    double A0[KMAX + 2]; for (int k = 0; k <= KMAX + 1; ++k) A0[k] = 0.0;
                    if (pluck) pluck_spectrum(base_freq * std::pow(2.0, v_detune / 1200.0), A0);
                    const int BLK = 32; bool primed = false;

                    for (size_t i = 0; i < n; ++i) {
                        const double t = static_cast<double>(i) / sr, ton = t - stagger;
                        double a = 0.0;
                        if (ton > 0.0) {
                            if (pluck) a = std::min(1.0, ton / 0.0012);
                            else {
                                a = (ton < attack_s) ? std::pow(ton / attack_s, b.attack_shape) * (1.0 + over) : 1.0 + over * std::exp(-(ton - attack_s) / over_tau);
                                if (t > gate_end) {
                                    const double rel = t - gate_end;
                                    a *= (ring_s > 0.0) ? std::exp(-rel / ring_s) : ((rel / rel_s >= 1.0) ? 0.0 : 0.5 + 0.5 * std::cos(M_PI * rel / rel_s));
                                }
                            }
                        }
                        const double qb = velocity * a;

                        double cents = v_detune;
                        if (note.bend_at(t) != 0.0) cents += 100.0 * note.bend_at(t);
                        if (has_glide && t < glide_time) cents += glide_start_cents * (1.0 - t / glide_time);
                        if (!pluck && ton > 0.0 && ton < scoop_s) cents -= b.scoop_cents * (1.0 - smooth01(ton / scoop_s));
                        double vib_wave = 0.0, vib_ramp = 0.0;
                        if (!pluck && b.vib_depth_cents > 0.0 && t > b.vib_delay_s) {
                            rate_walk += (rnd.tick_white() - rate_walk) * 0.0002;
                            lfo_ph += TWO_PI * rate_v * (1.0 + b.vib_wander * 3.0 * rate_walk) / sr;
                            vib_ramp = smooth01((t - b.vib_delay_s) / std::max(0.01, b.vib_onset_s));
                            vib_wave = std::sin(lfo_ph);
                            cents += vib_wave * b.vib_depth_cents * vib_ramp * (1.0 + eval_mod_sum(ModTarget::VibratoDepth, t));
                        }
                        if (nv > 1 && b.ensemble_drift_cents > 0.0) cents += b.ensemble_drift_cents * std::sin(TWO_PI * drift_rate * t + drift_ph);
                        if (!pluck && b.jitter_cents > 0.0) { jn += (jit.tick_white() - jn) * 0.002; cents += b.jitter_cents * jn * 55.0; }
                        cents += eval_mod_sum(ModTarget::Pitch, t) * 100.0;
                        const double f = base_freq * std::pow(2.0, cents / 1200.0);
                        phase += TWO_PI * f / sr; if (phase > TWO_PI) phase -= TWO_PI;

                        if (static_cast<int>(i % BLK) == 0) {
                            double scale;
                            if (pluck) {
                                // each partial decays at its own rate: high partials die first
                                const double tt = std::max(0.0, ton);
                                double P = 0.0; const double fscale = f / base_freq;
                                for (int k = 1; k <= KMAX; ++k) {
                                    const double tau = b.pluck_tau_s / (1.0 + b.pluck_hf_damp * (k - 1) * fscale);
                                    Atgt[k] = A0[k] * std::exp(-tt / std::max(0.003, tau)); P += Atgt[k] * Atgt[k];
                                }
                                (void)P; scale = norm * b.pluck_gain;
                            } else {
                                const double onset_extra = b.onset_tilt * std::exp(-std::max(0.0, ton) / onset_s);
                                const double P = bowed_spectrum(f, std::max(qb, 0.0), onset_extra, Atgt);
                                scale = norm * std::pow(P_ref / std::max(P, 1e-12), 0.5 * b.power_comp);
                            }
                            if (!primed) { for (int k = 1; k <= KMAX; ++k) Acur[k] = Atgt[k] * scale; primed = true; }
                            for (int k = 1; k <= KMAX; ++k) { const double nxt = Atgt[k] * scale; Adel[k] = (nxt - Acur[k]) / BLK; Atgt[k] = nxt; }
                        }
                        const double c1 = std::cos(phase), s1 = std::sin(phase);
                        double sk_prev = 0.0, sk = s1, tone = 0.0;
                        for (int k = 1; k <= KMAX; ++k) { tone += Acur[k] * sk; const double sn = 2.0 * c1 * sk - sk_prev; sk_prev = sk; sk = sn; Acur[k] += Adel[k]; }

                        double am = 1.0;
                        if (!pluck) {
                            am += b.vib_am * vib_wave * vib_ramp;
                            if (b.amp_jitter > 0.0) { an += (jit.tick_white() - an) * 0.004; am *= 1.0 + b.amp_jitter * an * 40.0; }
                            if (tremolo) am *= 1.0 - 0.8 * (0.5 - 0.5 * std::cos(TWO_PI * 13.0 * t));        // bow reversals ~13 Hz
                        }
                        double nz = 0.0, sc = 0.0;
                        if (!pluck && ton > 0.0) {
                            nz = bow_bp.tick(noise.tick_white()) * bow_cal * bow_lin * std::pow(std::max(qb, 0.02) / 0.66, b.bow_noise_follow);
                            sc = scr_bp.tick(scrape_noise.tick_white()) * scr_cal * scr_lin * std::exp(-ton / scr_tau);
                            if (tremolo) sc *= 1.0 + 3.0 * (0.5 + 0.5 * std::cos(TWO_PI * 13.0 * t));
                        } else if (pluck && ton > 0.0) {
                            const double th = thump_bp.tick(thump_noise.tick_white()) * thump_cal * thump_lin * std::exp(-ton / 0.006);
                            sc = th + scr_bp.tick(scrape_noise.tick_white()) * scr_cal * scr_lin * (wood ? 1.0 : 0.0) * std::exp(-ton / scr_tau);
                        }
                        const double mod_volume = std::max(0.0, 1.0 + eval_mod_sum(ModTarget::Volume, t));
                        voice[i] = pluck ? ((tone * a + sc) * velocity * mod_volume)
                                         : ((tone * am + nz) * velocity * a + sc * velocity) * mod_volume;
                    }
                    const double pan = (nv == 1) ? 0.0 : std::max(-0.95, std::min(0.95, vpos * 2.0 * std::min(1.0, r.stereo_width * 1.6)));
                    const double gl = std::cos((pan + 1.0) * M_PI / 4.0), gr = std::sin((pan + 1.0) * M_PI / 4.0), vs = 1.0 / std::sqrt(static_cast<double>(nv));
                    for (size_t i = 0; i < n; ++i) { Lacc[i] += voice[i] * gl * vs * 1.4142135; Racc[i] += voice[i] * gr * vs * 1.4142135; }
                }
                const double G = r.gain * b.level;
                StereoBuffer out; out.L.resize(n); out.R.resize(n);
                for (size_t i = 0; i < n; ++i) { out.L[i] = static_cast<float>(std::tanh(Lacc[i] * G)); out.R[i] = static_cast<float>(std::tanh(Racc[i] * G)); }
                if (nv == 1 && r.stereo_width > 0.0) {
                    Buffer m(n); for (size_t i = 0; i < n; ++i) m[i] = out.L[i];
                    StereoBuffer widened = haas_widen(m, r.stereo_width * 18.0, sample_rate);
                    for (size_t i = 0; i < n; ++i) out.R[i] = out.R[i] * static_cast<float>(1.0 - r.stereo_width) + widened.R[i] * static_cast<float>(r.stereo_width);
                }
                const size_t fd = std::min(out.L.size() / 4, static_cast<size_t>(0.006 * sample_rate));
                for (size_t i = 0; i < fd; ++i) {
                    const float g = static_cast<float>(0.5 - 0.5 * std::cos(M_PI * static_cast<double>(i) / static_cast<double>(std::max<size_t>(1, fd))));
                    out.L[out.L.size() - 1 - i] *= g; out.R[out.R.size() - 1 - i] *= g;
                }
                return out;
            }
        }

        inline StereoBuffer render_timbre(const TimbreRecipe& r, const Note& note, int sample_rate) {
            if (sample_rate <= 0) sample_rate = global_config().sample_rate;
            if (r.wind.kind != TimbreRecipe::Wind::Kind::None) return detail_wind::render(r, note, sample_rate);
            if (r.bowed.kind != TimbreRecipe::Bowed::Kind::None) return detail_bowed::render(r, note, sample_rate);
            const double duration = std::max(0.02, note.duration);
            const size_t n = static_cast<size_t>(duration * sample_rate);
            if (n == 0) return StereoBuffer();

            Articulation art = note.articulation;
            double decay_scale  = 1.0, noise_boost = 0.0, pitch_shift_semi = 0.0, filter_scale = 1.0;
            auto get = [](const std::map<Articulation,double>& m, Articulation a, double def) {
                auto it = m.find(a); return it != m.end() ? it->second : def;
            };
            decay_scale      = get(r.art_decay_scale, art, 1.0);
            noise_boost      = get(r.art_noise_boost, art, 0.0);
            pitch_shift_semi = get(r.art_pitch_shift, art, 0.0);
            filter_scale     = get(r.art_filter_scale, art, 1.0);

            bool percussive = r.percussive;
            if (art == Articulation::Pizzicato) { percussive = true; decay_scale = std::min(decay_scale, 0.28); noise_boost += 0.12; }
            if (art == Articulation::Staccato)  { decay_scale = std::min(decay_scale, 0.4); }
            if (art == Articulation::Muted || art == Articulation::PalmMute) { filter_scale *= 0.45; decay_scale = std::min(decay_scale, 0.55); }
            if (art == Articulation::Harmonic)  { pitch_shift_semi += 12.0; filter_scale *= 1.6; }
            if (art == Articulation::Hard)      { filter_scale *= 1.3; }
            if (art == Articulation::Soft)      { filter_scale *= 0.7; }
            double art_extra_noise = 0.0;
            bool art_click_only = false;
            if (art == Articulation::Spiccato)  { percussive = true; decay_scale = std::min(decay_scale, 0.18); art_extra_noise += 0.1; }
            if (art == Articulation::SulPonticello) { filter_scale *= 2.1; art_extra_noise += 0.08; }
            if (art == Articulation::ColLegno)  { percussive = true; decay_scale = std::min(decay_scale, 0.08); art_extra_noise += 0.55; art_click_only = true; }
            if (art == Articulation::ShortStab) { percussive = true; decay_scale = std::min(decay_scale, 0.15); }
            if (art == Articulation::Whisper)   { art_extra_noise += 0.7; filter_scale *= 0.8; }
            if (art == Articulation::Breath)    { art_extra_noise += 0.85; }
            if (art == Articulation::Humming)   { filter_scale *= 0.6; }

            double base_freq = (r.fixed_pitch_hz >= 0.0)
                ? r.fixed_pitch_hz
                : wauvio::theory::midi_to_freq(note.midi_note) * std::pow(2.0, pitch_shift_semi / 12.0);

            const double velocity = dynamics_to_velocity(note.dynamics) * std::max(0.0, std::min(1.0, note.expression));

            const bool has_glide = (note.glide_from_midi >= 0 && r.fixed_pitch_hz < 0.0);
            const double glide_start_cents = has_glide
                ? 1200.0 * std::log2(wauvio::theory::midi_to_freq(note.glide_from_midi) / std::max(1.0, base_freq))
                : 0.0;
            const double glide_time = std::max(0.001, note.glide_time);

            const bool has_continuous_bend = r.fixed_pitch_hz < 0.0;
            const bool has_tremolo = (art == Articulation::Tremolo);

            auto eval_mod_sum = [&](ModTarget target, double t) -> double {
                double sum = 0.0;
                for (auto& route : r.modulation) {
                    if (route.target != target) continue;
                    double src_val = 0.0;
                    switch (route.source) {
                        case ModSource::Velocity:    src_val = velocity; break;
                        case ModSource::ModWheel:    src_val = note.mod_wheel; break;
                        case ModSource::Aftertouch:  src_val = note.aftertouch; break;
                        case ModSource::Expression:  src_val = note.expression; break;
                        case ModSource::KeyPosition: src_val = note.midi_note / 127.0; break;
                        case ModSource::LFO1:        src_val = std::sin(TWO_PI * 5.0 * t); break;
                        case ModSource::LFO2:        src_val = std::sin(TWO_PI * 0.5 * t); break;
                        case ModSource::Envelope1:   src_val = (duration > 0.0) ? std::max(0.0, 1.0 - t / duration) : 0.0; break;
                    }
                    sum += src_val * route.amount;
                }
                return sum;
            };

            const bool modal = !r.modes.empty();
            std::vector<double> m_ph, m_inc, m_amp, m_lvl, m_g;
            double m_norm = 1.0;
            if (modal) {
                const double pitch_k = std::max(0.25, std::min(4.0, std::pow(261.6256 / std::max(20.0, base_freq), r.mode_decay_pitch_exp)));
                double amp_sum = 0.0; int k = 0;
                for (const auto& md : r.modes) {
                    const double f = base_freq * md.ratio;
                    if (f < 0.45 * sample_rate) {
                        const double t60 = std::max(0.02, md.t60 * pitch_k * decay_scale);
                        m_ph.push_back(0.0); m_inc.push_back(TWO_PI * f / sample_rate);
                        m_amp.push_back(md.amp * std::pow(std::max(0.05, velocity), r.mode_vel_brightness * k));
                        m_lvl.push_back(1.0); m_g.push_back(std::pow(10.0, -3.0 / (t60 * sample_rate)));
                        amp_sum += m_amp.back();
                    }
                    ++k;
                }
                m_norm = 1.0 / std::max(1.0, amp_sum);
            }

            wauvio::osc::Oscillator osc1(r.osc1_shape, base_freq, 1.0);
            wauvio::osc::Oscillator osc2(r.osc2_shape, base_freq * r.osc2_ratio * std::pow(2.0, r.detune_cents / 1200.0), 1.0);
            osc1.duty = 0.5; osc2.duty = 0.5;

            wauvio::osc::NoiseGenerator noise(static_cast<uint32_t>(note.midi_note * 7919u + 17u));
            wauvio::osc::NoiseGenerator drift_noise(static_cast<uint32_t>(note.midi_note * 104729u + 3u));
            double drift = 0.0;

            wauvio::filter::SVFilter filt(wauvio::filter::SVFilter::Mode::LowPass, std::max(80.0, r.filter_cutoff * filter_scale), r.filter_q);

            Buffer mono(n);

            const double fm_c_ratio = 1.0;
            double fm_cp = 0.0, fm_mp = 0.0;
            const double fm_c_inc = TWO_PI * base_freq * fm_c_ratio / sample_rate;
            const double fm_m_inc = TWO_PI * base_freq * r.fm_ratio / sample_rate;

            for (size_t i = 0; i < n; ++i) {
                double t = static_cast<double>(i) / sample_rate;

                double pitch_mult = 1.0;
                if (has_continuous_bend) {
                    double bend_semi = note.bend_at(t);
                    if (bend_semi != 0.0) pitch_mult *= std::pow(2.0, bend_semi / 12.0);
                }
                if (has_glide && t < glide_time) {
                    double f = 1.0 - (t / glide_time);
                    pitch_mult *= std::pow(2.0, (glide_start_cents * f) / 1200.0);
                }
                if (r.pitch_attack_cents != 0.0 && t < r.pitch_attack_time) {
                    double f = 1.0 - (t / r.pitch_attack_time);
                    pitch_mult *= std::pow(2.0, (r.pitch_attack_cents * f) / 1200.0);
                }
                double vibrato_depth_scale = 1.0 + eval_mod_sum(ModTarget::VibratoDepth, t);
                if (r.vibrato_depth_cents > 0.0 && t > r.vibrato_delay_sec) {
                    double vt = (t - r.vibrato_delay_sec);
                    double lfo = std::sin(TWO_PI * r.vibrato_rate_hz * vt);
                    pitch_mult *= std::pow(2.0, (lfo * r.vibrato_depth_cents * vibrato_depth_scale) / 1200.0);
                }
                if (r.pitch_instability > 0.0) {
                    double target = drift_noise.tick_pink();
                    drift += (target - drift) * 0.0004;
                    pitch_mult *= std::pow(2.0, (drift * r.pitch_instability) / 1200.0);
                }
                double mod_pitch_semi = eval_mod_sum(ModTarget::Pitch, t);
                if (mod_pitch_semi != 0.0) pitch_mult *= std::pow(2.0, mod_pitch_semi / 12.0);

                osc1.frequency = base_freq * pitch_mult;
                osc2.frequency = base_freq * r.osc2_ratio * std::pow(2.0, r.detune_cents / 1200.0) * pitch_mult;

                float sample;
                if (art_click_only) {
                    sample = 0.0f;
                } else if (modal) {
                    double acc = 0.0;
                    for (size_t k = 0; k < m_ph.size(); ++k) {
                        acc += m_amp[k] * m_lvl[k] * std::sin(m_ph[k]);
                        m_ph[k] += m_inc[k] * pitch_mult; if (m_ph[k] >= TWO_PI) m_ph[k] -= TWO_PI;
                        m_lvl[k] *= m_g[k];
                    }
                    sample = static_cast<float>(acc * m_norm);
                } else if (r.use_fm) {
                    float fm_val = static_cast<float>(std::sin(fm_cp + r.fm_index * std::sin(fm_mp)));
                    fm_cp += fm_c_inc * pitch_mult; fm_mp += fm_m_inc * pitch_mult;
                    if (fm_cp >= TWO_PI) fm_cp -= TWO_PI;
                    if (fm_mp >= TWO_PI) fm_mp -= TWO_PI;
                    float osc1v = osc1.tick(sample_rate);
                    sample = osc1v * static_cast<float>(1.0 - r.osc_mix) + fm_val * static_cast<float>(r.osc_mix);
                } else {
                    float osc1v = osc1.tick(sample_rate);
                    float osc2v = osc2.tick(sample_rate);
                    sample = osc1v * static_cast<float>(1.0 - r.osc_mix) + osc2v * static_cast<float>(r.osc_mix);
                }

                double n_amt = r.noise_mix + noise_boost + art_extra_noise;
                if (r.noise_attack_burst > 0.0 && t < 0.02) n_amt += r.noise_attack_burst * (1.0 - t / 0.02);
                if (n_amt > 0.0) {
                    float nz = r.pink_noise ? noise.tick_pink() : noise.tick_white();
                    sample = static_cast<float>(sample * (1.0 - std::min(1.0, n_amt)) + nz * std::min(1.0, n_amt));
                }

                if (has_tremolo) {
                    double tremolo_depth = std::max(0.0, 0.5 + eval_mod_sum(ModTarget::TremoloDepth, t));
                    double tlfo = std::sin(TWO_PI * 8.0 * t);
                    sample = static_cast<float>(sample * (1.0 - tremolo_depth * 0.5 + tremolo_depth * 0.5 * tlfo));
                }

                double mod_filter_hz = eval_mod_sum(ModTarget::FilterCutoff, t);
                if (r.filter_env_amount != 0.0 || mod_filter_hz != 0.0) {
                    double sweep = r.filter_env_amount * (1.0 - t / duration) + mod_filter_hz;
                    filt.cutoff = std::max(60.0, r.filter_cutoff * filter_scale + sweep);
                }
                sample = filt.tick(sample, sample_rate);

                double mod_volume = 1.0 + eval_mod_sum(ModTarget::Volume, t);
                sample = static_cast<float>(sample * std::max(0.0, mod_volume));

                mono[i] = sample * static_cast<float>(velocity);
            }

            wauvio::env::DAHDSR env = r.envelope;
            env.decay   *= decay_scale;
            env.release *= decay_scale;
            if (percussive) {
                env.attack  = std::min(env.attack, 0.006);
                env.hold    = 0.0;
                env.decay   = duration * 0.72 * decay_scale;
                env.sustain = 0.04;
                env.release = duration * 0.28 * decay_scale;
            }
            env.apply(mono, duration, -1.0, sample_rate);

            if (r.formant1_gain_db != 0.0) {
                wauvio::filter::BiquadEQ f1(wauvio::filter::BiquadEQ::Type::Peak, r.formant1_hz, r.formant1_gain_db, r.formant1_q);
                f1.process(mono, sample_rate);
            }
            if (r.formant2_gain_db != 0.0) {
                wauvio::filter::BiquadEQ f2(wauvio::filter::BiquadEQ::Type::Peak, r.formant2_hz, r.formant2_gain_db, r.formant2_q);
                f2.process(mono, sample_rate);
            }

            apply_gain(mono, static_cast<float>(r.gain));
            clamp_buffer(mono);

            StereoBuffer out = pan_mono(mono, 0.0f, sample_rate);
            if (r.stereo_width > 0.0) {
                StereoBuffer widened = haas_widen(mono, r.stereo_width * 18.0, sample_rate);
                for (size_t i = 0; i < out.size(); ++i) {
                    out.R[i] = out.R[i] * static_cast<float>(1.0 - r.stereo_width) +
                            widened.R[i] * static_cast<float>(r.stereo_width);
                }
            }
            {
                const size_t f = std::min(out.L.size() / 4, static_cast<size_t>(0.006 * sample_rate));
                for (size_t i = 0; i < f; ++i) {
                    const float g = static_cast<float>(0.5 - 0.5 * std::cos(M_PI * static_cast<double>(i) / static_cast<double>(std::max<size_t>(1, f))));
                    out.L[out.L.size() - 1 - i] *= g; out.R[out.R.size() - 1 - i] *= g;
                }
            }
            return out;
        }

        class ModeledInstrument : public SampledInstrument {
        public:
            TimbreRecipe recipe;

            explicit ModeledInstrument(std::string instrument_name = "") {
                name = std::move(instrument_name);
                fallback_model = [this](const Note& note, int sample_rate) {
                    return render_timbre(this->recipe, note, sample_rate);
                };
            }
        };

        class KeyboardInstrument : public ModeledInstrument {
        public:
            explicit KeyboardInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Soft, Articulation::Hard };
                recipe.osc1_shape = wauvio::osc::WaveShape::Triangle;
                recipe.osc2_shape = wauvio::osc::WaveShape::Sine;
                recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.25; recipe.detune_cents = 3.0;
                recipe.noise_attack_burst = 0.05;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.004, 0.0, 0.9, 0.0, 0.35};
                recipe.filter_cutoff = 6500.0; recipe.filter_q = 0.7;
                recipe.stereo_width = 0.15;
            }
        };

        class BowedStringInstrument : public ModeledInstrument {
        public:
            explicit BowedStringInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Pizzicato,
                                Articulation::Staccato, Articulation::Tremolo, Articulation::Harmonic,
                                Articulation::Spiccato, Articulation::SulPonticello, Articulation::ColLegno };
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth;
                recipe.osc2_shape = wauvio::osc::WaveShape::BL_Sawtooth;
                recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.4; recipe.detune_cents = 6.0;
                recipe.noise_mix = 0.03; recipe.noise_attack_burst = 0.05; recipe.pink_noise = true;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.06, 0.0, 0.15, 0.82, 0.25};
                recipe.vibrato_rate_hz = 5.5; recipe.vibrato_depth_cents = 18.0; recipe.vibrato_delay_sec = 0.2;
                recipe.filter_cutoff = 5200.0; recipe.filter_q = 0.9;
                recipe.formant1_hz = 800.0; recipe.formant1_gain_db = 3.0; recipe.formant1_q = 2.0;
                recipe.stereo_width = 0.22;
                recipe.art_decay_scale[Articulation::Pizzicato] = 0.22;
                recipe.art_decay_scale[Articulation::Staccato]  = 0.35;
            }
        };

        // Bowed strings with the physical model (Helmholtz source, body resonances, bow noise, scrape, section ensemble).
        // Erhu in world/ still uses BowedStringInstrument and is unchanged.
        class BowedModelInstrument : public BowedStringInstrument {
        public:
            explicit BowedModelInstrument(std::string n) : BowedStringInstrument(std::move(n)) {
                using K = TimbreRecipe::Bowed::Kind; recipe.bowed.kind = K::Bowed;
                recipe.filter_cutoff = 16000.0; recipe.noise_mix = 0.0; recipe.art_decay_scale.clear();
            }
        };

        class PluckedStringInstrument : public ModeledInstrument {
        public:
            explicit PluckedStringInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::PalmMute,
                                Articulation::Harmonic, Articulation::Muted, Articulation::Staccato };
                recipe.percussive = true;
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth;
                recipe.osc2_shape = wauvio::osc::WaveShape::Triangle;
                recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.2; recipe.detune_cents = 2.0;
                recipe.noise_attack_burst = 0.22;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.002, 0.0, 0.6, 0.02, 0.25};
                recipe.filter_cutoff = 4200.0; recipe.filter_q = 0.7; recipe.filter_env_amount = -1800.0;
                recipe.stereo_width = 0.2;
                recipe.art_filter_scale[Articulation::PalmMute] = 0.35;
                recipe.art_filter_scale[Articulation::Muted]     = 0.3;
            }
        };

        class WoodwindInstrument : public ModeledInstrument {
        public:
            explicit WoodwindInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Staccato,
                                Articulation::FlutterTongue, Articulation::Legato };
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine;
                recipe.osc2_shape = wauvio::osc::WaveShape::Triangle;
                recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.3; recipe.detune_cents = 1.5;
                recipe.noise_mix = 0.05; recipe.noise_attack_burst = 0.08; recipe.pink_noise = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.045, 0.0, 0.08, 0.85, 0.16};
                recipe.vibrato_rate_hz = 5.0; recipe.vibrato_depth_cents = 12.0; recipe.vibrato_delay_sec = 0.25;
                recipe.filter_cutoff = 4500.0; recipe.filter_q = 0.75;
                recipe.stereo_width = 0.14;
            }
        };

        class BrassInstrument : public ModeledInstrument {
        public:
            explicit BrassInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Staccato,
                                Articulation::Fall, Articulation::Doit, Articulation::Muted, Articulation::Marcato,
                                Articulation::ShortStab };
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Square;
                recipe.osc2_shape = wauvio::osc::WaveShape::BL_Sawtooth;
                recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.35; recipe.detune_cents = 5.0;
                recipe.noise_mix = 0.015; recipe.noise_attack_burst = 0.1;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.03, 0.0, 0.1, 0.85, 0.14};
                recipe.pitch_attack_cents = 35.0; recipe.pitch_attack_time = 0.045;
                recipe.vibrato_rate_hz = 5.5; recipe.vibrato_depth_cents = 8.0; recipe.vibrato_delay_sec = 0.3;
                recipe.filter_cutoff = 5800.0; recipe.filter_q = 1.1;
                recipe.formant1_hz = 1200.0; recipe.formant1_gain_db = 4.0; recipe.formant1_q = 2.5;
                recipe.stereo_width = 0.16;
                recipe.art_filter_scale[Articulation::Muted] = 0.4;
            }
        };

        // ---- wind families (opt-in wind model; WoodwindInstrument / BrassInstrument above stay as they were) ----------------------
        // Air jet: flute, piccolo, recorder, whistle, pan flute, ocarina. Near-sinusoidal source, jet noise, slow soft attack.
        class AirJetInstrument : public WoodwindInstrument {
        public:
            explicit AirJetInstrument(std::string n) : WoodwindInstrument(std::move(n)) {
                using K = TimbreRecipe::Wind::Kind; auto& w = recipe.wind;
                w.kind = K::AirJet;
                w.tilt_soft = 0.9; w.tilt_loud = 0.9; w.g_soft = 0.28; w.g_loud = 0.55; w.tilt_per_octave = 0.35;
                w.cutoff_hz = 5000.0; w.cutoff_loud_gain = 0.5;
                w.breath_db = -32.0; w.breath_center_hz = 2800.0; w.breath_center_mult = 6.0; w.breath_q = 0.5; w.breath_follow = 0.4; w.breath_tone_mod = 0.35;
                w.chiff_db = -22.0; w.chiff_ms = 28.0; w.chiff_center_hz = 2600.0;
                w.attack_ms = 55.0; w.attack_shape = 1.3; w.attack_vel_exp = 0.35; w.release_ms = 90.0;
                w.scoop_cents = 10.0; w.scoop_ms = 45.0; w.pressure_pitch_cents = 15.0;
                w.vib_rate_hz = 5.3; w.vib_depth_cents = 17.0; w.vib_delay_s = 0.30; w.vib_onset_s = 0.40; w.vib_am = 0.18; w.vib_wander = 0.15;
                w.jitter_cents = 1.5; w.amp_jitter = 0.01; w.power_comp = 0.3; w.level = 0.48;
                recipe.filter_cutoff = 16000.0; recipe.noise_mix = 0.0;
            }
        };

        // Single reed: clarinet family and saxophones. Odd-harmonic cylinder (clarinet) or full conical series (sax), tone-hole cutoff.
        class SingleReedInstrument : public WoodwindInstrument {
        public:
            explicit SingleReedInstrument(std::string n) : WoodwindInstrument(std::move(n)) {
                using K = TimbreRecipe::Wind::Kind; auto& w = recipe.wind;
                w.kind = K::SingleReed;
                w.tilt_soft = 1.4; w.tilt_loud = 0.3; w.g_soft = 0.70; w.g_loud = 0.97; w.fund_drop_db = 4.0;
                w.even_low = 0.12; w.even_open_hz = 1800.0;
                w.cutoff_hz = 2000.0; w.cutoff_loud_gain = 0.6;
                w.breath_db = -42.0; w.breath_center_hz = 2200.0; w.breath_q = 0.8; w.breath_follow = 0.8;
                w.chiff_db = -34.0; w.chiff_ms = 15.0; w.chiff_center_hz = 900.0;
                w.attack_ms = 28.0; w.attack_shape = 0.9; w.attack_vel_exp = 0.55; w.overshoot = 0.06; w.overshoot_ms = 120.0; w.release_ms = 60.0;
                w.pressure_pitch_cents = 6.0;
                w.vib_rate_hz = 5.0; w.vib_depth_cents = 6.0; w.vib_delay_s = 0.5; w.vib_onset_s = 0.5; w.vib_am = 0.03; w.vib_wander = 0.15;
                w.jitter_cents = 0.8; w.amp_jitter = 0.01; w.power_comp = 0.5; w.level = 0.48;
                recipe.filter_cutoff = 16000.0; recipe.noise_mix = 0.0;
            }
        };

        // Double reed: oboe, English horn, bassoon families, shawm. Pulse-like reed source with strong bore formants.
        class DoubleReedInstrument : public WoodwindInstrument {
        public:
            explicit DoubleReedInstrument(std::string n) : WoodwindInstrument(std::move(n)) {
                using K = TimbreRecipe::Wind::Kind; auto& w = recipe.wind;
                w.kind = K::DoubleReed;
                w.tilt_soft = 0.8; w.tilt_loud = 0.4; w.g_soft = 0.86; w.g_loud = 0.97; w.fund_drop_db = 1.0;
                w.cutoff_hz = 3200.0; w.cutoff_order = 1.5; w.cutoff_loud_gain = 0.5;
                w.res[0] = { 1150.0, 7.0, 1.4 }; w.res[1] = { 3000.0, 3.0, 2.0 };
                w.breath_db = -38.0; w.breath_center_hz = 3000.0; w.breath_q = 0.8; w.breath_follow = 0.6;
                w.chiff_db = -30.0; w.chiff_ms = 12.0; w.chiff_center_hz = 1500.0;
                w.attack_ms = 24.0; w.attack_shape = 0.9; w.attack_vel_exp = 0.3; w.overshoot = 0.08; w.overshoot_ms = 100.0; w.release_ms = 70.0;
                w.vib_rate_hz = 5.8; w.vib_depth_cents = 12.0; w.vib_delay_s = 0.35; w.vib_onset_s = 0.40; w.vib_am = 0.10; w.vib_wander = 0.20;
                w.jitter_cents = 1.0; w.amp_jitter = 0.015; w.power_comp = 0.5; w.level = 0.48;
                recipe.filter_cutoff = 16000.0; recipe.noise_mix = 0.0;
            }
        };

        // Lip reed: trumpets, horns, trombones, tubas. Harmonic k grows roughly as pressure^k, so high partials enter later in the
        // attack and with loudness (brassy at ff); bell radiation tilts the spectrum up.
        class LipReedInstrument : public BrassInstrument {
        public:
            explicit LipReedInstrument(std::string n) : BrassInstrument(std::move(n)) {
                using K = TimbreRecipe::Wind::Kind; auto& w = recipe.wind;
                w.kind = K::LipReed;
                w.tilt_soft = 0.5; w.tilt_loud = 0.3; w.g_soft = 0.28; w.g_loud = 1.0; w.g_pow = 1.6;
                w.cutoff_hz = 5500.0; w.cutoff_loud_gain = 0.6; w.bell_hz = 900.0; w.bell_depth = 1.0;
                w.res[0] = { 1300.0, 3.0, 1.0 };
                w.mute_res = { 1500.0, 6.0, 2.5 };
                w.breath_db = -40.0; w.breath_center_hz = 2500.0; w.breath_q = 0.8; w.breath_follow = 0.5;
                w.chiff_db = -30.0; w.chiff_ms = 18.0; w.chiff_center_hz = 1800.0;
                w.attack_ms = 35.0; w.attack_shape = 1.4; w.attack_vel_exp = 0.35; w.overshoot = 0.06; w.overshoot_ms = 150.0; w.release_ms = 70.0;
                w.scoop_cents = 14.0; w.scoop_ms = 45.0; w.pressure_pitch_cents = 8.0;
                w.vib_rate_hz = 5.0; w.vib_depth_cents = 7.0; w.vib_delay_s = 0.45; w.vib_onset_s = 0.5; w.vib_am = 0.04; w.vib_wander = 0.15;
                w.jitter_cents = 2.5; w.amp_jitter = 0.02; w.power_comp = 0.7; w.level = 0.62;
                recipe.filter_cutoff = 16000.0; recipe.noise_mix = 0.0;
                recipe.art_filter_scale.clear();
            }
        };

        // Saxophones: single reed on a conical bore, so the full harmonic series (no odd-only suppression); sizes set the cutoff and formant.
        class SaxophoneInstrument : public SingleReedInstrument {
        public:
            explicit SaxophoneInstrument(std::string n) : SingleReedInstrument(std::move(n)) {
                auto& w = recipe.wind;
                w.tilt_soft = 1.2; w.tilt_loud = 0.5; w.g_soft = 0.78; w.g_loud = 0.96; w.fund_drop_db = 2.0;
                w.even_low = 1.0;
                w.cutoff_hz = 1300.0; w.cutoff_loud_gain = 0.8;
                w.res[0] = { 1300.0, 5.0, 1.2 }; w.res[1] = { 3000.0, 2.0, 1.8 };
                w.breath_db = -33.0; w.breath_center_hz = 2000.0;
                w.chiff_db = -30.0; w.chiff_ms = 14.0; w.chiff_center_hz = 1200.0;
                w.attack_ms = 32.0; w.overshoot = 0.10;
                w.vib_rate_hz = 5.5; w.vib_depth_cents = 16.0; w.vib_delay_s = 0.40; w.vib_onset_s = 0.5; w.vib_am = 0.08;
                w.jitter_cents = 1.2;
            }
        };

        class MalletInstrument : public ModeledInstrument {
        public:
            explicit MalletInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Tremolo, Articulation::Muted };
                recipe.percussive = true;
                recipe.use_fm = true; recipe.fm_ratio = 3.01; recipe.fm_index = 1.4; recipe.osc_mix = 0.85;
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 1.4, 0.0, 0.4};
                recipe.filter_cutoff = 9000.0; recipe.filter_q = 0.6;
                recipe.stereo_width = 0.25;
                recipe.art_decay_scale[Articulation::Muted] = 0.2;
            }
        };

        class WorldInstrument : public ModeledInstrument {
        public:
            explicit WorldInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Staccato, Articulation::Tremolo };
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth;
                recipe.osc2_shape = wauvio::osc::WaveShape::Sine;
                recipe.osc_mix = 0.3; recipe.detune_cents = 4.0;
                recipe.noise_mix = 0.04;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.02, 0.0, 0.2, 0.65, 0.2};
                recipe.filter_cutoff = 5000.0; recipe.filter_q = 0.85;
                recipe.stereo_width = 0.2;
            }
        };

        class PercussionInstrument : public ModeledInstrument {
        public:
            explicit PercussionInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Rimshot, Articulation::Accent };
                recipe.percussive = true;
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine;
                recipe.noise_mix = 0.5; recipe.pink_noise = false;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.001, 0.0, 0.12, 0.0, 0.06};
                recipe.filter_cutoff = 3000.0; recipe.filter_q = 0.7;
                recipe.stereo_width = 0.1;
            }

            PlayedNote hit(Dynamics dyn = Dynamics::mf, double duration = 0.4,
                        Articulation art = Articulation::Sustain, int sample_rate = 0) const
            {
                return play(60, duration, dyn, art, sample_rate);
            }
        };

        class VocalInstrument : public ModeledInstrument {
        public:
            explicit VocalInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Legato, Articulation::Whisper,
                                Articulation::Breath, Articulation::Humming, Articulation::Staccato };
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine;
                recipe.osc2_shape = wauvio::osc::WaveShape::Triangle;
                recipe.osc_mix = 0.25; recipe.detune_cents = 6.0;
                recipe.noise_mix = 0.025;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.09, 0.0, 0.12, 0.8, 0.3};
                recipe.vibrato_rate_hz = 5.4; recipe.vibrato_depth_cents = 14.0; recipe.vibrato_delay_sec = 0.35;
                recipe.filter_cutoff = 3800.0; recipe.filter_q = 0.8;
                recipe.formant1_hz = 700.0;  recipe.formant1_gain_db = 6.0; recipe.formant1_q = 3.0;
                recipe.formant2_hz = 1200.0; recipe.formant2_gain_db = 4.0; recipe.formant2_q = 3.0;
                recipe.stereo_width = 0.3;
            }
        };

        class SynthInstrument : public ModeledInstrument {
        public:
            explicit SynthInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Staccato };
                recipe.osc1_shape = wauvio::osc::WaveShape::BL_Sawtooth;
                recipe.osc2_shape = wauvio::osc::WaveShape::BL_Square;
                recipe.osc_mix = 0.45; recipe.detune_cents = 7.0;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.01, 0.0, 0.15, 0.7, 0.2};
                recipe.filter_cutoff = 4000.0; recipe.filter_q = 1.0; recipe.filter_env_amount = 1500.0;
                recipe.stereo_width = 0.28;
            }
        };

        class GuitarInstrument : public PluckedStringInstrument {
        public:
            explicit GuitarInstrument(std::string n) : PluckedStringInstrument(std::move(n)) {}
        };

        class ExperimentalInstrument : public ModeledInstrument {
        public:
            explicit ExperimentalInstrument(std::string n) : ModeledInstrument(std::move(n)) {
                articulations = { Articulation::Sustain, Articulation::Tremolo, Articulation::Whisper };
                recipe.osc1_shape = wauvio::osc::WaveShape::Sine;
                recipe.osc2_shape = wauvio::osc::WaveShape::Triangle;
                recipe.osc_mix = 0.3;
                recipe.noise_mix = 0.06;
                recipe.envelope = wauvio::env::DAHDSR{0.0, 0.3, 0.0, 0.3, 0.6, 0.8};
                recipe.pitch_instability = 18.0;
                recipe.vibrato_rate_hz = 4.0; recipe.vibrato_depth_cents = 10.0; recipe.vibrato_delay_sec = 0.1;
                recipe.filter_cutoff = 3500.0; recipe.stereo_width = 0.35;
            }
        };

        class DrumKit {
        public:
            std::string name;

            explicit DrumKit(std::string kit_name) : name(std::move(kit_name)) {}
            virtual ~DrumKit() = default;

            void map(int midi_note, std::shared_ptr<Instrument> instr) {
                mapping_[midi_note] = std::move(instr);
            }

            bool has(int midi_note) const { return mapping_.find(midi_note) != mapping_.end(); }

            std::shared_ptr<Instrument> get(int midi_note) const {
                auto it = mapping_.find(midi_note);
                return it != mapping_.end() ? it->second : nullptr;
            }

            std::vector<int> mapped_notes() const {
                std::vector<int> out;
                out.reserve(mapping_.size());
                for (auto& kv : mapping_) out.push_back(kv.first);
                return out;
            }

            PlayedNote play(int midi_note, Dynamics dyn = Dynamics::mf, double duration = 0.4,
                            int sample_rate = 0) const
            {
                auto it = mapping_.find(midi_note);
                if (it == mapping_.end() || !it->second)
                    return PlayedNote{ StereoBuffer(), sample_rate > 0 ? sample_rate : global_config().sample_rate };
                return it->second->play(midi_note, duration, dyn, Articulation::Sustain, sample_rate);
            }

            StereoBuffer render_pattern(const std::vector<std::pair<double,int>>& hits,
                                        double total_duration, Dynamics dyn = Dynamics::mf,
                                        int sample_rate = 0) const
            {
                if (sample_rate <= 0) sample_rate = global_config().sample_rate;
                StereoBuffer out = make_stereo(total_duration, sample_rate);
                for (auto& h : hits) {
                    PlayedNote pn = play(h.second, dyn, 0.4, sample_rate);
                    const size_t off = static_cast<size_t>(h.first * sample_rate);
                    for (size_t i = 0; i < pn.audio.size() && off + i < out.size(); ++i) {
                        out.L[off + i] += pn.audio.L[i];
                        out.R[off + i] += pn.audio.R[i];
                    }
                }
                return out;
            }

        protected:
            std::unordered_map<int, std::shared_ptr<Instrument>> mapping_;
        };
    }
}