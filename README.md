# Wauvio-CPP

A C++17 (or newer) audio synthesis engine and music framework. What started as a single synthesis header grew into several layers that build on each other.

* `wauvio.hpp` - the actual audio engine. Oscillators, FM synthesis, filters, envelopes, reverb/chorus/delay/distortion/EQ, mixing, WAV export, playback. This is where all the samples are generated.
* `wauvio_ext/` - a higher-level layer on top of the existing engine. Instead of hand-wiring oscillators and envelopes, and manually writing instruments every time you want to produce a sound, you get 300+ pre-made instruments, ready to be instantiated and played, plus notes/chords/melodies/tracks/arrangements to compose with. It also contains the **genre presets** (see [Genres](#genres)).
* `wauvio_midi/` - loads a standard MIDI binary and turns it into playable Wauvio audio on its own. Simply point to a `*.mid` file, and let it do the rest. It also writes MIDI back out, and can restyle a loaded file in the manner of a genre.
* `wauvio_sf2/` - loads a real SoundFont 2 (`.sf2`) bank and turns its presets into the same kind of sampled instrument the rest of the library already uses, so a MIDI file can play back through actual recorded instrument samples instead of the built-in synthesized fallback.
* `wauvio_vocal/` - a Japanese singing-voice synthesizer (formant/glottal-source model, no samples). Two generations ship: `yune-v1` (frozen, the original single voice) and `yune-v2` (the current engine: separate voice profiles, personas, laughs and other vocal events, phoneme control, ornaments, speech). See [Vocal engine](#vocal-engine-wauvio_vocal).

Each layer only ends up depending on the ones below it (`wauvio_sf2` and `wauvio_midi` both sit on top of `wauvio_ext`, and can be combined together, but neither depends on the other; `wauvio_vocal` sits on `wauvio` + `wauvio_ext` and reuses the MIDI *parser* headers so a singer can read a `.mid`, but doesn't need the rest of `wauvio_midi`). `wauvio.hpp` never gets touched by anything above it; if you're already using it directly, absolutely nothing would change for you.

## In this repository

```
wauvio.hpp
wauvio_ext.hpp
wauvio_midi.hpp
wauvio_sf2.hpp
wauvio_vocal.hpp        (V1 entry point)
wauvio_vocal_v2.hpp     (V2 entry point)
wauvio_ext/
wauvio_midi/
wauvio_sf2/
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
instrument, plus reverb/limiting at the `Arrangement` (master bus) level.

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

# Rendering

The basic `render()`/`play()` methods on instruments, tracks, arrangements, and `MidiMusic` still work exactly as before. For more control over the output file itself, there's `RenderOptions`:

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

V2 is built so that **every new control defaults to "off": the default render is bit-identical to V1** (verified sample-exact against golden renders; the one documented exception is the ヴぁ fix, recorded in `tests/golden/CHANGES.md` in the test suite).

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
5. Run the checks (`test_voices` runs for every registered voice: round trip, range sanity, no NaN, levels).
6. `voice_tool demo <id> out.wav` and listen: same phrase, default look then `soft`.

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

## Diagnostics and tests

`s.diagnostics()` returns warnings about the score as built (it does not yet flag unknown *variants* inside an event tag). The V2 design was verified by a test suite (not part of this repository snapshot): a golden regression sample-exact against V1, an identity envelope per persona and voice, and ~145 objective measurements (F0, spectra, cepstral formants, level envelopes) for the expression/event/phoneme/shape/mic/persona controls, plus clean runs under `-Wall -Wextra` and ASan/UBSan. Those prove the controls do what they claim and that the default is unchanged. **They are not listening tests**: final tuning of how natural a persona or event sounds needs ears.

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
