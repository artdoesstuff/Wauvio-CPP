#pragma once

#include "../compose.hpp"
#include "../fx/ext.hpp"

namespace wauvio {
    namespace compose {

        inline StereoBuffer render_mastered(const ComposedSong& song,
                                            const wauvio::fx::MasterSettings& settings = wauvio::fx::MasterSettings::for_preset(wauvio::fx::MasterPreset::Clean),
                                            wauvio::fx::MasterReport* report_out = nullptr,
                                            const RenderOptions& opts = RenderOptions())
        {
            auto mix = wauvio::fx::mixer_from_arrangement(song.arrangement, opts);
            mix.set_mastering(settings);
            StereoBuffer out = mix.render();
            if (report_out) *report_out = mix.last_master_report();
            return out;
        }

        inline void save_mastered(const ComposedSong& song, const std::string& wav_path,
                                 const wauvio::fx::MasterSettings& settings = wauvio::fx::MasterSettings::for_preset(wauvio::fx::MasterPreset::Clean),
                                 const RenderOptions& opts = RenderOptions())
        {
            StereoBuffer out = render_mastered(song, settings, nullptr, opts);
            wauvio::fx::write_wav(out, wav_path, opts);
        }

    }
}
