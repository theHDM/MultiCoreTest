#pragma once
/*
 *  This is the background code that converts pinout data
 *  from the rotary knob into a queue of UI actions.
 *  This code is run on core1 in the background 
 *  and passes action messages to core0 for processing. 
 *
 *  Rotary knob code derived from:
 *      https://github.com/buxtronix/arduino/tree/master/libraries/Rotary
 *  Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 *  Contact: bb@cactii.net
 */

#include <stdint.h>
#include <functional>

#include <Wire.h>
#include "pico/util/queue.h"
#include "pico/time.h"
#include "config.h" // import hardware config constants

enum class Rotary_Action {
  no_action, turn_CW,  turn_CCW,
  turn_CW_with_press,  turn_CCW_with_press,
  click, double_click, double_click_release,
  long_press,          long_release
};
queue_t rotary_action_queue;

bool on_callback_rotary(repeating_timer *t);

class hexBoard_Rotary_Object {
protected:
  bool _active;
  uint8_t _Apin;
  uint8_t _Bpin;
  uint8_t _Cpin;

/*
 *  the A/B pins work together to measure turns.
 *  the C pin is a simple button switch
 *
 *                          quadrature encoding
 *    _             __       pin-down sequence
 *   / \ __ pin A     v CW   -,  A,  AB, B,  -
 *  |   |__                  1/1 0/1 0/0 1/0 1/1
 *   \_/    pin B   __^ CCW  -,  B,  BA, A,  -
 *                           1/1 1/0 0/0 0/1 1/1 
 *
 *  When the mechanical rotary knob is turned,
 *  the two pins go through a set sequence of
 *  states during one physical "click", as above.
 *
 *  The neutral state of the knob is 1\1; a turn
 *  is complete when 1\1 is reached again after
 *  passing through all four valid states above,
 *  at which point action should be taken depending
 *  on the direction of the turn.
 *  
 *  The variable "state" captures all this as follows
 *    Value    Meaning
 *    0        Knob is in neutral state (state 0)
 *    1, 2, 3  CCW turn state 1, 2, 3
 *    4, 5, 6   CW turn state 1, 2, 3
 *    8, 16    Completed turn CCW, CW (state 4)
 */
  const uint8_t stateMatrix[7][4] = {
               // From... To... (0/0) (0/1) (1/0) (1/1)
    {0,4,1,0}, // Neut (1/1)    Fail   CW    CCW  Stall
    {2,0,1,0}, // CCW  (1/0)    Next  Fail  Stall Fail 
    {2,3,1,0}, // CCW  (0/0)    Stall Next  Retry Fail
    {2,3,0,8}, // CCW  (0/1)    Retry Stall Fail  Success
    {5,4,0,0}, //  CW  (0/1)    Next  Stall Fail  Fail
    {5,4,6,0}, //  CW  (0/0)    Stall Retry Next  Fail
    {5,0,6,16} //  CW  (1/0)    Retry Fail  Stall Success
  };
  uint8_t _turnState;
  uint8_t _clickState;

  uint32_t _prevClickTime;
  uint32_t _prevHoldTime;

  bool _doubleClickRegistered;
  bool _longPressRegistered;

  bool _invert;                    // if A and B pins were reversed
  uint32_t _longPressThreshold;    // tolerance in microseconds; zero to ignore
  uint32_t _doubleClickThreshold;  // tolerance in microseconds; zero to ignore

  // however, GEM_Menu will set interval in milliseconds.
  void calibrate(bool& setInvert, int& setLP, int& setDC) {
    _invert = setInvert;
    _longPressThreshold = setLP * 1000;
    _doubleClickThreshold = setDC * 1000;
  }

  uint32_t        polling_frequency;
  repeating_timer polling_timer;
  uint8_t         ownership;

public:
  hexBoard_Rotary_Object(uint8_t Apin, uint8_t Bpin, uint8_t Cpin, uint32_t arg_poll_freq)
  : _Apin(Apin), _Bpin(Bpin), _Cpin(Cpin), polling_frequency(arg_poll_freq)
  , _active(false), _turnState(0), _clickState(0)
  , _prevClickTime(0), _prevHoldTime(0)
  , _invert(false), _longPressThreshold(0), _doubleClickThreshold(0)
  , _doubleClickRegistered(false) , _longPressRegistered(false) {
    pinMode(_Apin, INPUT_PULLUP);
    pinMode(_Bpin, INPUT_PULLUP);
    pinMode(_Cpin, INPUT_PULLUP);
  }
  void start() { _active = true;  }
  void stop()  { _active = false; }
  
  // wrapper to safely calibrate knob from core0
  void recalibrate(bool invert_yn, int longPress_mS, int doubleClick_mS) {
    while (ownership == 1) {}
    ownership = 0;
    calibrate(invert_yn, longPress_mS, doubleClick_mS);
    ownership = -1;
  }

  void writeAction(Rotary_Action rotary_action_in) {
    queue_add_blocking(&rotary_action_queue, &rotary_action_in);
  }
  void poll() {
    if (!_active) return;
    uint8_t A = digitalRead(_Apin);
    uint8_t B = digitalRead(_Bpin);
    while (ownership == 0) {}
    ownership = 1;

    uint8_t getRotation = (_invert ? ((A << 1) | B) : ((B << 1) | A));
    _turnState = stateMatrix[_turnState & 0b00111][getRotation];
    
    uint8_t C = digitalRead(_Cpin);
    _clickState = (0b00011 & ((_clickState << 1) + (C == LOW)));

    if ((_turnState & 0b01000) >> 3) {
      writeAction(C ? Rotary_Action::turn_CW  : Rotary_Action::turn_CW_with_press);
    }
    if ((_turnState & 0b10000) >> 4) {
      writeAction(C ? Rotary_Action::turn_CCW : Rotary_Action::turn_CCW_with_press);
    }
    uint64_t right_now = timer_hw->timerawl;
    switch (_clickState) { 
      case 0b11: // held
        if (!_longPressRegistered && (_longPressThreshold > 0) 
            &&  (_prevHoldTime > 0) &&
            (right_now >= _prevHoldTime + _longPressThreshold)) {
          writeAction(Rotary_Action::long_press);
          _longPressRegistered = true;
        }
        break;
      case 0b01: // click down
        if ((_prevClickTime > 0) && (_doubleClickThreshold > 0) &&
        (right_now <= _prevClickTime + _doubleClickThreshold)) {
            writeAction(Rotary_Action::double_click);            
            _doubleClickRegistered = true;
            _prevClickTime = 0;
            _prevHoldTime = 0;
        } else {
            _prevClickTime = right_now;
            _prevHoldTime = right_now;
        }
        break;
      case 0b10: // click up
        _prevHoldTime = 0;
        if (_longPressRegistered) {
          writeAction(Rotary_Action::long_release);
        } else if (_doubleClickRegistered) {
          writeAction(Rotary_Action::double_click_release);
        } else {
          writeAction(Rotary_Action::click);
        }
        _doubleClickRegistered = false;
        _longPressRegistered = false;
        break;
      default:
        break;
    }
    ownership = -1;
  }
  
  void begin(alarm_pool_t *alarm_pool) {
    // enter a positive timer value here because the poll
    // should occur X microseconds after the routine finishes
    alarm_pool_add_repeating_timer_us(
      alarm_pool, 
      polling_frequency,
      on_callback_rotary,
      this, 
      &polling_timer
    );
    start();
  }
};

bool on_callback_rotary(repeating_timer *t) {
  static_cast<hexBoard_Rotary_Object*>(t->user_data)->poll();
  return true;
}
