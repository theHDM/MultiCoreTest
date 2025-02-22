/*  
 *  HexBoard v1.2
 */
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

#include <stdint.h>
#include "pico/time.h"
#include "src/config.h"
#include "src/debug.h"
hexBoard_Debug_Object debug;
volatile int successes = 0;
volatile int screenupdate = 0;
#include "src/settings.h"
hexBoard_Setting_Array settings;
#include "src/file_system.h"
const char* settingFileName = "temp222.dat";

#include "src/synth.h"
hexBoard_Synth_Object  synth(synthPins, 2);
#include "src/rotary.h"
hexBoard_Rotary_Object rotary(rotaryPinA, rotaryPinB, rotaryPinC);
#include "src/keys.h"
hexBoard_Key_Object    keys(muxPins, colPins, analogPins);

bool on_callback_synth(struct repeating_timer *t) {
  synth.poll();
  return true;
}
bool on_callback_rotary(struct repeating_timer *t) {
  rotary.poll();
  return true;
}
bool on_callback_keys(struct repeating_timer *t) {
  keys.poll();
  return true;
}

void start_background_processes() {
  alarm_pool_t *core1pool;
  core1pool = alarm_pool_create(1, 4);
  
  synth.begin();
  struct repeating_timer timer_synth;
  // enter a negative timer value here because the poll
  // should occur X microseconds after the routine starts
  alarm_pool_add_repeating_timer_us(core1pool, 
    -audio_sample_interval_uS, on_callback_synth, 
    NULL, &timer_synth);

  rotary.begin();
  struct repeating_timer timer_rotary;
  // enter a positive timer value here because the poll
  // should occur X microseconds after the routine finishes
  alarm_pool_add_repeating_timer_us(core1pool,
    rotary_poll_interval_uS, on_callback_rotary,
    NULL, &timer_rotary);

  keys.begin();
  struct repeating_timer timer_keys;
  // enter a positive timer value here because the poll
  // should occur X microseconds after the routine finishes
  alarm_pool_add_repeating_timer_us(core1pool, 
    key_poll_interval_uS, on_callback_keys, 
    NULL, &timer_keys);

  while (1) {}
  // once these objects are run, then core_1 will
  // run background processes only
}

#include "src/synth.h"  // direct digital synthesis math
#include "src/music.h"  // microtonal and MIDI math
#include "src/MIDI_and_USB.h"
#include "src/hexBoard.h"
hexBoard_Grid_Object   hexBoard(hexBoard_layout_v1_2);

#include "src/LED.h"
#include "src/OLED.h"
OLED_screensaver       oled_screensaver(default_contrast, screensaver_contrast);

#include "src/menu.h"
#include "src/GUI.h"
#include "src/layout.h"

void hardwired_switch_handler(int16_t ID) {
  switch (ID) {
    case 140: {
      if (settings[_defaults].b) { // if you have not loaded existing settings
        load_factory_defaults_to(settings, 12); // replace with v1.2 firmware defaults
      }
    }
    default:
      break;
  }
}

void link_settings_to_objects(hexBoard_Setting_Array& refS) {
  debug.setStatus(&refS[_debug].b);
  oled_screensaver.setDelay(&refS[_SStime].i);
}
void set_audio_outs_from_settings(hexBoard_Setting_Array& refS) {
  synth.set_pin(piezoPin, refS[_synthBuz].b);
  synth.set_pin(audioJackPin, refS[_synthJac].b);
}
void calibrate_rotary_from_settings(hexBoard_Setting_Array& refS) {
  rotary.recalibrate(refS[_rotInv].b, refS[_rotLongP].i, refS[_rotDblCk].i);
}
void pre_cache_synth_waveform(hexBoard_Setting_Array& refS) {
  switch (refS[_synthWav].i) {
    case _synthWav_square:
      hexBoard.cached_waveform = linear_waveform(1.f, Linear_Wave::square, 0);
      break;
    case _synthWav_saw:
      hexBoard.cached_waveform = linear_waveform(1.f, Linear_Wave::saw, 0);
      break;
    case _synthWav_triangle:
      hexBoard.cached_waveform = linear_waveform(1.f, Linear_Wave::triangle, 0);
      break;
    case _synthWav_sine:
      hexBoard.cached_waveform = additive_synthesis(1, sineAmt, sinePhase);
      break;
    case _synthWav_strings:
      hexBoard.cached_waveform = additive_synthesis(10, stringsAmt, stringsPhase);
      break;
    case _synthWav_clarinet:
      hexBoard.cached_waveform = additive_synthesis(11, clarinetAmt, clarinetPhase);
      break;
    default:
      break;
  } 
}
void apply_settings_to_objects(hexBoard_Setting_Array& refS) {
  set_audio_outs_from_settings(refS);
  calibrate_rotary_from_settings(refS);
  pre_cache_synth_waveform(refS); 
  generate_layout(refS);
}
void menu_handler(int settingNumber) {
  switch (settingNumber) {
    case _run_routine_to_generate_layout:
      generate_layout(settings);
      menu.setMenuPageCurrent(pgHome);
      break;
    
    case _txposeS: case _txposeC:
      // recalculate pitches for everyone
      break;
    case _scaleLck:
      // set scale lock as appropriate
      break;
    case _rotInv: case _rotDblCk: case _rotLongP:
      calibrate_rotary_from_settings(settings);
      break;
    case _MIDIusb: case _MIDIjack:
      // turn MIDI jacks on/off
      break;    
    case _synthBuz: case _synthJac:
      set_audio_outs_from_settings(settings);
      break;    
    case _MIDIpc: case _MT32pc:
      // send program change
      break;
    case _synthWav:
      pre_cache_synth_waveform(settings); 
      // precalculate active waveform and cache?
      break;
    /*
    _animFPS,  //
    _palette,  //
    _animType, //
    _globlBrt, //
    _hueLoop,  //
    _tglWheel, //
    _whlMode,  //
    _mdSticky, //
    _pbSticky, //
    _vlSticky, //
    _mdSpeed,  //
    _pbSpeed,  //
    _vlSpeed,  //
    _MIDImode, //
    _MPEzoneC, //
    _MPEzoneL, //
    _MPEzoneR, //
    _MPEpb,    //
    _synthTyp, //
    */
    default: 
      break;
  }
}

void interpret_key_msg(Key_Msg& msg) {
  Button* b = &(hexBoard.button_at_linear_index(msg.switch_number));
  b->update_levels(msg.timestamp, msg.level);
  if (b->check_and_reset_just_pressed()) {
    if (!b->isBtn) {
      hardwired_switch_handler(b->pixel);
      return;
    }

    debug.add("note ");
    debug.add_num(b->midiPitch);
    debug.add("tier ");
    debug.add_num(b->paletteNum);
    debug.add("A ");
    debug.add_num(b->A_steps);
    debug.add("B ");
    debug.add_num(b->B_steps);
    debug.add("octave ");
    debug.add_num(b->scaleEquave);
    debug.add("step ");
    debug.add_num(b->scaleDegree);
    debug.add(" / ");
    debug.add_num(b->largeDegree);
    debug.add("L,");
    debug.add_num(b->smallDegree);
    debug.add("s\n");

    switch (app_state) {
      case App_state::play_mode:
      case App_state::menu_nav: {
        settings[_anchorX].i = b->coord.x;
        settings[_anchorY].i = b->coord.y;
        if (queue_is_empty(&open_synth_channel_queue)) {
          debug.add("emptyyyy\n");
          return;
        }
        queue_remove_blocking(
          &open_synth_channel_queue, 
          &(b->synthChPlaying)
        );
        Synth_Voice *v = &synth.voice[(b->synthChPlaying) - 1];
        double adj_f = frequency_after_pitch_bend(b->frequency, 0 /*pb*/, 2 /*pb range*/);
        v->update_pitch(frequency_to_interval(adj_f, audio_sample_interval_uS));
        if    ((settings[_synthWav].i == _synthWav_hybrid) || ( false /*mod > 0*/
          &&  ((settings[_synthWav].i == _synthWav_square) 
            || (settings[_synthWav].i == _synthWav_saw)
            || (settings[_synthWav].i == _synthWav_triangle)
        ))) {
          v->update_wavetable(linear_waveform(adj_f, Linear_Wave::hybrid, 0 /*mod*/));
        } else {
          v->update_wavetable(hexBoard.cached_waveform);
        }
        v->update_base_volume((settings[_synthVol].i * b->velocity * iso226(adj_f)) >> 15);
        switch (settings[_synthEnv].i) {
          case _synthEnv_hit:
            v->update_envelope(20/*ms*/, 50/*ms*/, 128/*0-255*/, 100/*ms*/);
            break;
          case _synthEnv_pluck:
            v->update_envelope(20/*ms*/, 1000/*ms*/, 24/*0-255*/, 100/*ms*/);
            break;
          case _synthEnv_strum:
            v->update_envelope(50/*ms*/, 2000/*ms*/, 128/*0-255*/, 500/*ms*/);
            break;
          case _synthEnv_slow:
            v->update_envelope(1000/*ms*/, 0/*ms*/, 255/*0-255*/, 1000/*ms*/);
            break;
          case _synthEnv_reverse:
            v->update_envelope(2000/*ms*/, 0/*ms*/, 0/*0-255*/, 0/*ms*/);
            break;
          default:
            v->update_envelope(0, 0, 255, 0);
            break;
        }
        v->note_on();
        break;
      }
      default: break;
    }
  } else if (b->check_and_reset_just_released()) {
    switch (app_state) {
      case App_state::play_mode:
      case App_state::menu_nav: {
        if (!b->synthChPlaying) break;
        Synth_Voice *v = &synth.voice[(b->synthChPlaying) - 1];
        v->note_off();
        if (queue_is_full(&open_synth_channel_queue)) {
          debug.add("negative ghost rider the channels full\n");
          return;
        }
        queue_add_blocking(
          &open_synth_channel_queue, 
          &(b->synthChPlaying)
        );
        b->synthChPlaying = 0;
        break;
      }
      default: break;
    }
  } else if (b->pressure) {
    //
  }
}

void process_play_mode_knob(Rotary_Action& A) {
  switch (A) {
    // TO-DO -- route other actions to various commands
    case Rotary_Action::long_press:
      menu.setMenuPageCurrent(pgHome);
      app_state = App_state::menu_nav;
      break;
    default:
      break;
  }    
}

void process_menu_input(Rotary_Action& A) {
  if (!menu.readyForKey()) return;
  screenupdate = 1;
	bool editingFloat = menu.isEditMode();
  if (editingFloat) {
    GEMItemPublic pubItem = *(menu.getCurrentMenuPage()->getCurrentMenuItem());
    editingFloat = (pubItem.getLinkedType() == GEM_VAL_DOUBLE);
  }
  switch (A) {
    case Rotary_Action::turn_CW:
    case Rotary_Action::turn_CW_with_press:
      menu.registerKeyPress(GEM_KEY_DOWN);
      break;
    case Rotary_Action::turn_CCW:
    case Rotary_Action::turn_CCW_with_press:
      menu.registerKeyPress(GEM_KEY_UP);
      break;
    case Rotary_Action::click:
      if (editingFloat) {
        menu.registerKeyPress(GEM_KEY_RIGHT);
      } else {
        menu.registerKeyPress(GEM_KEY_OK);
      }
      break;       
    case Rotary_Action::double_click:
      if (editingFloat) {
        menu.registerKeyPress(GEM_KEY_LEFT);
      }
      break;
    case Rotary_Action::double_click_release: 
      if (!editingFloat) {
        menu.registerKeyPress(GEM_KEY_OK);
      } else {
        menu.registerKeyPress(GEM_KEY_LEFT); 
      }
      break;
    case Rotary_Action::long_press: // act on long press as soon as hold time met
      if (!menu.isEditMode()) {
        if (menu.getCurrentMenuPage() == &pgHome) {
          app_state = App_state::play_mode;
          menu.setMenuPageCurrent(pgNoMenu);
        } else {
          menu.registerKeyPress(GEM_KEY_CANCEL);
        }
      } else {
        menu.registerKeyPress(GEM_KEY_OK); 
      }
      break;
    default:
      break;
  }    
  screenupdate = 0;
}

struct repeating_timer polling_timer_LED;
bool on_LED_frame_refresh(repeating_timer *t) {
  for (auto& b : hexBoard.btn) {
    if (!b.isBtn) continue;
    strip.setPixelColor(b.pixel, b.LEDcodeBase);
  }
  strip.show();
  return true;
}

struct repeating_timer polling_timer_OLED;
bool on_OLED_frame_refresh(repeating_timer *t) {
  switch (app_state) {
    case App_state::menu_nav:
    case App_state::edit_mode:
      if (screenupdate) break;
      menu.drawMenu(); // when menu is active, call GUI update through menu refresh
      oled_screensaver.jiggle();
      break;
    case App_state::play_mode:
      u8g2.clearBuffer();
      GUI.draw();
      u8g2.sendBuffer();
      break;
    default:
      break;
  }
  return true;
}

struct repeating_timer polling_timer_debug;
bool on_debug_refresh(repeating_timer *t) {
  //debug.add_num(successes);
  //debug.add("\n");
  debug.send();
  return true;
}

void setup() {
  queue_init(&key_press_queue, sizeof(Key_Msg), keys_count + 1);
  queue_init(&rotary_action_queue,  sizeof(Rotary_Action),  32);
  load_factory_defaults_to(settings);
  link_settings_to_objects(settings);

  mount_tinyUSB();
  connect_OLED_display(OLED_sdaPin, OLED_sclPin);
  connect_neoPixels(ledPin, ledCount);
  mount_file_system();  
  //if (!load_settings(settings, settingFileName)) { // attempt to load saved settings, and if not,  
  //}  
  apply_settings_to_objects(settings);
  init_MIDI();
  initialize_synth_channel_queue();
  menu_setup();
  add_repeating_timer_ms(
    LED_poll_interval_mS, 
    on_LED_frame_refresh, NULL, 
    &polling_timer_LED
  );
  add_repeating_timer_ms(
    OLED_poll_interval_mS,
    on_OLED_frame_refresh, NULL, 
    &polling_timer_OLED
  );
  add_repeating_timer_ms(
    1'000,
    on_debug_refresh, NULL, 
    &polling_timer_debug
  );
  multicore_launch_core1(start_background_processes);
  app_state = App_state::play_mode;
}

Key_Msg key_msg_out;
Rotary_Action rotary_action_out;

void loop() {
  if (queue_try_remove(&key_press_queue, &key_msg_out)) {
    interpret_key_msg(key_msg_out);
    // if you are in calibration mode, that routine can send msgs to calibration_queue
  }
  if (queue_try_remove(&rotary_action_queue, &rotary_action_out)) {
    switch (app_state) {
      case App_state::play_mode:  process_play_mode_knob(rotary_action_out); break;
      case App_state::menu_nav: 	process_menu_input(rotary_action_out);     break;
      default:                                                               break;
    }
  }
}