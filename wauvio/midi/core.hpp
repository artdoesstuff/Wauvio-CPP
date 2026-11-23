#pragma once

// MIDI file model without the instrument / genre / render layers: byte reader, event types, parser and tempo map.
// This is all the vocal engines need to read a .mid score.
#include "../core/common.hpp"
#include "core/byte_reader.hpp"
#include "core/midi_types.hpp"
#include "core/midi_parser.hpp"
#include "core/tempo_map.hpp"
