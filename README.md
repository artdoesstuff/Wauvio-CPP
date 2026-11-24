# Wauvio

A header-only C++17 audio engine: synthesis, a large instrument library, effects and mastering, MIDI and SoundFont playback, a sound-effect designer and a Japanese singing-voice synthesiser. No dependencies to build or link.

**Documentation: [GitHub Pages](http://artdoesstuff.github.io/Wauvio-CPP/)**

## What's inside

| Module | Include | Purpose |
|---|---|---|
| Core | `wauvio.hpp` | Oscillators, envelopes, filters, FM and voices, sequencing, WAV export, playback |
| Music layer | `wauvio/ext.hpp` | Notes, arrangements, 300+ modeled instruments, sampling, genre presets |
| FX suite | `wauvio/fx.hpp` | EQ, dynamics, saturation, reverb, mixer, loudness mastering |
| MIDI | `wauvio/midi.hpp` | Load, restyle, render and write MIDI files |
| SoundFont | `wauvio/sf2.hpp` | Use `.sf2` files as instrument banks |
| SFX | `wauvio/sfx.hpp` | Sound-effect engine with a 190-preset catalog and CLI |
| Vocal | `wauvio/vocal/v3.hpp` | Singing and speech synthesis (Yune and other voices) |

## Quick start

```cpp
#include "wauvio/ext.hpp"
using namespace wauvio;

int main() {
    music::Arrangement arr;
    instruments::Trumpet trumpet;
    music::Melody tune = { {60, 0.5}, {64, 0.5}, music::rest(0.5), {72, 1.0} };
    arr.add(trumpet, tune);
    arr.limiter();
    render(arr, "out.wav");
}
```

```sh
g++ -std=c++17 -O2 -I/path/to/wauvio my_program.cpp -o my_program
```

## Requirements

- A C++17 compiler.
- Windows: link `winmm.lib` for playback.
- Linux: install FFmpeg for live playback (WAV rendering needs nothing).

## License

GPL-3.0. See [LICENSE](LICENSE).
