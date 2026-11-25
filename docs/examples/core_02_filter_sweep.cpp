// A classic synth sound: a sawtooth whose filter opens over time (automation).
#include "wauvio.hpp"
using namespace wauvio;

int main() {
    osc::Oscillator saw;
    saw.shape     = osc::WaveShape::BL_Sawtooth;   // BL_ = band-limited, less aliasing
    saw.frequency = theory::notes::A2;
    Buffer raw = saw.render(4.0, 44100);

    // Automation = a list of (time in seconds, value) points; values between them are interpolated.
    std::vector<mod::AutoPoint> cutoff = {
        {0.0, 200.0},     // start dark
        {3.0, 6000.0},    // open up
        {4.0, 800.0},     // close again
    };

    // Run the buffer through a low-pass filter whose cutoff follows the curve.
    Buffer swept = mod::apply_filter_automation(raw, cutoff, /*Q=*/3.0,
                                                filter::SVFilter::Mode::LowPass, 44100);
    normalize(swept, 0.8);
    save_wav(swept, "core_02_filter_sweep.wav", 44100);
}
