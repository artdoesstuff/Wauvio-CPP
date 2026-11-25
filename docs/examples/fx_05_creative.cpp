// Creative effects: tempo-synced ping-pong delay, tremolo, auto-pan and bitcrushing.
#include "wauvio/fx/ext.hpp"
using namespace wauvio;

int main() {
    instruments::SynthLead lead; music::Arrangement a;
    a.add(lead, {{69, 0.25}, music::rest(0.25), {72, 0.25}, music::rest(0.25), {76, 0.5}});
    StereoBuffer audio = a.render();

    fx::Chain c;
    auto& d = c.add<fx::StereoDelay>();
    d.sync_bpm = 120; d.sync_beats_left = 0.75;     // dotted eighth, locked to 120 BPM
    d.ping_pong = true; d.feedback = 0.5; d.highcut_hz = 4500; d.mix = 0.4;

    auto& t = c.add<fx::Tremolo>(); t.rate_hz = 6; t.depth = 0.3;
    auto& p = c.add<fx::AutoPan>(); p.rate_hz = 0.4; p.depth = 0.6;
    auto& b = c.add<fx::Bitcrusher>(); b.bits = 10; b.downsample = 2; b.mix = 0.25;

    c.process(audio, 44100);
    fx::write_wav(audio, "fx_05_creative.wav");
}
