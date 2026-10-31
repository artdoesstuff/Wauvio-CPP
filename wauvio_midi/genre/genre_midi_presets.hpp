#pragma once

#include "genre_midi.hpp"
#include "../../wauvio_ext/genre/genre_presets.hpp"
#include "../../wauvio_ext/instruments/percussion/kits.hpp"
#include "../../wauvio_ext/instruments/synthesized/synths.hpp"
#include "../../wauvio_ext/instruments/guitars/guitars.hpp"

namespace wauvio {
    namespace midi {
        namespace genre {
            namespace presets {

                inline MidiGenrePresetPtr classical() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::classical());
                    p->percussion_kit = [] { return std::make_shared<instruments::OrchestralPercussionKit>(); };
                    return p;
                }

                inline MidiGenrePresetPtr jazz() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::jazz());
                    p->percussion_kit = [] { return std::make_shared<instruments::JazzKit>(); };
                    return p;
                }

                inline MidiGenrePresetPtr rock() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::rock());
                    p->percussion_kit = [] { return std::make_shared<instruments::RockKit>(); };
                    p->melodic_program_overrides[26] = [] { return std::make_shared<instruments::OverdrivenElectricGuitar>(); };
                    p->melodic_program_overrides[27] = [] { return std::make_shared<instruments::OverdrivenElectricGuitar>(); };
                    p->melodic_program_overrides[81] = [] { return std::make_shared<instruments::SynthLead>(); };
                    return p;
                }

                inline MidiGenrePresetPtr metal() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::metal());
                    p->percussion_kit = [] { return std::make_shared<instruments::IndustrialKit>(); };
                    p->melodic_program_overrides[26] = [] { return std::make_shared<instruments::DistortedElectricGuitar>(); };
                    p->melodic_program_overrides[27] = [] { return std::make_shared<instruments::DistortedElectricGuitar>(); };
                    p->melodic_program_overrides[29] = [] { return std::make_shared<instruments::DistortedElectricGuitar>(); };
                    return p;
                }

                inline MidiGenrePresetPtr house() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::house());
                    p->percussion_kit = [] { return std::make_shared<instruments::Kit909>(); };
                    p->melodic_program_overrides[38] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[39] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[80] = [] { return std::make_shared<instruments::SynthLead>(); };
                    p->melodic_program_overrides[89] = [] { return std::make_shared<instruments::SynthPad>(); };
                    return p;
                }

                inline MidiGenrePresetPtr techno() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::techno());
                    p->percussion_kit = [] { return std::make_shared<instruments::Kit808>(); };
                    p->melodic_program_overrides[38] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[39] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[80] = [] { return std::make_shared<instruments::SynthLead>(); };
                    p->melodic_program_overrides[89] = [] { return std::make_shared<instruments::AtmosphericSynth>(); };
                    return p;
                }

                inline MidiGenrePresetPtr drum_and_bass() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::drum_and_bass());
                    p->percussion_kit = [] { return std::make_shared<instruments::ElectronicKit>(); };
                    p->melodic_program_overrides[33] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[38] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[39] = [] { return std::make_shared<instruments::SynthBass>(); };
                    return p;
                }

                inline MidiGenrePresetPtr chiptune() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::chiptune());
                    p->percussion_kit = [] { return std::make_shared<instruments::LoFiKit>(); };
                    for (int prog : {0, 1, 2, 40, 41, 56, 65, 73, 80, 81, 84, 86})
                        p->melodic_program_overrides[prog] = [] { return std::make_shared<instruments::ChiptuneLead>(); };
                    for (int prog : {32, 33, 34, 35, 36, 37, 38, 39})
                        p->melodic_program_overrides[prog] = [] { return std::make_shared<instruments::ChiptuneBass>(); };
                    return p;
                }

                inline MidiGenrePresetPtr ambient() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::ambient());
                    p->melodic_program_overrides[89] = [] { return std::make_shared<instruments::AtmosphericSynth>(); };
                    p->melodic_program_overrides[88] = [] { return std::make_shared<instruments::AtmosphericSynth>(); };
                    p->melodic_program_overrides[95] = [] { return std::make_shared<instruments::Drone>(); };
                    return p;
                }

                inline MidiGenrePresetPtr touhou() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::touhou());
                    p->percussion_kit = [] { return std::make_shared<instruments::RockKit>(); };
                    p->melodic_program_overrides[26] = [] { return std::make_shared<instruments::OverdrivenElectricGuitar>(); };
                    p->melodic_program_overrides[81] = [] { return std::make_shared<instruments::SynthLead>(); };
                    return p;
                }

                inline std::shared_ptr<audio::DrumKit> bossa_nova_kit() {
                    auto kit = std::make_shared<audio::DrumKit>("Bossa Nova Kit");
                    kit->map(36, std::make_shared<instruments::Surdo>());
                    kit->map(38, std::make_shared<instruments::Claves>());
                    kit->map(40, std::make_shared<instruments::Rimshot>());
                    kit->map(42, std::make_shared<instruments::ClosedHiHat>());
                    kit->map(46, std::make_shared<instruments::OpenHiHat>());
                    kit->map(56, std::make_shared<instruments::Cowbell>());
                    kit->map(60, std::make_shared<instruments::Bongo>());
                    kit->map(61, std::make_shared<instruments::Bongo>());
                    kit->map(62, std::make_shared<instruments::Conga>());
                    kit->map(63, std::make_shared<instruments::Conga>());
                    kit->map(64, std::make_shared<instruments::Conga>());
                    kit->map(65, std::make_shared<instruments::Timbales>());
                    kit->map(66, std::make_shared<instruments::Timbales>());
                    kit->map(69, std::make_shared<instruments::Cabasa>());
                    kit->map(70, std::make_shared<instruments::Maracas>());
                    kit->map(73, std::make_shared<instruments::Guiro>());
                    kit->map(74, std::make_shared<instruments::Guiro>());
                    return kit;
                }

                inline MidiGenrePresetPtr funk() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::funk());
                    p->percussion_kit = [] { return std::make_shared<instruments::StudioKit>(); };
                    return p;
                }

                inline MidiGenrePresetPtr blues() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::blues());
                    p->percussion_kit = [] { return std::make_shared<instruments::StudioKit>(); };
                    p->melodic_program_overrides[26] = [] { return std::make_shared<instruments::OverdrivenElectricGuitar>(); };
                    p->melodic_program_overrides[27] = [] { return std::make_shared<instruments::OverdrivenElectricGuitar>(); };
                    return p;
                }

                inline MidiGenrePresetPtr reggae() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::reggae());
                    p->percussion_kit = [] { return std::make_shared<instruments::JazzKit>(); };
                    p->melodic_program_overrides[27] = [] { return std::make_shared<instruments::CleanElectricGuitar>(); };
                    return p;
                }

                inline MidiGenrePresetPtr trance() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::trance());
                    p->percussion_kit = [] { return std::make_shared<instruments::Kit909>(); };
                    p->melodic_program_overrides[38] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[39] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[80] = [] { return std::make_shared<instruments::SynthLead>(); };
                    p->melodic_program_overrides[89] = [] { return std::make_shared<instruments::AtmosphericSynth>(); };
                    return p;
                }

                inline MidiGenrePresetPtr dubstep() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::dubstep());
                    p->percussion_kit = [] { return std::make_shared<instruments::ElectronicKit>(); };
                    p->melodic_program_overrides[33] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[38] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[39] = [] { return std::make_shared<instruments::SynthBass>(); };
                    return p;
                }

                inline MidiGenrePresetPtr bossa_nova() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::bossa_nova());
                    p->percussion_kit = [] { return bossa_nova_kit(); };
                    p->melodic_program_overrides[24] = [] { return std::make_shared<instruments::NylonGuitar>(); };
                    p->melodic_program_overrides[25] = [] { return std::make_shared<instruments::NylonGuitar>(); };
                    p->melodic_program_overrides[26] = [] { return std::make_shared<instruments::NylonGuitar>(); };
                    p->melodic_program_overrides[27] = [] { return std::make_shared<instruments::NylonGuitar>(); };
                    return p;
                }

                inline MidiGenrePresetPtr hardcore() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::hardcore());
                    p->percussion_kit = [] { return std::make_shared<instruments::Kit909>(); };
                    p->melodic_program_overrides[38] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[39] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[80] = [] { return std::make_shared<instruments::SynthLead>(); };
                    return p;
                }

                inline MidiGenrePresetPtr speedcore() {
                    auto p = std::make_shared<MidiGenrePreset>(wauvio::genre::presets::speedcore());
                    p->percussion_kit = [] { return std::make_shared<instruments::Kit808>(); };
                    p->melodic_program_overrides[38] = [] { return std::make_shared<instruments::SynthBass>(); };
                    p->melodic_program_overrides[39] = [] { return std::make_shared<instruments::SynthBass>(); };
                    return p;
                }

            }
        }
    }
}