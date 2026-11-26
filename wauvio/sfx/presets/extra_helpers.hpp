#pragma once
#include "preset_helpers.hpp"

namespace wauvio {
    namespace sfx {
        namespace presets {
            // Pitch that steps through `notes`, one note per equal slice of the layer (MIDI values, held until the next slice).
            inline Traj stair(const std::vector<double>& n) {
                Traj t(n.at(0));
                for (size_t i = 1; i < n.size(); ++i) t.to(static_cast<double>(i) / static_cast<double>(n.size()), n[i], Interp::Step);
                return t;
            }
            // A short decaying tone placed at `start` seconds, as its own layer. Returns the layer so a chain can be added.
            inline LayerSpec& tone_at(Recipe& r, const std::string& name, Wave w, double midi, double start, double len, double level, double attack = 0.01) {
                return r.add(name).osc(w, midi).shape(env_hit(attack, 0.002)).at(start).len(len).gain(level).fade(0.3, 8);
            }
            // The same pitch sequence as separate notes `step` seconds apart (arpeggios, fanfares, chimes).
            inline void note_seq(Recipe& r, const std::string& prefix, Wave w, const std::vector<double>& notes, double start, double step, double len, double level, double pw = 0.5) {
                for (size_t i = 0; i < notes.size(); ++i) {
                    LayerSpec& l = tone_at(r, prefix + std::to_string(i), w, notes[i], start + step * static_cast<double>(i), len, level);
                    if (w == Wave::Pulse) l.pw(pw);
                }
            }
        }
    }
}
