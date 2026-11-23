#pragma once

#include "track/midi_music.hpp"

namespace wauvio {
    inline void play(const midi::track::MidiMusic& music, int sample_rate = 0) {
        StereoBuffer buf = music.render(sample_rate);
        play(buf, sample_rate);
    }

    inline PlaybackHandle play_async(const midi::track::MidiMusic& music, int sample_rate = 0) {
        StereoBuffer buf = music.render(sample_rate);
        return play_async(buf, sample_rate);
    }
}