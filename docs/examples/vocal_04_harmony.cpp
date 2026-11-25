// A lead voice with a harmony and double-tracked takes, mixed to stereo by an Ensemble.
#include "wauvio/vocal/v3.hpp"
using namespace wauvio::vocal::v3;

int main() {
    Singer lead;
    lead.set_tempo(90); lead.natural();
    for (auto [midi, ticks, syl] : { std::tuple<int,int,const char*>{67,480,"ら"}, {69,480,"ら"}, {71,480,"ら"}, {72,1440,"あ"} })
        lead.note(midi, ticks, syl);

    Ensemble choir;
    choir.add(lead, "lead");                                   // centre
    choir.add_double(lead, /*copies=*/2);                      // two extra takes, slightly detuned and spread
    choir.add_harmony(lead, +4, "third", -0.5f, 0.6f);         // a major third above, panned left, quieter
    choir.add_harmony(lead, -5, "fourth below", 0.5f, 0.55f);  // a fourth below, panned right

    wauvio::StereoBuffer mix = choir.render_stereo();
    wauvio::normalize(mix, 0.9);
    wauvio::save_wav_stereo(mix, "vocal_04_harmony.wav");
}
