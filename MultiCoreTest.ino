#include <stdint.h>
#include <functional>
#include "src/timing.h"
#include "src/config.h"
#include "src/debug.h"
#include "pico/util/queue.h"

using InterCore_Msg = uint32_t;
queue_t pacing_queue;
InterCore_Msg pacing_msg_to_core_1;
InterCore_Msg pacing_msg_from_core_0;
queue_t debug_queue;
InterCore_Msg debug_msg_to_core_0;
InterCore_Msg debug_msg_from_core_1;


#include "src/synth.h"
#include "src/rotary.h"
#include "src/keys.h"
#include "src/taskmgr.h"

#include "src/hexBoard.h"
hexBoard_Grid_Object   hexBoard(hexBoard_layout_v1_2);
App_state app_state = App_state::setup;

// set up so that the LED and OLED routines do not try to run at the same time
Task_Alarm timer_refresh_lights(pixel_refresh_period_in_uS, 5'000);
Task_Alarm timer_refresh_OLED(OLED_refresh_period_in_uS, 20'000);
Task_Alarm timer_send_debug(1'000'000, 0);

void setup() {
  queue_init(&pacing_queue,        sizeof(InterCore_Msg),                 4);
  queue_init(&debug_queue,         sizeof(InterCore_Msg),               128);
  queue_init(&calibration_queue,   sizeof(Calibration_Msg),  keys_count + 4);
  queue_init(&key_press_queue,     sizeof(Key_Msg),                     512);
  queue_init(&rotary_setting_queue,sizeof(Rotary_Settings),               4);
  queue_init(&rotary_action_queue, sizeof(Rotary_Action), rotary_queue_size);
  queue_init(&synth_msg_queue,     sizeof(Synth_Msg),                   512);

  pacing_msg_to_core_1 = 1;
  queue_add_blocking(&pacing_queue, &pacing_msg_to_core_1);

  timer_refresh_lights.start();
  timer_refresh_OLED.start();
  timer_send_debug.start();

  app_state = App_state::play_mode;
}

void loop() {
  if (queue_try_remove(&key_press_queue, &key_msg_to_core_0)) {
    hexBoard.interpret_key_msg(key_msg_to_core_0);
    // if you are in calibration mode, that routine can send msgs to calibration_queue
  }
  if (queue_try_remove(&rotary_action_queue, &rotary_action_to_core_0)) {
    // process menu or other items
    // menu routine can send messages to rotary_setting or synth_msg queue
  }
  if (queue_try_remove(&core1_debug_queue, &debug_msg_to_core_0)) {
    debug.add_num(debug_msg_to_core_0);
  }
  if (timer_refresh_lights.ifDone_thenRepeat()) {
    // LED show next frame
  }
  if (timer_refresh_OLED.ifDone_thenRepeat()) {
    // OLED update
  }
  if (timer_send_debug.ifDone_thenRepeat()) {    
    debug.timestamp();
    debug.add("message queue:\n");
    debug.send();
  }
}

hexBoard_Synth_Object  synth(synthPins, 2);
hexBoard_Rotary_Object rotary(rotaryPinA, rotaryPinB, rotaryPinC);
hexBoard_Key_Object    keys(muxPins, colPins, analogPins);
hexBoard_Task_Manager  task_mgr(hardware_tick_in_uS);

void setup1() {
  queue_remove_blocking(&pacing_queue, &pacing_msg_from_core_0);

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
  if (queue_try_remove(&synth_msg_queue, &synth_msg_to_core_1)) {
    synth.interpret_synth_msg(synth_msg_to_core_1);
  }
  if (queue_try_remove(&rotary_setting_queue, &rotary_setting_to_core_1)) {
    rotary.import_settings(rotary_setting_to_core_1);
  }
  if (!queue_is_empty(&calibration_queue)) {
    keys.stop();
    while (queue_try_remove(&calibration_queue, &calibration_msg_to_core_1)) {
      keys.read_calibration_msg(calibration_msg_to_core_1);
    }
    keys.start();
  }
  bool success = queue_try_add(&core1_debug_queue, new uint32_t = 123);
}