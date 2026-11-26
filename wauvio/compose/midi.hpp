#pragma once

#include "../compose.hpp"
#include "../midi.hpp"

namespace wauvio {
    namespace compose {

        inline void save_midi(const ComposedSong& song, const std::string& path, int ppq = 480) {
            using namespace wauvio::midi::track::writer_detail;
            const double bpm = song.config.bpm > 0.0 ? song.config.bpm : 120.0;
            const double ticks_per_sec = ppq * bpm / 60.0;
            const uint32_t mpq = static_cast<uint32_t>(std::llround(60000000.0 / bpm));

            std::vector<std::vector<uint8_t>> chunks;
            MidiTrackBuilder tempo_track;
            tempo_track.add_tempo(0, mpq);
            chunks.push_back(tempo_track.serialize());

            const int chan_for[8] = { 0, 1, 2, 3, 9, 9, 9, 9 };
            for (size_t ti = 0; ti < song.arrangement.track_count(); ++ti) {
                const wauvio::music::Track& t = song.arrangement.track_at(ti);
                const int ch = chan_for[ti < 8 ? ti : 0];
                MidiTrackBuilder tb;
                if (!t.name.empty()) tb.add_track_name(0, t.name);
                if (ch != 9) {
                    int gm = (ch == 0) ? 0 : (ch == 1) ? 4 : (ch == 2) ? 33 : 48;
                    tb.add_program_change(0, ch, gm);
                }

                auto onsets = t.entry_onsets();
                double base_offset = song.arrangement.start_time_at(ti);
                for (size_t ei = 0; ei < t.entry_count(); ++ei) {
                    double onset_sec = base_offset + onsets[ei];
                    uint64_t onset_tick = static_cast<uint64_t>(std::llround(onset_sec * ticks_per_sec));
                    if (t.entry_is_chord(ei)) {
                        const auto& c = t.entry_chord(ei);
                        uint64_t end_tick = static_cast<uint64_t>(std::llround((onset_sec + c.duration) * ticks_per_sec));
                        int vel = static_cast<int>(std::lround(wauvio::music::dynamics_to_velocity(c.dynamics) * 127.0));
                        for (int n : c.midi_notes) tb.add_note_on(onset_tick, ch, n, vel);
                        for (int n : c.midi_notes) tb.add_note_off(end_tick, ch, n);
                    } else {
                        const auto& n = t.entry_note(ei);
                        if (n.is_rest()) continue;
                        uint64_t end_tick = static_cast<uint64_t>(std::llround((onset_sec + n.duration) * ticks_per_sec));
                        int vel = static_cast<int>(std::lround(wauvio::music::dynamics_to_velocity(n.dynamics) * 127.0));
                        tb.add_note_on(onset_tick, ch, n.midi_note, vel);
                        tb.add_note_off(end_tick, ch, n.midi_note);
                    }
                }
                chunks.push_back(tb.serialize());
            }

            write_file(path, 1, static_cast<uint16_t>(chunks.size()), static_cast<uint16_t>(ppq), chunks);
        }

    }
}
