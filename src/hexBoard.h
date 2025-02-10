#pragma once
#include <array>
#include <stdint.h>
#include <map>
#include "hexagon.h"

enum class App_state {  // OLED             LED         Audio        Keys       Knob
  setup,                // splash/status    off         off          off        off
  play_mode,            // full GUI         musical     active       play       control, hold for menu
  menu_nav,             // menu             musical     active       play       menu, hold at home page to escape
  edit_mode,            // menu->edit mode  edit mode   off          selection  menu
  calibrate,            // TBD
  data_mgmt,            // TBD
  crash,                // display error    off         off          off        TBD
  low_power,            // off              off         off          off        off, except hold to awake
};

/* serialize members with a !! */
struct Button {
/*!!*/  bool isUsed = false;  // is it a button or a hardwired circuit
/*!!*/  int8_t  atMux  = -1;
/*!!*/  int8_t  atCol  = -1;

/*!!*/  bool isBtn  = false; // is it a button as opposed to a hardwired circuit
/*!!*/  Hex  coord  = {0,0}; // physical location

/*!!*/  bool isNote = false; // is it a music note object
/*!!*/  bool isCmd  = false;  // is it a command button
/*!!*/  int16_t  pixel  = -1; // the button's index in the grid, which is equal to its associated pixel number if it has one

/*!!*/  uint32_t LEDcodeBase = 0;     // for now
/*!!*/  uint32_t LEDcodeAnim = 0;     // calculate it once and store value, to make LED playback snappier 
/*!!*/  uint32_t LEDcodePlay = 0;     // calculate it once and store value, to make LED playback snappier
/*!!*/  uint32_t LEDcodeRest = 0;     // calculate it once and store value, to make LED playback snappier
/*!!*/  uint32_t LEDcodeOff  = 0;      // calculate it once and store value, to make LED playback snappier
/*!!*/  uint32_t LEDcodeDim  = 0;      // calculate it once and store value, to make LED playback snappier
                            // gradient rule, to be added
/*!!*/  double   frequency = 0.0; // equivalent pitch in Hz
/*!!*/  uint8_t  midiCh = 0;      // what channel assigned (if not MPE mode)   [1..16]
/*!!*/  uint8_t  midiTuningTable = 255; // assigned MIDI note (if MTS mode) [0..127]
/*!!*/  int8_t   scaleEquave = 0;
/*!!*/  uint8_t  scaleDegree = 0;     // order in scale relative to equave
/*!!*/  bool     inScale = false; // for scale-lock purposes
/*!!*/  uint8_t  cmd = 0;  // control parameter corresponding to this hex

  uint8_t midiNote = 0;    // nearest MIDI pitch, 0 to 128
  int16_t midiBend = 0;    // pitch bend for MPE purposes

  uint64_t timeLastUpdate = 0; // store time that key level was last updated
  uint8_t  lastKnownLevel = 0; // press level currently
  uint64_t timePressBegan = 0; // store time that full press occurred
  uint64_t velocityData = 0; // proxy for velocity
  bool just_pressed_flag = false;
  bool just_released_flag = false;

  int8_t   animate = 0; // store value to track animations

  uint8_t  midiChPlaying = 0;          // what midi channel is there currrently a note-on
  uint8_t  synthChPlaying = 0;         // what synth channel is there currrently a note-on

  void update_levels(uint64_t& timestamp, uint8_t& level) {
    if (lastKnownLevel == level) return;
    timeLastUpdate = timestamp;
    if (level == 0) {
      just_released_flag = true;
    } else if (level >= 127) {
      just_pressed_flag = true;
      velocityData = timeLastUpdate - timePressBegan;
      // need a velocity curve, eventually. this is ignored in v1.2.
      timePressBegan = 0;
    } else if (timePressBegan == 0) {
      timePressBegan = timeLastUpdate;
    }
    lastKnownLevel = level;
  }
  bool just_pressed() {
    bool result = just_pressed_flag;
    just_pressed_flag = false;
    return result;
  }
  bool just_released() {
    bool result = just_released_flag;
    just_released_flag = false;
    return result;
  }
};



// write to synth midi queue
// send midi out
/*
    start with freq
    freq *= 2^(pitchbend * pitchbendsemis/(12*8192))
    freq *= poll _interval/1000000
    uint32_t interval = lround(ldexp(freq, 32))

      uint8_t iso226(float f) {
        // a very crude implementation of ISO 226
        // equal loudness curves
        //   Hz dB  Amp ~ sqrt(10^(dB/10))
        //  200  0  8
        //  800 -3  6   
        // 1500  0  8
        // 3250 -6  4
        // 5000  0  8
        if (f <      8.0) return 0;
        if (f <    200.0) return 8;
        if (f <   1500.0) return 6 + 2 * (float)(abs(f- 800) /  700);
        if (f <   5000.0) return 4 + 4 * (float)(abs(f-3250) / 1750);
        if (f < highest_MIDI_note_in_Hz) return 8;
        return 0;
      }

    // hybrid algorithm
    struct Hybrid_Oscillator {
      // 255 |     B/--\C Hybrid waveforms are defined
      //     |     /    \   by the location of points
      //     +----------------- A, B, C, and D
      //     |   /        \ 
      //   0 |--/A        D\---
      uint8_t a;
      uint8_t b;
      uint8_t c;
      uint16_t ab;
      uint16_t cd;
      const float hybrid_square   =  220.0;
      const float hybrid_saw_low  =  440.0;
      const float hybrid_saw_high =  880.0;
      const float hybrid_triangle = 1760.0;
      void update_freq(float f) {
        if (f > hybrid_square) {
          b = 128;
        } else if (f < hybrid_saw_low) {
          b = 128 + (uint8_t)(127 * (f - hybrid_square) / (hybrid_saw_low - hybrid_square));
        } else if (f < hybrid_saw_high) {
          b = 255;
        } else if (f < hybrid_triangle) {
          b = 127 + (uint8_t)(128 * (hybrid_triangle - f) / (hybrid_triangle - hybrid_saw_high));
        } else {
          b = 127;
        }
        if (f < hybrid_saw_low) {
          a = 255 - b;
          c = 255;
        } else {
          a = 0;
          c = b;
        }
        ab = 65535;
        if (a < 127) {
          ab /= (b - a - 1);
        }
        cd = 65535 / (256 - c);
      }
    }
*/


struct hexBoard_Grid_Object {
  std::array<Button, keys_count> btn;
  std::map<Hex, int16_t>    coord_to_pixel;

  hexBoard_Grid_Object(const int16_t layout[keys_count][_layout_table_size]) {
    // first count how many rows represent usable inputs,
    // how many of these are buttons vs. hard switches,
    // and the largest pixel number used in the grid.
    // buttons will be numbered by their pixel ID, with
    // hard switches following, and the remaining
    // unused inputs initialized empty at the back of the array
    size_t hardwireIndex = ledCount;
    for (size_t i = 0; i < keys_count; ++i) {
      switch (layout[i][_layout_table_switch_type]) {
        case hex_button: {
          int16_t p =  layout[i][_layout_table_pixel_number];
          Hex h = {layout[i][_layout_table_coord_x],
                   layout[i][_layout_table_coord_y]};
          coord_to_pixel[h] = p;
          btn[p].coord  = h;
          btn[p].pixel  = p;
          btn[p].isUsed = true;
          btn[p].isBtn  = true;
          btn[p].atMux  = layout[i][_layout_table_multiplex_value];
          btn[p].atCol  = layout[i][_layout_table_column_pin];
          break;
        }
        case hardwired: {
          btn[hardwireIndex].isUsed = true;
          btn[hardwireIndex].atMux  = layout[i][_layout_table_multiplex_value];
          btn[hardwireIndex].atCol  = layout[i][_layout_table_column_pin];
          ++hardwireIndex;
          break;
        }
        default:
          break;
      }
    }
  }

  Button& button_at_coord(Hex& coord) {
    return btn[coord_to_pixel.at(coord)];
  }

  bool in_bounds(Hex& coord) {
    return (coord_to_pixel.find(coord) != coord_to_pixel.end());
  }

  void on_key_press(Button* b) {
    Synth_Msg* msg = &synth_msg_from_core_0;
    Synth_Generation* gen = &(msg->synth_generation);
    switch (app_state) {
      case App_state::play_mode: {
        msg->command = synth_msg_note_on;
        b->synthChPlaying = 0; // find next on channel queue 
        msg->item_number = b->synthChPlaying;

        msg->command += update_pitch;
        // calculate from b->pitchInfo
        gen->pitch_as_increment = 1;
        msg->command += update_volume;
        // calculate velocity, multiply together with other info to get volume
        gen->base_volume = 127;
        msg->command += update_waveform;
        // determine from settings
        // and if it's hybrid then make a function of F
        gen->waveform_ID = 255;
        // gen->envelope
        // gen->hybrid_params
        // gen->harmonics[0], [1], etc.

        queue_add_blocking(&synth_msg_queue, msg);
        break;
      }
      default: break;
    }
  }
  
  void on_key_release(Button* b) {
    
  }

  void on_key_hold(Button* b) {
    
  }

  void interpret_key_msg(Key_Msg& msg) {
    Button* thisBtn = &btn[msg.switch_number];
    thisBtn->update_levels(msg.timestamp, msg.level);
    if (!thisBtn->isBtn) {
      // take action on dipswitch, e.g. button140
      return;
    } else if (thisBtn->just_pressed()) {
      on_key_press(thisBtn);
    } else if (thisBtn->just_released()) {
      on_key_release(thisBtn);
    } else {
      on_key_hold(thisBtn);
    }
  }

};