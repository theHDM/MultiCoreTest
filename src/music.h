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

uint32_t frequency_to_interval(
  const double&   frequency, 
  const uint64_t& interval_in_uS) {
  return lround(ldexp(frequency * interval_in_uS / 1000000.d, 32));
}

uint8_t iso226(const double& f) {
  // a very crude implementation of ISO 226 equal loudness curves
  //   Hz dB  Amplitude ~ sqrt(10^(dB/10))
  //  200 +0  255
  //  800 -3  191
  // 1500 +0  255
  // 3250 -6  127
  // 5000 +0  255
  if (f <      8.0) return 0;
  if (f <    200.0) return 255;
  if (f <   1500.0) return 191 + ldexp(abs(f- 800) /  700.d, 6);
  if (f <   5000.0) return 127 + ldexp(abs(f-3250) / 1750.d, 7);
  if (f < highest_MIDI_note_Hz) return 255;
  return 0;
}

