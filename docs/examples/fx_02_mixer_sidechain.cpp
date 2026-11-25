// A mini mix: sources, per-source inserts, a reverb bus with sends, sidechain "pumping", and mastering.
#include "wauvio/fx/ext.hpp"
using namespace wauvio;

static StereoBuffer play(music::Instrument& i, music::Melody m) {
    music::Track t(i); t.add(m); return t.render();
}

int main() {
    instruments::Kit808 kit; instruments::SynthLead lead_i; instruments::SynthPad pad_i;
    StereoBuffer kick, pad, lead;

    // Four-on-the-floor kick (note 36) for 8 beats at 120 BPM, as one buffer.
    std::vector<std::pair<double,int>> hits;
    for (int b = 0; b < 16; ++b) hits.push_back({b * 0.5, 36});
    kick = kit.render_pattern(hits, 8.0, music::Dynamics::f);

    pad  = play(pad_i,  {{57, 8.0, music::Dynamics::mp}});
    lead = play(lead_i, {{69, 0.5}, {72, 0.5}, {76, 1.0}, {74, 0.5}, {72, 0.5}, {69, 1.0}, music::rest(4.0)});

    fx::Mixer mix(44100);
    int iKick = mix.add_source({"kick", kick, 0.0, 1.0});
    int iPad  = mix.add_source({"pad",  pad,  0.0, 0.7});
    int iLead = mix.add_source({"lead", lead, 0.0, 0.6});

    mix.insert(iPad).add<fx::EQ>().highpass(120);                 // per-source insert effects
    int verbBus = mix.add_bus("reverb");
    mix.bus(verbBus).add<fx::FdnReverb>().mix = 1.0;               // 100% wet: the bus is the reverb
    mix.send(iLead, verbBus, 0.35);                                // send some lead to it
    mix.send(iPad,  verbBus, 0.25);

    fx::SidechainSettings sc; sc.threshold_db = -35; sc.ratio = 8; sc.release_ms = 180;
    mix.sidechain(iPad, iKick, sc);                                // kick ducks the pad

    mix.set_mastering(fx::master_for_genre("house"));              // loudness target, EQ, glue, limiter
    StereoBuffer out = mix.render();
    fx::write_wav(out, "fx_02_mixer_sidechain.wav");
}
