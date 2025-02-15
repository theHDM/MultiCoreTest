#pragma once
#include <Adafruit_TinyUSB.h>   // library of code to get the USB port working
#include <MIDI.h>               // library of code to send and receive MIDI messages
#include "pico/time.h"
#include "debug.h"

Adafruit_USBD_MIDI usb_midi_over_Serial0;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi_over_Serial0, UMIDI);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, SMIDI);

void mount_tinyUSB() {
  uint64_t mountTime = timer_hw->timerawl;
  while (!TinyUSBDevice.mounted()) {}
  mountTime = timer_hw->timerawl - mountTime;
  Serial.begin(115200);
  debug.add("it took ");
  debug.add_num(mountTime);
  debug.add("uS to mount TinyUSB.\n");
}

void init_MIDI() {
  usb_midi_over_Serial0.setStringDescriptor("HexBoard MIDI");  // Initialize MIDI, and listen to all MIDI channels
  UMIDI.begin(MIDI_CHANNEL_OMNI);                 // This will also call usb_midi's begin()
  SMIDI.begin(MIDI_CHANNEL_OMNI);
}