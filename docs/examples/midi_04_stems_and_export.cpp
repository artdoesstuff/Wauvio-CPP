// Export one WAV per part (stems) for use in a DAW, and write the (possibly edited) score back out as MIDI.
#include "wauvio/midi.hpp"
using namespace wauvio;

int main() {
    { music::Arrangement a; instruments::Flute f; instruments::Cello c;
      a.add(f, {{76, 0.5}, {79, 0.5}, {83, 1.0}}); a.add(c, {{52, 1.0}, {55, 1.0}});
      midi::track::save_midi(a, "demo4.mid"); }

    midi::track::MidiMusic song = midi::track::load_midi("demo4.mid");

    RenderOptions opt;
    opt.bit_depth = 24;
    opt.normalize = false;                       // keep each stem's relative level

    // render_stems writes <dir>/<part name>.wav for every part and returns the paths.
    // (The directory must exist.)
    std::system("mkdir -p stems");
    auto files = midi::track::render_stems(song, "stems", opt);
    for (auto& f : files) std::printf("wrote %s\n", f.c_str());

    midi::track::save_midi(song, "demo4_out.mid");
}
