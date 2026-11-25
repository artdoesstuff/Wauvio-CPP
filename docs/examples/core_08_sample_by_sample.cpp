// Under the hood: generate audio one sample at a time with tick().
// Here an LFO wobbles the pitch of an oscillator (vibrato) and a filter smooths it.
#include "wauvio.hpp"
using namespace wauvio;

int main() {
    const int sr = 44100;
    osc::Oscillator o;  o.shape = osc::WaveShape::BL_Sawtooth;
    mod::LFO vib;       vib.shape = mod::LFOShape::Sine; vib.rate_hz = 5.5; vib.depth = 1.0;
    filter::SVFilter lp; lp.mode = filter::SVFilter::Mode::LowPass; lp.cutoff = 2500; lp.Q = 1.0;
    lp.update_coeffs(sr);                          // call after changing cutoff / Q

    Buffer out(sr * 3);
    for (size_t i = 0; i < out.size(); ++i) {
        double wobble = vib.tick(sr);                       // -1..1
        o.frequency = 330.0 * (1.0 + 0.015 * wobble);       // +-1.5% pitch
        out[i] = lp.tick(o.tick(sr) * 0.5f, sr);
    }
    save_wav(out, "core_08_sample_by_sample.wav", sr);
}
