# Wauvio-CPP

A C++17 (or newer) audio synthesis engine and music framework. What started as a single synthesis header grew into several layers that build on each other.

* `wauvio.hpp` - the actual audio engine. Oscillators, FM synthesis, filters, envelopes, reverb/chorus/delay/distortion/EQ, mixing, WAV export, playback. This is where all the samples are generated.
* `wauvio_ext/` - a higher-level layer on top of the existing engine. Instead of hand-wiring oscillators and envelopes, and manually writing instruments every time you want to produce a sound, you get 300+ pre-made instruments, ready to be instantiated and played, plus notes/chords/melodies/tracks/arrangements to compose with. It also contains the **genre presets** (see [Genres](#genres)).
* `wauvio_midi/` - loads a standard MIDI binary and turns it into playable Wauvio audio on its own. Simply point to a `*.mid` file, and let it do the rest. It also writes MIDI back out, and can restyle a loaded file in the manner of a genre.
* `wauvio_sf2/` - loads a real SoundFont 2 (`.sf2`) bank and turns its presets into the same kind of sampled instrument the rest of the library already uses, so a MIDI file can play back through actual recorded instrument samples instead of the built-in synthesized fallback.
* `wauvio_fx/` - an effects, dynamics, reverb and mastering layer that works on finished audio: EQ, compressor with real sidechain, true-peak limiter, multiband, saturation with oversampling, stereo delay, FDN and convolution reverb, an EBU R128 loudness meter, one-call mastering to a LUFS target, and a `Mixer` with per-track inserts, send buses and track-to-track sidechain. See [Wauvio FX](#wauvio-fx-wauvio_fx).
* `wauvio_sfx/` - 190 procedurally synthesised sound-effect presets in 12 categories (risers, impacts, glitches, textures, bass effects, ornaments, vocal-like textures, rhythmic effects, synthetic percussion, experimental sounds, tonal motion, reverses and tails), with validated parameters, seeds, tempo-aware timing, layering, custom recipes and an auditioning command-line tool. See [Wauvio SFX](#wauvio-sfx-wauvio_sfx).
* `wauvio_vocal/` - a Japanese singing-voice synthesizer (formant/glottal-source model, no samples). Two generations ship: `yune-v1` (frozen, the original single voice) and `yune-v2` (the current engine: separate voice profiles, personas, laughs and other vocal events, phoneme control, ornaments, speech). See [Vocal engine](#vocal-engine-wauvio_vocal).

Each layer only ends up depending on the ones below it (`wauvio_sf2` and `wauvio_midi` both sit on top of `wauvio_ext`, and can be combined together, but neither depends on the other; `wauvio_vocal` sits on `wauvio` + `wauvio_ext` and reuses the MIDI *parser* headers so a singer can read a `.mid`, but doesn't need the rest of `wauvio_midi`; `wauvio_fx` and `wauvio_sfx` need only `wauvio.hpp`, and two tiny optional adapter headers connect `wauvio_fx` to `wauvio_ext` and `wauvio_midi`). `wauvio.hpp` never gets touched by anything above it; if you're already using it directly, absolutely nothing would change for you.

## In this repository

```
wauvio.hpp
wauvio_ext.hpp
wauvio_midi.hpp
wauvio_sf2.hpp
wauvio_fx.hpp           (effects / mastering; needs only wauvio.hpp)
wauvio_fx_ext.hpp       (optional entry: fx::Mixer from an Arrangement)
wauvio_fx_midi.hpp      (optional entry: fx::Mixer from a MidiMusic)
wauvio_sfx.hpp          (sound-effect presets; needs only wauvio.hpp)
wauvio_vocal.hpp        (V1 entry point)
wauvio_vocal_v2.hpp     (V2 entry point)
wauvio_ext/
wauvio_midi/
wauvio_sf2/
wauvio_fx/
wauvio_sfx/
wauvio_vocal/
```
Yeah, that's it. Examples and demos will be accessible [here](https://github.com/artdoesstuff-ex/Wauvio-Graveyard), but as it stands, only the framework itself (and officially supported extensions) will remain here. However, if the examples/demos are too overwhelming at a glance, you can start writing it yourself almost immediately (it's like 3 lines; see below).

Folder layout in a bit more detail:

```
wauvio_ext/
  core/            Note/Melody/Track/Arrangement, ModeledInstrument, DrumKit, rendering
  samples/         sample engine (zones, layers, round robin, loops, release samples)
  instruments/     keyboards, strings, guitars, woodwinds (+ saxophones), brass, world,
                   percussion (orchestral, latin, tuned, kits), vocals, experimental, synthesized
  genre/           genre_types / genre_apply / genre_presets   (melody-level genre layer)
wauvio_midi/
  core/            byte reader, MIDI parser, tempo map, MIDI types
  resolution/      GM program map, instrument resolver
  track/           MidiMusic, loader, render (+ stems), MIDI writer
  genre/           genre_midi / genre_midi_presets             (file-level genre layer)
  playback.hpp
wauvio_sf2/
  core/            RIFF reader, SF2 types, SF2 parser
  sf2_instrument.hpp
wauvio_fx/
  core/            fx_common (dB, RNG, biquads, FFT), fx_effect (Effect, Chain), fx_io (WAV reader)
  effects/         eq, dynamics, saturation, modulation, delay, stereo, reverb
  mastering/       loudness (LUFS meter), master (mastering to a target)
  mixer/           mixer, from_arrangement (needs wauvio_ext), from_midi (needs wauvio_midi)
wauvio_sfx/
  core/            sfx_common (RNG, scales), sfx_traj (trajectories, LFOs), sfx_params, sfx_recipe (data model), sfx_helpers
  render/          sources, processors, engine
  presets/         registry (Preset, Registry, Effect), preset_helpers, builtin + the 12 catalog files
  layers/          layered (composition)
  tools/           wauvio_sfx_tool.cpp (list / info / render / catalog)
wauvio_vocal/
  yune-v1/         original single-voice singer (+ tools/sing_score.cpp)
  yune-v2/         current engine, voices/, tools/, V2_PROPOSALS.md (design rationale)
```

## Requirements

C++17 (or newer), and that's literally it. I specifically ensured not to include any external dependencies, nothing to link, nothing to install. Everything you'd need is inside the headers. The one exception: parallel rendering uses `std::thread`, so on Linux/GCC you'll want `-pthread` on the compile line if you use it (single-threaded rendering doesn't need it).

## Usage

Clone this repository into its own folder
```bash
git clone https://github.com/artdoesstuff/Wauvio.git
```
then copy (or move) the files into your project, and then just include what you need:
```cpp
#include "wauvio.hpp"           // needed no matter what
#include "wauvio_ext.hpp"       // instruments / notes / tracks / genre presets
#include "wauvio_midi.hpp"      // only if you're loading MIDI files
#include "wauvio_sf2.hpp"       // only if you're loading SF2 samples
#include "wauvio_fx.hpp"        // only if you want effects / mastering (also: wauvio_fx_ext.hpp, wauvio_fx_midi.hpp)
#include "wauvio_sfx.hpp"       // only if you want the sound-effect presets
#include "wauvio_vocal_v2.hpp"  // only if you want the singing voice (use ONE of v1 / v2, see below)
```

Compile with C++17 (or newer). There are no extra steps beyond compiling your own `*.cpp` file.

(Run `git pull` inside the cloned folder any time later to grab updates).

## Getting started
### The simplest thing possible
```cpp
#include "wauvio.hpp"
#include "wauvio_ext.hpp"

int main() {
    auto piano = wauvio::instruments::AcousticGrandPiano();
    auto note  = piano.play(60, 1.0); // middle C for a second

    wauvio::save_wav_stereo(note.audio, "note.wav");
}
```

### Putting a few instruments together
```cpp
#include "wauvio.hpp"
#include "wauvio_ext.hpp"

using namespace wauvio::audio;

int main() {
    auto piano  = wauvio::instruments::AcousticGrandPiano();
    auto violin = wauvio::instruments::SoloViolin();
    violin.reverb(0.5f, 0.2f);

    Arrangement song;
    song.add(piano,  Melody{ Note(60, 0.5), Note(64, 0.5), Note(67, 1.0) });
    song.add(violin, Melody{ Note(67, 1.0), Note(71, 1.0) }, 0.25);

    wauvio::save_wav_stereo(song.render(), "song.wav");
}
```

### Or simply give it a MIDI file
```cpp
#include "wauvio.hpp"
#include "wauvio_ext.hpp"
#include "wauvio_midi.hpp"

int main() {
    auto music = wauvio::track::load_midi("song.mid");
    wauvio::play(music);
}
```

This one is probably the best for most people. It parses the file, works out the tempo (including tempo changes mid-file), figures out which instrument each track/channel should be, and mixes them all together. You don't need to create an `Instrument` or `Track` for this by hand; it's genuinely just two lines.

Want it to sound like a particular style? One more line (details in [Genres](#genres)):
```cpp
music.setGenre(wauvio::midi::genre::presets::jazz());   // swing, jazz kit, hall-ish reverb...
```

### Or make it sing
```cpp
#include "wauvio_vocal_v2.hpp"
using namespace wauvio;

int main() {
    vocal::Singer s;                      // Yune, the default voice
    s.note(69, 480, "さ");                // MIDI note, ticks, lyric
    s.note(71, 480, "く");
    s.note(72, 960, "ら");
    Buffer b = s.render();                // mono
    normalize(b, 0.9f);
    save_wav(b, "sakura.wav");
}
```

# Wauvio Library Extension (`wauvio_ext`)

There are 300+ instrument names available (A bit over 300 actual classes, plus about 30 aliases for things that are the same under a different name: e.g. `Bongo`/`Bongos`, `Gong`/`TamTam`; and as a result, you aren't really stuck if you don't remember what name _we_ specifically picked). Rough grouping:

- **Keyboards**: grand/upright/felt/tack/prepared piano, Rhodes,
  Wurlitzer, Clavinet, harpsichord, celesta, Mellotron, a handful of
  organs, accordion and its relatives.
- **Strings**: solo and section violin/viola/cello/bass, Baroque violin
  and cello, viola da gamba, Hardanger fiddle, nyckelharpa, chamber and
  symphonic string sections.
- **Guitars & bass**: the usual acoustic/nylon/steel/electric family
  (clean, overdriven, distorted, muted, power chords, harmonics), plus
  mandolin, banjo, ukulele, harp, oud, bouzouki, balalaika, resonator
  guitar, and the full bass family (`BassGuitar`, `PickBass`, `SlapBass`,
  `MutedBass`, `FretlessBass`, `UprightBassPluck`, ...).
- **Woodwinds & saxophones**: pretty much the whole concert range,
  piccolo through contrabass.
- **Brass**: piccolo trumpet through tuba and sousaphone, plus some
  historical stuff like natural horn, alphorn, serpent, ophicleide.
- **World / folk**: erhu, guzheng, koto, shamisen, sitar, duduk,
  shakuhachi, kalimba, handpan, uilleann pipes, hurdy-gurdy, and more.
- **Percussion**: tuned percussion (including a properly unstable-sounding
  Waterphone), full kit/orchestral percussion, Latin/hand percussion, and
  10 ready-made drum kits with GM-style note mappings (see [Drum kits](#drum-kits)).
- **Vocals**: solo voices by range, various choir sizes, vowel presets,
  whisper/breath. (These are the lightweight instrument-style vocals; for
  actual lyrics and Japanese singing see [Vocal engine](#vocal-engine-wauvio_vocal).)
- **Experimental**: musical saw, theremin, Ondes Martenot, tape-style
  textures, that kind of thing. Good for weird ambient stuff.
- **Synths**: the usual lead/pad/bass/pluck/bell/arp/drone patches, plus
  `ChiptuneLead`, `ChiptuneBass`, `AtmosphericSynth` and `Drone`, built
  from Wauvio's own oscillators, no samples involved.

They're all real, usable classes:
```cpp
auto flute      = wauvio::instruments::Flute();
auto waterphone = wauvio::instruments::Waterphone();
auto theremin   = wauvio::instruments::Theremin();
```

### Drum kits

Ten ready-made `DrumKit`s live in `instruments/percussion/kits.hpp`, each mapping GM percussion note numbers to individual instruments:

| Kit | Flavour |
|---|---|
| `JazzKit` | brushes-and-ride jazz set |
| `RockKit` | punchy acoustic rock set |
| `StudioKit` | tight, dry studio set |
| `VintageKit` | older, looser kit |
| `ElectronicKit` | generic electronic kit |
| `IndustrialKit` | hard, metallic, mechanical |
| `OrchestralPercussionKit` | timpani, bass drum, cymbals, etc. |
| `LoFiKit` | crunchy 8-bit-ish kit |
| `Kit808` | the classic analogue drum-machine sound |
| `Kit909` | the other classic analogue drum-machine sound |

A kit is just a `DrumKit` object: `kit.map(note, instrument)`, `kit.get(note)`, `kit.mapped_notes()`, `kit.play(note, dynamics, duration, sample_rate)`. Build your own by mapping notes to any instruments you like. The MIDI genre layer uses kits (and adds one more, a bossa-nova kit built from surdo, claves, rimshot, hats, congas, bongos, timbales, cabasa, maracas and guiro).

### Notes, chords, dynamics, and articulations

```cpp
Note n(60, 0.5, Dynamics::mf, Articulation::Staccato);
auto c_major = wauvio::audio::scale::major(60);
```

Dynamics go from `ppp` to `fff`. Articulations: `Sustain`, `Legato`, `Staccato`, `Marcato`, `Accent`, `Soft`, `Hard`, `Pizzicato`, `Tremolo`, `PalmMute`, `Harmonic`, `Muted`, `Fall`, `Doit`, `FlutterTongue`, `Rimshot`, `Spiccato`, `SulPonticello`, `ColLegno`, `ShortStab`, plus a few extras like `Whisper`, `Breath` and `Humming` for vocals; and each instrument only lists the ones that actually make sense for it (`instrument.supports(articulation)`). They change the actual, audible sound, instead of just the label.

Notes also carry `expression`, `pitch_bend_semitones`, and a `glide_from_midi`/`glide_time` pair for portamento. These mostly get set automatically when you load a MIDI file, but nothing is stopping you from setting them yourself.

### Samples

Every instrument makes sound out of the box using synthesised fallback; you don't need any sampled audio files to get started; however, if you do, you can drop them in later without changing how you call the instrument:
```cpp
wauvio::instruments::AcousticGrandPiano piano;
piano.load_sample_file("piano_C4.wav", 60, 48, 72);
```
Underneath that, there's a real sample engine — key zones, velocity layers, round robin, loop points, release samples, pitch shifting — that stuff.

### Effects

`reverb()`, `chorus()`, `delay()`, `eq()`, `distortion()` on any
instrument, plus reverb/limiting at the `Arrangement` (master bus) level. For anything beyond that (real compressors and limiters, sidechain, stereo reverb, loudness metering, mastering) see [Wauvio FX](#wauvio-fx-wauvio_fx).

# Genres

A *genre preset* restyles music without you touching individual notes: it nudges timing (swing, quantizing, human looseness), velocity/accent shape, articulation, register, density, and the instruments' own tone and effects, and can scale the tempo. There are **18 presets**, and they exist at two levels:

| Level | Namespace | Works on | Use it when |
|---|---|---|---|
| Melody level | `wauvio::genre` (in `wauvio_ext`) | a `Melody` + an `Instrument` | you're composing with `Arrangement`/`Track` yourself |
| File level | `wauvio::midi::genre` (in `wauvio_midi`) | a whole `MidiMusic` | you loaded a `.mid` and want it in a style |

The file-level preset (`MidiGenrePreset`) *wraps* a melody-level one (`GenrePreset`) and adds the things that only make sense for a MIDI file: a drum kit and GM-program → instrument overrides.

## The presets

| Preset | Tempo × | Character | Drum kit (MIDI level) | Instrument swaps (MIDI level, by GM program) |
|---|---|---|---|---|
| `classical` | 1.00 | expressive human timing, legato, hall reverb | `OrchestralPercussionKit` | – |
| `jazz` | 1.00 | swung eighths, backbeat accents, loose timing (percussion thinned a little) | `JazzKit` | – |
| `rock` | 1.00 | straight, punchy backbeat, driven guitars | `RockKit` | 26, 27 → `OverdrivenElectricGuitar`; 81 → `SynthLead` |
| `metal` | 1.05 | tight/mechanical, heavy distortion, tightened low end | `IndustrialKit` | 26, 27, 29 → `DistortedElectricGuitar` |
| `house` | 1.04 | quantized four-on-the-floor, bright low end, light shuffle | `Kit909` | 38, 39 → `SynthBass`; 80 → `SynthLead`; 89 → `SynthPad` |
| `techno` | 1.08 | perfectly quantized, mechanical, driving and dark | `Kit808` | 38, 39 → `SynthBass`; 80 → `SynthLead`; 89 → `AtmosphericSynth` |
| `drum_and_bass` | 1.35 | fast, sub-register bass, shuffled breakbeat feel | `ElectronicKit` | 33, 38, 39 → `SynthBass` |
| `chiptune` | 1.00 | bright, quantized, dry 8-bit-style | `LoFiKit` | 0, 1, 2, 40, 41, 56, 65, 73, 80, 81, 84, 86 → `ChiptuneLead`; 32–39 → `ChiptuneBass` |
| `ambient` | 0.85 | sparse, slow, wide, reverberant | – | 88, 89 → `AtmosphericSynth`; 95 → `Drone` |
| `touhou` | 1.12 | bright, fast, driving fusion (melodic + rock elements) | `RockKit` | 26 → `OverdrivenElectricGuitar`; 81 → `SynthLead` |
| `funk` | 1.00 | syncopated, dry, ghost-note-heavy, hits hard on beat one | `StudioKit` | – |
| `blues` | 0.95 | loose shuffle, warm light overdrive, relaxed | `StudioKit` | 26, 27 → `OverdrivenElectricGuitar` |
| `reggae` | 0.92 | off-beat skank emphasis, bass that drags behind the beat | `JazzKit` | 27 → `CleanElectricGuitar` |
| `trance` | 1.06 | uplifting, quantized, bright and wide, slow filter movement | `Kit909` | 38, 39 → `SynthBass`; 80 → `SynthLead`; 89 → `AtmosphericSynth` |
| `dubstep` | 0.78 | heavy sub-bass wobble, sparse percussion, half-time feel | `ElectronicKit` | 33, 38, 39 → `SynthBass` |
| `bossa_nova` | 0.97 | gentle syncopation, soft dynamics, nylon-and-hand-percussion feel | bossa-nova kit (`bossa_nova_kit()`) | 24–27 → `NylonGuitar` |
| `hardcore` | 1.45 | very fast, perfectly mechanical, distorted electronic aggression | `Kit909` | 38, 39 → `SynthBass`; 80 → `SynthLead` |
| `speedcore` | 2.10 | extreme tempo, hyper-mechanical quantization, maximal distortion | `Kit808` | 38, 39 → `SynthBass` |

"Tempo ×" is a speed multiplier (2.1 = a bit more than twice as fast, 0.78 = slower). "Instrument swaps" are only used when instrumentation is switched on (see below). Every preset has its own presets for the three *roles* a part can play: `Melodic`, `Bass` and `Percussion` (e.g. metal distorts guitars but leaves the bass undistorted and darkens it instead).

## Using a genre with a MIDI file

```cpp
auto music = wauvio::track::load_midi("song.mid");

music.setGenre(wauvio::midi::genre::presets::techno());
wauvio::play(music);

music.setGenre(wauvio::midi::genre::presets::bossa_nova());   // replaces techno; does NOT stack on it
music.clearGenre();                                           // back to the file as loaded

if (music.has_genre()) std::cout << music.current_genre()->name();
```

Behaviour worth knowing:

- **Non-destructive and repeatable.** The first `setGenre` snapshots the loaded music (notes and each instrument's tone/effect settings). Every later `setGenre`/`clearGenre` starts again from that snapshot, so you can flip between genres freely without effects or timing piling up.
- **Role detection.** A part is `Percussion` if it's a drum part, `Bass` if its GM program is 32–39, otherwise `Melodic`.
- **What it does to each part, in order:** thin out quiet notes (`thin_probability`; notes at `f` or louder are never thinned), shift register, force an articulation (and, if the instrument doesn't support it, shorten the note by `fallback_gate_ratio` instead), quantize, swing off-beat eighths, apply a fixed timing offset (laid-back/pushed feel), add human timing jitter, then shape velocity (accent pattern per beat of the bar, downbeat de-emphasis, random variance, scale and floor). After that, the instruments are restyled (filter cutoff, detune, noise mix, stereo width, a filter "wobble" LFO, plus reverb/chorus/delay/distortion where the preset asks for them and the instrument doesn't already have them), and finally `tempo_scale` is applied to the whole thing.
- **Everything is deterministic.** Thinning, jitter and variance are hashed from the note itself plus `GenreApplyOptions::seed`, not a global RNG: same input and seed, same output, at any thread count.
- **Instrument/kit swapping is opt-in.** By default only timing/dynamics/tone change and your instruments stay as they were. To also swap in the preset's drum kit and GM-program overrides:

```cpp
wauvio::midi::genre::GenreApplyOptions go;
go.allow_instrumentation      = true;    // swap in the preset's kit / instruments
go.override_explicit_programs = false;   // true = also replace parts whose program came from the file
go.seed                       = 7;       // different "take" of the random parts
music.setGenre(wauvio::midi::genre::presets::house(), go);
```

  Parts you pinned (`music.pin_instrument(i)`) are never swapped. A part whose channel sent a Program Change in the file counts as "explicitly chosen" and is left alone unless `override_explicit_programs` is on (this includes drum channels that sent one).
- **Timing grid.** Quantizing, swing and accent patterns use the file's *initial* BPM and its first time signature (4/4 if none). Tempo changes mid-file are still honoured for playback, but the genre grid doesn't follow them, so heavy quantizing on a file with lots of tempo changes can land slightly off the grid.

## Using a genre without MIDI (melody level)

```cpp
auto jazz  = wauvio::genre::presets::jazz();
auto bass  = wauvio::instruments::BassGuitar();
wauvio::audio::Melody line{ Note(40, 0.5), Note(43, 0.5), Note(45, 1.0) };

wauvio::genre::apply_to_melody(line, *jazz, wauvio::genre::Role::Bass, &bass, /*seed=*/1);
wauvio::genre::style_instrument(bass, *jazz, wauvio::genre::Role::Bass);
```

Two honest limits of this level, because a `Melody` has no absolute clock or tempo to align to:

- `apply_to_melody` handles density thinning, register shift, articulation (only if the instrument supports it), and velocity/accent/variance (accents cycle per *note*, not per beat). It does **not** do swing, quantizing, humanize, gate shortening or `tempo_scale`; those happen on the MIDI path.
- `style_instrument` edits the instrument **in place** and is not self-reverting (the wobble route, for example, is appended each call). Call it once per instrument, or bracket it with `capture_style_baseline(instr)` / `restore_style_baseline(instr, baseline)` if you want to undo it. The MIDI path does that bookkeeping for you.

## Writing your own genre

```cpp
using namespace wauvio;

auto g = std::make_shared<genre::GenrePreset>();
g->name = "My Groove";
g->tempo_scale = 0.9;

genre::RoleProfile d;                    // default profile: used for every role...
d.swing_amount         = 0.4;
d.timing_humanize_sec  = 0.01;
d.accent_pattern       = {1.0, 0.7, 0.9, 0.7};
d.want_reverb          = true;
g->default_profile = d;

genre::RoleProfile perc = d;             // ...unless a role has its own
perc.thin_probability = 0.15;
g->set_percussion(perc);                 // also set_bass(...)

auto mine = std::make_shared<midi::genre::MidiGenrePreset>(g);   // make it usable on MIDI files
mine->percussion_kit = [] { return std::make_shared<instruments::StudioKit>(); };
mine->melodic_program_overrides[26] = [] { return std::make_shared<instruments::CleanElectricGuitar>(); };

music.setGenre(mine);
```

`RoleProfile` knobs, grouped:

| Group | Fields |
|---|---|
| Velocity | `velocity_scale`, `velocity_floor`, `velocity_variance`, `accent_pattern`, `downbeat_deemphasis` |
| Timing | `swing_amount`, `timing_humanize_sec`, `quantize_grid_fraction` (of a beat; 0.25 = 16ths), `timing_offset_sec` |
| Density / pitch | `thin_probability`, `register_shift_semitones` |
| Articulation | `force_articulation`, `articulation`, `fallback_gate_ratio` |
| Tone | `filter_cutoff_mult`, `detune_cents_delta`, `noise_mix_delta`, `stereo_width_override`, `wobble_amount` |
| Effects | `want_reverb` (+ room/wet/damping), `want_chorus` (+ rate/depth/wet), `want_delay` (+ time/feedback/wet), `want_distortion` (+ drive) |

Tone and effect fields only affect synthesised (`ModeledInstrument`) instruments; sampled instruments (including SF2-backed ones) get the timing/velocity/articulation treatment and effects but not the recipe edits.

# Wauvio MIDI (`wauvio_midi`)

The entire point of this layer (and its creation) is that you shouldn't have to manually map every track in a MIDI file to an instrument just to hear it played back.
`load_midi()`:

1. Parses the actual binary file: header/track chunks, variable-length quantities, running status, all of it. No external MIDI library; it's all in here.
2. Builds a tempo map. If the file changes tempo halfway through, that gets respected instead of averaged away.
3. Splits things up by track, channel, _and_ whatever GM program is active at the time. This matters more than it sounds like it should, because some MIDI exports switch GM program mid-track just to signal a change in articulation, and if you only look at channel, you'd miss that.
4. Picks an instrument for each part automatically, using a General MIDI program table that covers all 128 programs plus the standard percussion key map. If a track never sends a program change at all (happens), it'll attempt to guess from the track name before giving up and using piano as the default.
5. Applies velocity, sustain pedal, portamento, pitch bend, and expression/volume, so it's not just "play this note at this fixed volume".
6. Hands you back something you can render or play.

If you don't like how a particular automatic choice may sound, you may override it:
```cpp
wauvio::track::LoadOptions opts;
opts.resolver.override_program(40, [] {
    return std::make_shared<wauvio::instruments::HardangerFiddle>();
});
auto music = wauvio::track::load_midi("music.mid", opts);

music.set_instrument<wauvio::instruments::SoloViolin>(0);
```
Overrides work at the level of the specific program, a specific percussion note, a whole channel, or just swapping out a part's instrument after the fact.

`LoadOptions` has a few more switches:

| Option | Default | Meaning |
|---|---|---|
| `honor_sustain_pedal` | `true` | CC64 extends held notes |
| `honor_portamento` | `true` | portamento CC turns into glides |
| `pitch_bend_range_semitones` | `2.0` | initial bend range (RPN changes in the file still apply) |
| `default_glide_time` | `0.08` | seconds, used when a glide has no explicit time |
| `retain_raw_midi` | `true` | keep the parsed file so `save_midi` can reproduce the original tempo map |

A loaded `MidiMusic` is a list of `MidiPart`s (name, channel, GM program, bank, notes, and either an instrument or a drum kit) that you can inspect and edit: `set_instrument`, `set_drum_kit`, `pin_instrument` / `unpin_instrument` (protects a part from being swapped by a genre), `render_part`, and so on.

Bad input gets an error instead of crashing: missing file, broken header, truncated data, corrupt variable-length values, invalid running status; those things throw a `wauvio::midi::MidiParseError` with a message that tells you exactly what went wrong.

For styling a loaded file as a particular genre, see [Genres](#genres).

# Wauvio SoundFont (`wauvio_sf2`)

Every instrument in `wauvio_ext` sounds fine out of the box through synthesis, but if you have a real SoundFont (`.sf2`) file and want actual recorded instrument samples instead, this loads one directly into the same `SampledInstrument`/`MultiSample` machinery the rest of the library already uses; it's not a separate sample engine bolted on the side.

```cpp
#include "wauvio.hpp"
#include "wauvio_ext.hpp"
#include "wauvio_sf2.hpp"

wauvio::sf2::SoundFont sf("GeneralUser.sf2");
auto piano = sf.create_instrument(/*bank=*/0, /*bank_lsb=*/0, /*program=*/0);
auto note = piano->play(60, 2.0);
```

It parses the actual RIFF/hydra structure (INFO, sample data, presets, instruments, zones, generators; all of it), not just enough to fake a preset list. Key/velocity ranges, root key, coarse/fine tune, pan, attenuation, loop points and loop mode, the volume envelope, and the low-pass filter generators all get resolved (including the preset-level generators that are meant to add on top of the instrument-level ones, per the SF2 spec) into a real `SampleZone` per sample region. All the sample audio for one loaded SoundFont is decoded once and shared (via `shared_ptr`) across every instrument and zone built from it, so loading a big font and pulling a dozen instruments out of it doesn't duplicate megabytes of PCM data a dozen times over.

To use it with the MIDI loader so a `.mid` file automatically plays through real samples wherever a matching preset exists:

```cpp
auto sf = std::make_shared<wauvio::sf2::SoundFont>("GeneralUser.sf2");
wauvio::track::LoadOptions opts;
opts.resolver.set_bank_provider(sf);
auto music = wauvio::track::load_midi("song.mid", opts);
wauvio::play(music);
```

If a bank/program isn't in the font, resolution just falls back to the normal synthesised GM instrument; nothing breaks, it just sounds like it did before you had a SoundFont.

Percussion works the same way: `sf.create_percussion_instrument(bank, bank_lsb, gm_note)` finds a drum kit preset (GM convention: bank 128, falling back to 120) and builds an instrument from it.

Known limits: modulators are read but not wired into a live MIDI-CC-driven routing graph yet (velocity's effect on loudness works through the normal note-velocity path, not the SF2 modulator table specifically); exclusive classes are parsed and stored on each zone but not yet enforced (a hi-hat preset with open/closed choke groups won't actually choke each other); and bank LSB is accepted but SF2's own single `wBank` field is matched against bank MSB only, since that's what the format actually stores.

# Wauvio FX (`wauvio_fx`)

An effects, dynamics, reverb and mastering layer that works on **finished audio** (a `StereoBuffer`, or a `Buffer` as dual-mono). It needs only `wauvio.hpp`, is header-only, has no dependencies, and is fully deterministic: noise and modulation are seeded, never drawn from a global RNG, so the same input gives bit-identical output.

The instrument-level `reverb()`/`chorus()`/`delay()`/`eq()`/`distortion()` calls and the core `Flanger`/`Phaser`/`Chorus` stay exactly as they were; `wauvio_fx` adds the things they can't do, and the core effects can run inside an fx `Chain` through `DualMono<T>`.

```cpp
#include "wauvio.hpp"
#include "wauvio_ext.hpp"
#include "wauvio_fx.hpp"
using namespace wauvio;

int main() {
    auto piano = wauvio::instruments::AcousticGrandPiano();
    StereoBuffer b = piano.play(60, 2.0).audio;

    fx::Chain chain;
    chain.add<fx::EQ>().highpass(80, 24).peak(3000, 2.0).high_shelf(10000, 1.5);
    auto& comp = chain.add<fx::Compressor>();
    comp.threshold_db = -20; comp.ratio = 3;
    chain.add<fx::FdnReverb>().mix = 0.2;

    chain.process(b, 44100);                 // b grows by the reverb tail; latency is compensated
    save_wav_stereo(b, "out.wav");
}
```

## How effects work

- **Parameters are plain public fields** (`comp.threshold_db = -20`) and are re-read at the start of every `process()` call. They are not smoothed, so changing one *between blocks of a running stream* can click; for offline rendering this never matters.
- **`Chain`** runs effects in order over a whole buffer. `chain.add<E>(...)` constructs the effect in place and returns a reference. A chain resets every effect first (so rendering twice gives identical results), **removes the latency** of lookahead effects so the output lines up with the input, and **extends the buffer by the tails** of reverbs and delays (`keep_tail = false` keeps the original length). Effects with `bypass = true` are skipped, along with their latency and tail.
- **Streaming**: every effect also has a raw `process(float* L, float* R, size_t n)` that you can call block by block after `prepare(sample_rate)`. It is tested to give the same result whether you pass one big block or several small ones. In that mode latency is *not* compensated; `latency_samples()` and `tail_seconds()` tell you what to account for.
- Every effect is checked at 22.05, 44.1, 48 and 96 kHz for finite output, and reverb decay times and loudness readings are checked for correct values at several rates.

```cpp
fx::Compressor c;
c.prepare(44100);
c.process(L, R, n);            // call repeatedly, state carries over
```

## The effects

| Effect | What it does | Main parameters |
|---|---|---|
| `EQ` | Any number of bands: high/low-pass at 12/24/36/48 dB per octave (Butterworth), shelves, peaks, notch. `response_db(f)` gives the exact curve. | `.highpass(f, slope)` `.lowpass` `.low_shelf(f, dB)` `.high_shelf` `.peak(f, dB, q)` `.notch(f, q)` |
| `Compressor` | Soft knee, peak or RMS detector, stereo link, parallel mix, detector high-pass, **external sidechain key**, gain-reduction meter. | `threshold_db` `ratio` `attack_ms` `release_ms` `knee_db` `makeup_db` `mix` `stereo_link` `sidechain_hpf_hz` `detector` |
| `Limiter` | Lookahead brickwall; sample peaks never exceed the ceiling; inter-sample (true) peak is detected with a 16-tap interpolator. Also a loudness maximiser via `input_gain_db`. | `ceiling_db` `input_gain_db` `release_ms` `lookahead_ms` `true_peak` |
| `Gate` | Open/close with hold and hysteresis. | `threshold_db` `range_db` `attack_ms` `hold_ms` `release_ms` |
| `Pump` | Tempo-synced "sidechain" ducking **without needing a kick track**: a gain curve placed on a beat grid. | `bpm` `depth` `release_ms` `shape` `pattern` `loop_beats` `offset_sec` |
| `MultibandCompressor` | Three bands, Linkwitz-Riley crossovers that sum perfectly flat (within 0.1 dB) when nothing is compressed. | `low_freq` `high_freq` and `low` / `mid` / `high` band settings |
| `Saturator` | Six shapers (`Soft`, `Tube`, `Tape`, `Hard`, `Fold`, `Asymmetric`) with optional 2x/4x/8x oversampling to keep the added harmonics from aliasing. | `mode` `drive_db` `bias` `tone_hz` `mix` `output_db` `oversample` |
| `Bitcrusher` | Bit depth (2^bits - 1 levels) and sample-rate reduction, optional deterministic dither. | `bits` `downsample` `dither` `mix` |
| `Tremolo`, `AutoPan` | LFO amplitude / equal-power panning; free-running or tempo-synced; stereo phase offset. | `rate_hz` or `sync_bpm` + `sync_beats`, `depth`, `shape` |
| `StereoChorus` | Up to 6 voices per side with cubic-interpolated delays. | `rate_hz` `depth_ms` `delay_ms` `voices` `spread` `mix` |
| `StereoDelay` | Tempo-syncable (`div::eighth`, `div::dotted(...)`, `div::triplet(...)`), independent L/R times, ping-pong, filtered feedback (each repeat gets darker), optional tape wobble. | `time_ms` or `sync_bpm` + `sync_beats_left/right`, `feedback`, `ping_pong`, `lowcut_hz` `highcut_hz`, `mix` |
| `StereoImager` | Mid/side width, mid and side gain, **mono below a frequency** (keeps the bass centred), balance. | `width` `mid_gain_db` `side_gain_db` `mono_below_hz` `balance` |
| `Utility` | Gain, equal-power pan, mute, polarity, channel swap, mono sum, DC removal. | |
| `FdnReverb` | 8-line feedback delay network with a lossless Householder matrix, per-line damping, pre-delay, allpass diffusion and slow modulation. True stereo (the two outputs are decorrelated). | `decay_s` `predelay_ms` `size` `damping_hz` `diffusion` `width` `lowcut_hz` `highcut_hz` `mix` |
| `ConvolutionReverb` | Partitioned FFT convolution with any impulse response (see below). | `set_ir(...)` `load_ir(path)` `mix` `predelay_ms` |
| `DualMono<T>` | Runs a core Wauvio mono effect (`Flanger`, `Phaser`, `Chorus`, ...) in a `Chain`. | `.configure([](Flanger& f){ f.rate_hz = 0.3f; })` |

Every `mix` is a dry/wet crossfade (0 = dry, 1 = wet).

### Sidechain compression

```cpp
fx::Compressor duck;
duck.threshold_db = -30; duck.ratio = 8; duck.attack_ms = 1; duck.release_ms = 120;
duck.prepare(44100);
duck.process_sidechain(pad.L.data(), pad.R.data(), pad.size(), kick.data(), kick.data());
```

No kick track to key from (for example a generated song)? Use `Pump`, which applies the same kind of ducking from a tempo and a hit pattern:

```cpp
fx::Chain c;
auto& p = c.add<fx::Pump>();
p.bpm = 128; p.depth = 0.8; p.release_ms = 250;
c.process(pad, 44100);
```

For several tracks at once, `Mixer::sidechain` (below) wires one track to duck another.

### Reverb

`FdnReverb` is the quick, always-available one. **`decay_s` is the RT60 at low frequencies**; `damping_hz` makes the highs die faster than that (set it to 0 to switch damping off), and the slow modulation costs a little more high-frequency decay. With damping and modulation off, the measured RT60 matches `decay_s` to within about 6% at every sample rate tested; with the defaults the full-band decay measures somewhat shorter, which is what damping is for.

`ConvolutionReverb` uses an impulse response from one of three sources:

```cpp
fx::ConvolutionReverb cv;
cv.set_ir(fx::ir::hall());                 // procedural: ir::room() chamber() hall() cathedral() plate()
cv.load_ir("my_church.wav");               // a WAV (8/16/24/32-bit PCM or float; resampled to the render rate if needed)
cv.set_ir(my_stereo_buffer, 44100);        // or a buffer you already have
```

The procedural impulse responses (`ir::Params`: `rt60`, `predelay_ms`, `hf_damping`, `early`, `stereo`, `seed`) are a seeded noise tail that decays exponentially and darkens over time, plus sparse early reflections. The same parameters and seed always make the same IR. By default the IR is scaled to unit energy so the wet level is about the input level. A stereo IR is applied as left-input x left-IR and right-input x right-IR (dual mono), not as a full four-path "true stereo" matrix. The effect has a fixed latency of 512 samples, which `Chain` removes for you.

## Loudness and mastering

`measure_loudness()` implements ITU-R BS.1770-4 / EBU R128: K-weighting, 400 ms blocks with the absolute (-70 LUFS) and relative (-10 LU) gates, momentary and short-term maxima, loudness range (LRA), sample peak and a 4x-oversampled true peak. A 1 kHz stereo sine at -23 dBFS reads -22.99 LUFS (EBU reference: -23.0), at 44.1 and 48 kHz.

```cpp
fx::LoudnessReport m = fx::measure_loudness(buf, 44100);   // m.integrated_lufs, m.true_peak_db, m.loudness_range_lu ...
fx::normalize_loudness(buf, 44100, -14.0, -1.0);           // gain (+ limiter) to -14 LUFS under a -1 dBTP ceiling
```

`master()` goes further: EQ, optional saturation, multiband compression, glue compression, stereo width with mono bass, then a limiter driven to the loudness target. It checks the result with the accurate meter and tightens the limiter if the true peak is still over the ceiling, so the ceiling is met for real (within 0.05 dB in the tests), not just estimated.

```cpp
fx::MasterReport rep = fx::master(buf, 44100, fx::master_for_genre("techno"));
// rep.before / rep.after (LoudnessReport), rep.target_reached, rep.limiter_reduction_db
```

| Preset | Character | Default target |
|---|---|---|
| `Clean` | transparent: high-pass and light glue | -14 LUFS |
| `Warm` | gentle low-end lift, softer top, tape saturation | -14 LUFS |
| `Club` | strong lows, a little presence and air, multiband, soft saturation, wider, fast glue | -9 LUFS |
| `Loud` | like `Club` with heavier compression and saturation | -8 LUFS |
| `Gentle` | very light touch for dynamic music | -16 LUFS |
| `LoFi` | band-limited, tape-saturated, slightly narrowed | -16 LUFS |

Start from `MasterSettings::for_preset(...)` and change `target_lufs`, `ceiling_dbtp`, `glue`, `saturation`, `width`, `tilt_db` or `bass_mono`. `build_master_chain(settings)` returns the pre-limiter chain if you want to inspect or edit it.

`master_for_genre(name)` maps the genre presets to a starting point (case, spaces and punctuation are ignored; "Drum & Bass", `drum_and_bass` and `dnb` all work; unknown names get `Clean` at -14 LUFS). These are starting points, not rules:

| Genres | Mastering preset | Target |
|---|---|---|
| classical, ambient | `Gentle` | -18 LUFS |
| jazz, bossa nova | `Warm` | -16 LUFS |
| blues | `Warm` | -15 LUFS |
| reggae | `Warm` | -14 LUFS |
| funk | `Warm` | -13 LUFS |
| chiptune | `Clean` | -12 LUFS |
| rock | `Loud` | -11 LUFS |
| touhou | `Loud` | -10 LUFS |
| metal | `Loud` | -9 LUFS |
| house, trance | `Club` | -9 LUFS |
| techno, dubstep, drum and bass | `Club` | -8 LUFS |
| hardcore, speedcore | `Loud` | -7 LUFS |

Because it goes by name, it works directly with the [genre layer](#genres), without either library knowing about the other:

```cpp
auto music = wauvio::track::load_midi("song.mid");
music.setGenre(wauvio::midi::genre::presets::techno());
fx::Mixer mix = fx::mixer_from_midi(music);
mix.set_mastering(fx::master_for_genre(music.current_genre()->name()));
fx::write_wav(mix.render(), "song.wav");
```

> **Don't normalise a mastered file.** `RenderOptions::normalize` defaults to *on* in the normal render path and would rescale the result to a peak of 0.95, undoing the loudness target. Use `fx::write_wav(...)` (below), which never normalises.

## Mixer

A `Mixer` takes already-rendered sources and gives each one a signal path: an insert chain, an optional sidechain, a fader, sends to shared buses, then a master chain and an optional mastering stage.

```cpp
fx::Mixer mix = fx::mixer_from_arrangement(song);     // one source per track
mix.insert(0).add<fx::EQ>().highpass(150, 24);        // per-track insert chain
size_t verb = mix.add_bus("verb");                    // shared reverb bus
mix.bus(verb).add<fx::FdnReverb>().mix = 1.0;
mix.send(0, verb, 0.3);                               // 30 % of track 0 into it (post-fader by default)
mix.sidechain(0, 1);                                  // track 0 ducks under track 1 (e.g. pad under kick)
mix.master().add<fx::Compressor>();                   // chain on the sum
mix.set_mastering(fx::master_for_genre("house"));     // optional loudness stage

StereoBuffer out = mix.render();
fx::write_wav(out, "song.wav");                       // writes the level exactly as is (clamped, never normalised)
```

- `mixer_from_arrangement(arr, opts)` (in `wauvio_fx_ext.hpp`) and `mixer_from_midi(music, opts)` (in `wauvio_fx_midi.hpp`) build the sources; or add your own with `mix.add_source({"name", buffer, start_sec, gain})`. Rendering the tracks uses `RenderOptions::worker_threads`, and the result is bit-identical at any thread count.
- Order of operations: insert chains, then sidechain ducking (keys are each trigger's post-insert, pre-fader signal), then fader, then sends and buses, then the sum, the master chain and the mastering stage. Insert and bus chains keep their reverb and delay tails.
- `mixer_from_arrangement` does **not** apply the `Arrangement`'s own `master_reverb()` / limiter; use fx effects for that. `mixer_from_midi` gives one source per MIDI part and honours a genre set with `setGenre`.
- `last_master_report()` returns the `MasterReport` of the last render.
- Bad bus names and sidechain indices throw `std::out_of_range`.

## Limits and notes

- **Offline only.** There is no real-time audio output; the streaming `process()` API is there for block-wise processing, not for a live device.
- **Limiter true peak.** The `Limiter` alone keeps inter-sample peaks within about 0.3 dB of its ceiling (any 4x method has an inherent error of up to ~0.7 dB on worst-case material); `master()` and `normalize_loudness()` verify with the oversampling meter and close that gap.
- **Memory.** A `Mixer` holds every source in memory at once.
- **No parameter smoothing or automation** inside effects (see above). Changing a parameter mid-stream can click.
- **Saturator/Oversampler latency** is exact and integer (2x: 48, 4x: 72, 8x: 84 samples) and is compensated by `Chain`; the saturator keeps its dry path aligned for the `mix` control.
- `Limiter` latency is the lookahead plus 7 samples when `true_peak` is on.

## Tests

`tests/wauvio_fx/run.sh [compiler flags]` compiles every header on its own (with `-Wall -Wextra -Werror`), then runs two suites: `test_fx.cpp` (321 checks on the core library) and `test_fx_ext.cpp` (13 checks on the `Arrangement` and MIDI adapters). They measure known values rather than just "it runs": gain-computer curves, attack constants, limiter ceilings, EQ response against `response_db`, oversampler latency and passband, aliasing reduction, RT60 of the reverb against `decay_s` (at three sample rates), FFT convolution against direct convolution, the EBU loudness reference, mastering targets and true-peak ceilings, and for every effect: determinism, one block vs two blocks, silence in/silence out, finite output on noise/DC/huge impulses, and `reset()`. As with the vocal tests, they prove the controls do what they claim; **they are not listening tests.**

# Wauvio SFX (`wauvio_sfx`)

Procedurally synthesised sound-effect presets and reusable effect recipes for Wauvio. 190 presets in 12 categories: risers, impacts, glitches, textures, bass effects, ornaments, vocal-like textures, rhythmic effects, synthetic percussion, experimental sounds, tonal motion, and reverses and tails. Everything is generated from oscillators, noise, filters and other processing at render time; there are no recorded samples.

The library is aimed at music production: transitions, builds, drops, ornaments and sound design. It does not schedule anything. You decide where each effect goes, and you can mix the results with `wauvio_fx`, an `Arrangement`, or your own code.

### Quick start

```cpp
#include "wauvio_sfx.hpp"

int main() {
    namespace sfx = wauvio::sfx;

    sfx::Effect riser = sfx::make("noise_riser");
    riser.set("duration", 2.0).set("brightness", 0.8).seed(7);

    sfx::Rendered audio = riser.render();     // 44.1 kHz stereo, peak -1 dBFS
    audio.save("riser.wav");
}
```

```bash
g++ -std=c++17 -O2 -I<repo root> your_program.cpp -o your_program
```

Find what is available with the command-line tool (`wauvio_sfx_tool list`, `info`, `search`) or from code (`sfx::all_presets()`, `sfx::search("riser")`).

### Installation and requirements

- C++17, nothing else. No external libraries and no linking; the library depends only on `wauvio.hpp`.
- Copy `wauvio_sfx.hpp` and the `wauvio_sfx/` folder next to `wauvio.hpp`, then `#include "wauvio_sfx.hpp"`. `tests/` and `examples/` are optional.
- No threads are used. The command-line tool uses `<filesystem>` (GCC 9 or newer needs no extra flag; GCC 8 needs `-lstdc++fs`).
- Everything lives in `wauvio::sfx`. The core library is not modified.

Layout (same conventions as the other libraries: an entry header plus a folder of headers grouped by area):

```
wauvio_sfx.hpp          entry point
wauvio_sfx/
  core/                 sfx_common (errors, RNG, scale and chord tables), sfx_traj (trajectories, LFOs),
                        sfx_params, sfx_recipe (the data model), sfx_helpers (envelopes, patterns)
  render/               sources, processors, engine
  presets/              registry (Preset, Registry, Effect), preset_helpers, builtin, and the 12 catalog files
  layers/               layered (Layered)
  tools/                wauvio_sfx_tool.cpp
tests/wauvio_sfx/       test_sfx.cpp, run.sh
examples/wauvio_sfx/    01 .. 08
```

### Public API

#### Discovery

```cpp
const std::vector<sfx::Preset>& all_presets();
const sfx::Preset* find(std::string_view id);                 // nullptr if unknown
std::vector<const Preset*> presets_in(sfx::Category c);
std::vector<const Preset*> search(std::string_view text);     // case-insensitive over id, name, tags, description
bool parse_category(std::string_view text, Category& out);    // id, display name, or unambiguous prefix
const char* category_id(Category), category_name(Category);   // "ear_candy", "Ear Candy & Ornaments"
```

A `Preset` has `id`, `name`, `description`, `category`, `behavior` (one-shot, evolving or sustained), `tags`, `params` (name, default, min, max, unit, description), `default_seed`, and `reversed` (true if the effect is built by reversing audio, so it is rendered offline). Ids are stable and machine-friendly (`orchestral_style_synthetic_hit`): lower case, digits and underscores, derived from the name. Do not use list positions as identifiers.

#### Instantiating and rendering

```cpp
sfx::Effect e = sfx::make("cinematic_impact");     // throws sfx::Error for an unknown id, with suggestions
e.set("sub", 1.0).set("tail", 4.0);                // validated; throws sfx::Error for unknown names or bad values
e.seed(42).tempo(120).duration_beats(4);           // see Determinism and Tempo
sfx::Recipe r = e.recipe();                        // what would be rendered, for inspection
sfx::Rendered audio = e.render(sfx::RenderSettings());
```

`RenderSettings`:

| field | default | meaning |
|---|---|---|
| `sample_rate` | 44100 | 8000 to 192000 |
| `channels` | 2 | 1 or 2. With 1, both channels of `Rendered` are identical and `save()` writes a mono file |
| `normalize` | true | scale the finished effect so its peak equals `peak_db` |
| `peak_db` | -1 | target peak, -60 to 0 |
| `max_seconds` | 120 | effects longer than this are refused (memory guard) |
| `dc_cutoff_hz` | 12 | DC blocker; 0 turns it off |

`Rendered` holds `left`, `right`, `sample_rate`, `channels`, and offers `frames()`, `seconds()`, `stats()` (peak, RMS, DC, clipping ratio, crest factor, silent flag), `to_stereo()` (a `wauvio::StereoBuffer`), `to_mono()` and `save(path)`.

Errors are `sfx::Error` (derived from `std::runtime_error`). Invalid input is rejected, never silently clamped.

### Parameters

Parameters come from one shared vocabulary, so the same name means the same thing in every preset. Each preset declares only the ones that make sense for it, with its own defaults; `wauvio_sfx_tool info ID` lists them. `duration` is always present.

| name | unit | default* | range | meaning |
|---|---|---|---|---|
| `bits` | bits | 6 | 1 .. 16 | Bit depth used by digital degradation (lower is rougher). |
| `body` | 0..1 | 0.8 | 0 .. 1 | Strength of the main body layer. |
| `brightness` | 0..1 | 0.6 | 0 .. 1 | Spectral brightness: filter ceilings and high-frequency content. |
| `chord` | index | 0 | 0 .. 10 (whole numbers) | Chord type used for tonal content. |
| `decay` | 0..1 | 0.5 | 0 .. 1 | Decay length of the resonant or percussive part, from short to long. |
| `density` | 0..1 | 0.5 | 0 .. 1 | Event or grain density, from sparse to dense. |
| `depth` | 0..1 | 0.7 | 0 .. 1 | Modulation or gating depth. |
| `duration` | s | 2 | 0.05 .. 60 | Nominal length of the effect (the tail, if any, is appended after it). |
| `intensity` | 0..1 | 0.6 | 0 .. 1 | Overall aggressiveness: drive, resonance, modulation strength. |
| `pitch` | midi | 60 | 12 .. 108 | Root note of the effect. |
| `range` | semitones | 24 | 0 .. 72 | Size of the pitch or frequency movement. |
| `rate` | Hz | 8 | 0.1 .. 1000 | Modulation, retrigger, flutter or attractor-evolution rate. |
| `scale` | index | 1 | 0 .. 10 (whole numbers) | Scale used for tonal content. |
| `spread` | 0..1 | 0.5 | 0 .. 1 | Detune, inharmonicity or cluster width. |
| `steps` | count | 8 | 2 .. 32 (whole numbers) | Number of steps or notes. |
| `sub` | 0..1 | 0.6 | 0 .. 1 | Strength of the sub-bass layer. |
| `subdivision` | beats | 0.25 | 0.0625 .. 4 | Length of one rhythmic step in beats (0.25 = sixteenth note). |
| `swing` | 0..0.5 | 0 | 0 .. 0.5 | Delay applied to every second rhythmic step, as a fraction of a step. |
| `tail` | s | 0 | 0 .. 20 | Seconds of synthetic tail appended after the duration. |
| `transient` | 0..1 | 0.7 | 0 .. 1 | Strength of the attack transient. |
| `width` | 0..1 | 0.6 | 0 .. 1 | Stereo width, from mono to fully decorrelated. |

\* The dictionary default; every preset overrides the ones it declares.

Scales (`scale`): 0 chromatic, 1 major, 2 natural minor, 3 harmonic minor, 4 pentatonic major, 5 pentatonic minor, 6 whole tone, 7 dorian, 8 phrygian, 9 lydian, 10 mixolydian.
Chords (`chord`): 0 major, 1 minor, 2 sus2, 3 sus4, 4 major seventh, 5 minor seventh, 6 dominant seventh, 7 diminished, 8 augmented, 9 add9, 10 cluster.
Tonal presets build on `pitch` (the root), `scale` and `chord`, so nothing is tied to one key. The helpers `sfx::scale_degrees(i)`, `sfx::scale_step(...)` and `sfx::chord_intervals(i)` expose the tables.

Because the finished effect is peak-normalised, `intensity`, `body`, `sub` and similar parameters change the sound, not the final level.

### Categories

| id | name | presets |
|---|---|---|
| `risers` | Risers & Sweeps | 16 |
| `impacts` | Impacts & Transitions | 16 |
| `glitch` | Glitch & Digital | 16 |
| `textures` | Textures & Atmospheres | 16 |
| `bass` | Bass FX | 16 |
| `ear_candy` | Ear Candy & Ornaments | 16 |
| `vocal` | Vocal Production FX | 16 |
| `rhythmic` | Rhythmic FX | 16 |
| `percussion` | Synthetic Percussion & Cymbals | 14 |
| `experimental` | Experimental & Extreme | 16 |
| `tonal` | Musical Motion & Tonal FX | 16 |
| `reverses` | Reverses, Tails & Transitions | 16 |

All 190 presets are implemented, with the names and order of the original specification. None was merged, renamed or replaced. the [preset catalog](#preset-catalog) lists every preset with its id, behaviour, default duration, tags and description; it is generated by `wauvio_sfx_tool catalog`.

Notes on specific groups:

- **Vocal Production FX** are synthetic vocal-*like* textures: formant filters on a pulse or noise source. They do not sound like realistic human voices and are not a replacement for `wauvio_vocal`.
- **Reverse effects** are rendered forwards and then reversed (offline). `Preset::reversed` is true for them.
- **Tails** use the small synthetic reverb and feedback echoes of this library. They are not convolution reverb.
- **Experimental presets** are bounded by construction: attractors are clamped and reset if they ever leave their range, comb feedback is capped below 0.95, filter resonance is capped, and the engine normalises the result.

### Rendering model

An effect is a `Recipe`: a list of layers plus timing. Each layer renders like this:

1. **Source**: oscillator (sine, triangle, saw, square, pulse; unison, stacks, FM), coloured noise or dust, additive partials, modal resonances (struck inharmonic bodies), formant synthesis, granular cloud, Karplus-Strong pluck, or a chaotic attractor (Lorenz, logistic, Henon). The source is scaled to a standard level.
2. **Envelope** (`shape`): an amplitude trajectory applied to the source, before the chain, so reverb and echo tails can ring on after it.
3. **Processor chain**: low/high/band-pass and notch filters (2 or 4 pole, optional LFO), drive (tanh, hard clip, wavefold, asymmetric, rectify), bit and sample-rate reduction, tempo-synced gates, tremolo, pulse trains, stutter, buffer freeze, dropouts, comb filter, ring modulation, a small Schroeder reverb, feedback echo, reverse, and STFT-based spectral processing (disintegrate, phase scramble, tilt fade, freeze).
4. **Peak normalisation** of the layer, then `after` (post-chain automation) and `gain`. A layer's `gain` is therefore roughly its peak level in the mix.
5. **Placement**: start time, length, tail, pan (equal power) and width. With `width` above 0 the layer is rendered a second time with an independent seed and blended, which gives genuinely decorrelated channels rather than a delay trick.

The mix then goes through a DC blocker, short fades at both ends, an optional whole-effect reverse, and peak normalisation to `peak_db`.

Length: `duration` is the nominal length. The rendered length is the end of the latest layer including its tail, and never less than `duration + tail`. Layers that ring or overrun make an effect slightly longer than `duration`; `Recipe::total_seconds()` gives the exact figure.

Memory is validated before rendering: `max_seconds` (default 120 s), at most 64 layers and 32 processors per layer, and parameter ranges, so no request can ask for unbounded buffers.

### Determinism

The same preset id, parameter values, seed, tempo, sample rate and render settings produce the same audio. Every random decision derives from the seed through a hash-based generator (`xoshiro256**` seeded with `splitmix64`), per layer and per processor. There is no global random state, no `<random>` distributions (whose output differs between standard libraries), and no dependence on what else was rendered first.

- `Effect::seed(n)` sets the seed; the default is derived from the preset id, so it is stable.
- A different seed gives a different take of the same effect.
- Every layer also has `seed_offset` (`LayerSpec::variation(n)`) to re-roll one layer.

What was checked: bit-identical output on repeated renders in one process, independent of rendering other effects in between, and byte-identical WAV files for the whole catalog across separate runs of the tool.

What is not guaranteed: bit-identical output across platforms or compilers. The integer hashing and generator are portable, but the audio uses `sin`, `exp`, `pow` and `tan` from the platform's maths library, and compiler flags such as `-ffast-math` or fused multiply-add contraction change the last bits. For the chaotic presets (Lorenz, logistic and Henon based) tiny numerical differences can grow, so on another platform they may sound different from run to run of a different build, though still deterministic on any one build. Only one platform (Linux, x86-64, GCC) has been tested.

### Layering

`Layered` mixes effects (presets, configured `Effect` objects, or audio you already rendered) at offsets. It does not sequence anything.

```cpp
sfx::Layered t;
t.tempo(128).seed(7);
t.add("noise_riser",      sfx::LayerOpts().dur(3.75).gain(-3).place(-0.2));
t.add("reverse_cymbal",   sfx::LayerOpts().dur(1.8).at(1.95).gain(-6));
t.add("cinematic_impact", sfx::LayerOpts().at_beats(8).dur(2.0));
sfx::Rendered audio = t.render();
```

`LayerOpts`:

| option | meaning |
|---|---|
| `gain(dB)` | relative level. Effect layers are rendered at 0 dBFS first; rendered audio is used as it is |
| `at(sec)` / `at_beats(b)` | start time (beats use the `Layered` tempo) |
| `dur(sec)` | sets the layer effect's `duration` |
| `transpose(semitones)` | presets with a `pitch` parameter are re-pitched through it (clamped to its range, length kept); other presets and rendered audio are resampled, which also changes their length |
| `place(pan)` | -1 left to +1 right, equal power |
| `with_seed(n)` | layer-specific seed; otherwise derived from the composite seed and the layer's position |
| `flip()` | reverse the layer |
| `set(name, value)` | parameter override for the layer effect |

The composite is peak-normalised according to the `RenderSettings` you pass. Rendered audio must have the sample rate you render at; a mismatch is an error, not a silent mis-timing.

### Tempo

- `Effect::tempo(bpm)` (20 to 400, default 120) sets the tempo used by tempo-synced internals.
- `Effect::duration_beats(b)` expresses the duration in beats; it is resolved against the tempo at render time. `set("duration", seconds)` switches back to seconds.
- Rhythmic presets use `subdivision` (beats per step; 0.25 is a sixteenth note) and `swing`. Gates, tremolo, LFO rates and pulse rates are derived from the tempo.
- Gate steps and layer start times are computed from absolute time. Pulse and LFO phases accumulate in double precision; over the 60-second parameter maximum the drift is far below one sample.

Only presets that are tempo-based react to `tempo()`; a plain riser has no beat grid, but `duration_beats()` still applies to it.

### Trajectories and modulators

`Traj` is a parameter trajectory over the layer's normalised time (0 to 1) and is what most source and processor parameters accept; a plain number converts to a constant.

```cpp
Traj::linear(2, 6);  Traj::exp(200, 12000);  Traj::smooth(0, 1);          // exp = even in musical terms
Traj::pitch_hz(110, 440);                                                  // given in Hz, stored as MIDI
Traj::stairs(40, 64, 8);                                                   // staircase
Traj(0).to(0.1, 1, Interp::Smooth).to(1, 0.001, Interp::Exp);              // keyframes
```

Interpolation is `Linear`, `Exp`, `Smooth` (smoothstep) or `Step`. `Exp` needs positive end values and falls back to linear otherwise. Non-finite values and out-of-order keys are rejected, and `at()` sanitises its input, so zero-length or degenerate cases cannot produce NaN. `Mod` is a stateless LFO (sine, triangle, square, saw, smooth random, stepped random), in Hz or tempo-synced, used by `LayerSpec::filter_lfo()`.

Helpers for writing recipes (`wauvio::sfx`): `env_hit`, `env_swell`, `env_bell`, `lfo`, `lfo_beats`, `random_stairs`, `hashed_pattern`.

### Custom recipes and presets

A `Recipe` is plain data, so you can build one directly and render it:

```cpp
sfx::Recipe r;
r.duration = 2.0; r.tail = 1.0;
r.add("air").noise(sfx::NoiseColor::Pink).bp(sfx::Traj::exp(200, 6000), 3.0, 4)
    .shape(sfx::env_swell()).place(0.0, 0.9).reverb(1.2, 0.5, 0.5);
r.add("tone").osc(sfx::Wave::Saw, sfx::Traj::linear(40, 76)).unison(3, 12).lp(5000).gain(0.4);
sfx::Rendered audio = sfx::render(r, /*seed=*/5);
```

To make it a preset with parameters, fill in a `Preset` (name, description, tags, parameters from the shared vocabulary, a build function that fills a `Recipe` from the parameters) and add it to a copy of the built-in registry:

```cpp
sfx::Registry mine = sfx::builtin_registry();
sfx::Preset p;
p.name = "My Whoosh"; p.category = sfx::Category::Risers; p.behavior = sfx::Behavior::Evolving;
p.description = "A pink-noise whoosh with a saw underneath."; p.tags = { "custom" };
p.params = sfx::params_from({ "duration=2", "brightness=0.6", "pitch=40" });
p.build = [](sfx::Recipe& rec, const sfx::BuildCtx& c) { /* use c.p("brightness"), c.beats(...), c.rng(tag) ... */ };
mine.add(p);                                   // validates id, metadata, "duration" and defaults; throws on a duplicate
sfx::Effect e(*mine.find("my_whoosh"));
```

To add a preset to the built-in catalog, add a `d("Name", Behavior, "description", {"tags"}, {"parameters"}, build)` entry in the matching category file in `wauvio_sfx/presets/`. The id is derived from the name. Presets should stay distinct in structure or parameterisation: the test suite rejects near-duplicates (see Tests). Use `BuildCtx::rng(tag)` for any randomness, never a global generator.

### Command-line tool

```bash
g++ -std=c++17 -O2 -I<repo root> wauvio_sfx/tools/wauvio_sfx_tool.cpp -o wauvio_sfx_tool

wauvio_sfx_tool list [--category risers] [--search glitch]
wauvio_sfx_tool categories
wauvio_sfx_tool search shimmer
wauvio_sfx_tool info noise_riser
wauvio_sfx_tool render noise_riser --duration 2 --brightness 0.9 --seed 9 --sr 48000
wauvio_sfx_tool render-category textures --out my_audition
wauvio_sfx_tool render-all --tempo 128 --seed 1
wauvio_sfx_tool catalog > catalog.md
```

Options: `--out DIR` (default `sfx_out`), `--seed N`, `--sr HZ`, `--mono`, `--peak DB`, `--duration SEC` or `--beats N`, `--tempo BPM`, and `--set NAME=VALUE` (repeatable) with shortcuts `--intensity --brightness --pitch --tail --width`.

Rendering writes `DIR/<category>/<id>.wav` (16-bit PCM) and `DIR/manifest.json`. The manifest lists, for every preset, its id, name, category, behaviour, status, file, seed, tempo, parameter values, frame count, duration, peak, RMS, DC offset and clipping ratio, plus the global settings. Failures are recorded with their error text, printed in a summary and reflected in the exit status (1); nothing is skipped silently. In a batch, an option that a preset does not have (for example `--pitch` on a noise preset) is listed under `ignored_options` for that entry; for a single `render` it is an error. File names are the stable ids, one folder per category, so they cannot collide; if two entries of one run ever mapped to the same path, the second would get a numeric suffix.

### Examples

Each file in `examples/wauvio_sfx/` compiles and runs on its own.

| file | shows |
|---|---|
| `01_render_preset.cpp` | render a preset to WAV |
| `02_customize.cpp` | inspect and change parameters, seed, sample rate; rejected values |
| `03_layered_transition.cpp` | riser, reverse cymbal and impact composed on a beat grid |
| `04_tempo_riser.cpp` | durations in beats at different tempi; tempo-locked gating |
| `05_atmosphere.cpp` | a 20-second layered atmosphere |
| `06_deterministic_random.cpp` | same seed identical, different seed different |
| `07_custom_recipe.cpp` | build a recipe from scratch and register it as a preset |
| `08_collection.cpp` | render every preset tagged `riser` into one folder |

### Using the results with the rest of Wauvio

`Rendered::to_stereo()` returns a `wauvio::StereoBuffer`, the buffer type used throughout Wauvio, so a rendered effect can be written with `save_wav_stereo`, used as a `Source` of a `wauvio_fx` `Mixer`, or passed through a `wauvio_fx` `Chain`:

```cpp
wauvio::StereoBuffer fx_audio = sfx::make("cinematic_impact").render().to_stereo();
wauvio::fx::Chain chain;
chain.add<wauvio::fx::Compressor>();
chain.process(fx_audio, 44100);
```

There is no automatic integration with `Arrangement` instruments or MIDI: effects are rendered buffers that you place yourself. `wauvio_sfx` does not depend on `wauvio_fx`, `wauvio_ext` or `wauvio_midi`.

### Performance

Rendering is offline and not real-time safe: it allocates freely and some processors (spectral, granular, modal) are not cheap. Pre-render effects and keep the `Rendered` buffers. On the development machine (`-O2`) the 16 risers, about 50 seconds of audio, render and write as WAV in about 2 seconds; the whole catalog (about 600 seconds of audio, 190 WAV files) takes about 25 seconds. Memory is bounded by `max_seconds` and the layer limits; a layer with `width` above 0 is rendered twice.

### Tests

`tests/wauvio_sfx/run.sh [compiler flags]` compiles every header on its own with `-Wall -Wextra -Werror`, runs `test_sfx`, compiles and runs all examples, and renders one category through the command-line tool and checks its manifest. `SFX_FAST=1` tests every fourth preset (useful under sanitizers).

`test_sfx` checks, among other things:

- the catalog: 190 unique, well-formed ids, the right number of presets per category, complete metadata, valid defaults, unique default seeds;
- every preset: correct length, sample rate and channels, finite samples, peak at the target, no clipping, no significant DC, not silent, bit-identical on a second render, different with another seed, unaffected by rendering other effects first, mono rendering, the minimum duration, and tail handling;
- reverse processing: the metadata matches what the recipe does, a reverse processor mirrors the audio sample for sample, and reversed presets are louder at the end;
- behaviour: risers rise and falls fall in spectral centroid, impacts decay, textures hold a steady level, builds build, the `pitch`, `chord`, `scale`, `brightness` and `width` parameters change what they claim, gating speed follows tempo, durations in beats follow tempo;
- layering: length, gain, offsets in seconds and beats, pan, pitch, seeds, determinism, error handling;
- input validation: bad ids, parameters, settings, recipes, trajectories;
- distinctness: no two presets have near-identical feature vectors (levels over time, spectral centroid over time, flatness, crest factor, stereo correlation, duration) or near-identical waveforms.

These are objective measurements. They show the controls do what they claim and that nothing is broken. They cannot say whether a preset sounds good: tuning by ear is still needed.

### Known limitations

- Offline only; no real-time use and no streaming.
- Levels are peak-normalised, so loudness differs widely between presets (default catalog: RMS from about -30 dB to -5 dB, median -18 dB). Balance them with `LayerOpts::gain` or a loudness stage.
- The reverb is a small Schroeder design and the echoes are plain feedback delays. They are synthetic tails, not convolution reverb.
- Modal and plucked sources stop at the end of their layer plus its tail (with a few milliseconds of fade); a long ring needs a longer `tail`.
- Vocal-like presets are formant textures, not voices.
- Bit-identical output across platforms is not guaranteed (see Determinism); chaos-based presets are the most sensitive.
- A preset's audio can change between library versions when it is retuned. Ids stay stable; seeds reproduce audio within one version.
- Sample rates below about 16 kHz remove high-frequency content because filters are limited to 0.45 times the sample rate.
- The presets have been verified with measurements, not by listening; expect to adjust defaults by ear.

### Preset catalog

All 190 presets, with the names and order of the original specification (none merged, renamed or replaced). This table is generated from the registry by `wauvio_sfx_tool catalog`, so it cannot drift from what is implemented. Ids are stable.

<details>
<summary><b>Risers & Sweeps (`risers`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `noise_riser` | Noise Riser | evolving | 3 s | riser, noise, build, transition | White noise through a low-pass whose cutoff sweeps up exponentially while the level swells; the classic build-up. |
| `tonal_riser` | Tonal Riser | evolving | 4 s | riser, tonal, saw, build | A supersaw-style stack plus its fifth glides upward through an opening low-pass; a pitched build. |
| `pitch_riser` | Pitch Riser | evolving | 2.5 s | riser, pitch, sweep, fm | A single triangle voice with a growing FM shimmer (a sine modulator) whose pitch rises exponentially across a wide range. |
| `reverse_riser` | Reverse Riser | evolving, reversed | 3 s | riser, reverse, noise, swell | A noise burst with a long reverb tail, reversed: the tail swells into a sudden stop. |
| `filter_sweep` | Filter Sweep | evolving | 4 s | sweep, filter, saw, resonant | A detuned saw stack through a resonant low-pass that opens and then closes again. |
| `resonance_sweep` | Resonance Sweep | evolving | 3 s | sweep, resonance, noise, whistle | Noise through a very narrow band-pass that sweeps upward; a whistling, vocal resonance. |
| `pitch_dive` | Pitch Dive | one-shot | 1.6 s | dive, pitch, fall, sweep | A sine and saw voice that falls exponentially from a high note to a low one with a little drive. |
| `downlifter` | Downlifter | evolving | 3 s | downlifter, fall, noise, transition | Noise and a saw voice fall together: both the cutoff and the pitch sink while the level fades. |
| `noise_fall` | Noise Fall | one-shot | 1.4 s | fall, noise, short, whoosh | A short fall of pink noise: the cutoff drops quickly through the spectrum. |
| `transition_sweep` | Transition Sweep | evolving | 2.5 s + 1 s tail | transition, sweep, whoosh, tail | A band-passed noise sweep and a rising sine glide meet in a reverberant tail: a whoosh for scene changes. |
| `harmonic_riser` | Harmonic Riser | evolving | 4 s | riser, harmonic, additive, brightening | A fixed-pitch harmonic series whose upper partials fade in over time: the spectrum brightens without the pitch moving. |
| `detuned_riser` | Detuned Riser | evolving | 4 s | riser, detuned, supersaw, unstable | Seven saws that start in tune and spread apart by tens of cents as the pitch climbs: a thickening, unstable rise. |
| `formant_sweep` | Formant Sweep | evolving | 3 s | riser, formant, vowel, vocal-like | A voiced formant source rises in pitch while its vowel travels from 'a' towards 'u' and the formants lift. |
| `bandpass_sweep` | Bandpass Sweep | evolving | 3.5 s | sweep, bandpass, saw, tonal | A low saw drone through a medium-Q band-pass sweeping up: the harmonics pass through the band one by one. |
| `staircase_riser` | Staircase Riser | evolving | 3 s | riser, stairs, stepped, saw | A saw that climbs in equal pitch steps under an opening filter, each step re-gated for a stepped feel. |
| `tension_swell` | Tension Swell | evolving | 5 s | tension, swell, dissonant, cluster | A dissonant cluster of additive partials with slow beating swells to a peak and drops away. |

</details>

<details>
<summary><b>Impacts & Transitions (`impacts`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `cinematic_impact` | Cinematic Impact | one-shot | 2.5 s + 2 s tail | impact, cinematic, boom, trailer | A deep sine drop, a distorted saw body, a noise transient and a long synthetic tail: the standard trailer hit. |
| `sub_drop` | Sub Drop | one-shot | 2.2 s | sub, drop, bass, weight | A pure sine falling from a low note to sub-bass with a gentle saturation; weight without any noise. |
| `reverse_impact` | Reverse Impact | one-shot, reversed | 2.5 s + 2 s tail | impact, reverse, swell, transition | A full cinematic hit rendered forwards and then reversed: the tail swells and the hit lands at the very end. |
| `reverse_swell` | Reverse Swell | evolving, reversed | 3.5 s | reverse, swell, pad, tonal | A chord pad and bright noise built from a decaying reverb tail reversed; slow and tonal rather than a hit. |
| `transition_hit` | Transition Hit | one-shot | 0.9 s + 0 s tail | transition, hit, whoosh, short | A short mid-focused whoosh-and-thump: band-passed noise and a falling saw, tuned for scene cuts. |
| `noise_burst` | Noise Burst | one-shot | 0.25 s | burst, noise, short, click | A very short burst of high-passed white noise with an instant attack: a hit with no body. |
| `bass_impact` | Bass Impact | one-shot | 1.3 s | impact, bass, punch, kick-like | A punchy kick-like hit: a fast sine dive with a saturated saw layer and a tight noise click. |
| `tail_out` | Tail-Out | one-shot | 0.4 s + 4 s tail | tail, reverb, decay, end | A tiny click that excites a long, darkening reverb tail: mostly tail, for ending a section. |
| `transition_decay` | Transition Decay | one-shot | 1.8 s + 1 s tail | transition, stab, echo, decay | A filtered saw-chord stab whose cutoff closes while echoes repeat and fade: a decaying hand-off between sections. |
| `layered_impact` | Layered Impact | one-shot | 2.2 s + 1 s tail | impact, layered, heavy, cinematic | Five explicit layers (click, mid body, low body, sub and noise wash) so each part can be balanced separately. |
| `metallic_impact` | Metallic Impact | one-shot | 2.5 s | impact, metallic, gong, inharmonic | Struck inharmonic metal modes with a noise clang and a low thud; a heavy, gong-like hit. |
| `orchestral_style_synthetic_hit` | Orchestral-Style Synthetic Hit | one-shot | 1.6 s | impact, orchestral, stab, brass-like | A brassy additive chord stab, a timpani-like sine drop and a cymbal-ish noise swell: an orchestral hit built from scratch. |
| `granular_impact` | Granular Impact | one-shot | 1.8 s | impact, granular, cloud, debris | A cloud of short noise grains whose density collapses from dense to sparse, over a low thump. |
| `dissonant_impact` | Dissonant Impact | one-shot | 2 s + 1 s tail | impact, dissonant, tritone, horror | A tritone-and-semitone stack of saws with FM noise, driven hard, then a reverb decay: an unsettling hit. |
| `low_boom` | Low Boom | one-shot | 3.2 s | boom, low, sub, slow | A very low, slowly decaying sine with a slight pitch drop and soft clipping, band-limited to the bottom octaves. |
| `upsweep_impact` | Upsweep Impact | evolving | 3.5 s + 1 s tail | impact, upsweep, transition, build | A noise-and-tone upsweep that ends exactly where a low hit lands: the build and the impact in one effect. |

</details>

<details>
<summary><b>Glitch & Digital (`glitch`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `bitcrushed_burst` | Bitcrushed Burst | one-shot | 0.8 s | glitch, bitcrush, burst, lofi | A noise-and-saw burst quantised to a few bits with sample-and-hold decimation, decaying quickly. |
| `digital_stutter` | Digital Stutter | one-shot | 1.5 s | glitch, stutter, repeat, tonal | A held saw tone whose buffer keeps jumping back and repeating in short slices (about 70 ms by default; the rate parameter sets the slice length). |
| `buffer_repeat` | Buffer Repeat | one-shot | 2 s | glitch, buffer, repeat, tempo-sync | A bright pluck arpeggio whose buffer is grabbed on beat-length slices and repeated, in time with the tempo. |
| `granular_glitch` | Granular Glitch | one-shot | 1.6 s | glitch, granular, scatter, shatter | Tiny 5-20 ms grains of a tone, scattered in pitch and position at high density: a shattered sound. |
| `digital_dropout` | Digital Dropout | sustained | 2 s | glitch, dropout, stream, tonal | A steady tone riddled with random holes of 5-60 ms, like a failing digital stream. |
| `aliased_sweep` | Aliased Sweep | evolving | 2.5 s | glitch, alias, sweep, metallic | A square wave sweeping upward while sample-and-hold decimation grows, so the sweep folds back on itself with metallic aliasing. |
| `glitch_fill` | Glitch Fill | one-shot | 1.2 s | glitch, fill, blips, random | Short random-pitched square blips at scattered times: a fast digital fill for gaps. |
| `rhythmic_glitch` | Rhythmic Glitch | one-shot | 2 s | glitch, rhythmic, gated, tempo-sync | A crushed tone and noise gated by a tempo-locked 16-step pattern chosen from the seed. |
| `micro_stutter` | Micro-Stutter | one-shot | 1 s | glitch, micro, stutter, buzz | A gliding saw repeated in 4-10 ms slices: the repetition turns into a buzzing pitched tone. |
| `glitch_tail` | Glitch Tail | one-shot | 0.6 s + 1 s tail | glitch, tail, echo, crushed | A short blip followed by a feedback echo that is crushed and stuttered as it decays. |
| `digital_fragmentation` | Digital Fragmentation | evolving | 3 s | glitch, spectral, fragment, disintegrate | A saw chord whose spectrum is progressively deleted bin by bin until only fragments remain. |
| `sample_rate_degradation` | Sample-Rate Degradation | evolving | 3 s | glitch, degradation, sample-rate, lofi | A clean chord whose sample-rate falls steadily from full to a coarse 1/40: the sound turns into aliasing. |
| `randomized_retrigger` | Randomized Retrigger | one-shot | 1.6 s | glitch, retrigger, random, pitched | A pitched tone retriggered at jittered intervals with a different random pitch on each repeat. |
| `buffer_freeze` | Buffer Freeze | sustained | 2.5 s | glitch, freeze, loop, stuck | A bright arpeggio that suddenly sticks: the last 70 ms are looped for the rest of the effect. |
| `spectral_glitch` | Spectral Glitch | one-shot | 1.8 s | glitch, spectral, scramble, smear | A noisy tone whose phases are scrambled in pulses: the sound smears into noise and snaps back. |
| `broken_signal_burst` | Broken-Signal Burst | one-shot | 1.2 s | glitch, harsh, broken, noise | Harsh: rectified, crushed noise and a square with dropouts, as if a transmission were failing. |

</details>

<details>
<summary><b>Textures & Atmospheres (`textures`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `noise_bed` | Noise Bed | sustained | 8 s | texture, noise, bed, steady | Pink noise through a low-pass that drifts slowly; a plain, steady bed of sound. |
| `air_texture` | Air Texture | sustained | 8 s | texture, air, hiss, high | High-passed white noise with a faint breathing movement: just the top of the spectrum. |
| `metallic_drone` | Metallic Drone | sustained | 8 s | texture, drone, metallic, inharmonic | A stack of band-limited squares at inharmonic ratios, slowly sweeping filter: a bell-metal hum. |
| `dissonant_drone` | Dissonant Drone | sustained | 8 s | texture, drone, dissonant, tense | Semitone and tritone partials with a slow tremolo: a sustained unease. |
| `harmonic_shimmer` | Harmonic Shimmer | sustained | 8 s | texture, shimmer, harmonic, bright | High partials of a fifth-and-octave chord with a gentle tremolo and echo: glassy and bright. |
| `swirling_texture` | Swirling Texture | sustained | 8 s | texture, swirl, comb, movement | Noise through a moving band-pass and a drifting comb filter, rendered with decorrelated channels: a slow phasing swirl. |
| `granular_cloud` | Granular Cloud | sustained | 8 s | texture, granular, cloud, bell | A dense cloud of 80-200 ms bell grains scattered in pitch and position. |
| `frozen_texture` | Frozen Texture | sustained | 8 s | texture, frozen, spectral, pad | A chord that is spectrally frozen early on: the same spectrum is held with randomised phase for the whole effect. |
| `spectral_wash` | Spectral Wash | sustained | 8 s | texture, wash, spectral, smeared, chord | A rich chord whose phases are re-randomised in every analysis frame: the pitches survive but turn into a smeared, ghostly wash. |
| `inharmonic_texture` | Inharmonic Texture | sustained | 8 s | texture, inharmonic, resonant, ringing | Struck inharmonic modes retriggered every second or so with random velocities: ringing metal, constantly renewed. |
| `evolving_noise` | Evolving Noise | sustained | 10 s | texture, evolving, noise, random-walk | Noise whose band-pass centre wanders on a smooth random modulator while a slow tremolo moves its level: it never settles. |
| `detuned_atmosphere` | Detuned Atmosphere | sustained | 8 s | texture, pad, detuned, warm | Two groups of detuned saws a fifth apart through a dark low-pass: a wide, warm pad. |
| `resonant_ambience` | Resonant Ambience | sustained | 8 s | texture, resonant, room, comb | Noise rings through parallel comb resonators tuned to a chord, then a long reverb: the hum of a resonant room. |
| `pulsating_drone` | Pulsating Drone | sustained | 8 s | texture, drone, pulse, tempo-sync | A low saw-and-sine drone amplitude-pulsed in time with the tempo (quarter notes by default). |
| `static_wash` | Static Wash | sustained | 8 s | texture, static, crackle, hiss | Dense crackle (dust) layered over blue-noise hiss: a radio-static carpet. |
| `harmonic_haze` | Harmonic Haze | sustained | 8 s | texture, haze, warm, harmonic | Low, many-partialled additive tones softened by a dark filter and echo: warm haze with no sharp edges. |

</details>

<details>
<summary><b>Bass FX (`bass`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `wobble_bass_stab` | Wobble Bass Stab | one-shot | 1.6 s | bass, wobble, stab, tempo-sync | A saw-and-sub stab through a high-Q low-pass modulated by a tempo-synced LFO: the dubstep wobble in one hit. |
| `fm_bass_stab` | FM Bass Stab | one-shot | 0.9 s | bass, fm, stab, short | A sine carrier phase-modulated by a sine at its own frequency, with a modulation index that decays from bright to pure: a hard, glassy stab. |
| `reese_style_sweep` | Reese-Style Sweep | evolving | 3.5 s | bass, reese, detuned, sweep | Two detuned saws drifting against each other through a slowly sweeping low-pass: the beating, phasing bass of drum and bass. |
| `bass_drop` | Bass Drop | one-shot | 2.2 s | bass, drop, fall, growl | A falling sine with a growling saw layer whose filter closes behind it, then driven: the drop after a build. |
| `distorted_bass_burst` | Distorted Bass Burst | one-shot | 0.7 s | bass, distorted, burst, fuzz | A static-pitch saw hard-clipped and high-passed slightly: a short, aggressive, fuzzy blast. |
| `formant_bass_sweep` | Formant Bass Sweep | evolving | 3 s | bass, formant, sweep, vowel | A low voiced formant source whose vowel slowly sweeps a to u: the bass seems to open and close its mouth. |
| `resonant_bass_stab` | Resonant Bass Stab | one-shot | 0.8 s | bass, resonant, acid, stab | A saw through a resonant low-pass with a fast-decaying filter envelope: the squelchy, acid-style stab. |
| `rhythmic_bass_pulse` | Rhythmic Bass Pulse | sustained | 4 s | bass, pulse, rhythmic, tempo-sync | A saw-and-sub bass pulsing in time: each beat division gets a short filter-and-level envelope. |
| `pitch_sliding_bass` | Pitch-Sliding Bass | one-shot | 2 s | bass, 808, glide, slide | An 808-style bass: a saturated sine that holds, glides up an interval and rings out. |
| `sub_bass_swell` | Sub-Bass Swell | evolving | 4 s | bass, sub, swell, pad | A sine and a quiet second harmonic that swell in and out slowly; nothing above the bass. |
| `talking_bass_burst` | Talking Bass Burst | one-shot | 1 s | bass, formant, talking, vowel | A low voiced formant source that jumps between vowels in quick steps, driven: a bass that seems to speak. |
| `metallic_bass_hit` | Metallic Bass Hit | one-shot | 1.4 s | bass, metallic, clang, hit | Low-pitched inharmonic metal modes with a sub sine under them: a clanging bass hit. |
| `growl_sweep` | Growl Sweep | evolving | 3 s | bass, growl, fm, fold | A saw phase-modulated by a sine an octave below it, wavefolded, then swept: a moving, vocal growl. |
| `bass_stutter` | Bass Stutter | one-shot | 2 s | bass, stutter, chopped, tempo-sync | A sustained bass note chopped by a tempo-locked gate and repeating slices. |
| `inharmonic_bass_hit` | Inharmonic Bass Hit | one-shot | 1.2 s | bass, inharmonic, fm, hit | A sine carrier modulated at a non-integer ratio (1.414) with a decaying index and a sub under it: dense, inharmonic and low. |
| `bass_tail` | Bass Tail | one-shot | 1.2 s + 1 s tail | bass, tail, echo, decay | A decaying bass note with a darkening feedback echo and a small reverb tail: a bass note that fades into the room. |

</details>

<details>
<summary><b>Ear Candy & Ornaments (`ear_candy`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `sparkle` | Sparkle | one-shot | 1.4 s | sparkle, glass, high, scatter | A scatter of very high, short glass-mode pings at random times and velocities. |
| `shimmer_burst` | Shimmer Burst | one-shot | 1 s + 1 s tail | shimmer, burst, bloom, high | A short bloom of high additive partials with fast tremolo and a reverb tail: a brief wash of glitter. |
| `bell_cascade` | Bell Cascade | one-shot | 2.5 s | bells, cascade, scale, descending, dense | A fast, dense run of struck bells that alternates left and right while stepping down the chosen scale; the notes pile into one long shared ring. |
| `pitch_bent_chime` | Pitch-Bent Chime | one-shot | 1.6 s | chime, bend, pitch, sine | A chime built from sine partials at bell ratios whose pitch bends down after the strike. |
| `reverse_sparkle` | Reverse Sparkle | evolving, reversed | 2 s | sparkle, reverse, glass, swell | Glass pings with a long reverb tail, reversed: the glitter builds up and cuts off. |
| `metallic_ping` | Metallic Ping | one-shot | 0.6 s | ping, metallic, short, bright | A single high, bright strike of inharmonic metal modes with a very short decay. |
| `glissando_flourish` | Glissando Flourish | one-shot | 1.4 s | flourish, glissando, pluck, scale | A fast harp-like run of plucked strings sweeping up the chosen scale. |
| `microtonal_twinkle` | Microtonal Twinkle | one-shot | 1.6 s | twinkle, microtonal, glass, random | Quarter-tone-spaced glass pings in random order: the pitches sit between the keys. |
| `digital_chime` | Digital Chime | one-shot | 1.4 s | chime, fm, digital, echo | Two-operator FM bell (modulator at ratio 3.5, index falling towards zero) with an echo: a clean, electric chime. |
| `glass_shimmer` | Glass Shimmer | evolving | 3 s + 1 s tail | shimmer, glass, inharmonic, swell | A slowly swelling stack of slightly stretched (inharmonic) high partials with tremolo and reverb: a glassy bloom. |
| `arpeggiated_flourish` | Arpeggiated Flourish | one-shot | 1.2 s | flourish, arpeggio, chord, pluck | A quick arpeggio of the chosen chord over several octaves, plucked and slightly detuned. |
| `falling_chimes` | Falling Chimes | one-shot | 3 s | chimes, falling, glass, descending | Glass-like chimes stepping down the scale with widening gaps and a soft echo: a slow, sparse fall. |
| `rising_chimes` | Rising Chimes | one-shot | 3 s | chimes, rising, bells, ascending | Bell hits climbing the scale with shrinking gaps: the mirror image of Falling Chimes. |
| `randomized_pluck_cloud` | Randomized Pluck Cloud | one-shot | 2.5 s | pluck, cloud, random, scale | Many plucked strings at random times and random notes of the chosen scale, panned across the field. |
| `harmonic_droplets` | Harmonic Droplets | one-shot | 1.8 s | droplets, harmonic, plink, water | Short, round plinks whose pitches follow the harmonic series, like water drops of different sizes. |
| `tiny_impact_cluster` | Tiny Impact Cluster | one-shot | 0.9 s | impact, cluster, ticks, short | A burst of micro-hits: band-passed noise ticks and wood-mode knocks piled into a short accelerating cluster. |

</details>

<details>
<summary><b>Vocal Production FX (`vocal`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `synthetic_breath` | Synthetic Breath | one-shot | 1.2 s | breath, whisper, formant, short | Whispered noise shaped by three formants: a short rise and fall like a breath out of a mouth that is nearly closed. |
| `breathy_noise_swell` | Breathy Noise Swell | evolving | 2.5 s | breath, swell, whisper, vowel | A whispered vowel that opens from 'u' to 'a' while its level swells, then cuts: a breath that builds. |
| `vocal_like_riser` | Vocal-Like Riser | evolving | 3 s | vocal-like, riser, formant, pitch | A voiced formant tone rising two octaves with the vowel opening from 'u' to 'a' and a growing breath component. |
| `formant_transition` | Formant Transition | sustained | 2.5 s | vocal-like, formant, vowel, glide | A steady voiced tone whose vowel glides a -> i -> u: movement without any change of pitch. |
| `whispery_texture` | Whispery Texture | sustained | 6 s | vocal-like, whisper, texture, evolving | A long whispered texture whose formants wander between vowels at random: breath without words. |
| `vocal_chop_burst` | Vocal Chop Burst | one-shot | 1.5 s | vocal-like, chop, rhythmic, burst | Short voiced vowel bursts in a rhythmic pattern, each with a different vowel: a synthetic chopped-vocal figure. |
| `stuttered_vocal_like_texture` | Stuttered Vocal-Like Texture | one-shot | 2 s | vocal-like, stutter, formant, repeat | A sustained vowel tone whose buffer is repeated in 50-90 ms slices: a stuck, stammering voice. |
| `synthetic_inhale` | Synthetic Inhale | one-shot | 1.1 s | breath, inhale, whisper, formant | A whisper with a slow rise and quick cut, its formants lifting as it goes: an inhaled breath. |
| `synthetic_exhale` | Synthetic Exhale | one-shot | 1.6 s | breath, exhale, whisper, formant | A whisper with a quick onset and long fade, its formants sinking: an exhaled breath. |
| `formant_glitch` | Formant Glitch | one-shot | 1.4 s | vocal-like, glitch, formant, crushed | A voiced formant tone with random vowel jumps, bit reduction and dropouts: a broken speech-synthesis fragment. |
| `vowel_sweep` | Vowel Sweep | evolving | 3 s | vocal-like, vowel, sweep, formant | A voiced tone that glides up through all five vowels and back, with a slight pitch lift in the middle. |
| `vocal_like_impact` | Vocal-Like Impact | one-shot | 1.4 s + 1 s tail | vocal-like, impact, formant, low | A low 'oh' that drops in pitch with a noise breath on the attack and a reverb tail: a hit with a human-ish body. |
| `breath_rhythm` | Breath Rhythm | sustained | 4 s | breath, rhythmic, whisper, tempo-sync | Whispered breaths on every beat division, each with its own rise and fall: a rhythmic panting texture. |
| `pitch_granulated_vocal_like_texture` | Pitch-Granulated Vocal-Like Texture | sustained | 6 s | vocal-like, granular, choir-like, texture | Tiny grains of a sung vowel tone scattered across pitch and position: a shimmering, choir-like granular cloud. |
| `synthetic_gasp` | Synthetic Gasp | one-shot | 0.55 s | breath, gasp, whisper, sharp | A very fast, bright whispered inhale with raised formants: a sharp intake of breath. |
| `vocal_texture_tail` | Vocal Texture Tail | one-shot | 1 s + 4 s tail | vocal-like, tail, reverb, whisper | A short voiced 'ah' that dissolves into a long whispered, reverberant tail. |

</details>

<details>
<summary><b>Rhythmic FX (`rhythmic`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `gated_noise` | Gated Noise | sustained | 2 s | rhythmic, gate, noise, tempo-sync | White noise chopped by a tempo-locked 16-step pattern chosen from the seed. |
| `rhythmic_sweep` | Rhythmic Sweep | evolving | 3 s | rhythmic, sweep, gated, build | A band-passed noise sweep rising across the effect and gated on every step: a build that ticks in time. |
| `stutter_fill` | Stutter Fill | one-shot | 1.5 s | rhythmic, fill, snare-like, accelerating | Snare-like noise-and-tone hits at accelerating intervals with rising pitch: a fill into the next bar. |
| `syncopated_glitch` | Syncopated Glitch | one-shot | 2 s | rhythmic, syncopated, glitch, swing | A crushed pulse tone gated in a 3+3+2 syncopation with swing: the pattern is fixed, only the sound is random. |
| `rhythmic_noise_roll` | Rhythmic Noise Roll | evolving | 2.5 s | rhythmic, roll, noise, accelerating | A noise roll whose rate rises exponentially from 4 to 40 hits per second while the filter opens. |
| `pulse_burst` | Pulse Burst | one-shot | 1.6 s | rhythmic, pulse, blips, tempo-sync | Square blips at the chosen step length, each with a fast decay and the pitch stepping up every other pulse. |
| `rhythmic_filter_chop` | Rhythmic Filter Chop | sustained | 3 s | rhythmic, filter, chop, tempo-sync | A saw chord whose low-pass is switched between two cutoffs by a square LFO locked to the tempo: a hard, stepped filter figure. |
| `retriggered_impact` | Retriggered Impact | one-shot | 2 s | rhythmic, impact, retrigger, accelerating | A low tonal hit retriggered at shrinking intervals while its pitch falls: the impact stutters into silence. |
| `accelerating_pulses` | Accelerating Pulses | evolving | 2.5 s | rhythmic, accelerating, pulses, sine | Sine blips whose rate rises from 3 to 24 per second: the pulse train speeds up until it becomes a tone-like roll. |
| `decelerating_pulses` | Decelerating Pulses | evolving | 2.5 s | rhythmic, decelerating, pulses, sine | Sine blips slowing from 24 to 3 per second while the level fades: a pulse train winding down. |
| `rhythmic_static` | Rhythmic Static | sustained | 2 s | rhythmic, static, crackle, tempo-sync | Crackle and hiss gated by a tempo-locked pattern: static that grooves. |
| `beat_synced_tremolo_burst` | Beat-Synced Tremolo Burst | one-shot | 1.5 s | rhythmic, tremolo, burst, tempo-sync | A bright saw chord chopped by a full-depth tremolo locked to the tempo, with a swell-and-cut envelope. |
| `polyrhythmic_texture` | Polyrhythmic Texture | sustained | 4 s | rhythmic, polyrhythm, texture, tonal | Three pings looping over the same bar in 3, 4 and 5 steps: the pulses drift against each other. |
| `rhythmic_riser` | Rhythmic Riser | evolving | 3.5 s | rhythmic, riser, accelerating, build | A noise-and-tone riser gated by an accelerating pulse train: a build whose chop rate rises from 4 to 36 per second. |
| `rhythmic_downlifter` | Rhythmic Downlifter | evolving | 3.5 s | rhythmic, downlifter, decelerating, fall | The riser's mirror: chops slow from 36 to 4 per second while the filter and pitch fall. |
| `broken_beat_texture` | Broken-Beat Texture | sustained | 4 s | rhythmic, broken-beat, texture, swing | Noise ticks and dusty clicks on a hashed, swung pattern with irregular gaps: a loop that never quite repeats. |

</details>

<details>
<summary><b>Synthetic Percussion & Cymbals (`percussion`) - 14 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `synthetic_crash` | Synthetic Crash | one-shot | 2.2 s | crash, cymbal, metallic, analogue | Six inharmonic square oscillators plus high-passed noise with a fast burst and a long exponential decay: an analogue-style crash. |
| `noise_cymbal` | Noise Cymbal | one-shot | 1.4 s | cymbal, noise, wideband, simple | Only high-passed and band-limited noise with an exponential decay: a wideband, un-pitched cymbal. |
| `reverse_cymbal` | Reverse Cymbal | evolving, reversed | 2.2 s | cymbal, reverse, swell, transition | A cymbal wash with a long decay rendered backwards: it swells and cuts off right on the beat. |
| `metallic_wash` | Metallic Wash | sustained | 3.5 s | wash, metallic, shimmer, cymbal-like | Dozens of inharmonic metal-mode strikes scattered over a few seconds: a shimmering, bell-metal wash. |
| `synthetic_snare_fill` | Synthetic Snare Fill | one-shot | 1.6 s | snare, fill, drum, accelerating | Eight snare-like hits (band-passed noise over a pitched triangle tone) at accelerating gaps with rising pitch: a fill. |
| `noise_snare_roll` | Noise Snare Roll | evolving | 2.5 s | snare, roll, noise, build | High-passed noise rolling at 10 to 50 hits per second with a steadily rising level: a classic snare-roll build. |
| `electronic_tom_flourish` | Electronic Tom Flourish | one-shot | 1.2 s | tom, flourish, drum, descending | Four sine toms stepping down in pitch, each with its own pitch dive: a quick electronic tom run. |
| `synthetic_kick_impact` | Synthetic Kick Impact | one-shot | 0.6 s | kick, impact, drum, punch | A sine that dives from 160 Hz towards a low note set by the pitch parameter in the first tens of milliseconds, with a click and light saturation. |
| `clap_burst` | Clap Burst | one-shot | 0.5 s + 0 s tail | clap, burst, noise, percussion | Four closely spaced band-passed noise bursts followed by a short room tail: a synthetic hand-clap. |
| `metallic_percussion_hit` | Metallic Percussion Hit | one-shot | 0.7 s | percussion, metallic, hit, short | A short inharmonic strike (high metal modes plus a noise click): a bell-metal tick with no sustain. |
| `percussive_click_cluster` | Percussive Click Cluster | one-shot | 0.9 s | percussion, clicks, cluster, crackle | A dense, jittered cloud of very short high-passed noise clicks: a crackling percussive texture. |
| `synthetic_shaker_roll` | Synthetic Shaker Roll | evolving | 1.8 s | shaker, roll, noise, percussion | High noise pulsed at 8 to 24 per second with short decays; the level rises then drops away: a shaker roll. |
| `percussion_reverse_swell` | Percussion Reverse Swell | evolving, reversed | 2 s | percussion, reverse, swell, metallic | A metallic strike and a noise burst with a reverb tail, reversed: percussion that builds up into the beat. |
| `resonant_percussion_tail` | Resonant Percussion Tail | one-shot | 0.7 s + 2 s tail | percussion, resonant, tail, wood | A woody knock that excites a comb resonator and a feedback echo: the ring is longer than the strike. |

</details>

<details>
<summary><b>Experimental & Extreme (`experimental`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `chaotic_fm_burst` | Chaotic FM Burst | one-shot | 1.2 s | chaos, fm, burst, lorenz | A carrier whose pitch is driven by a Lorenz attractor running at audio-ish speed: a burst of unpredictable, bounded FM. |
| `unstable_oscillator_texture` | Unstable Oscillator Texture | sustained | 6 s | chaos, oscillator, logistic, drone | A saw whose pitch is steered by a logistic map in its chaotic regime and smoothed: a wandering, never-repeating drone. |
| `inharmonic_cluster` | Inharmonic Cluster | one-shot | 3 s | inharmonic, cluster, additive, dissonant | A dense cluster of strongly stretched partials (inharmonicity 0.05) on closely spaced notes: bell-like but unpitched. |
| `nonlinear_pitch_sweep` | Nonlinear Pitch Sweep | evolving | 3 s | sweep, nonlinear, pitch, fm | A sine-FM voice whose pitch follows an irregular multi-segment curve: slow, a lurch upward, then a fast final climb. |
| `fractal_like_noise_texture` | Fractal-Like Noise Texture | sustained | 8 s | fractal, noise, texture, self-similar | Six octave-spaced noise bands. Each band's centre wanders on smooth random movement and its level swells sinusoidally, both at rates that double per octave: self-similar, 1/f-ish motion. |
| `chaotic_modulation_swell` | Chaotic Modulation Swell | evolving | 4 s | chaos, swell, modulation, lorenz | A saw swells while a Lorenz attractor bends its pitch and level more and more strongly. |
| `frequency_folded_burst` | Frequency-Folded Burst | one-shot | 1 s | fold, burst, harsh, sine | A sine swept upward and wavefolded with an increasing fold amount: the harmonics fold back on themselves into a harsh, bright burst. |
| `spectral_disintegration` | Spectral Disintegration | evolving | 4 s | spectral, disintegrate, crumble, chord | A rich saw chord whose spectrum is erased bin by bin while high frequencies fade first: the sound crumbles to nothing. |
| `unstable_resonator` | Unstable Resonator | sustained | 5 s | resonator, comb, unstable, ringing | Noise driving a feedback comb (gain 0.80 to 0.94, set by intensity) whose tuning wanders on random steps: a ringing resonance that keeps re-tuning itself. |
| `attractor_like_modulation_texture` | Attractor-Like Modulation Texture | sustained | 6 s | chaos, henon, texture, attractor | The output of a Henon map, smoothed, is the sound itself: a rough, quasi-periodic texture that sits between noise and tone. |
| `glissando_cluster` | Glissando Cluster | evolving | 4 s | glissando, cluster, sine, spread | Eight sine voices glide in different directions from a tight cluster to a wide spread: a sheet of crossing glissandi. |
| `deterministic_noise_sculpture` | Deterministic Noise Sculpture | sustained | 3 s | noise, sculpture, deterministic, harsh | Noise cut by a 64-step hashed gate and rectified, then filtered through stepped band-pass frequencies: a hard-edged, repeatable noise object. |
| `extreme_formant_distortion` | Extreme Formant Distortion | one-shot | 1.5 s | formant, extreme, distortion, vocal-like | A voiced formant source with its formants shifted by up to an octave, jumping between vowels and wavefolded: a mangled, speech-like scream. |
| `tonal_fragmentation` | Tonal Fragmentation | one-shot | 2 s | tonal, fragmentation, shards, glitch | A chord broken into pieces by random dropouts, short repeated slices and light bit reduction: a tonal sound in shards. |
| `microtonal_cluster` | Microtonal Cluster | sustained | 6 s | microtonal, cluster, beating, sine | Five sines a quarter-tone apart beat against each other slowly; the cluster shimmers without resolving to a pitch. |
| `feedback_like_synthetic_swell` | Feedback-Like Synthetic Swell | evolving | 4 s | feedback, swell, howl, bounded | Noise through a high-feedback comb whose tuning rises while a drive stage keeps the level bounded: a howl that swells without ever running away. |

</details>

<details>
<summary><b>Musical Motion & Tonal FX (`tonal`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `chord_swell` | Chord Swell | evolving | 3.5 s + 1 s tail | chord, swell, pad, tonal | The chosen chord on additive saw-like partials swells in under an opening low-pass and fades into a short tail. |
| `chord_fall` | Chord Fall | one-shot | 2.5 s | chord, fall, descending, tonal | The chosen chord drops an octave in pitch while the filter closes and the level decays. |
| `harmonic_transition` | Harmonic Transition | evolving | 3 s | chord, transition, harmonic, modulation | One chord cross-fades into a second chord built on another degree of the chosen scale. |
| `tonal_riser_cluster` | Tonal Riser Cluster | evolving | 3.5 s | riser, chord, converging, tonal | One glissando voice per chord tone, each starting a different distance below its target: the cluster converges onto the chord as it rises. |
| `arpeggio_rise` | Arpeggio Rise | one-shot | 2 s | arpeggio, rising, pluck, scale | Plucked notes climbing the chosen scale, evenly spaced across the effect and ringing into each other. |
| `arpeggio_fall` | Arpeggio Fall | one-shot | 2 s | arpeggio, falling, pluck, scale | Plucked notes stepping down the chosen scale: the mirror image of Arpeggio Rise. |
| `pitch_staircase` | Pitch Staircase | evolving | 3 s | staircase, scale, stepped, soft | A soft triangle tone that climbs the chosen scale one degree at a time, each step re-gated with a short envelope. |
| `octave_sweep` | Octave Sweep | evolving | 3 s | sweep, octave, saw, rising | A saw glides up two octaves while a second saw an octave above fades in: the sweep seems to double. |
| `harmonic_glissando` | Harmonic Glissando | evolving | 3 s | glissando, harmonic, additive, rising | An additive voice sliding up a fifth-and-octave interval while its upper partials grow stronger. |
| `dissonance_to_consonance_sweep` | Dissonance-to-Consonance Sweep | evolving | 4 s | chord, resolution, dissonance, sweep | Chord tones start crowded within a few semitones of each other and glide apart onto the chosen chord: tension resolving. |
| `tonal_reverse_swell` | Tonal Reverse Swell | evolving, reversed | 3 s | chord, reverse, swell, tonal | A chord stab with a long reverb, reversed: the chord appears to be sucked in and lands on its attack. |
| `tension_cluster` | Tension Cluster | sustained | 6 s | tension, cluster, sustained, dissonant | A static semitone cluster of saws with slow beating and a gentle tremolo: sustained tension that never moves. |
| `resolved_shimmer` | Resolved Shimmer | evolving | 3.5 s + 1 s tail | shimmer, resolution, chord, bright | High shimmering partials begin as a dissonant cluster and cross-fade into the chosen chord, then ring out: shimmer that resolves. |
| `detuned_chord_bloom` | Detuned Chord Bloom | evolving | 3.5 s | chord, bloom, detuned, supersaw | Detuned saw voices on every chord tone: the detune starts wide and settles while the filter opens, so the chord blooms into focus. |
| `pitch_spiral` | Pitch Spiral | evolving | 3 s | spiral, pitch, sine, motion | A sine voice swirling up and down around a centre pitch with a growing swing and a rising centre: a spiralling motion. |
| `harmonic_drop` | Harmonic Drop | one-shot | 2.2 s | drop, harmonic, chord, fall | A chord's pitch collapses two octaves while its upper partials die away first, leaving a low thud: a harmonic drop-off. |

</details>

<details>
<summary><b>Reverses, Tails & Transitions (`reverses`) - 16 presets</b></summary>

| id | name | behavior | duration | tags | description |
|---|---|---|---|---|---|
| `reverse_pluck` | Reverse Pluck | evolving, reversed | 1.8 s | reverse, pluck, string, swell | A plucked string note with a reverb tail, reversed: the string seems to grow out of silence into the pick. |
| `reverse_bell` | Reverse Bell | evolving, reversed | 2.5 s | reverse, bell, swell, inharmonic | A struck bell and its reverb, reversed: the ring swells up to the strike. |
| `reverse_noise_burst` | Reverse Noise Burst | evolving, reversed | 1.8 s | reverse, noise, burst, swell | A short band-passed noise burst with a long reverb tail, reversed: a hiss that swells and stops dead. |
| `reverse_chord` | Reverse Chord | evolving, reversed | 2.5 s | reverse, chord, swell, tonal | A plucked chord and its tail reversed: a tonal swell that ends on the chord's attack. |
| `reverse_bass_swell` | Reverse Bass Swell | evolving, reversed | 2 s | reverse, bass, swell, low | A decaying bass note, reversed: a low swell that builds into a sudden stop. |
| `reverse_percussion_flourish` | Reverse Percussion Flourish | evolving, reversed | 1.8 s | reverse, percussion, flourish, drum | A quick tom-and-snare run with its tail, reversed: the hits arrive in reverse order and swell. |
| `reverb_like_synthetic_tail` | Reverb-Like Synthetic Tail | one-shot | 0.3 s + 5 s tail | tail, reverb-like, decay, ambience | A tiny noise click feeding a damped synthetic reverb with a long decay and a slow echo: the tail on its own. |
| `infinite_rise_illusion` | Infinite-Rise Illusion | evolving | 6 s | illusion, shepard, riser, endless | A Shepard-style stack: five saw layers an octave apart each glide up an octave under a bell-shaped level window over the whole 5-octave span, so the pitch seems to rise forever. |
| `tail_swell` | Tail Swell | evolving | 3.5 s | tail, swell, reverb-like, chord | A short chord pluck flooded with reverb whose tail then swells in level: the wash comes up after the note has gone. |
| `tail_suction` | Tail Suction | evolving | 2 s | tail, suction, fall, transition | A falling, darkening noise-and-sine pull whose level rises towards the end and then vanishes: a sound drawn into a drain. |
| `transition_vacuum` | Transition Vacuum | one-shot | 2.2 s + 0 s tail | transition, vacuum, silence, fall | A dull low-passed hiss collapses to near-silence while a muffled thud lands at the end: the room empties before the next section. |
| `spectral_fade` | Spectral Fade | evolving | 4 s + 1 s tail | fade, spectral, darkening, tail | A bright, dense chord whose highs are removed progressively by a spectral tilt while a long tail rings: the colour drains out of the sound. |
| `pitch_reversed_illusion` | Pitch-Reversed Illusion | evolving, reversed | 2 s | reverse, pitch, illusion, swell | A note whose pitch falls as it decays, reversed: it swells upward in pitch and level and ends on the attack. |
| `decaying_shimmer` | Decaying Shimmer | one-shot | 2 s + 1 s tail | shimmer, decay, echo, tail | A bright shimmer whose partials fade from the top down while repeats echo and darken: glitter dying out. |
| `reverse_glitch_cascade` | Reverse Glitch Cascade | evolving, reversed | 2.5 s | reverse, glitch, cascade, chord | A chord run through bit reduction, dropouts and repeated slices, reversed: the fragments climb in on themselves. |
| `transition_sweep_down` | Transition Sweep-Down | one-shot | 2 s + 1 s tail | transition, sweep, falling, echo | A falling band-pass noise sweep and a falling sine, trailed by a darkening echo: the outgoing counterpart to a riser. |

</details>


# Rendering

The basic `render()`/`play()` methods on instruments, tracks, arrangements, and `MidiMusic` still work exactly as before. (Mastering to a loudness target? Normalisation is on by default here and would undo it; write with `fx::write_wav` instead. See [Wauvio FX](#wauvio-fx-wauvio_fx).) For more control over the output file itself, there's `RenderOptions`:

```cpp
wauvio::RenderOptions opts;
opts.bit_depth = 24;
opts.dither = wauvio::DitherType::Triangular;
opts.render_tail_sec = 2.0;       // pad silence at the end (reverb tails etc.)
opts.worker_threads = 0;          // 0 = auto-detect, 1 = single-threaded

wauvio::render(arrangement, "song.wav", opts);
wauvio::render(midi_music, "song.wav", opts);
```

All the options:

| Option | Default | Meaning |
|---|---|---|
| `sample_rate` | 44100 | output rate |
| `channels` | 2 | output channels |
| `bit_depth` | 16 | 16 / 24 / ... |
| `normalize`, `normalize_peak` | on, 0.95 | peak-normalize the result |
| `dither` | `None` | e.g. `Triangular` (useful when reducing bit depth) |
| `render_tail_sec` | 0.0 | extra silence so reverb/delay tails aren't cut off |
| `loop_count` | 1 | repeat the render |
| `interpolation` | `Linear` | resampling quality for sampled instruments |
| `worker_threads` | 0 | 0 = auto, 1 = single-threaded |
| `deterministic_seed_enabled`, `seed` | off, 0 | fix the global seed for reproducible randomness |

Multithreaded rendering only parallelises the independent per-track/per-part synthesis step; the actual mixdown always happens afterwards, single-threaded, in a fixed order; so `worker_threads = 1` and `worker_threads = 8` produce bit-identical output. That's true throughout the library: noise generators are seeded from the note itself rather than any global RNG state, so the same input always renders the same audio regardless of thread count.

Rendering each part to its own file:

```cpp
auto music = wauvio::track::load_midi("song.mid");
wauvio::track::render_stems(music, "output/stems/");
```

Each stem is rendered through the exact same instrument, controllers, and timing it would get in the normal mix; it's just isolated rather than left out. (A genre set with `setGenre` applies to stems too.)

And the reverse direction: writing a real, standards-compliant `.mid` file back out from something you built or loaded:

```cpp
wauvio::track::save_midi(arrangement, "output.mid");   // from an Arrangement you composed
wauvio::track::save_midi(music, "output.mid");         // from a loaded/edited MidiMusic
```

For a `MidiMusic` that was loaded with `retain_raw_midi` on (the default), the original tempo map is reproduced exactly rather than flattened to one BPM. One honest limitation: `Arrangement` tracks are built from `wauvio_ext` `Instrument` objects, not GM program numbers, so there's no reliable way to know what GM instrument a given `Instrument` "is"; exported Arrangement tracks all get program 0. Note pitch, timing, duration, velocity, and channel all round-trip correctly either way (verified: reloading a re-exported file reproduces the exact same note count and sub-millisecond-accurate timing as the original).

# Vocal engine (`wauvio_vocal`)

A Japanese singing-voice synthesizer built from Wauvio's own DSP: a glottal-source + formant-filter model with a Japanese kana/romaji front end. It's all procedural; no recordings, no ML, no data files.

**Pick one generation per program.** `wauvio_vocal.hpp` (V1) and `wauvio_vocal_v2.hpp` (V2) both define `wauvio::vocal::Singer`, `Style`, `Mora` and friends, so including both in one translation unit is a redefinition error. For new work, use V2.

| | V1 (`yune-v1`) | V2 (`yune-v2`) |
|---|---|---|
| Entry point | `wauvio_vocal.hpp` | `wauvio_vocal_v2.hpp` |
| Voices | one built-in (Yune) via `VoiceModel` | any number: engine and voice are separate `VoiceProfile`s (Yune, Reiha, Koharu, Itsuki shipped, plus your own `.voice` files) |
| Controls | `Style` (voice/pitch/articulation/dynamics) | everything in V1, plus personas, phonation, events, phoneme input, ornaments, natural-imperfection, speech, mic model |
| Status | frozen reference | current |

## Building

Header-only, C++17. From the repo root:
```cpp
#include "wauvio_vocal_v2.hpp"      // or "wauvio_vocal.hpp" for V1
```
`Singer::render()` returns a mono `Buffer`; use `normalize(b, 0.9f)` and `save_wav(b, "out.wav")`. To put a voice inside an `Arrangement` or a `MidiMusic` part, use `vocal::SingerInstrument` (a normal `Instrument` that sings one fixed lyric per note, with no coarticulation between neighbouring notes; use `Singer` for real phrases).

## Quick tour (V2)

```cpp
Singer s;                                   // Yune
s.set_tempo(96);
s.persona("soft", 0.6f);                    // gentler look
s.natural(0.6f, /*take=*/1);                // human imperfection
s.note(69, 480, "さ").ornament("scoop_in", 0.5f);
s.note(71, 480, "く");
s.note(72, 960, "ら");
s.rest(240);
s.event("laugh", 69, 1.0, 0.6f, "");        // a generated laugh
s.say("こんにちは", 1.2, "question", 64);    // speech with intonation
Buffer b = s.render();
```

`s.note(...)` returns a `NoteRef`; chain setters on it to tweak that one note (valid until the next note is added). In V1 these are `velocity`, `articulation`, `vibrato`, `no_vibrato`, `portamento`, `glide_from`, `bend`, `overshoot`, `scoop`, and `style()`; V2 adds the rest of the list below.

Timing can be given in ticks (`note`, `rest`; default 480 per quarter, `set_tempo(bpm, ppq)`) or seconds (`note_sec`, `rest_sec`, `note_at(start_sec, ...)`). `s.load_midi_file("song.mid")` turns a MIDI track into sung notes (using the file's lyric meta events if present).

## Lyrics

Lyrics are UTF-8 kana (hiragana or katakana) or, if the string is plain ASCII, romaji. Whitespace, `|`, `,` and `/` are ignored. `ー`, `〜` and `-` lengthen the previous vowel, `っ` is a geminate, `ん` a moraic nasal, and sequences like `おう`/`えい` inside one lyric merge into a single long vowel (`ParseOptions::merge_long_vowels`). Romaji doubled consonants (`kka`) make a geminate; write `n'` or `nn` for ん before a vowel.

V2 additionally accepts:

- **Phonemes:** `{k a n a}`, IPA, `*1.5` (strength), `@0.7` (weight).
- **Splitting a note's time between morae:** `さ[0.7]ん[0.3]`.
- **Foreign kana:** とぅ ヴぁ いぇ くぁ and similar.
- **Event tags** in the lyric: `<laugh>`, `<giggle:fu>`, `<sob>`, `<hum>`, `<shout>`, `<kyaa>`, and interjections like `<eeto>`, `<fuun>`.
- **Parsing/pronunciation options:** `bidakuon`, `n_nasalize`, `mumble`, `devoice_max`, `vv_glide`, `transition_speed`, `mora_weighting`, `glottal_final`, and `parse_options().cute`.

## Style (the main control surface)

`Style` fields are set as singer-wide defaults (`s.style().x = ...`, applies to notes added afterwards) and can be overridden per note (`note.style().x`). Values are 0..1 unless a unit is shown.

| Group | Fields |
|---|---|
| Voice | `brightness`, `breathiness`, `tension`, `softness`, `power`, `resonance`, `femininity` (0.5 = model default, ±6% formant shift) |
| Pitch | `vibrato_depth` (semitones), `vibrato_rate` (Hz), `vibrato_delay`/`vibrato_fade` (s), `portamento` (s), `pitch_response`, `overshoot`, `scoop` (semitones), `exit_fall` (semitones), `drift`, `micro` |
| Articulation | `consonant_strength`, `consonant_duration`, `vowel_clarity`, `legato`, `attack_sharpness`, `release`, `devoicing` (bool) |
| Dynamics | `volume`, `velocity_response`, `compression`, `dynamic_timbre` |
| V2 phonation | `fry`, `growl`, `tremor`, `air`, `reg`, `reg_auto`, `glottal_onset`, `formant_shift`, `mouth_open`, and `grit` (male voice, when pushed) |

Negative vibrato values mean "use the voice's own default".

## What V2 adds

| Area | Use |
|---|---|
| Personas | `s.persona("soft", 0.6)`, `persona_blend(a, b, t)`, `persona_lane(name, curve)` (change over the song), `note.persona(...)`, `note.persona_morph(...)`. Each voice has two: `normal` (its default look, the former "Sweet") and `soft`. Bounded by the identity checks. |
| Phonation | `Style::fry growl tremor air reg reg_auto glottal_onset formant_shift mouth_open`; `lane("breathiness", curve)` etc. for change over time. |
| Events | Lyric tags `<laugh>`, `<giggle:fu>`, `<sob>`, `<hum>`, `<shout>`, `<kyaa>`, interjections (`<eeto>`, `<fuun>` …); or `s.event(kind, midi, secs, intensity, variant)`. |
| Phonemes | `{k a n a}`, IPA, `*1.5` strength, `@0.7` weight; `さ[0.7]ん[0.3]`; foreign kana; plus the options listed under Lyrics. |
| Pitch | `note.ornament("shakuri"/"fall"/"doit"/"scoop_in"/"kobushi"/"turn"/"mordent"/"trill", depth)`, `pitch_loud`, `pitch_snap`, `undershoot`, `porta_interval`, `exit_fall_min`, `micro`. |
| Living voice | `s.natural(0.6, take)`: voice-level imperfection (`Style::natural`: jitter/shimmer, slow pitch, loudness, breath and formant wander, irregular vibrato) plus per-note timing/tuning/entry error and phrase breath-pressure. `take` = a different performance. Per-note: `style().natural` or `lane("natural", curve)`. |
| Shaping | `humanize(timing, pitch, length, bias, take)`, `auto_expression()`, `phrase_arc("arch"/"crescendo"/"decrescendo"/"swell", t0, t1)`. |
| Speech | `say("こんにちは", secs, "question"/"statement"/"flat"/"trail"/"emphatic"/"ne"/"yo"/"ka"/"no", midi)`. |
| Recording | `apply_mic(buffer, sr, MicModel{proximity, air, de_ess, mouth_noise})`: a post-process separate from the voice; default = no-op. |
| Voices | `VoiceProfile`, `voice_yune()`, `make_voice(id\|file)`, `profile_to_text` / `save_profile` / `load_profile` (typos in a file are errors). |
| Tooling | `s.diagnostics()`; `sing_score`, `voice_tool`, `listen` (see below). |

## Voices

**Engine and voices are separate.** The engine knows nothing about any particular singer. A character is a `VoiceProfile` in `wauvio_vocal/yune-v2/voices/` (model, default look, lyric defaults, personas). Yune is `voices/voice-yune.hpp`, also exported as editable text, `voices/voice-yune.voice`.

```cpp
Singer s;                                    // sings as Yune
Singer s(voice_yune());                      // same, explicit
Singer s(p);                                 // any VoiceProfile
s.set_profile(p);                            // switch character (do this before adding notes)

VoiceProfile p;
std::string err;
if (!make_voice("itsuki", p, &err)) { /* unknown id or bad .voice file: err says why */ }
make_voice("my.voice", p);                   // or load from a file (partial files override Yune)
```

Score files start with `voice <id|file.voice>`.

| Voice | Character | Range (comfortable) |
|---|---|---|
| `yune` 夢音 | bright, clear female (default) | E3-E6 |
| `reiha` 玲羽 | cool, crystalline, airy female; delicate, detached | G3-G6 |
| `koharu` 小春 | warm, punchy, expressive female; chesty, crisp, slightly husky | D3-D6 |
| `itsuki` 樹 | deep, resonant, smooth male; rounded vowels, grainy when pushed (`Style::grit`) | F2-G4 |

V2 keeps each voice's identity (the `VoiceModel`: tract proportions, vowel centres, source family) and adds **systems** on top: expression channels, bounded personas, generated vocal events, phoneme control, ornaments and speech.

### Adding a voice (checklist)

The engine (`synth.hpp`, `pitch.hpp`, `expr.hpp`, `events.hpp`, ...) is voice-agnostic, and **nothing outside `voices/` may mention a particular voice**. A voice is a `VoiceProfile` (`voices/profile.hpp`): the character's model, default look, lyric defaults, personas and recommended `natural` strength.

1. `voice_tool export yune voices/voice-<id>.voice` and edit it (or copy `voice-yune.hpp` to `voice-<id>.hpp`).
2. Set the **identity** (what makes it that singer): `vowel` rows (F1/F2/F3, bandwidths), `f4..f6`, `singer_formant_hz`, `rd_base`, `tilt_hz_*`, `aspiration`, `synthetic`, vibrato defaults, `range` (comfortable and hard, MIDI). Rules of thumb: a larger tract (male) lowers all formants by about 15-20% and lowers the singer's formant to ~2.8 kHz; `range` must follow the voice's real compass; keep a/i/u/e/o in their order along F1/F2 so lyrics stay intelligible.
3. Set the default **look** (`style` lines) and write its **personas** as absolute looks. Personas are bounded variations; the identity-envelope check must report every one "within envelope". Every voice has exactly two personas: `normal` (its default look) and `soft`. See `voices/voice-reiha.hpp` for the pattern.
4. If using C++, add the header to `voices/voices.hpp` (`builtin_voices()` gets one line). A `.voice` file needs no registration: `make_voice("path/to/file.voice", p)`.
5. `voice_tool demo <id> out.wav` and listen: same phrase, default look then `soft`.

## Command-line tools (V2)

All three are single files in `wauvio_vocal/yune-v2/tools/` that include the headers by relative path:

```bash
g++ -std=c++17 -O2 wauvio_vocal/yune-v2/tools/sing_score.cpp -o sing_score
g++ -std=c++17 -O2 wauvio_vocal/yune-v2/tools/voice_tool.cpp -o voice_tool
g++ -std=c++17 -O2 wauvio_vocal/yune-v2/tools/listen.cpp     -o listen
```

**`voice_tool`**
```
voice_tool list                         built-in voices
voice_tool export ID out.voice          write a voice as an editable text file
voice_tool check file.voice             parse a .voice file (typos are errors) and report
voice_tool demo ID|file.voice out.wav   a short phrase in that voice (default look, then 'soft')
```

**`sing_score`**: `sing_score score.txt|song.mid out.wav [--voice id|file.voice] [--dump timeline.csv] [--check]`. `--dump` writes what the engine will actually sing (start, duration, pitch, velocity, morae, event); `--check` only runs diagnostics and returns non-zero if there are warnings. Score files are one command per line, `#` for comments:

```
voice yune|path.voice            # first line: which character sings
tempo BPM                        rest TICKS
style NAME VALUE                 # any Style field; "style devoicing off"
persona NAME [AMOUNT]            persona_blend A B T
persona_lane NAME t:v;t:v;...    lane NAME t:v;t:v;...      # absolute seconds; NAME "pitch" = semitones
seed N    humanize [TIMING PITCH LENGTH BIAS TAKE]    auto_expression
arc KIND T0 T1 [DEPTH]           # phrase_arc over seconds
note MIDI TICKS LYRIC [VEL] [extras]       # lyric may be {k a n a} phoneme input
event KIND MIDI SECONDS [INTENSITY] [VARIANT]
say "TEXT" SECONDS [KIND] [MIDI]           # speech with intonation (TEXT without spaces)
```
Note extras: `vib=D,R,DELAY`  `porta=S`  `novib`  `stac`  `whisper`  `hum`  `orn=NAME[,DEPTH]`  `persona=NAME[,AMOUNT]`  `lane=NAME@t:v;t:v` (seconds from note start)  `NAME=VALUE` (any Style field).

Example:
```
voice reiha
tempo 90
persona soft 0.5
natural 0.5
note 67 480 さ
note 69 480 く orn=scoop_in,0.6
note 71 960 ら vib=0.4,5.5,0.3
event giggle 72 0.8 0.5
```
(`natural` is a Singer method; in a score file use `humanize` / `auto_expression` / `style natural VALUE`.)

**`listen`**: `listen [outdir=listen_out] [only-substring]` renders a named A/B set of WAVs for every V2 feature (default, personas, events, ornaments, speech, mumble, mic, ...). Files named `*_default*` are the V1-identical reference to compare against.

**V1's `sing_score`** (`wauvio_vocal/yune-v1/tools/sing_score.cpp`) is the smaller original: `tempo`, `rest`, `style`, and `note MIDI TICKS LYRIC [VEL] [vib=… porta=… novib stac whisper hum]`.

## Known limits

- Events are rule-generated from the same voice (no samples); a laugh is convincing as a sung/acted laugh, not as a recording. Tune `intensity` and `variant` by ear.
- `diagnostics()` does not (yet) flag unknown variants inside an event tag.
- `SingerInstrument` sings one note at a time, so there's no coarticulation across notes; use `Singer` for sung phrases.
- Japanese only.

## Some extra notes

- Nothing lower in the stack knows or cares about anything higher up.
  `wauvio.hpp` has no idea `wauvio_ext` exists.
- Sampled and synthesised instruments are interchangeable — same
  interface, same `play()` call, so you can swap one for the other (or
  upgrade from synthesised to sampled) without rewriting anything.
- "Automatic" doesn't mean "no say in the matter." Every decision the MIDI
  loader makes on your behalf can be overridden, and so can everything a
  genre preset does (override it, pin the part, or leave instrumentation off).
- Pitch bend is a real continuous curve now, not a single value sampled at
  note-on. A held note that gets bent up and back down mid-note actually
  sweeps; the MIDI loader builds this automatically from every pitch-bend
  message that occurs while a note is held.
- There's a small generic modulation system (`ModRoute`) on synthesised
  instruments' `TimbreRecipe` — velocity/mod-wheel/aftertouch/expression/
  key-position/LFOs routed to pitch, filter cutoff, volume, vibrato depth,
  or tremolo depth. It's meant to stay simple; it's not a full modular
  synth graph. (The genre "wobble" is built on it: an LFO routed to filter cutoff.)
