// Finishing a vocal: mic model + the ready-made FX chain (EQ, compression, delay, reverb, limiter).
#include "wauvio/vocal/v3_fx.hpp"
using namespace wauvio::vocal::v3;

int main() {
    Singer s;
    s.set_tempo(110); s.natural(); s.auto_expression();
    s.note(72, 480, "ひ"); s.note(74, 480, "か"); s.note(76, 480, "り"); s.note(79, 1440, "へ");

    wauvio::Buffer dry = s.render();

    MicModel mic;                       // optional "recording" colour
    mic.proximity = 0.4; mic.air = 0.3; mic.de_ess = 0.5;

    // Presets: natural, pop, intimate, hall. bpm > 0 syncs the delay in "pop".
    wauvio::StereoBuffer finished = process_vocal(dry, "pop", &mic, 44100, 110);
    wauvio::normalize(finished, 0.9);
    wauvio::save_wav_stereo(finished, "vocal_05_polish.wav");
}
