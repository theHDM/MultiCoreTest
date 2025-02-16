#pragma once
#include <stdint.h>

double frequency_after_pitch_bend(
  const double&   base_frequency, 
  const int16_t&  global_pitch_bend, 
  const uint8_t&  pitch_bend_range_in_semitones) {
  return base_frequency 
       * exp2(ldexp(global_pitch_bend 
       * pitch_bend_range_in_semitones / 3.d, 
        -15));
}