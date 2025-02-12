#pragma once
#include <array>
#include <stdint.h>
#include <map>
#include "settings.h"
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
App_state app_state = App_state::setup;
/* serialize members with a !! */
struct Button {
/*!!*/  bool     isUsed = false;  // is it a button or a hardwired circuit
/*!!*/  int8_t   atMux  = -1;
/*!!*/  int8_t   atCol  = -1;

/*!!*/  bool     isBtn  = false;  // is it a button not a hardwired circuit
/*!!*/  Hex      coord  = {0,0};  // physical location
/*!!*/  int16_t  pixel  = -1;     // the button's index in the grid, which is equal to its associated pixel number if it has one
/*!!*/  uint32_t LEDcodeBase = 0; // for now
/*!!*/  uint32_t LEDcodeAnim = 0; // calculate it once and store value, to make LED playback snappier 
/*!!*/  uint32_t LEDcodePlay = 0; // calculate it once and store value, to make LED playback snappier
/*!!*/  uint32_t LEDcodeRest = 0; // calculate it once and store value, to make LED playback snappier
/*!!*/  uint32_t LEDcodeOff  = 0; // calculate it once and store value, to make LED playback snappier
/*!!*/  uint32_t LEDcodeDim  = 0; // calculate it once and store value, to make LED playback snappier

/*!!*/  bool     isNote = false;  // is it a music note object
/*!!*/  uint8_t  midiCh = 0;      // what channel assigned (if not MPE mode)   [1..16]
/*!!*/  uint8_t  midiTuningTable = 255; // assigned MIDI note (if MTS mode) [0..127]
/*!!*/  int8_t   scaleEquave = 0;
/*!!*/  uint8_t  scaleDegree = 0;     // order in scale relative to equave
/*!!*/  bool     inScale = false; // for scale-lock purposes
/*!!*/  double   frequency = 0.0; // equivalent pitch in Hz
/*!!*/  uint8_t  midiNote = 0;    // nearest MIDI pitch, 0 to 128
/*!!*/  int16_t  midiBend = 0;    // pitch bend for MPE purposes

/*!!*/  bool     isCmd  = false;  // is it a command button
/*!!*/  uint8_t  cmd = 0;  // control parameter corresponding to this hex

  uint64_t timeLastUpdate = 0; // store time that key level was last updated
  uint64_t timePressBegan = 0; // store time that full press occurred
  uint64_t timeHeldSince  = 0;
  uint8_t  pressure       = 0; // press level currently
  uint8_t  velocity       = 0; // proxy for velocity
  bool     just_pressed   = false;
  bool     just_released  = false;
  uint8_t  midiChPlaying  = 0;          // what midi channel is there currrently a note-on
  uint8_t  synthChPlaying = 0;         // what synth channel is there currrently a note-on

  void update_levels(uint64_t& timestamp, uint8_t& new_level) {
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

          /* placeholder to put note values for testing */
          b->LEDcodeBase     = 0x11111100; // for now
          b->LEDcodeAnim     = 0x11111100; // calculate it once and store value, to make LED playback snappier 
          b->LEDcodePlay     = 0x11111100; // calculate it once and store value, to make LED playback snappier
          b->LEDcodeRest     = 0x11111100; // calculate it once and store value, to make LED playback snappier
          b->LEDcodeOff      = 0; // calculate it once and store value, to make LED playback snappier
          b->LEDcodeDim      = 0x11111100; // calculate it once and store value, to make LED playback snappier
          b->frequency       = 440.0; // equivalent pitch in Hz
          b->midiCh          = 1;      // what channel assigned (if not MPE mode)   [1..16]
          b->midiTuningTable = 255; // assigned MIDI note (if MTS mode) [0..127]
          b->scaleEquave     = 0;
          b->scaleDegree     = 0;     // order in scale relative to equave
          b->inScale         = true; // for scale-lock purposes
          b->cmd             = 0;  // control parameter corresponding to this hex
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

  Button& button_at_coord(Hex& coord) {
    return btn[coord_to_pixel.at(coord)];
  }

  Button& button_at_linear_index(size_t l_index) {
    return btn[index_to_pixel.at(l_index)];
  }

  bool in_bounds(Hex& coord) {
    return (coord_to_pixel.find(coord) != coord_to_pixel.end());
  }

  void on_key_press(Button* b) {
    if (!b->isBtn) {
      hardwired_switch_handler(b->pixel);
      return;
    }
    Synth_Msg* msg = &synth_msg_in;
    Synth_Generation* gen = &(msg->synth_generation);
    switch (app_state) {
      case App_state::play_mode: {
        if (queue_is_empty(&open_synth_channel_queue)) {
          debug.add("emptyyyy\n");
          return;
        }
        if (queue_is_full(&synth_msg_queue)) {
          debug.add("2many synth msgs bro\n");
          return;
        }
        queue_remove_blocking(
          &open_synth_channel_queue, 
          &(b->synthChPlaying)
        );
        msg->command = synth_msg_note_on;
        msg->item_number = b->synthChPlaying;
        msg->command += synth_msg_update_pitch;
        double adj_f = frequency_after_pitch_bend(
          b->frequency, 0, 2);
          // TO-DO link to settings
        gen->pitch_as_increment = frequency_to_interval(
          adj_f, actual_audio_sample_period_in_uS);

        msg->command += synth_msg_update_volume;
        // iso eq = 8 bit,
        // velocity = 7 bit, 
        // global volume = 8 bit
        gen->base_volume = (settings[_synthVol].i * b->velocity * iso226(adj_f)) >> 15;

        msg->command += synth_msg_update_waveform;
        // determine from settings
        // if (settings[_synthWav] == squ/saw/tri/w.e.)
        gen->waveform_ID = hybrid_generator;
        uint8_t temp = 0;
        uint8_t temp2 = 32;
        calculate_hybrid_params(gen->hybrid_params, adj_f,
          // hybrid shape
          temp,
          // mod wheel value
          temp2);
          // TO-DO link to settings
        // gen->envelope
                  // TO-DO make some presets
        // gen->harmonics[0], [1], etc.
                  // TO-DO make some presets
        // force wait until message is passed.
        // a 45ms d 2000ms s 64/255 r 1000ms
        // 
        gen->envelope.attack = 45'000 / actual_audio_sample_period_in_uS;
        gen->envelope.decay  = 2'000'000 / actual_audio_sample_period_in_uS;
        gen->envelope.sustain = 64;
        gen->envelope.release = 1'000'000 / actual_audio_sample_period_in_uS;
        debug.add("synth cmd ");
        debug.add_num(msg->command);
        debug.add("item ");
        debug.add_num(msg->item_number);
        debug.add("freq ");
        debug.add_num(adj_f);
        debug.add("incr ");
        debug.add_num(gen->pitch_as_increment);
        debug.add("vol ");
        debug.add_num(gen->base_volume);
        debug.add("hyb ");
        debug.add_num(gen->hybrid_params.a);
        debug.add_num(gen->hybrid_params.b);
        debug.add_num(gen->hybrid_params.c);
        debug.add_num(gen->hybrid_params.d_minus_one);
        debug.add_num(gen->hybrid_params.ab);
        debug.add_num(gen->hybrid_params.cd);
        debug.add("\n");
        queue_add_blocking(&synth_msg_queue, msg);
        break;
      }
      default: break;
    }
  }
  
  void on_key_release(Button* b) {
    Synth_Msg* msg = &synth_msg_in;
    Synth_Generation* gen = &(msg->synth_generation);
    switch (app_state) {
      case App_state::play_mode: {
        if (!b->synthChPlaying) break;

        msg->command = synth_msg_note_off;
        msg->item_number = b->synthChPlaying;
        if (queue_is_full(&synth_msg_queue)) {
          debug.add("2many synth msgs bro\n");
          return;
        }
        queue_add_blocking(&synth_msg_queue, msg);
        if (queue_is_full(&open_synth_channel_queue)) {
          debug.add("negative ghost rider the channels full\n");
          return;
        }
        queue_add_blocking(&open_synth_channel_queue, &(b->synthChPlaying));
        b->synthChPlaying = 0;
        break;
      }
      default: break;
    }    
  }

  void on_key_hold(Button* b) {
    // nothing right now
  }

  void interpret_key_msg(Key_Msg& msg) {
    Button* thisBtn = &(button_at_linear_index(msg.switch_number));
    thisBtn->update_levels(msg.timestamp, msg.level);
           if (thisBtn->check_and_reset_just_pressed())  {
      on_key_press(thisBtn);
    } else if (thisBtn->check_and_reset_just_released()) {
      on_key_release(thisBtn);
    } else if (thisBtn->pressure) {
      on_key_hold(thisBtn);
    }
  }

};