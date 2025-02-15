#include <stdint.h>
#include "pico/time.h"
#include "src/config.h"
#include "src/debug.h"
hexBoard_Debug_Object debug;

#include "src/settings.h"
hexBoard_Setting_Array settings;
#include "src/file_system.h"

#include "src/audio.h"
#include "src/rotary.h"
#include "src/keys.h"

#include "src/synth.h"
#include "src/hexBoard.h"
hexBoard_Grid_Object   hexBoard(hexBoard_layout_v1_2);

#include "src/MIDI_and_USB.h"
#include "src/LED.h"
#include "src/OLED.h"
OLED_screensaver       oled_screensaver(default_contrast, screensaver_contrast);
#include "src/menu.h"
#include "src/GUI.h"

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
/*
void send_rotary_settings_from(hexBoard_Setting_Array& refS) {
  rotary_setting_in.invert = settings[_rotInv].b;
  rotary_setting_in.double_click_timing = (settings[_rotDblCk].i * 1000);
  rotary_setting_in.long_press_timing = (settings[_rotLongP].i * 1000);
  queue_add_blocking(&rotary_setting_queue, &rotary_setting_in);
}
*/

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
}

struct repeating_timer polling_timer_LED;
bool on_LED_frame_refresh(repeating_timer *t) {
  return true;
}

struct repeating_timer polling_timer_OLED;
bool on_OLED_frame_refresh(repeating_timer *t) {
  switch (app_state) {
    case App_state::menu_nav:
    case App_state::edit_mode:
      menu.drawMenu(); // when menu is active, call GUI update through menu refresh
      oled_screensaver.jiggle();
      break;
    case App_state::play_mode:
      u8g2.clearBuffer();
      draw_GUI(_show_HUD + _show_dashboard);
      u8g2.sendBuffer();
      break;
    default:
      break;
  }
  return true;
}

struct repeating_timer polling_timer_debug;
bool on_debug_refresh(repeating_timer *t) {
  debug.send();
  return true;
}

hexBoard_Synth_Object  synth(synthPins, 2, audio_sample_interval_uS);
hexBoard_Rotary_Object rotary(rotaryPinA, rotaryPinB, rotaryPinC, rotary_poll_interval_uS);
hexBoard_Key_Object    keys(muxPins, colPins, analogPins, key_poll_interval_uS);

void start_background_processes() {
  alarm_pool_t *core1pool;
  core1pool = alarm_pool_create(2, 16);
  synth .begin(core1pool);
  rotary.begin(core1pool);
  keys  .begin(core1pool);
  // once these objects are run, then core_1 will
  // run background processes only
}

void setup() {
  queue_init(&key_press_queue, sizeof(Key_Msg), keys_count + 1);
  queue_init(&rotary_action_queue,  sizeof(Rotary_Action),  32);
  load_factory_defaults_to(settings);
  mount_tinyUSB();
  connect_OLED_display(OLED_sdaPin, OLED_sclPin);
  connect_neoPixels(ledPin, ledCount);
  mount_file_system();  
  link_settings_to_objects(settings);
  //if (!load_settings(settings)) { // attempt to load saved settings, and if not,  
  //}  
  //  send_rotary_settings_from(settings);
  init_MIDI();
  initialize_synth_channel_queue();
  menu_setup();
  add_repeating_timer_ms(
    -LED_poll_interval_mS, 
    on_LED_frame_refresh, NULL, 
    &polling_timer_LED
  );
  add_repeating_timer_ms(
    -OLED_poll_interval_mS,
    on_OLED_frame_refresh, NULL, 
    &polling_timer_OLED
  );
  add_repeating_timer_ms(
    -1'000,
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
    hexBoard.interpret_key_msg(key_msg_out);
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