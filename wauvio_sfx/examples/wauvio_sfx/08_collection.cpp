// Render a collection: every preset tagged "riser", at the same tempo and seed, into one folder.
#include "../../wauvio_sfx.hpp"
#include <filesystem>
#include <iostream>

int main() {
    namespace sfx = wauvio::sfx;
    std::filesystem::create_directories("sfx_examples_out/risers");
    size_t n = 0;
    for (const sfx::Preset* p : sfx::search("riser")) {
        sfx::Effect e(*p);
        e.tempo(128).seed(42);
        try { e.render().save("sfx_examples_out/risers/" + p->id + ".wav"); ++n; }
        catch (const sfx::Error& err) { std::cerr << p->id << ": " << err.what() << "\n"; return 1; }   // never skip a failure silently
    }
    std::cout << "rendered " << n << " presets\n";
}
