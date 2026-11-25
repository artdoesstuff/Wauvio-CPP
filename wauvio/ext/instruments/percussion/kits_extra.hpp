#pragma once
#include "../../core/model_instruments.hpp"
#include "kits.hpp"
#include "percussion_extra.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        class TrapKit : public DrumKit {
        public:
            TrapKit() : DrumKit("Trap Kit") {
                auto kick = std::make_shared<Kick>();
                kick->recipe.fixed_pitch_hz = 42.0; kick->recipe.envelope.decay = 0.9; kick->recipe.pitch_attack_cents = -1500.0;
                map(36, kick);
                auto snare = std::make_shared<Snare>(); snare->recipe.filter_cutoff = 5200.0; map(38, snare);
                auto clap = std::make_shared<Snare>(); clap->recipe.filter_cutoff = 7000.0; clap->recipe.noise_mix = 0.95; clap->recipe.envelope.decay = 0.18; map(39, clap);
                map(42, std::make_shared<ClosedHiHat>());
                map(46, std::make_shared<OpenHiHat>());
                map(49, std::make_shared<CrashCymbal>());
                map(56, std::make_shared<Cowbell>());
            }
        };

        class LatinKit : public DrumKit {
        public:
            LatinKit() : DrumKit("Latin Kit") {
                map(36, std::make_shared<Kick>());
                map(38, std::make_shared<Rimshot>());
                map(42, std::make_shared<ClosedHiHat>());
                map(46, std::make_shared<OpenHiHat>());
                map(49, std::make_shared<CrashCymbal>());
                map(56, std::make_shared<Cowbell>());
                map(60, std::make_shared<Bongo>()); map(61, std::make_shared<Bongo>());
                map(62, std::make_shared<Conga>()); map(63, std::make_shared<Conga>()); map(64, std::make_shared<Conga>());
                map(65, std::make_shared<Timbales>()); map(66, std::make_shared<Timbales>());
                map(67, std::make_shared<Agogo>()); map(68, std::make_shared<Agogo>());
                map(69, std::make_shared<Cabasa>()); map(70, std::make_shared<Maracas>());
                map(73, std::make_shared<Guiro>()); map(74, std::make_shared<Guiro>());
                map(75, std::make_shared<Claves>());
            }
        };

        class SambaKit : public DrumKit {
        public:
            SambaKit() : DrumKit("Samba Kit") {
                map(36, std::make_shared<Surdo>());
                map(38, std::make_shared<Repinique>());
                map(40, std::make_shared<Pandeiro>());
                map(54, std::make_shared<Pandeiro>());
                map(60, std::make_shared<Bongo>());
                map(67, std::make_shared<Agogo>()); map(68, std::make_shared<Agogo>());
                map(69, std::make_shared<Cabasa>()); map(70, std::make_shared<Maracas>());
                map(73, std::make_shared<Guiro>()); map(75, std::make_shared<Claves>());
                map(78, std::make_shared<Cuica>()); map(79, std::make_shared<Cuica>());
            }
        };

        class FolkKit : public DrumKit {
        public:
            FolkKit() : DrumKit("Folk Kit") {
                map(36, std::make_shared<Bodhran>());
                auto rim = std::make_shared<Bodhran>(); rim->recipe.fixed_pitch_hz = 150.0; rim->recipe.filter_cutoff = 2400.0; map(38, rim);
                map(42, std::make_shared<Shaker>());
                map(46, std::make_shared<Shaker>());
                map(54, std::make_shared<Tambourine>());
                map(49, std::make_shared<FingerCymbals>());
                map(56, std::make_shared<Woodblock>());
            }
        };

        class IndianKit : public DrumKit {
        public:
            IndianKit() : DrumKit("Indian Percussion Kit") {
                map(36, std::make_shared<Dhol>());
                map(38, std::make_shared<Dholak>());
                map(40, std::make_shared<Mridangam>());
                map(41, std::make_shared<Ghatam>());
                map(42, std::make_shared<Kanjira>());
                map(60, std::make_shared<Tabla>()); map(61, std::make_shared<Tabla>());
                map(54, std::make_shared<Daf>());
                map(69, std::make_shared<Cabasa>());
            }
        };

        class AfroKit : public DrumKit {
        public:
            AfroKit() : DrumKit("Afro Kit") {
                map(36, std::make_shared<Udu>());
                map(38, std::make_shared<Djembe>());
                map(42, std::make_shared<ClosedHiHat>());
                map(56, std::make_shared<Cowbell>());
                map(62, std::make_shared<Conga>()); map(63, std::make_shared<Conga>()); map(64, std::make_shared<Conga>());
                map(67, std::make_shared<Agogo>()); map(68, std::make_shared<Agogo>());
                map(70, std::make_shared<Shaker>());
                map(75, std::make_shared<Claves>());
            }
        };

        class MarchingKit : public DrumKit {
        public:
            MarchingKit() : DrumKit("Marching Kit") {
                map(36, std::make_shared<ConcertBassDrum>());
                map(38, std::make_shared<MarchingSnare>());
                map(40, std::make_shared<FieldDrum>());
                map(41, std::make_shared<TenorDrum>()); map(43, std::make_shared<TenorDrum>()); map(45, std::make_shared<TenorDrum>());
                map(49, std::make_shared<CrashCymbal>());
                map(51, std::make_shared<SuspendedCymbal>());
            }
        };

    }
}
