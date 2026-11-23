// =============================================================================
//  W A U V I O . H P P  -  Advanced Music Synthesis Engine  v2.0.0
//  Header-only C++ music synthesis library
//
//  Refer to https://github.com/artdoesstuff-ex/Wauvio-Graveyard if examples are needed.
//
//  This is the umbrella for the core engine. Each part lives in wauvio/core/ and
//  can be included on its own; each owns one namespace:
//
//    core/common.hpp    wauvio::           constants, Config, Buffer/StereoBuffer, buffer + panning helpers
//    core/osc.hpp       wauvio::osc        waveforms (wave, wave_bl), WaveShape, Oscillator, Supersaw,
//                                          WavetableOscillator, NoiseGenerator
//    core/env.hpp       wauvio::env        ADSR, DAHDSR, LoopableEnvelope
//    core/mod.hpp       wauvio::mod        LFO, automation curves (AutoPoint, auto_eval, apply_*_automation)
//    core/filter.hpp    wauvio::filter     SVFilter, Low/HighPassFilter, BiquadEQ, ParametricEQ
//    core/synth.hpp     wauvio::synth      FMSynth, Voice, VoiceManager
//    core/fx_basic.hpp  wauvio::fx::basic  distortion, DelayLine, Reverb, Chorus, Flanger, Phaser,
//                                          SidechainCompressor   (the full effects suite is wauvio/fx.hpp)
//    core/theory.hpp    wauvio::theory     notes, midi_to_freq
//    core/seq.hpp       wauvio::seq        Note, Instrument, Track, DrumSequencer, MasterBus
//    core/io.hpp        wauvio::           save_wav / save_wav_stereo, play / play_async, PlaybackHandle
//
//  Rich instrument / arrangement types live in wauvio::music (wauvio/ext.hpp), not in wauvio::seq.
//
//  Compile (add -I<repo root> so both "wauvio.hpp" and "wauvio/..." resolve):
//    Linux  : g++ -std=c++17 -O2 your_program.cpp -o your_program
//    Windows: cl /std:c++17 /O2 your_program.cpp /link winmm.lib
// =============================================================================

#pragma once

#include "wauvio/core/common.hpp"
#include "wauvio/core/osc.hpp"
#include "wauvio/core/env.hpp"
#include "wauvio/core/filter.hpp"
#include "wauvio/core/mod.hpp"
#include "wauvio/core/synth.hpp"
#include "wauvio/core/fx_basic.hpp"
#include "wauvio/core/theory.hpp"
#include "wauvio/core/seq.hpp"
#include "wauvio/core/io.hpp"
