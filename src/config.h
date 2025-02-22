/* config.h
 *
 * This file defines hardware-specific constants
 * If you rewire or add peripherals, you'll need to
 * update this file or provide new constants to
 * link to other modules.
 *
 * Definition created 2025-01-31 for HexBoard Hardware v1.2
 *
 * Pinout
 *  0  Serial  (USB)
 *  1  Serial1 (MIDI)
 *  2  Multiplexer, bit 2 (0100)
 *  3  Multiplexer, bit 3 (1000)
 *  4  Multiplexer, bit 0 (0001)
 *  5  Multiplexer, bit 1 (0010)
 *  6  Key switch, column 0
 *  7  Key switch, column 1
 *  8  Key switch, column 2
 *  9  Key switch, column 3
 * 10  Key switch, column 4
 * 11  Key switch, column 5
 * 12  Key switch, column 6
 * 13  Key switch, column 7
 * 14  Key switch, column 8
 * 15  Key switch, column 9
 * 16  OLED display, I2C data pin
 * 17  OLED display, I2C clock pin
 * 18    -- open --
 * 19    -- open --
 * 20  Rotary knob, left
 * 21  Rotary knob, right
 * 22  Adafruit NeoPixel LED strip
 * 23  Piezoelectric buzzer
 * 24  Rotary knob, switch
 * 25  Audio out
 * 26    -- open --
 * 27    -- open --
 * 28    -- open --
 * 29    -- open --
 */

#pragma once
#include <stdint.h>

const uint8_t GPIO_pin_count = 32; // maximum size of certain object arrays

// If you rewire the HexBoard then change these pin values
const uint8_t muxPins[] = {4,5,2,3}; // 1bit 2bit 4bit 8bit
const uint8_t colPins[] = {6,7,8,9,10,11,12,13,14,15};

// 1 if analog (firmware 2.0), 0 if digital
const    bool analogPins[]  = {0,0,0,0,0,0,0,0,0,0};

const size_t mux_pins_count = sizeof(muxPins)/sizeof(muxPins[0]);  // should equal 4
const size_t col_pins_count = sizeof(colPins)/sizeof(colPins[0]);  // should equal 10
constexpr size_t mux_channels_count = 1 << mux_pins_count;         // should equal 16
constexpr size_t keys_count = mux_channels_count * col_pins_count; // should equal 160

size_t linear_index(uint8_t argM, uint8_t argC) {  // should return value 0 thru 159
  return (argC << mux_pins_count) + argM;
}

const uint8_t rotaryPinA = 20;
const uint8_t rotaryPinB = 21;
const uint8_t rotaryPinC = 24;
const uint8_t piezoPin = 23;
const uint8_t audioJackPin = 25;
const uint8_t synthPins[] = {piezoPin, audioJackPin};
const uint8_t ledPin = 22;
const uint8_t OLED_sdaPin = 16;
const uint8_t OLED_sclPin = 17;

const uint32_t highest_MIDI_note_Hz = 13290;
const uint32_t target_sample_rate_Hz = 2 * highest_MIDI_note_Hz;
constexpr int32_t audio_sample_interval_uS = 31250 / (target_sample_rate_Hz >> 5);
const int32_t key_poll_interval_uS = 96;         // ideal is 1/16th microsecond so the whole thing is under 1 millisecond.
const int32_t rotary_poll_interval_uS = 768; // tested at 512 microseconds and it was too short

const uint8_t LED_frame_rate_Hz = 60;
const uint8_t OLED_frame_rate_Hz = 24;
constexpr int32_t LED_poll_interval_mS = 1'000 / LED_frame_rate_Hz;
constexpr int32_t OLED_poll_interval_mS = 1'000 / OLED_frame_rate_Hz;

// TO-DO: test on hardware v2
const uint16_t default_analog_calibration_up = 480;
const uint16_t default_analog_calibration_down = 280;
const size_t   ledCount = 140;  // based on the size of the NeoPixel installed
const uint8_t  default_contrast = 64; // range: 0-127
const uint8_t  screensaver_contrast = 1; // range: 0-127

const uint8_t synth_polyphony_limit = 16;
const uint8_t audio_bits = 9;
constexpr uint16_t neutral_level = (1u << (audio_bits - 1)) - 1;
// names of the columns
enum {
  _layout_table_column_pin,
  _layout_table_multiplex_value,
  _layout_table_switch_type,
  _layout_table_coord_x,
  _layout_table_coord_y,
  _layout_table_pixel_number,
  _layout_table_size  
};
// wiring switch states for each pin
enum {
  unused_pin = 0,  // ignored, even if it does something
  hardwired = 1,   // read only once at startup 
  hex_button = 2, // monitor during operation
};
const int16_t N_A = -127;
// and this big data table, too, while you're at it:
const int16_t hexBoard_layout_v1_2[keys_count][_layout_table_size] = {
 //col mux switch type   x   y  pxl
  { 0,  0, hex_button, -10,  0,   0 },
  { 0,  1, hex_button,  -9, -5,  10 },
  { 0,  2, hex_button, -11,  1,  20 },
  { 0,  3, hex_button,  -9, -3,  30 },
  { 0,  4, hex_button, -10,  2,  40 },
  { 0,  5, hex_button,  -9, -1,  50 },
  { 0,  6, hex_button, -11,  3,  60 },
  { 0,  7, hex_button,  -9,  1,  70 },
  { 0,  8, hex_button, -10,  4,  80 },
  { 0,  9, hex_button,  -9,  3,  90 },
  { 0, 10, hex_button, -11,  5, 100 },
  { 0, 11, hex_button,  -9,  5, 110 },
  { 0, 12, hex_button, -10,  6, 120 },
  { 0, 13, hex_button,  -9,  7, 130 },
  { 0, 14, hardwired,  N_A,N_A, N_A }, // off = FW 1.0/1.1, on = FW 1.2
  { 0, 15, unused_pin, N_A,N_A, N_A },
  //col mux switch type  x   y  pxl
  { 1,  0, hex_button,  -8, -6,   1 },
  { 1,  1, hex_button,  -7, -5,  11 },
  { 1,  2, hex_button,  -8, -4,  21 },
  { 1,  3, hex_button,  -7, -3,  31 },
  { 1,  4, hex_button,  -8, -2,  41 },
  { 1,  5, hex_button,  -7, -1,  51 },
  { 1,  6, hex_button,  -8,  0,  61 },
  { 1,  7, hex_button,  -7,  1,  71 },
  { 1,  8, hex_button,  -8,  2,  81 },
  { 1,  9, hex_button,  -7,  3,  91 },
  { 1, 10, hex_button,  -8,  4, 101 },
  { 1, 11, hex_button,  -7,  5, 111 },
  { 1, 12, hex_button,  -8,  6, 121 },
  { 1, 13, hex_button,  -7,  7, 131 },
  { 1, 14, unused_pin, N_A,N_A, N_A },
  { 1, 15, unused_pin, N_A,N_A, N_A },
  //col mux switch type  x   y  pxl
  { 2,  0, hex_button,  -6, -6,   2 },
  { 2,  1, hex_button,  -5, -5,  12 },
  { 2,  2, hex_button,  -6, -4,  22 },
  { 2,  3, hex_button,  -5, -3,  32 },
  { 2,  4, hex_button,  -6, -2,  42 },
  { 2,  5, hex_button,  -5, -1,  52 },
  { 2,  6, hex_button,  -6,  0,  62 },
  { 2,  7, hex_button,  -5,  1,  72 },
  { 2,  8, hex_button,  -6,  2,  82 },
  { 2,  9, hex_button,  -5,  3,  92 },
  { 2, 10, hex_button,  -6,  4, 102 },
  { 2, 11, hex_button,  -5,  5, 112 },
  { 2, 12, hex_button,  -6,  6, 122 },
  { 2, 13, hex_button,  -5,  7, 132 },
  { 2, 14, unused_pin, N_A,N_A, N_A },
  { 2, 15, unused_pin, N_A,N_A, N_A },
  //col mux switch type  x   y  pxl
  { 3,  0, hex_button,  -4, -6,   3 },
  { 3,  1, hex_button,  -3, -5,  13 },
  { 3,  2, hex_button,  -4, -4,  23 },
  { 3,  3, hex_button,  -3, -3,  33 },
  { 3,  4, hex_button,  -4, -2,  43 },
  { 3,  5, hex_button,  -3, -1,  53 },
  { 3,  6, hex_button,  -4,  0,  63 },
  { 3,  7, hex_button,  -3,  1,  73 },
  { 3,  8, hex_button,  -4,  2,  83 },
  { 3,  9, hex_button,  -3,  3,  93 },
  { 3, 10, hex_button,  -4,  4, 103 },
  { 3, 11, hex_button,  -3,  5, 113 },
  { 3, 12, hex_button,  -4,  6, 123 },
  { 3, 13, hex_button,  -3,  7, 133 },
  { 3, 14, unused_pin, N_A,N_A, N_A },
  { 3, 15, unused_pin, N_A,N_A, N_A },
  //col mux switch type  x   y  pxl
  { 4,  0, hex_button,  -2, -6,   4 },
  { 4,  1, hex_button,  -1, -5,  14 },
  { 4,  2, hex_button,  -2, -4,  24 },
  { 4,  3, hex_button,  -1, -3,  34 },
  { 4,  4, hex_button,  -2, -2,  44 },
  { 4,  5, hex_button,  -1, -1,  54 },
  { 4,  6, hex_button,  -2,  0,  64 },
  { 4,  7, hex_button,  -1,  1,  74 },
  { 4,  8, hex_button,  -2,  2,  84 },
  { 4,  9, hex_button,  -1,  3,  94 },
  { 4, 10, hex_button,  -2,  4, 104 },
  { 4, 11, hex_button,  -1,  5, 114 },
  { 4, 12, hex_button,  -2,  6, 124 },
  { 4, 13, hex_button,  -1,  7, 134 },
  { 4, 14, unused_pin, N_A,N_A, N_A },
  { 4, 15, unused_pin, N_A,N_A, N_A },
  //col mux switch type  x   y  pxl
  { 5,  0, hex_button,   0, -6,   5 },
  { 5,  1, hex_button,   1, -5,  15 },
  { 5,  2, hex_button,   0, -4,  25 },
  { 5,  3, hex_button,   1, -3,  35 },
  { 5,  4, hex_button,   0, -2,  45 },
  { 5,  5, hex_button,   1, -1,  55 },
  { 5,  6, hex_button,   0,  0,  65 },
  { 5,  7, hex_button,   1,  1,  75 },
  { 5,  8, hex_button,   0,  2,  85 },
  { 5,  9, hex_button,   1,  3,  95 },
  { 5, 10, hex_button,   0,  4, 105 },
  { 5, 11, hex_button,   1,  5, 115 },
  { 5, 12, hex_button,   0,  6, 125 },
  { 5, 13, hex_button,   1,  7, 135 },
  { 5, 14, unused_pin, N_A,N_A, N_A },
  { 5, 15, unused_pin, N_A,N_A, N_A },
  //col mux switch type  x   y  pxl
  { 6,  0, hex_button,   2, -6,   6 },
  { 6,  1, hex_button,   3, -5,  16 },
  { 6,  2, hex_button,   2, -4,  26 },
  { 6,  3, hex_button,   3, -3,  36 },
  { 6,  4, hex_button,   2, -2,  46 },
  { 6,  5, hex_button,   3, -1,  56 },
  { 6,  6, hex_button,   2,  0,  66 },
  { 6,  7, hex_button,   3,  1,  76 },
  { 6,  8, hex_button,   2,  2,  86 },
  { 6,  9, hex_button,   3,  3,  96 },
  { 6, 10, hex_button,   2,  4, 106 },
  { 6, 11, hex_button,   3,  5, 116 },
  { 6, 12, hex_button,   2,  6, 126 },
  { 6, 13, hex_button,   3,  7, 136 },
  { 6, 14, unused_pin, N_A,N_A, N_A },
  { 6, 15, unused_pin, N_A,N_A, N_A },
  //col mux switch type  x   y  pxl
  { 7,  0, hex_button,   4, -6,   7 },
  { 7,  1, hex_button,   5, -5,  17 },
  { 7,  2, hex_button,   4, -4,  27 },
  { 7,  3, hex_button,   5, -3,  37 },
  { 7,  4, hex_button,   4, -2,  47 },
  { 7,  5, hex_button,   5, -1,  57 },
  { 7,  6, hex_button,   4,  0,  67 },
  { 7,  7, hex_button,   5,  1,  77 },
  { 7,  8, hex_button,   4,  2,  87 },
  { 7,  9, hex_button,   5,  3,  97 },
  { 7, 10, hex_button,   4,  4, 107 },
  { 7, 11, hex_button,   5,  5, 117 },
  { 7, 12, hex_button,   4,  6, 127 },
  { 7, 13, hex_button,   5,  7, 137 },
  { 7, 14, unused_pin, N_A,N_A, N_A },
  { 7, 15, unused_pin, N_A,N_A, N_A },
  //col mux switch type  x   y  pxl
  { 8,  0, hex_button,   6, -6,   8 },
  { 8,  1, hex_button,   7, -5,  18 },
  { 8,  2, hex_button,   6, -4,  28 },
  { 8,  3, hex_button,   7, -3,  38 },
  { 8,  4, hex_button,   6, -2,  48 },
  { 8,  5, hex_button,   7, -1,  58 },
  { 8,  6, hex_button,   6,  0,  68 },
  { 8,  7, hex_button,   7,  1,  78 },
  { 8,  8, hex_button,   6,  2,  88 },
  { 8,  9, hex_button,   7,  3,  98 },
  { 8, 10, hex_button,   6,  4, 108 },
  { 8, 11, hex_button,   7,  5, 118 },
  { 8, 12, hex_button,   6,  6, 128 },
  { 8, 13, hex_button,   7,  7, 138 },
  { 8, 14, unused_pin, N_A,N_A, N_A },
  { 8, 15, unused_pin, N_A,N_A, N_A },
  //col mux switch type  x   y  pxl
  { 9,  0, hex_button,   8, -6,   9 },
  { 9,  1, hex_button,   9, -5,  19 },
  { 9,  2, hex_button,   8, -4,  29 },
  { 9,  3, hex_button,   9, -3,  39 },
  { 9,  4, hex_button,   8, -2,  49 },
  { 9,  5, hex_button,   9, -1,  59 },
  { 9,  6, hex_button,   8,  0,  69 },
  { 9,  7, hex_button,   9,  1,  79 },
  { 9,  8, hex_button,   8,  2,  89 },
  { 9,  9, hex_button,   9,  3,  99 },
  { 9, 10, hex_button,   8,  4, 109 },
  { 9, 11, hex_button,   9,  5, 119 },
  { 9, 12, hex_button,   8,  6, 129 },
  { 9, 13, hex_button,   9,  7, 139 },
  { 9, 14, unused_pin, N_A,N_A, N_A },
  { 9, 15, unused_pin, N_A,N_A, N_A },
};