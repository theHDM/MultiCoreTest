#pragma once
/* 
 *  This is the background code for direct digital synthesis
 *  of synthesizer sounds for the HexBoard.
 *  This code is run on core1 in the background, calculates
 *  one audio sample every polling period and sends to
 *  designated pins via PWM. When core1 is inactive, core0
 *  may update the object with music messages such as note-on,
 *  waveform change, mod wheel, volume, pressure, etc.
 */
#include <stdint.h>
#include <cmath>
#include <array>
#include <vector>
#include <functional>
#include <algorithm> // std::find

#include "hardware/pwm.h"       // library of code to access the processor's built in pulse wave modulation features
#include "pico/util/queue.h"
#include "pico/time.h"
#include "config.h" // import hardware config constants

enum class ADSR_Phase {
  off, attack, decay, sustain, release
};

struct Synth_Voice {
  uint8_t wavetable[256];
  uint32_t pitch_as_increment;
  uint8_t base_volume;
  uint32_t attack; // express in # of samples
  uint32_t decay;  // express in # of samples
  uint8_t sustain; // express from 0-255
  uint32_t release; // express in # of samples

  uint32_t loop_counter;
  uint32_t envelope_counter; // used to apply envelope
  ADSR_Phase phase;  
  uint32_t attack_inverse;
  uint32_t decay_inverse;
  uint32_t release_inverse;
  uint8_t  ownership;

  // define a series of setter functions for core0
  // which will block if core1 is trying to calculate
  // the next sample.
  void update_wavetable(uint8_t* tbl) {
    while (ownership == 1) {}
    ownership = 0;
    for (size_t i = 0; i <  256; ++i) {
      wavetable[i] = tbl[i];
    }
    ownership = -1;
  }
  void update_pitch(uint32_t& increment) {
    while (ownership == 1) {}
    ownership = 0;
    pitch_as_increment = increment;
    ownership = -1;
  }
  void update_base_volume(uint8_t& volume) {
    while (ownership == 1) {}
    ownership = 0;
    base_volume = volume;
    ownership = -1;
  }
  void update_envelope(uint32_t& a, uint32_t& d, uint8_t& s, uint32_t& r) {
    while (ownership == 1) {}
    ownership = 0;
    attack = a;
    decay = d;
    sustain = s;
    release = r;
    attack_inverse  = !a ? 0 : 0xFFFFFFFF / a;
    decay_inverse   = !d ? 0 : ((256 - s) << 24) / d;
    release_inverse = !r ? 0 : 0xFFFFFFFF / r;
    ownership = -1;
  }
  void note_on() {
    while (ownership == 1) {}
    ownership = 0;
    envelope_counter = 0;
    phase = ADSR_Phase::attack;
    ownership = -1;
  }
  void note_off() {
    while (ownership == 1) {}
    ownership = 0;
    envelope_counter = 0;
    phase = ADSR_Phase::release;
    ownership = -1;
  }
  // called from core 1 only
  uint32_t next_sample() {
    while (ownership == 0) {}
    ownership = 1;
    loop_counter += pitch_as_increment;
    uint8_t sample = wavetable[loop_counter >> 24];

    uint8_t envelope_level = 0;
    switch (phase) {
      case ADSR_Phase::attack:
        if (envelope_counter == attack) {
          phase = ADSR_Phase::decay;
          envelope_counter = 0;
          envelope_level = 255;
        } else {
          ++envelope_counter;
          envelope_level = (envelope_counter * attack_inverse) >> 24; 
        }
        break;
      case ADSR_Phase::decay:
        if (envelope_counter >= decay) {
          phase = ADSR_Phase::sustain;
          envelope_counter = 0;
          envelope_level = sustain;
        } else {
          ++envelope_counter;
          envelope_level = 255 - ((envelope_counter * decay_inverse) >> 24); 
        }
        break;
      case ADSR_Phase::sustain:
        envelope_level = sustain;
      case ADSR_Phase::release:
        if (envelope_counter == release) {
          phase = ADSR_Phase::off;
          envelope_level = 0;
        } else {
          ++envelope_counter;
          envelope_level = 255 - ((envelope_counter * release_inverse) >> 24); 
        }
    }
    ownership = -1;
    return sample * base_volume * envelope_level;
  }

};

bool on_callback_synth(repeating_timer *t);

class hexBoard_Synth_Object {
protected:
  bool active;
  std::array<Synth_Voice, synth_polyphony_limit> voice;
  std::vector<uint8_t> pins;
  bool pin_status[GPIO_pin_count];
  pwm_config cfg;
  uint32_t        polling_frequency;
  repeating_timer polling_timer;
  uint8_t         ownership;

  void internal_set_pin(uint8_t pin, bool activate) {
    auto n = std::find(pins.begin(), pins.end(), pin);
    if (n == pins.end()) pins.emplace_back(pin);
      pin_status[pin] = activate;
  }

public:
  hexBoard_Synth_Object(const uint8_t* pins, size_t count, uint32_t arg_poll_freq)
  : active(false), polling_frequency(arg_poll_freq) {
    for (size_t i = 0; i < GPIO_pin_count; ++i) {
      pin_status[i] = false;
    }
    for (size_t i = 0; i < count; ++i) {
      internal_set_pin(pins[i], true);
    }
  }
  void start() {active = true;}
  void stop() {active = false;}

  Synth_Voice& ch(uint8_t c) {
    return voice[c-1];
  }

  void set_pin(uint8_t pin, bool activate) {
    while (ownership == 1) {}
    ownership = 0;
    internal_set_pin(pin, activate);
    ownership = -1;
  }

  void poll() {
    if (!active) return;
    uint8_t voiceCount = 0;
    uint32_t mixLevels = 0;
    uint8_t mixDown = 0;

    while (ownership == 0) {}
    for (auto& v : voice) {
      if (v.phase == ADSR_Phase::off) continue;
      if (!v.base_volume) continue;
      ++voiceCount;
      mixLevels += v.next_sample();
    }
    // 24 bits per voice
    // attenuation factor = 8 bits = 255 / sqrt(max_poly * actual_poly)
    uint8_t attenuation[] = {255,90,64,52,45,40,37,34,32};
    mixLevels *= attenuation[voiceCount];
    mixDown = mixLevels >> 24;
    ownership = 1;
    for (size_t i = 0; i < pins.size(); ++i) {
      pwm_set_gpio_level(pins[i], (pin_status[pins[i]]) ? mixDown : 0);
    }
    ownership = -1;
  }

  void begin(alarm_pool_t *alarm_pool) {
    cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 1.0f);
    pwm_config_set_wrap(&cfg, (1u << 8) - 2);
    pwm_config_set_phase_correct(&cfg, true);
    for (size_t i = 0; i < pins.size(); ++i) {
      uint8_t p = pins[i];
      uint8_t s = pwm_gpio_to_slice_num(p);
      gpio_set_function(p, GPIO_FUNC_PWM);    // set that pin as PWM
      pwm_init(s, &cfg, true);                // configure and start!
      pwm_set_gpio_level(p, 0);               // initialize at zero to prevent whining sound
    }
    // enter a negative timer value here because the poll
    // should occur X microseconds after the routine starts
    alarm_pool_add_repeating_timer_us(
      alarm_pool, 
      - polling_frequency,
      on_callback_synth,
      this, 
      &polling_timer
    );
    start();
  }
};

bool on_callback_synth(repeating_timer *t) {
  static_cast<hexBoard_Synth_Object*>(t->user_data)->poll();
  return true;
}

// core0 uses a queue to determine
// how to assign voices to new notes
queue_t open_synth_channel_queue;
void reset_synth_channel_queue() {
  uint8_t discard;
  while (!queue_is_empty(&open_synth_channel_queue)) {
    queue_try_remove(&open_synth_channel_queue, &discard);
  }
  uint8_t i = 1;
  while (i <= synth_polyphony_limit) {
    bool success = queue_try_add(&open_synth_channel_queue, &i);
    i += (uint8_t)success;
  }

}
void initialize_synth_channel_queue() {
  queue_init(&open_synth_channel_queue, sizeof(uint8_t), synth_polyphony_limit);
  reset_synth_channel_queue();
}