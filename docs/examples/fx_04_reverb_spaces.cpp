// Reverb two ways: algorithmic (FdnReverb) and convolution with a generated room impulse response.
#include "wauvio/fx/ext.hpp"
using namespace wauvio;

int main() {
    // A short percussive sound is the best test signal for a reverb.
    instruments::Woodblock wb; music::Arrangement a;
    a.add(wb, {{72, 0.3}, {72, 0.3}, {79, 0.6}});
    StereoBuffer dry = a.render();

    // 1. Algorithmic reverb.
    StereoBuffer algo = dry;
    fx::Chain c1; auto& v = c1.add<fx::FdnReverb>(); v.decay_s = 3.0; v.size = 1.4; v.mix = 0.4;
    c1.process(algo, 44100);
    fx::write_wav(algo, "fx_04_reverb_algorithmic.wav");

    // 2. Convolution: set_ir(ir::Params) generates an impulse response. Presets: room, chamber, hall, cathedral, plate.
    for (auto [name, params] : { std::pair<const char*, fx::ir::Params>{"room", fx::ir::room()},
                                 {"cathedral", fx::ir::cathedral()} }) {
        StereoBuffer wet = dry;
        fx::Chain c2; auto& conv = c2.add<fx::ConvolutionReverb>();
        conv.set_ir(params); conv.mix = 0.4;
        c2.process(wet, 44100);
        fx::write_wav(wet, std::string("fx_04_reverb_conv_") + name + ".wav");
    }
    // To use a real recorded space instead: conv.load_ir("my_hall.wav");
}
