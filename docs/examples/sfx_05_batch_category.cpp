// Render every preset in a category (like `wauvio_sfx_tool render-category risers`).
#include "wauvio/sfx.hpp"
#include <cstdio>
#include <cstdlib>
namespace sfx = wauvio::sfx;

int main() {
    std::system("mkdir -p sfx_risers");
    for (const sfx::Preset* p : sfx::presets_in(sfx::Category::Risers)) {
        try {
            sfx::make(p->id).render().save("sfx_risers/" + p->id + ".wav");
            std::printf("ok   %s\n", p->id.c_str());
        } catch (const sfx::Error& e) {
            std::printf("FAIL %s: %s\n", p->id.c_str(), e.what());
        }
    }
}
