#pragma once
#include <Wire.h>
#include <U8g2lib.h>
#include "timing.h"

// GUI context -- keep as integer-castable because
// you can use bitwise comparisons to check
// multiple contexts
enum {
  _hide_GUI = 0,
  _show_HUD = 1,
  _show_pixel_ID = 2,
  _show_custom_msg = 4,
  _show_dashboard = 8
};

// Create an instance of the U8g2 graphics library.
U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2(U8G2_R2);

const unsigned char _OLED_HEIGHT = 128;
const unsigned char _OLED_WIDTH  = 128;
const unsigned char _LG_FONT_WIDTH  = 6;
const unsigned char _LG_FONT_HEIGHT = 8;
const unsigned char _SM_FONT_HEIGHT = 6;
const unsigned char _LEFT_MARGIN = 6;
const unsigned char _RIGHT_MARGIN = 8;

void connect_OLED_display(unsigned SDA, unsigned SCL) {
  // the microprocessor is the RP2040 / RP2350 series
  // and we use Earle Philhower's pico library to interface
  // with hardware. the u8g2 library to control the OLED screen
  // has a function to set the pins but it is not compatible
  // with Earle's library. we therefore set the u8g2 pins
  // using the Wire initializers as shown below.
  Wire.setSDA(SDA);
  Wire.setSCL(SCL);
  u8g2.begin();
  u8g2.setBusClock(1000000);
  u8g2.setContrast(255);
  u8g2.setFont(u8g2_font_inr57_mn);
  u8g2.clearBuffer();
  u8g2.drawStr(6,10,"HexBoard");
  u8g2.sendBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
}

struct OLED_screensaver {
  bool screensaver_mode;
  unsigned contrast_on;
  unsigned contrast_off;
  unsigned long long int switch_time;
  int *_ptr_delay;
  OLED_screensaver(unsigned contrast_on_, unsigned contrast_off_)
  : switch_time(0), screensaver_mode(true), _ptr_delay(nullptr)
  , contrast_on(contrast_on_), contrast_off(contrast_off_) {}
  void jiggle() {
    if (_ptr_delay == nullptr) return;
    switch_time = now();
    switch_time += ((unsigned long long int)(*_ptr_delay) << 20);
    if (!screensaver_mode) return;
    screensaver_mode = false;
    u8g2.setContrast(contrast_on);
  }
  void setDelay(int *_ptr) {
    _ptr_delay = _ptr;
    jiggle();
  }
  void poll() {
    if (screensaver_mode) return;
    if (now() >= switch_time) {
      screensaver_mode = true;
      u8g2.setContrast(contrast_off);
    }
  }
};