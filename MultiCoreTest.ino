#include <stdint.h>
#include <functional>
#include "src/timing.h"
#include "src/config.h"
#include "src/debug.h"
#include "pico/util/queue.h"

#include "src/settings.h"
hexBoard_Setting_Array settings;
#include "src/file_system.h"

#include "src/synth.h"
#include "src/rotary.h"
#include "src/keys.h"
#include "src/taskmgr.h"

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

void send_rotary_settings_from(hexBoard_Setting_Array& refS) {
  rotary_setting_in.invert = settings[_rotInv].b;
  rotary_setting_in.double_click_timing = (settings[_rotDblCk].i * 1000);
  rotary_setting_in.long_press_timing = (settings[_rotLongP].i * 1000);
  queue_add_blocking(&rotary_setting_queue, &rotary_setting_in);
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

// set up so that the LED and OLED routines do not try to run at the same time
Task_Alarm timer_refresh_lights(pixel_refresh_period_in_uS, 5'000);
Task_Alarm timer_refresh_OLED(OLED_refresh_period_in_uS, 20'000);
Task_Alarm timer_send_debug(1'000'000, 0);

void setup() {
  load_factory_defaults_to(settings);
  mount_tinyUSB();
  connect_OLED_display(OLED_sdaPin, OLED_sclPin);
  connect_neoPixels(ledPin, ledCount);


  pacing_msg_in = 1;
  queue_add_blocking(&pacing_queue, &pacing_msg_in);

  mount_file_system();  
  //if (!load_settings(settings)) { // attempt to load saved settings, and if not,  
  //}  

  // send calibration messages to core 1 based on current settings
  link_settings_to_objects(settings);
  send_rotary_settings_from(settings);

  init_MIDI();
  initialize_synth_channel_queue();
  menu_setup();
  timer_refresh_lights.start();
  timer_refresh_OLED.start();
  timer_send_debug.start();
  app_state = App_state::play_mode;
}

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
  if (queue_try_remove(&debug_queue, &debug_msg_out)) {
    debug.add("core 1 msg @ ");
    debug.timestamp();
    debug.add(" = ");
    debug.add_num(debug_msg_out);
    debug.add("\n");
  }
  if (timer_refresh_lights.ifDone_thenRepeat()) {
    // LED show next frame
  }
  if (timer_refresh_OLED.ifDone_thenRepeat()) {
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
  }
  if (timer_send_debug.ifDone_thenRepeat()) {    
    debug.send();
  }
}

hexBoard_Synth_Object  synth(synthPins, 2);
hexBoard_Rotary_Object rotary(rotaryPinA, rotaryPinB, rotaryPinC);
hexBoard_Key_Object    keys(muxPins, colPins, analogPins);
hexBoard_Task_Manager  task_mgr(hardware_tick_in_uS);

void setup1() {
  queue_init(&pacing_queue,         sizeof(InterCore_Msg),   4);
  queue_init(&debug_queue,          sizeof(InterCore_Msg),   128);
  queue_init(&calibration_queue,    sizeof(Calibration_Msg), keys_count + 4);
  queue_init(&key_press_queue,      sizeof(Key_Msg),         512);
  queue_init(&rotary_setting_queue, sizeof(Rotary_Settings), 4);
  queue_init(&rotary_action_queue,  sizeof(Rotary_Action),   rotary_queue_size);
  queue_init(&synth_msg_queue,      sizeof(Synth_Msg),       512);
  // wait for pacing message before starting background processes
  queue_remove_blocking(&pacing_queue, &pacing_msg_out);
  synth.begin();
  task_mgr.add_task( // audio sample update - highest priority (stable period needed)
    1, actual_audio_sample_period_in_uS, 
    std::bind(&hexBoard_Synth_Object::poll,  &synth)); 
  rotary.start();
  task_mgr.add_task( // rotary knob - 2nd highest priority (input drop risk)
    2, rotary_pin_fire_period_in_uS,
    std::bind(&hexBoard_Rotary_Object::poll, &rotary)); 
  keys.start();
  task_mgr.add_task( // keyboard - lowest priority
    3, keyboard_pin_reset_period_in_uS,
    std::bind(&hexBoard_Key_Object::poll,    &keys)); 
  task_mgr.begin();
}

void loop1() {
  if (queue_try_remove(&synth_msg_queue, &synth_msg_out)) {
    push_core1_debug(synth_msg_out.command + 2000);
    synth.interpret_synth_msg(synth_msg_out);
  }
  //if (queue_try_remove(&rotary_setting_queue, &rotary_setting_out)) {
  //  rotary.import_settings(rotary_setting_out);
  //  push_core1_debug(11);
  //}
  //if (!queue_is_empty(&calibration_queue)) {
  //  keys.stop();
  //  while (queue_try_remove(&calibration_queue, &calibration_msg_out)) {
  //    keys.read_calibration_msg(calibration_msg_out);
  //  }
  //  keys.start();
  //}
}