// Build an effect chain: effects run in the order you add them.
#include "wauvio/fx/ext.hpp"
using namespace wauvio;

int main() {
    // Make some audio to process (a chord on an instrument from the music layer).
    instruments::Wurlitzer200A keys;
    music::Arrangement a;
    a.add_track([&]{ music::Track t(keys); t.add_chord({{57, 60, 64, 67}, 2.0}); t.add_chord({{53, 57, 60, 64}, 2.0}); return t; }());
    StereoBuffer audio = a.render();

    fx::Chain chain;
    chain.add<fx::EQ>().highpass(80).peak(250, -2.0, 1.0).high_shelf(8000, 2.0);   // clean up lows, add air

    auto& comp = chain.add<fx::Compressor>();                                        // keep a reference to tweak settings
    comp.threshold_db = -22; comp.ratio = 3.0; comp.attack_ms = 15; comp.release_ms = 120;

    auto& sat = chain.add<fx::Saturator>();
    sat.mode = fx::Saturator::Mode::Tape; sat.drive_db = 6; sat.mix = 0.6;

    chain.add<fx::StereoChorus>().mix = 0.3;
    auto& verb = chain.add<fx::FdnReverb>(); verb.decay_s = 2.2; verb.mix = 0.2;

    // process() removes latency so the result stays in time, and keeps reverb tails by lengthening the buffer.
    chain.process(audio, 44100);

    fx::write_wav(audio, "fx_01_effect_chain.wav");
}
