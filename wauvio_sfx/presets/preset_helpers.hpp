#pragma once
#include "registry.hpp"
#include "../core/sfx_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            using Rc = Recipe&;
            using Cc = const BuildCtx&;
            inline Behavior OneShot() { return Behavior::OneShot; }
            inline Behavior Evolving() { return Behavior::Evolving; }
            inline Behavior Sustained() { return Behavior::Sustained; }
        }
    }
}