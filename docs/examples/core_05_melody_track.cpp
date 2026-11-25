// Sequencing with seq::Track, plus helpers for chords and arpeggios.
#include "wauvio.hpp"
using namespace wauvio;
using namespace wauvio::theory::notes;

int main() {
    seq::Instrument lead;
    lead.shape    = osc::WaveShape::BL_Square;
    lead.envelope = env::ADSR(0.01, 0.08, 0.5, 0.1);
    lead.use_lpf  = true;           // gentle one-pole low-pass
    lead.lpf_cutoff = 3500;

    // A Track plays notes one after another. REST is silence.
    seq::Track melody;
    melody.instrument = lead;
    melody.add(C5, 0.25).add(E5, 0.25).add(G5, 0.25).add(C6, 0.5)
          .rest(0.25)
          .add(G5, 0.25).add(E5, 0.25).add(C5, 0.75);
    Buffer tune = melody.render();

    // A chord (all notes together) and an arpeggio (notes in a cycle).
    Buffer chord = seq::render_chord({C4, E4, G4}, 2.0, lead);
    Buffer arp   = seq::render_arpeggio({C4, E4, G4, C5}, 0.125, 4, lead);

    // Place them on a timeline with MasterBus: schedule(buffer, start_seconds, gain).
    seq::MasterBus bus;
    bus.schedule(tune,  0.0, 0.6);
    bus.schedule(chord, 0.0, 0.3);
    bus.schedule(arp,   2.5, 0.5);
    StereoBuffer mix = bus.mix_stereo();
    normalize(mix);
    save_wav_stereo(mix, "core_05_melody_track.wav");
}
