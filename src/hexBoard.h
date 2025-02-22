#pragma once
#include <array>
#include <stdint.h>
#include <map>
#include "settings.h"
#include "hexagon.h"
#include "music.h"

struct Button {
  // basic identification
  bool      isUsed = false;  // is it a button or a hardwired circuit
  int8_t    atMux  = -1;
  int8_t    atCol  = -1;
  bool      isBtn  = false;  // is it a button not a hardwired circuit
  Hex       coord  = {0,0};  // physical location
  int16_t   pixel  = -1;     // the button's index in the grid, which is equal to its associated pixel number if it has one
  bool      isNote = false;  // is it a music note object
  bool      isCmd  = false;  // is it a command button

  // layout and scale variables
  int8_t    A_steps = 0;      // cached number of steps along A axis from anchor
  int8_t    B_steps = 0;      // cached number of steps along B axis from anchor
  bool      inScale = false;  // for scale-lock purposes
  int8_t    scaleEquave = 0;  // not used for JI lattice
  int8_t    scaleDegree = 0;  // for 1-dimension
  int8_t    smallDegree = 0;  // # of small steps (microtonal / MOS)
  int8_t    largeDegree = 0;  // # of large steps (microtonal / MOS)

  // MIDI and pitch assignment
  uint8_t   midiCh = 0;      // what channel assigned (if not MPE mode)   [1..16]
  uint8_t   midiTuningTable = 255; // assigned MIDI note (if MTS mode) [0..127]
  double    midiPitch = 0.0; // pitch, 69 = A440, every 1.0 is 100.0 cents
  double    frequency = 0.0; // equivalent of pitch in Hz
  uint8_t   cmd = 0;  // control parameter corresponding to this hex

  // palette and cached LED codes
  int8_t    paletteNum = 0;  // used for tiered key coloring (all except JI)
  uint32_t  LEDcodeBase = 0; // calculate it once and store value, to make LED playback snappier 
  uint32_t  LEDcodeAnim = 0; // calculate it once and store value, to make LED playback snappier 
  uint32_t  LEDcodePlay = 0; // calculate it once and store value, to make LED playback snappier
  uint32_t  LEDcodeRest = 0; // calculate it once and store value, to make LED playback snappier
  uint32_t  LEDcodeOff  = 0; // calculate it once and store value, to make LED playback snappier
  uint32_t  LEDcodeDim  = 0; // calculate it once and store value, to make LED playback snappier

  // key press data
  uint32_t  timeLastUpdate = 0; // store time that key level was last updated
  uint32_t  timePressBegan = 0; // store time that full press occurred
  uint32_t  timeHeldSince  = 0;
  uint8_t   pressure       = 0; // press level currently
  uint8_t   velocity       = 0; // proxy for velocity
  bool      just_pressed   = false;
  bool      just_released  = false;

  // music playback status
  uint8_t   midiChPlaying  = 0;          // what midi channel is there currrently a note-on
  uint8_t   midiNote = 0;    // nearest MIDI pitch, 0 to 128
  int16_t   midiBend = 0;    // pitch bend for MPE purposes
  uint8_t   synthChPlaying = 0;         // what synth channel is there currrently a note-on

  // member functions
  void update_levels(uint32_t& timestamp, uint8_t& new_level) {
    if (pressure == new_level) return;
    timeLastUpdate = timestamp;
    if (new_level == 0) {
      just_released = true;
      velocity = 0;
      timeHeldSince = 0;
    } else if (new_level >= 127) {
      just_pressed = true;
      velocity = 127;
      // velocity = function of timeLastUpdate - timePressBegan;
      // need a velocity curve, eventually. this is ignored in v1.2.
      timePressBegan = 0;
      timeHeldSince = timeLastUpdate;
    } else if (timePressBegan == 0) {
      timePressBegan = timeLastUpdate;
    }
    pressure = new_level;
  }

  bool check_and_reset_just_pressed() {
    bool result = just_pressed;
    just_pressed = false;
    return result;
  }
  bool check_and_reset_just_released() {
    bool result = just_released;
    just_released = false;
    return result;
  }
};



void hardwired_switch_handler(int16_t ID);

struct hexBoard_Grid_Object {
  std::array<Button, keys_count> btn;
  std::map<Hex,      int16_t>    coord_to_pixel;
  std::map<uint8_t,  int16_t>    index_to_pixel;
  wave_tbl                       cached_waveform;

  hexBoard_Grid_Object(const int16_t layout[keys_count][_layout_table_size]) {
    // first count how many rows represent usable inputs,
    // how many of these are buttons vs. hard switches,
    // and the largest pixel number used in the grid.
    // buttons will be numbered by their pixel ID, with
    // hard switches following, and the remaining
    // unused inputs initialized empty at the back of the array
    size_t hardwireIndex = ledCount;
    size_t unusedIndex = keys_count - 1;

    for (size_t i = 0; i < keys_count; ++i) {
      size_t L = linear_index(
        layout[i][_layout_table_multiplex_value],
        layout[i][_layout_table_column_pin]
      );      
      switch (layout[i][_layout_table_switch_type]) {
        case hex_button: {
          int16_t p = layout[i][_layout_table_pixel_number];
          Hex h = {layout[i][_layout_table_coord_x],
                   layout[i][_layout_table_coord_y]};
          coord_to_pixel[h] = p;
          index_to_pixel[L] = p;
          Button* b = &btn[p];
          b->coord  = h;
          b->pixel  = p;
          b->isUsed = true;
          b->isBtn  = true;
          b->atMux  = layout[i][_layout_table_multiplex_value];
          b->atCol  = layout[i][_layout_table_column_pin];
          // eventually will load saved values or put a default
          /* placeholder to put note values for testing */
          b->isNote          = true;
          b->LEDcodeBase     = 0x00000000; // for now
          b->LEDcodeAnim     = 0x00000000; // calculate it once and store value, to make LED playback snappier 
          b->LEDcodePlay     = 0x00000000; // calculate it once and store value, to make LED playback snappier
          b->LEDcodeRest     = 0x00000000; // calculate it once and store value, to make LED playback snappier
          b->LEDcodeOff      = 0; // calculate it once and store value, to make LED playback snappier
          b->LEDcodeDim      = 0x00000000; // calculate it once and store value, to make LED playback snappier
          b->frequency       = 10.0 + (b->pixel * 10.0); // equivalent pitch in Hz
          b->midiCh          = 1;      // what channel assigned (if not MPE mode)   [1..16]
          b->midiTuningTable = 255; // assigned MIDI note (if MTS mode) [0..127]
          b->scaleEquave     = 0;
          b->scaleDegree     = 0;     // order in scale relative to equave
          b->inScale         = true; // for scale-lock purposes
          b->cmd             = 0;  // control parameter corresponding to this hex
          b->midiPitch       = 69.0;
          b->midiNote        = 69;
          b->midiBend        = 0;

          break;
        }
        case hardwired: {
          index_to_pixel[L] = hardwireIndex;
          btn[hardwireIndex].isUsed = true;
          btn[hardwireIndex].atMux  = layout[i][_layout_table_multiplex_value];
          btn[hardwireIndex].atCol  = layout[i][_layout_table_column_pin];
          ++hardwireIndex;
          break;
        }
        default:
          index_to_pixel[L] = unusedIndex;
          --unusedIndex;
          break;
      }
    }
  }

  Button& button_at_coord(const Hex& coord) {
    return btn[coord_to_pixel.at(coord)];
  }

  Button& button_at_linear_index(size_t l_index) {
    return btn[index_to_pixel.at(l_index)];
  }

  bool in_bounds(const Hex& coord) {
    return (coord_to_pixel.find(coord) != coord_to_pixel.end());
  }

  void set_cached_wavetable(const wave_tbl& inputWaveTbl) {
    for (uint8_t i = 0; i < 256; ++i) {
      cached_waveform[i] = inputWaveTbl[i];
    }
  }

};