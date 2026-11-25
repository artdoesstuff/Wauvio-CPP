// Choose which instruments play. Three ways: before loading (resolver), after loading (set_instrument), or per channel.
#include "wauvio/midi.hpp"
using namespace wauvio;

int main() {
    { music::Arrangement a; instruments::Trumpet t; instruments::UprightBass b;
      a.add(t, {{72, 0.5}, {74, 0.5}, {76, 1.0}}); a.add(b, {{48, 1.0}, {43, 1.0}});
      midi::track::save_midi(a, "demo2.mid"); }

    // (Note: save_midi writes every track as GM program 0 = piano, so without the rules below both parts would be piano.)

    // 1. Before loading: tell the resolver what to use. Rules are checked in a fixed priority order:
    //    channel override > bank/program override > bank provider (SF2) > program override > built-in GM.
    //    Here we assign by MIDI channel (0-based; the demo file uses channel 0 and 1).
    midi::track::LoadOptions opts;
    opts.resolver.override_channel(0, [] { return std::make_shared<instruments::Trumpet>(); });
    opts.resolver.override_channel(1, [] { return std::make_shared<instruments::UprightBass>(); });
    //   Other rules:  override_program(gm_program, factory), override_bank_program(msb, lsb, program, factory),
    //                 override_percussion_note(note, factory)

    midi::track::MidiMusic song = midi::track::load_midi("demo2.mid", opts);

    // 2. After loading: swap a part by index (any music::Instrument works).
    song.set_instrument<instruments::Harp>(1);
    song.pin_instrument(1);          // pinned parts are never changed by a genre preset later

    render(song, "midi_02_choose_instruments.wav");
}
