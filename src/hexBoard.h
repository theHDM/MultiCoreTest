#pragma once
#include <array>
#include <stdint.h>
#include <map>
#include "hexagon.h"
#include "music.h"

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
/*!!*/  bool     isUsed = false;  // is it a button or a hardwired circuit
/*!!*/  int8_t   atMux  = -1;
/*!!*/  int8_t   atCol  = -1;
/*!!*/  bool     isBtn  = false;  // is it a button not a hardwired circuit
/*!!*/  Hex      coord  = {0,0};  // physical location
/*!!*/  bool     isNote = false;  // is it a music note object
/*!!*/  bool     isCmd  = false;  // is it a command button
/*!!*/  int16_t  pixel  = -1;     // the button's index in the grid, which is equal to its associated pixel number if it has one

/*!!*/  uint32_t LEDcodeBase = 0; // for now
/*!!*/  uint32_t LEDcodeAnim = 0; // calculate it once and store value, to make LED playback snappier 
/*!!*/  uint32_t LEDcodePlay = 0; // calculate it once and store value, to make LED playback snappier
/*!!*/  uint32_t LEDcodeRest = 0; // calculate it once and store value, to make LED playback snappier
/*!!*/  uint32_t LEDcodeOff  = 0; // calculate it once and store value, to make LED playback snappier
/*!!*/  uint32_t LEDcodeDim  = 0; // calculate it once and store value, to make LED playback snappier
/*!!*/  double   frequency = 0.0; // equivalent pitch in Hz
/*!!*/  uint8_t  midiCh = 0;      // what channel assigned (if not MPE mode)   [1..16]
/*!!*/  uint8_t  midiTuningTable = 255; // assigned MIDI note (if MTS mode) [0..127]
/*!!*/  int8_t   scaleEquave = 0;
/*!!*/  uint8_t  scaleDegree = 0;     // order in scale relative to equave
/*!!*/  bool     inScale = false; // for scale-lock purposes
/*!!*/  uint8_t  cmd = 0;  // control parameter corresponding to this hex

  uint8_t  midiNote = 0;    // nearest MIDI pitch, 0 to 128
  int16_t  midiBend = 0;    // pitch bend for MPE purposes
  uint64_t timeLastUpdate = 0; // store time that key level was last updated
  uint8_t  lastKnownLevel = 0; // press level currently
  uint64_t timePressBegan = 0; // store time that full press occurred
  uint64_t velocityData = 0; // proxy for velocity
  bool     just_pressed_flag = false;
  bool     just_released_flag = false;
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
        if (!queue_try_remove(
          &open_synth_channel_queue, 
          b->*synthChPlaying
        ) return;
        msg->command = synth_msg_note_on;
        msg->item_number = b->synthChPlaying;
        msg->command += update_pitch;
        double adj_f = frequency_after_pitch_bend(
          b->frequency, 0, 2);
          // TO-DO link to settings
        gen->pitch_as_increment = frequency_to_interval(
          adj_f, actual_audio_sample_period_in_uS);

        msg->command += update_volume;
        // iso eq = 8 bit,
        // velocity = 7 bit, 
        // global volume = 8 bit
        gen->base_volume = (127 * 255 * iso226(adj_f)) >> 15;

        msg->command += update_waveform;
        // determine from settings
        // if (settings[_synthWav] == squ/saw/tri/w.e.)
        gen->waveform_ID = hybrid_generator;
        calculate_hybrid_params(gen->hybrid_params, adj_f,
          //hybrid
          0,
          // mod wheel
          32)
          // TO-DO link to settings
        // gen->envelope
                  // TO-DO make some presets
        // gen->harmonics[0], [1], etc.
                  // TO-DO make some presets

        // force wait until message is passed.
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