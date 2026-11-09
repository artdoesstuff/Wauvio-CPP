// Render one preset to a WAV file.
//   g++ -std=c++17 -O2 -I<repo root> 01_render_preset.cpp -o ex01 && ./ex01
#include "../../wauvio_sfx.hpp"
#include <filesystem>
#include <iostream>

int main() {
    namespace sfx = wauvio::sfx;
    std::filesystem::create_directories("sfx_examples_out");

    sfx::Effect riser = sfx::make("noise_riser");     // a preset id; throws sfx::Error (with suggestions) if unknown
    sfx::Rendered audio = riser.render();             // 44.1 kHz stereo, peak normalised to -1 dBFS
    audio.save("sfx_examples_out/noise_riser.wav");

    const sfx::Stats s = audio.stats();
    std::cout << "noise_riser: " << s.seconds << " s, peak " << s.peak_db() << " dB, rms " << s.rms_db() << " dB\n";
}
