#pragma once

// Default vocal engine: the latest Yune generation, reachable as wauvio::vocal::current.
// Include vocal/v1.hpp, v2.hpp, v3.hpp, or v4.hpp directly to use a specific generation; they can coexist in one program.
#include "vocal/v3.hpp"

namespace wauvio {
    namespace vocal {
        namespace current = v3;
    }
}
