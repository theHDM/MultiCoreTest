#pragma once
#include "hardware/pwm.h"       // library of code to access the processor's built in pulse wave modulation features
#include <algorithm> // std::find
#include <stdint.h>
#include "pico/util/queue.h"
#include <array>
#include <vector>


queue_t open_synth_channel_queue;

void reset_synth_channel_queue() {
  uint8_t discard;
  while (!queue_is_empty(&open_synth_channel_queue)) {
    queue_try_remove(&open_synth_channel_queue, &discard);
  }
  uint8_t i = 1;
  while (i <= synth_polyphony_limit) {
    i += queue_try_add(&open_synth_channel_queue, &i);
  }
}

void initialize_synth_channel_queue() {
  queue_init(&open_synth_channel_queue, sizeof(uint8_t), synth_polyphony_limit);
  reset_synth_channel_queue();
}

int8_t sine8[] =
  {   0,   3,   6,   9,  12,  16,  19,  22,  25,  28,  31,  34,  37,  40,  43,  46
  ,  49,  51,  54,  57,  60,  63,  65,  68,  71,  73,  76,  78,  81,  83,  85,  88
  ,  90,  92,  94,  96,  98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116
  , 117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127
  , 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118
  , 117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100,  98,  96,  94,  92
  ,  90,  88,  85,  83,  81,  78,  76,  73,  71,  68,  65,  63,  60,  57,  54,  51
  ,  49,  46,  43,  40,  37,  34,  31,  28,  25,  22,  19,  16,  12,   9,   6,   3
  ,   0,  -3,  -6,  -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46
  , -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88
  , -90, -92, -94, -96, -98,-100,-102,-104,-106,-107,-109,-111,-112,-113,-115,-116
  ,-117,-118,-120,-121,-122,-122,-123,-124,-125,-125,-126,-126,-126,-127,-127,-127
  ,-127,-127,-127,-127,-126,-126,-126,-125,-125,-124,-123,-122,-122,-121,-120,-118
  ,-117,-116,-115,-113,-112,-111,-109,-107,-106,-104,-102,-100, -98, -96, -94, -92
  , -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51
  , -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12,  -9,  -6,  -3};

struct ADSR_Envelope {
  uint8_t attack; // express in # of samples
  uint8_t decay;  // express in # of samples
  uint8_t sustain; // express from 0-255
  uint8_t release; // express in # of samples
};

struct Hybrid_Parameters {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d_minus_one;
  uint16_t ab;
  uint16_t cd;
};

enum {
  wave_shape_hybrid   = 0;
  wave_shape_square   = 1;
  wave_shape_saw      = 2;
  wave_shape_triangle = 3;
};

const float hybrid_transition_square   =  220.f;
const float hybrid_transition_saw_low  =  440.f;
const float hybrid_transition_saw_high =  880.f;
const float hybrid_transition_triangle = 1760.f;

void calculate_hybrid_params(
  Hybrid_Parameters& param,
  double& freq,
  uint8_t& shape_type,
  uint8_t& modulation) {

  uint8_t model_shape = shape_type;
  float transition_pct = 0.f;
  if (shape_type == wave_shape_hybrid) {
    if (freq < hybrid_transition_saw_low) {
      model_shape = wave_shape_square;
      if (freq > hybrid_transition_square) {
        transition_pct =  (freq - transition_square) 
          / (transition_saw_low - transition_square);
      }
    } else if (freq > hybrid_transition_saw_high) {
      model_shape = wave_shape_triangle;
      if (freq < hybrid_transition_triangle) {
        transition_pct = 
            (transition_triangle - freq) 
          / (transition_triangle - transition_saiangle);
      }
    } else {
      model_shape = wave_shape_saw;
    }
  }
  uint8_t duty_cycle = 127 - modulation;
  switch (model_shape) {
    case wave_shape_square: {
      param.a = (duty_cycle * (1.f - transition_pct)) - 1;
      param.b = (duty_cycle * (1.f + transition_pct));
      param.c = (duty_cycle << 1) - 1;
      param.d_minus_one = param.c;
    }
    case wave_shape_saw: {
      param.a =  0;
      param.b = (duty_cycle << 1) - 1;
      param.c = (duty_cycle << 1) - 1;
      param.d_minus_one = param.c;
    }
    case wave_shape_triangle: {
      param.a =  0;
      param.b =  duty_cycle * (2.f - transition_pct);
      param.c =  param.b;
      param.d_minus_one = (duty_cycle << 1) - 1;
    }
  }
  param.ab = (param.b == param.a + 1) ? 0 : 65536 / (param.b - 1 - param.a);
  param.cd = (param.d_minus_one == param.c) 
      ? 0 : 65536 / (param.d_minus_one - param.c);
}


struct Synth_Generation {
  uint32_t pitch_as_increment;
  uint8_t base_volume;
  uint8_t waveform_ID;     // 254 = sine, 255 = linear, others = lookup
  ADSR_Envelope envelope;
  union {
    Hybrid_Parameters hybrid_params; // linear waves
    uint8_t harmonics[oscillator_harmonic_limit];   // sine waves
  };
};

enum {
  synth_msg_note_off = 0,
  synth_msg_note_on = 1,
  synth_msg_update_pitch = 2,
  synth_msg_update_volume = 4,
  synth_msg_update_waveform = 8,
  synth_msg_update_hybrid_only = 16,
  synth_msg_turn_pin_on = 32,
  synth_msg_turn_pin_off = 64
};

enum {
  synth_phase_off = 0,
  synth_phase_attack = 1,
  synth_phase_decay = 2,
  synth_phase_sustain = 3,
  synth_phase_release = 4
};

const uint8_t sine_generator = 254;
const uint8_t hybrid_generator = 255;

struct Synth_Msg {
  uint8_t command;
  uint8_t item_number;
  Synth_Generation synth_generation;
};

queue_t synth_msg_queue;
Synth_Msg synth_msg_from_core_0;
Synth_Msg synth_msg_to_core_1;

struct Synth_Voice {
  Synth_Generation generator;
  uint32_t loop_counter;
  uint32_t sine_normalization;
  
  uint8_t  phase; // off, attack, decay, sustain, release
  uint32_t envelope_counter; // used to apply envelope
  uint16_t attack_inverse;
  uint16_t decay_inverse;
  uint16_t release_inverse;

  void pass_synth_generator(uint8_t& cmd, Synth_Generation& arg_synth_gen) {
    if (cmd == synth_msg_note_off) {
      phase = synth_phase_release;
      envelope_counter = 0;
      return;
    }
    if (cmd & synth_msg_update_pitch) {
      generator.pitch_as_increment = arg_synth_gen.pitch_as_increment;
    }
    if (cmd & synth_msg_update_volume) {
      generator.base_volume = arg_synth_gen.base_volume;
    }
    if (cmd & synth_msg_update_waveform) {
      generator.waveform_ID = arg_synth_gen.waveform_ID;
      generator.envelope = arg_synth_gen.envelope;
      attack_inverse  = ( !generator.envelope.attack  ? 0 :
                          (1u << 16) / generator.envelope.attack);
      decay_inverse  =  ( !generator.envelope.decay   ? 0 :
                          ((256 - generator.envelope.sustain) << 8)
                                        / generator.envelope.decay);
      release_inverse = ( !generator.envelope.release ? 0 :
                          (1u << 16) / generator.envelope.release);
      if (generator.waveform_ID = sine_generator) {
        uint16_t amplitude = 0;
        for (uint8_t h = 0; h < oscillator_harmonic_limit; ++h) {
          generator.harmonics[h] = arg_synth_gen.harmonics[h];
          amplitude += generator.harmonics[h];
        }
        sine_normalization = (!amplitude ? 0 : 65536 / amplitude);
      }
      if (generator.waveform_ID = hybrid_generator) {
        generator.hybrid_params = arg_synth_gen.hybrid_params;
      }
    }
    if (cmd & synth_msg_update_hybrid_only) {
      generator.hybrid_params = arg_synth_gen.hybrid_params;
    }
    if (cmd & synth_msg_note_on) {
      phase = synth_phase_attack;
      envelope_counter = 0;
    }
  }

  uint32_t next_sample() {
    if (phase = synth_phase_off) return 0;
    if (!generator.base_volume) return 0;
    loop_counter += generator.pitch_as_increment;
    uint16_t sample = 0;
    switch (generator.waveform_ID) {
      case sine_generator: { 
        uint32_t sine_counter = 0;
        for (uint8_t h = 0; h < oscillator_harmonic_limit; ++h) {
          sine_counter += loop_counter;
          sample += (static_cast<uint16_t>(
            (static_cast<int32_t>(sine8[sine_counter >> 24])
            * generator.harmonics[h])
            + (1u << 15)) >> 8);
        }
        sample *= sine_normalization;
        break;
      }
      case hybrid_generator: {
        uint8_t t = (loop_counter >> 24);
        if (t > generator.hybrid_params.a) {
          if (t < generator.hybrid_params.b) {
            sample = generator.hybrid_params.ab 
              * (t - generator.hybrid_params.a);
          } else if (t > generator.hybrid_params.c) {
            if (t < generator.hybrid_params.d_minus_one) {
              sample = generator.hybrid_params.cd 
              * (1 + (generator.hybrid_params.d_minus_one - t));
            }
          } else {
            sample = 65535;
          }
        }
        break;
      }
      case 0: // you can add your own loops here
        break;
      default:
        break;
    }
    uint8_t envelope_level = 0;
    switch (phase) {
      case synth_phase_attack:
        if (envelope_counter == generator.envelope.attack) {
          phase = synth_phase_decay;
          envelope_counter = 0;
          envelope_level = 255;
        } else {
          ++envelope_counter;
          envelope_level = (envelope_counter * attack_inverse) >> 8; 
        }
        break;
      case synth_phase_decay:
        if (envelope_counter >= generator.envelope.decay) {
          phase = synth_phase_sustain;
          envelope_counter = 0;
          envelope_level = generator.envelope.sustain;
        } else {
          ++envelope_counter;
          envelope_level = 255 - ((envelope_counter * decay_inverse) >> 8); 
        }
        break;
      case synth_phase_sustain:
        envelope_level = generator.envelope.sustain;
      case synth_phase_release:
        if (envelope_counter == generator.envelope.release) {
          phase = synth_phase_off;
          envelope_level = 0;
        } else {
          ++envelope_counter;
          envelope_level = 255 - ((envelope_counter * release_inverse) >> 8); 
        }
    }
    // sample: 16bits
    // volume: 8bits
    // envelope: 8bits
    return sample * generator.base_volume * envelope_level;
  }
};

class hexBoard_Synth_Object {
  protected:
    bool active;
    std::array<Synth_Voice, synth_polyphony_limit> voice;
    std::vector<uint8_t> pins;
    bool pin_status[GPIO_pin_count];
    pwm_config cfg;

    void set_pin(uint8_t pin, bool activate) {
      auto n = std::find(pins.begin(), pins.end(), pin);
      if (n == pins.end()) pins.emplace_back(pin);
        pin_status[pin] = activate;
    }
  public:
    hexBoard_Synth_Object(const uint8_t* pins, size_t count)
    : active(false) {
      for (size_t i = 0; i < GPIO_pin_count; ++i) {
        pin_status[i] = false;
      }
      for (size_t i = 0; i < count; ++i) {
        set_pin(pins[i], true);
      }
    }

    void interpret_synth_msg(Synth_Msg& msg) {
      if (msg.command & synth_msg_turn_pin_off) {
        set_pin(msg.item_number, false);
      } else if (msg.command & synth_msg_turn_pin_on) {
        set_pin(msg.item_number, true);
      } else {
        voice[msg.item_number]
          .pass_synth_generator(msg.command, msg.synth_generation);
      }
    }

    void begin() {
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
      initialize_synth_channel_queue();
      active = true;
    }

    void poll() {
      if (!active) return;
      // sample = 32 bits
      // attenuation = 6 bits
      // total = 38 bits
      // downsampling 30 bits to 8 bit audio
      uint8_t voiceCount = 0;
      uint64_t mixLevels = 0;
      uint8_t mixDown = 0;
      for (auto& v : voice) {
        if (v.phase == synth_phase_off) continue;
        ++voiceCount;
        mixLevels += v.next_sample();
      }
      uint8_t attenuation[] = {64,24,17,14,12,11,10,9,8};
      mixLevels *= attenuation[voiceCount];
      mixDown = mixLevels >> 30;
      for (size_t i = 0; i < pins.size(); ++i) {
        pwm_set_gpio_level(pins[i], (pin_status[pins[i]]) ? mixDown : 0);
      }
    }
};