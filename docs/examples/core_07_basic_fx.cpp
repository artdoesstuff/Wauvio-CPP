// Adding effects. Every effect follows the same recipe: set parameters -> init(sample_rate) -> process(buffer).
#include "wauvio.hpp"
using namespace wauvio;
using namespace wauvio::theory::notes;

int main() {
    seq::Instrument pluck;
    pluck.shape = osc::WaveShape::BL_Triangle;
    pluck.envelope = env::ADSR(0.005, 0.15, 0.2, 0.2);
    seq::Track t; t.instrument = pluck;
    t.add(E4, 0.3).add(G4, 0.3).add(B4, 0.3).add(E5, 0.6);
    Buffer dry = t.render();
    dry.resize(dry.size() + 44100 * 3, 0.0f);     // leave room for the echo/reverb tail

    fx::basic::DelayLine echo;
    echo.time_ms = 375; echo.feedback = 0.45; echo.wet = 0.4;
    echo.init(44100);
    echo.process(dry);

    fx::basic::Reverb room;
    room.room_size = 0.85; room.wet = 0.35;
    room.init(44100);
    room.process(dry);

    normalize(dry);
    save_wav(dry, "core_07_basic_fx.wav");
}
