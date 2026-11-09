#pragma once
#include "risers.hpp"
#include "impacts.hpp"
#include "glitch.hpp"
#include "textures.hpp"
#include "bass.hpp"
#include "candy.hpp"
#include "vocal.hpp"
#include "rhythmic.hpp"
#include "percussion.hpp"
#include "experimental.hpp"
#include "tonal.hpp"
#include "reverses.hpp"

namespace wauvio {
    namespace sfx {
        inline void register_builtin_presets(Registry& R) {
            presets::register_risers(R);
            presets::register_impacts(R);
            presets::register_glitch(R);
            presets::register_textures(R);
            presets::register_bass(R);
            presets::register_candy(R);
            presets::register_vocal(R);
            presets::register_rhythmic(R);
            presets::register_percussion(R);
            presets::register_experimental(R);
            presets::register_tonal(R);
            presets::register_reverses(R);
        }

        inline const Registry& builtin_registry() {
            static const Registry reg = [] { Registry r; register_builtin_presets(r); return r; }();
            return reg;
        }
    }
}
