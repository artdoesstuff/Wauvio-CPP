#pragma once
#include "sfx_traj.hpp"
#include <variant>

namespace wauvio {
    namespace sfx {
        enum class Wave { Sine, Triangle, Saw, Square, Pulse };
        enum class NoiseColor { White, Pink, Brown, Blue, Violet, Dust };
        enum class ChaosKind { Lorenz, Logistic, Henon };
        enum class GrainSource { Noise, Tone, Chord, Bell, Vowel };

        struct Mode { double ratio = 1.0; double t60 = 1.0; double amp = 1.0; };   ///< one resonant mode of a modal source

        /// Mode tables for metallic / resonant bodies (frequency ratios are relative to the root).
        inline std::vector<Mode> modes_bell()  { return { {1.0,3.0,1.0},{2.0,2.2,0.6},{2.4,1.8,0.5},{3.0,1.4,0.4},{4.5,1.0,0.3},{5.33,0.8,0.25},{6.8,0.6,0.15} }; }
        inline std::vector<Mode> modes_glass() { return { {1.0,2.6,1.0},{2.32,2.0,0.55},{4.25,1.5,0.45},{6.63,1.1,0.3},{9.38,0.8,0.2} }; }
        inline std::vector<Mode> modes_metal() { return { {1.0,1.6,1.0},{1.47,1.3,0.8},{1.98,1.1,0.7},{2.42,1.0,0.55},{2.89,0.8,0.5},{3.51,0.7,0.4},{4.12,0.5,0.35},{5.6,0.4,0.25} }; }
        inline std::vector<Mode> modes_gong()  { return { {1.0,5.0,1.0},{1.58,4.0,0.8},{2.12,3.4,0.7},{2.76,2.6,0.55},{3.3,2.0,0.45},{4.37,1.4,0.3},{5.8,1.0,0.2} }; }
        inline std::vector<Mode> modes_wood()  { return { {1.0,0.35,1.0},{2.57,0.22,0.6},{4.2,0.14,0.35},{6.1,0.1,0.2} }; }
        inline std::vector<Mode> modes_inharmonic(uint64_t seed, int n, double t60, double spread = 1.0) {
            Rng r(seed); std::vector<Mode> m; for (int i = 0; i < n; ++i) m.push_back({1.0 + i * (0.9 + 0.8 * spread * r.uniform()) + 0.3 * r.uniform(), t60 * (1.0 - 0.6 * i / std::max(1, n)), 1.0 / (1.0 + 0.4 * i)});
            return m;
        }

        enum class SourceKind { None, Osc, Noise, Additive, Modal, Formant, Grain, Pluck, Chaos };

        struct Source {
            SourceKind kind = SourceKind::None;
            Traj pitch = 60.0;                       ///< MIDI note (fractional allowed); used by every pitched kind

            // Osc
            Wave wave = Wave::Saw; int voices = 1; Traj detune_cents = 0.0; Traj pulse_width = 0.5;
            double sub = 0.0;                        ///< sine one octave below, relative level
            std::vector<double> ratios;              ///< non-empty: a stack of oscillators at pitch * ratio (e.g. metallic square stacks)
            Traj fm_index = 0.0; double fm_ratio = 1.0;   ///< phase modulation of the carrier by a sine at pitch * fm_ratio

            // Noise
            NoiseColor color = NoiseColor::White; Traj density = 200.0;   ///< density = events per second (Dust only)

            // Additive: each note (semitone offset from `pitch`) gets `harmonics` partials
            std::vector<double> notes { 0.0 }; int harmonics = 1; Traj rolloff = 1.0;   ///< partial n has amplitude n^-rolloff
            double inharmonicity = 0.0;              ///< partial n sits at n*sqrt(1 + B*n^2)
            Traj partial_spread = 0.0;               ///< random per-partial detune in cents (smooth, seeded)

            // Modal / Pluck: events happen at normalised times `hits` (+/- jitter); hit k plays pitch + hit_notes[k % size]
            std::vector<Mode> modes; std::vector<double> hits; std::vector<double> hit_notes; double hit_jitter = 0.0; double velocity_var = 0.3;
            double strike_ms = 1.5;                  ///< noise burst at each hit (the "click" of the attack)
            double t60_scale = 1.0;                  ///< multiplies every mode / string decay time
            double damping = 0.5;                    ///< Pluck: 0 bright & ringing .. 1 dull & short

            // Formant
            Traj vowel = 0.0;                        ///< 0=a 1=e 2=i 3=o 4=u, interpolated
            Traj formant_shift = 0.0;                ///< semitones, moves all formants
            double breath = 0.0;                     ///< 0 voiced .. 1 whispered (noise excitation)
            bool   noise_excited = false;

            // Grain
            GrainSource grain_source = GrainSource::Noise; Traj grain_ms = 60.0; Traj grain_pitch = 0.0;   ///< semitones
            double pitch_scatter = 0.0; Traj position = 0.0; double position_scatter = 0.2; double grain_chord = 0.0;

            // Chaos
            ChaosKind chaos = ChaosKind::Lorenz; Traj chaos_rate = 40.0;   ///< attractor evolution speed
            Traj chaos_depth = 12.0;                 ///< semitones of pitch modulation (carrier mode)
            double chaos_amp = 0.0;                  ///< amount the attractor also modulates the level
            bool   chaos_direct = false;             ///< output the attractor itself instead of an oscillator
            Traj logistic_r = 3.9;
        };

        struct FilterP  { enum Type { LowPass, HighPass, BandPass, Notch } type = LowPass; Traj cutoff = 1000.0; Traj q = 0.707; int poles = 2; Mod lfo; };   // lfo depth in octaves
        struct DriveP   { enum Kind { Tanh, Hard, Fold, Asym, Rectify } kind = Tanh; Traj amount = 2.0; double mix = 1.0; };
        struct CrushP   { Traj bits = 8.0; Traj down = 1.0; };
        struct GateP    { std::vector<double> steps { 1.0, 0.0 }; double step_beats = 0.25; double step_sec = 0.0; double depth = 1.0; double smooth_ms = 3.0; double swing = 0.0; };
        struct TremP    { Traj rate = 5.0; Traj depth = 0.5; };
        struct PulsesP  { Traj rate = 8.0; std::vector<double> times; double decay_ms = 40.0; double attack_ms = 0.5; double depth = 1.0; double jitter = 0.0; };
        struct StutterP { double seg_sec = 0.1; int repeats = 3; double prob = 0.5; double from = 0.0; double to = 1.0; };
        struct FreezeP  { double at = 0.5; double window_sec = 0.06; };
        struct DropoutP { double rate_per_sec = 4.0; double min_ms = 5.0; double max_ms = 40.0; double depth = 1.0; };
        struct CombP    { Traj freq = 220.0; double feedback = 0.8; double mix = 0.5; };
        struct RingP    { Traj freq = 300.0; double mix = 1.0; };
        struct ReverbP  { double t60 = 2.0; double damping = 0.4; double mix = 0.3; double predelay_ms = 0.0; };   // small synthetic Schroeder tail, not a convolution reverb
        struct EchoP    { double time_sec = 0.25; double feedback = 0.4; double mix = 0.4; double damping = 0.3; };
        struct ReverseP {};
        struct SpectralP { enum Mode { Disintegrate, Scramble, TiltFade, Freeze } mode = Scramble; Traj amount = 0.5; double at = 0.5; };

        using Proc = std::variant<FilterP, DriveP, CrushP, GateP, TremP, PulsesP, StutterP, FreezeP, DropoutP, CombP, RingP, ReverbP, EchoP, ReverseP, SpectralP>;

        struct LayerSpec {
            std::string name;
            Source src;
            std::vector<Proc> chain;
            Traj env = 1.0;            ///< amplitude envelope applied to the source, before the chain (tails can ring on)
            Traj post = 1.0;           ///< amplitude automation applied after the chain
            double level = 1.0;        ///< linear peak level in the mix
            double start = 0.0;        ///< seconds from the start of the effect
            double length = -1.0;      ///< seconds of source; < 0 = until the end of the effect
            double tail = 0.0;         ///< extra seconds the chain may ring after the source ends
            double pan = 0.0;          ///< -1 left .. +1 right (equal power)
            double width = 0.0;        ///< 0 mono (just panned) .. 1 independently rendered L/R (decorrelated)
            double fade_in_ms = 0.5, fade_out_ms = 4.0;
            uint64_t seed_offset = 0;  ///< change to get a different random take of just this layer

            // sources
            LayerSpec& osc(Wave w, Traj pitch_midi) { src = Source(); src.kind = SourceKind::Osc; src.wave = w; src.pitch = pitch_midi; return *this; }
            LayerSpec& noise(NoiseColor c, Traj density = 200.0) { src = Source(); src.kind = SourceKind::Noise; src.color = c; src.density = density; return *this; }
            LayerSpec& additive(Traj pitch_midi, std::vector<double> notes, int harmonics, Traj rolloff = 1.0) { src = Source(); src.kind = SourceKind::Additive; src.pitch = pitch_midi; src.notes = std::move(notes); src.harmonics = harmonics; src.rolloff = rolloff; return *this; }
            LayerSpec& modal(std::vector<Mode> modes, Traj pitch_midi, std::vector<double> hits) { src = Source(); src.kind = SourceKind::Modal; src.modes = std::move(modes); src.pitch = pitch_midi; src.hits = std::move(hits); return *this; }
            LayerSpec& formant(Traj vowel, Traj pitch_midi, double breath = 0.0) { src = Source(); src.kind = SourceKind::Formant; src.vowel = vowel; src.pitch = pitch_midi; src.breath = breath; return *this; }
            LayerSpec& grain(GrainSource gs, Traj density, Traj size_ms, Traj pitch_midi = 60.0) { src = Source(); src.kind = SourceKind::Grain; src.grain_source = gs; src.density = density; src.grain_ms = size_ms; src.pitch = pitch_midi; return *this; }
            LayerSpec& pluck(Traj pitch_midi, std::vector<double> hits, double damping = 0.5) { src = Source(); src.kind = SourceKind::Pluck; src.pitch = pitch_midi; src.hits = std::move(hits); src.damping = damping; return *this; }
            LayerSpec& chaos(ChaosKind k, Traj rate, Traj pitch_midi = 60.0) { src = Source(); src.kind = SourceKind::Chaos; src.chaos = k; src.chaos_rate = rate; src.pitch = pitch_midi; return *this; }

            // source options
            LayerSpec& unison(int voices, Traj cents) { src.voices = voices; src.detune_cents = cents; return *this; }
            LayerSpec& fm(Traj index, double ratio) { src.fm_index = index; src.fm_ratio = ratio; return *this; }
            LayerSpec& sub(double g) { src.sub = g; return *this; }
            LayerSpec& stack(std::vector<double> ratios) { src.ratios = std::move(ratios); return *this; }
            LayerSpec& pw(Traj w) { src.pulse_width = w; return *this; }
            LayerSpec& inharmonic(double b) { src.inharmonicity = b; return *this; }
            LayerSpec& spread(Traj cents) { src.partial_spread = cents; return *this; }
            LayerSpec& hit_notes(std::vector<double> n) { src.hit_notes = std::move(n); return *this; }
            LayerSpec& hit_jitter(double j, double vel_var = 0.3) { src.hit_jitter = j; src.velocity_var = vel_var; return *this; }
            LayerSpec& strike(double ms) { src.strike_ms = ms; return *this; }
            LayerSpec& decay_scale(double s) { src.t60_scale = s; return *this; }
            LayerSpec& scatter(double pitch_semis, double position_frac) { src.pitch_scatter = pitch_semis; src.position_scatter = position_frac; return *this; }
            LayerSpec& grain_pos(Traj p) { src.position = p; return *this; }
            LayerSpec& grain_pitch(Traj semis) { src.grain_pitch = semis; return *this; }
            LayerSpec& shift(Traj semis) { src.formant_shift = semis; return *this; }
            LayerSpec& whisper(double breath) { src.breath = breath; return *this; }
            LayerSpec& chaos_depth(Traj semis, double amp = 0.0) { src.chaos_depth = semis; src.chaos_amp = amp; return *this; }
            LayerSpec& chaos_out(bool direct) { src.chaos_direct = direct; return *this; }

            // shaping and placement
            LayerSpec& shape(Traj e) { env = e; return *this; }
            LayerSpec& after(Traj p) { post = p; return *this; }
            LayerSpec& gain(double g) { level = g; return *this; }
            LayerSpec& at(double sec) { start = sec; return *this; }
            LayerSpec& len(double sec) { length = sec; return *this; }
            LayerSpec& ring(double sec) { tail = sec; return *this; }
            LayerSpec& place(double p, double w = 0.0) { pan = p; width = w; return *this; }
            LayerSpec& fade(double in_ms, double out_ms) { fade_in_ms = in_ms; fade_out_ms = out_ms; return *this; }
            LayerSpec& variation(uint64_t s) { seed_offset = s; return *this; }

            // processors
            LayerSpec& lp(Traj cutoff, Traj q = 0.707, int poles = 2) { FilterP f; f.type = FilterP::LowPass; f.cutoff = cutoff; f.q = q; f.poles = poles; chain.push_back(f); return *this; }
            LayerSpec& hp(Traj cutoff, Traj q = 0.707, int poles = 2) { FilterP f; f.type = FilterP::HighPass; f.cutoff = cutoff; f.q = q; f.poles = poles; chain.push_back(f); return *this; }
            LayerSpec& bp(Traj cutoff, Traj q = 2.0, int poles = 2) { FilterP f; f.type = FilterP::BandPass; f.cutoff = cutoff; f.q = q; f.poles = poles; chain.push_back(f); return *this; }
            LayerSpec& notch(Traj cutoff, Traj q = 2.0) { FilterP f; f.type = FilterP::Notch; f.cutoff = cutoff; f.q = q; chain.push_back(f); return *this; }
            LayerSpec& filter_lfo(Mod m) { for (size_t i = chain.size(); i-- > 0;) if (auto* f = std::get_if<FilterP>(&chain[i])) { f->lfo = m; return *this; } throw Error("filter_lfo: no filter in the chain yet"); }
            LayerSpec& drive(Traj amount, DriveP::Kind k = DriveP::Tanh, double mix = 1.0) { DriveP d; d.kind = k; d.amount = amount; d.mix = mix; chain.push_back(d); return *this; }
            LayerSpec& crush(Traj bits, Traj down = 1.0) { CrushP c; c.bits = bits; c.down = down; chain.push_back(c); return *this; }
            LayerSpec& gate(std::vector<double> steps, double step_beats, double depth = 1.0, double smooth_ms = 3.0, double swing = 0.0) { GateP g; g.steps = std::move(steps); g.step_beats = step_beats; g.depth = depth; g.smooth_ms = smooth_ms; g.swing = swing; chain.push_back(g); return *this; }
            LayerSpec& gate_sec(std::vector<double> steps, double step_sec, double depth = 1.0, double smooth_ms = 2.0) { GateP g; g.steps = std::move(steps); g.step_sec = step_sec; g.depth = depth; g.smooth_ms = smooth_ms; chain.push_back(g); return *this; }
            LayerSpec& trem(Traj rate_hz, Traj depth) { TremP t; t.rate = rate_hz; t.depth = depth; chain.push_back(t); return *this; }
            LayerSpec& pulses(Traj rate_hz, double decay_ms, double depth = 1.0, double jitter = 0.0) { PulsesP p; p.rate = rate_hz; p.decay_ms = decay_ms; p.depth = depth; p.jitter = jitter; chain.push_back(p); return *this; }
            LayerSpec& pulses_at(std::vector<double> times, double decay_ms, double attack_ms = 0.5, double depth = 1.0) { PulsesP p; p.times = std::move(times); p.decay_ms = decay_ms; p.attack_ms = attack_ms; p.depth = depth; chain.push_back(p); return *this; }
            LayerSpec& stutter(double seg_sec, int repeats, double prob, double from = 0.0, double to = 1.0) { StutterP s; s.seg_sec = seg_sec; s.repeats = repeats; s.prob = prob; s.from = from; s.to = to; chain.push_back(s); return *this; }
            LayerSpec& freeze(double at_frac, double window_sec) { FreezeP f; f.at = at_frac; f.window_sec = window_sec; chain.push_back(f); return *this; }
            LayerSpec& dropout(double rate_per_sec, double min_ms, double max_ms, double depth = 1.0) { DropoutP d; d.rate_per_sec = rate_per_sec; d.min_ms = min_ms; d.max_ms = max_ms; d.depth = depth; chain.push_back(d); return *this; }
            LayerSpec& comb(Traj freq_hz, double feedback, double mix = 0.5) { CombP c; c.freq = freq_hz; c.feedback = feedback; c.mix = mix; chain.push_back(c); return *this; }
            LayerSpec& ringmod(Traj freq_hz, double mix = 1.0) { RingP r; r.freq = freq_hz; r.mix = mix; chain.push_back(r); return *this; }
            LayerSpec& reverb(double t60, double damping = 0.4, double mix = 0.3, double predelay_ms = 0.0) { ReverbP r; r.t60 = t60; r.damping = damping; r.mix = mix; r.predelay_ms = predelay_ms; chain.push_back(r); return *this; }
            LayerSpec& echo(double time_sec, double feedback, double mix = 0.4, double damping = 0.3) { EchoP e; e.time_sec = time_sec; e.feedback = feedback; e.mix = mix; e.damping = damping; chain.push_back(e); return *this; }
            LayerSpec& reverse() { chain.push_back(ReverseP{}); return *this; }
            LayerSpec& spectral(SpectralP::Mode m, Traj amount, double at_frac = 0.5) { SpectralP s; s.mode = m; s.amount = amount; s.at = at_frac; chain.push_back(s); return *this; }
        };

        struct Recipe {
            std::string name;
            double duration = 1.0;     ///< nominal length in seconds (what "duration" means to the caller)
            double tail = 0.0;         ///< extra seconds after `duration` (decay of reverb-like layers)
            double bpm = 120.0;        ///< tempo used by tempo-synced layers
            bool   reverse_all = false;///< reverse the finished mix (offline)
            double fade_in_ms = 1.0, fade_out_ms = 6.0;
            std::vector<LayerSpec> layers;

            LayerSpec& add(const std::string& layer_name) { layers.emplace_back(); layers.back().name = layer_name; return layers.back(); }
            double total_seconds() const {
                double t = duration + tail;
                for (const auto& l : layers) { const double len = l.length < 0.0 ? duration - l.start : l.length; t = std::max(t, l.start + len + l.tail); }
                return t;
            }
        };
    }
}
