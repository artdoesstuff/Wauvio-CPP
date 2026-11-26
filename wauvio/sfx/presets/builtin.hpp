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
#include "nature.hpp"
#include "scifi.hpp"
#include "game.hpp"
#include "foley.hpp"
#include "cinematic.hpp"
#include "action.hpp"
#include "machines.hpp"
#include "magic.hpp"
#include "cartoon.hpp"
#include "ambience.hpp"

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
            presets::register_nature(R);
            presets::register_scifi(R);
            presets::register_game(R);
            presets::register_foley(R);
            presets::register_cinematic(R);
            presets::register_action(R);
            presets::register_machines(R);
            presets::register_magic(R);
            presets::register_cartoon(R);
            presets::register_ambience(R);
        }

        inline const Registry& builtin_registry() {
            static const Registry reg = [] { Registry r; register_builtin_presets(r); return r; }();
            return reg;
        }
    }
}
