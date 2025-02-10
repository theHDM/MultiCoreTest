#pragma once
#include <Wire.h>
#include "timing.h" // querying system clock
#include "config.h" // import hardware config constants
#include <stdint.h>
#include "pico/util/queue.h"
/*
  Documentation:
    Rotary knob code derived from:
      https://github.com/buxtronix/arduino/tree/master/libraries/Rotary
  Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
  Contact: bb@cactii.net

  when the mechanical rotary knob is turned,
  the two pins go through a set sequence of
  states during one physical "click", as follows:
    Direction          Binary state of pin A\B
    Counterclockwise = 1\1, 0\1, 0\0, 1\0, 1\1
    Clockwise        = 1\1, 1\0, 0\0, 0\1, 1\1

  The neutral state of the knob is 1\1; a turn
  is complete when 1\1 is reached again after
  passing through all the valid states above,
  at which point action should be taken depending
  on the direction of the turn.
  
  The variable "state" captures all this as follows
    Value    Meaning
    0        Knob is in neutral state
    1, 2, 3  CCW turn state 1, 2, 3
    4, 5, 6   CW turn state 1, 2, 3
    8, 16    Completed turn CCW, CW
*/
enum class Rotary_Action {
  no_action,
  turn_CW,
  turn_CCW,
  turn_CW_with_press,
  turn_CCW_with_press,
  click,
  double_click,
  double_click_release,
  long_press,
  long_release
};

queue_t rotary_action_queue;
Rotary_Action rotary_action_to_core_0;

struct Rotary_Settings {
  bool invert;
  uint32_t double_click_timing;
  uint32_t long_press_timing;
};

queue_t rotary_setting_queue;
Rotary_Settings rotary_setting_from_core_0;
Rotary_Settings rotary_setting_to_core_1;

class hexBoard_Rotary_Object {
protected:
  bool _active;
  uint8_t _Apin;
  uint8_t _Bpin;
  uint8_t _Cpin;

  uint8_t _turnState;
  const uint8_t stateMatrix[7][4] = {
    {0,4,1,0},
    {2,0,1,0},{2,3,1,0},{2,3,0,8},
    {5,4,0,0},{5,4,6,0},{5,0,6,16}
  };
  uint8_t _clickState;

  uint64_t _prevClickTime;
  uint64_t _prevHoldTime;

  bool _doubleClickRegistered;
  bool _longPressRegistered;

  bool _invert;
  uint32_t _longPressThreshold;    // tolerance in microseconds; zero to ignore
  uint32_t _doubleClickThreshold;  // tolerance in microseconds; zero to ignore

public:
  hexBoard_Rotary_Object(uint8_t Apin, uint8_t Bpin, uint8_t Cpin)
  : _Apin(Apin), _Bpin(Bpin), _Cpin(Cpin)
  , _invert(false), _turnState(0), _clickState(0)
  , _prevClickTime(0), _prevHoldTime(0)
  , _longPressThreshold(0), _doubleClickThreshold(0)
  , _doubleClickRegistered(false)
  , _longPressRegistered(false)
  , _active(false) {
    pinMode(_Apin, INPUT_PULLUP);
    pinMode(_Bpin, INPUT_PULLUP);
    pinMode(_Cpin, INPUT_PULLUP);
  }
  void start() {
    _active = true;
  }
  void stop() {
    _active = false;
  }
  void import_settings(Rotary_Settings& input) {
    _invert = input.invert;
    _longPressThreshold = input.long_press_timing;
    _doubleClickThreshold = input.double_click_timing;
  }
  void writeAction(Rotary_Action rotary_msg_from_core_1) {
    queue_add_blocking(&rotary_action_queue, &rotary_msg_from_core_1);
  }
  void poll() {
    if (!_active) return;
    uint8_t A = digitalRead(_Apin);
    uint8_t B = digitalRead(_Bpin);
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
    uint64_t right_now = now();
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
  }
};