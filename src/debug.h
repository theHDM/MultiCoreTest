#pragma once
#include <Arduino.h>
#include <string>
#include "timing.h"
struct hexBoardDebug {
  bool *_ptrIsOn;
  std::string msg;
  hexBoardDebug()  : _ptrIsOn(nullptr) {}
  void setStatus(bool *_ptr) {
    _ptrIsOn = _ptr;
  }
  bool isOn() {
    if (_ptrIsOn == nullptr) return false;
    return *_ptrIsOn;
  }
  bool isOff() {
    if (_ptrIsOn == nullptr) return false;
    return !(*_ptrIsOn);
  }
  void add(std::string s) {    
    if (isOff()) return;
    msg += s; 
  }
  template <typename T> void add_num(T i) { 
    if (isOff()) return;
    msg += std::to_string(i); 
    msg += " ";
  }
  void timestamp() {
    if (isOff()) return;
    msg += "@ time ";
    msg += std::to_string(now()/1000000.f);
    msg += ": ";
  }
  void clear() {
    msg.clear();
  }
  void send() {
    if (isOn()) {
      Serial.flush();
      Serial.print(msg.c_str()); 
      clear();
    }
  }		
  void knobTurns(int turns) {
    if (turns != 0) {
      timestamp();
      add_num(turns);
      add(" knob turns since last OLED frame\n");
    }
  }
  void knobClick(unsigned long long int duration) {
    if (duration > 0) {
      timestamp();
      add_num((float)duration / 1000000.f);
      add(" second knob click acknowledged\n");
    }
  }
};
hexBoardDebug debug;