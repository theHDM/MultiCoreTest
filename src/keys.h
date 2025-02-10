#pragma once
#include <array>    // structure for managing info for each key
#include <stdint.h>
#include <Wire.h>   // needed to set pin states
#include "timing.h" // querying system clock
#include "config.h" // import hardware config constants
#include "pico/util/queue.h"

struct Calibration_Parameters {
  uint16_t low_level;
  uint16_t high_level;
  uint8_t resolution_bits;
};

Calibration_Parameters default_analog_calibration = {
  .low_level = default_analog_calibration_down,
  .high_level = default_analog_calibration_up,
  .resolution_bits = 7
};

Calibration_Parameters default_digital_calibration = {
  .low_level = LOW,
  .high_level = HIGH,
  .resolution_bits = 7
};

struct Calibration_Msg {
  bool has_calibration_data;   // true = calibration data; false = calibration mode
  union {
    bool pressure_mode; // (1 = pressure, 0 = velocity)
    uint8_t switch_number;
  }; 
  Calibration_Parameters calibration_parameters;
};

queue_t calibration_queue;
Calibration_Msg calibration_msg_from_core_0;
Calibration_Msg calibration_msg_to_core_1;

struct Calibration {
  uint16_t high;
  uint16_t range;
  uint8_t  resolution_bits;
  uint8_t  floating_bits;
  uint8_t  ceiling_value;
  uint16_t invert_range;
  void import_calibration_parameters(Calibration_Parameters& input) {
    high            = input.high_level;
    range           = high - input.low_level;
    resolution_bits = input.resolution_bits;
    ceiling_value   = (1u << resolution_bits) - 1;
    floating_bits   = 16 - resolution_bits;
    invert_range    = (ceiling_value << floating_bits) / range;
  }
  uint8_t calculate_level(uint16_t input, bool read_pressure) {
    if (input >= high)         return 0;
    if (input <= high - range) return ceiling_value;
    if (read_pressure)         return (invert_range * (high - input)) >> floating_bits;
                               return 1;
  }
};

struct Key_Msg {
  uint64_t timestamp;
  uint8_t switch_number;
  uint8_t level;
};

queue_t key_press_queue;
Key_Msg key_msg_to_core_0;
Key_Msg key_msg_from_core_1;

struct hexBoard_Key_Object {
  const uint8_t *mux; // reference to existing constant
  const uint8_t *col; // reference to existing constant
  const bool *analog;  // reference to existing constant
  uint8_t m_ctr; // mux counter
  uint8_t m_val; // mux value
  bool active;
  bool send_pressure;
  std::array<uint8_t, keys_count> previous_level;
  std::array<Calibration, keys_count> calibration;
  std::array<bool, keys_count> was_last_read_successful;
    
  hexBoard_Key_Object(const uint8_t *arrM, const uint8_t *arrC, const bool *arrA)
  : mux(arrM), col(arrC), analog(arrA), m_ctr(0), m_val(0), active(false)
  , send_pressure(false) {
    for (size_t i = 0; i < mux_pins_count; ++i) {
      pinMode(*(mux + i), OUTPUT);
      digitalWrite(*(mux + i), 0);
    }
    for (size_t i = 0; i < col_pins_count; ++i) {
      if (*(analog + i)) {
        pinMode(*(col + i), INPUT);
      } else {
        pinMode(*(col + i), INPUT_PULLUP); 
      }
      for (size_t j = 0; j < mux_channels_count; ++j) {
        if (*(analog + i)) {
          calibration[linear_index(j,i)]
            .import_calibration_parameters(default_analog_calibration);
        } else { 
          calibration[linear_index(j,i)]
            .import_calibration_parameters(default_digital_calibration);
        }    
      }
    }
  }

  void read_calibration_msg(Calibration_Msg& msg) {
    if (msg.has_calibration_data) {
      calibration[msg.switch_number]
        .import_calibration_parameters(msg.calibration_parameters);
    } else {
      send_pressure = msg.pressure_mode;
    }
  }

  void start() {
    active = true;
  }
  
  void stop() {
    active = false;
  }

  void poll() {
    // don't try to send messages until manual begin is set 
    if (!active) return;
    uint8_t level;
    uint8_t index;
    bool attempt_to_send;
    for (size_t i = 0; i < col_pins_count; ++i) {
      index = linear_index(m_val, i);
      if (!was_last_read_successful[index]) {
        level = previous_level[index];
        attempt_to_send = true;
      } else {
        level = calibration[index].calculate_level(
            *(analog + i) ? analogRead(*(col + i))
                          : digitalRead(*(col + i)),
          send_pressure
        );
        attempt_to_send = (level != previous_level[index]);
      }
      if (attempt_to_send) {
        key_msg_from_core_1.timestamp = now();
        key_msg_from_core_1.switch_number = index;
        key_msg_from_core_1.level = level;
        was_last_read_successful[index] = 
          queue_try_add(&key_press_queue, &key_msg_from_core_1);
        previous_level[index] = level;
      }
    }
    // this algorithm cycles through the multiplexer
    // by changing one bit at a time and still
    // making sure all permutations are reached
    if (++m_ctr == mux_channels_count) {m_ctr = 0;}
    size_t b = mux_pins_count - 1;
    for (size_t i = 0; i < b; ++i) {
      if ((m_ctr >> i) & 1) { b = i; }
    }
    m_val ^= (1 << b);
    digitalWrite(*(mux + b), (m_val >> b) & 1);
  }
};