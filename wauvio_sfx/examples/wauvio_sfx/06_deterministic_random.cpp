// Seeds: the same seed always gives the same audio; a different seed gives a different take.
#include "../../wauvio_sfx.hpp"
#include <filesystem>
#include <iostream>

int main() {
    namespace sfx = wauvio::sfx;
    std::filesystem::create_directories("sfx_examples_out");

    auto take = [](uint64_t seed) { sfx::Effect e = sfx::make("glitch_fill"); e.seed(seed); return e.render(); };
    const sfx::Rendered a = take(100), b = take(100), c = take(101);
    std::cout << "same seed identical:      " << (a.left == b.left && a.right == b.right ? "yes" : "NO") << "\n"
              << "different seed identical: " << (a.left == c.left ? "yes" : "no") << "\n";
    for (uint64_t s = 1; s <= 4; ++s) take(s).save("sfx_examples_out/glitch_fill_take" + std::to_string(s) + ".wav");
}
