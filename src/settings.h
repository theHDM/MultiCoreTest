#pragma once
#include <stdint.h> // import common definition of uint8_t
#include <array>

// use a running enum to identify settings by a number.
// this is useful for serializing (converting to bytes for file storage)
// and for processing menu callbacks efficiently (you can cast as integer)
// while also being very legible (settings[_settingName] returns correct array member)

// if you change the list of settings,
// make sure also to change:
// * defaults in this file
// * menu item creation in <menu.h>
// * onChg handlers in <menu.h>
// * applying settings in main

enum {
  _defaults, // is this the factory default set?
  _changed,  // has this changed since last save?
  _debug,    // send debug messages through serial port?
  _anchorX,  // X coordinate of hex assigned to the root
  _anchorY,  // Y coordinate of hex assigned to the root
  _anchorN, // MIDI note closest to anchor note
  _anchorC, // +/- cents
  _txposeS,   // # of transpose steps
  _txposeC,  // +/- cents, size of each transpose step
  _axisA,    // one of 6 directions
  _axisB,    // one of 6 directions
  _equaveJI, //
  _equaveC,  //
  _equaveN,  //
  _equaveD,  //
  _tuneSys,  //
  _eqDivs,   //
  _eqStepA,  //
  _eqStepB,  //
  _lgSteps,  //
  _smSteps,  //
  _lgStepA,  //
  _smStepA,  //
  _lgStepB,  //
  _smStepB,  //
  _lgToSmND, // rational number?
  _lgToSmR,  //
  _lgToSmN,  //
  _lgToSmD,  //
  _modeLgSm, //
  _JInumA,   //
  _JIdenA,   //
  _JInumB,   //
  _JIdenB,   //
  _scaleLck, //
  _animFPS,  //
  _palette,  //
  _animType, //
  _globlBrt, //
  _hueLoop,  //
  _tglWheel, //
  _whlMode,  //
  _mdSticky, //
  _pbSticky, //
  _vlSticky, //
  _mdSpeed,  //
  _pbSpeed,  //
  _vlSpeed,  //
  _rotInv,   //
  _rotDblCk, //
  _rotLongP, //
  _SStime,   //
  _MIDImode, //
  _MIDIusb,  //
  _MIDIjack, //
  _MPEzoneC, //
  _MPEzoneL, //
  _MPEzoneR, //
  _MPEpb,    //
  _MIDIorMT, //
  _MIDIpc,   //
  _MT32pc,   //
  _synthTyp, //
  _synthWav, //
  _synthEnv, //
  _synthVol, //
  _synthBuz, //
  _synthJac, //
  _settingSize // the largest index plus one 
};

// use regular enum, not enum class, to identify options.
// we need to be able to cast these to integers
// for the menu system to work.
enum {
  _tuneSys_normal,
  _tuneSys_equal,
  _tuneSys_lg_sm,
  _tuneSys_just
};
enum {
  _palette_rainbow,
  _palette_tiered,
  _palette_alternate
};
enum {
  _animType_none,
  _animType_star,
  _animType_splash,
  _animType_orbit,
  _animType_octave,
  _animType_by_note,
  _animType_beams,
  _animType_splash_reverse,
  _animType_star_reverse
};
enum {
  _globlBrt_max = 255,
  _globlBrt_high = 210,
  _globlBrt_mid = 180,
  _globlBrt_low = 150,
  _globlBrt_dim = 110,
  _globlBrt_dimmer = 70,
  _globlBrt_off = 0
};
enum {
  _MIDImode_standard,
  _MIDImode_MPE,
  _MIDImode_tuning_table,
  _MIDImode_2_point_oh
};
enum {
  _synthTyp_off,
  _synthTyp_mono,
  _synthTyp_arpeggio,
  _synthTyp_poly
};
enum {
  _synthWav_hybrid,
  _synthWav_square,
  _synthWav_saw,
  _synthWav_triangle,
  _synthWav_sine,
  _synthWav_strings,
  _synthWav_clarinet
};
enum {
  _synthEnv_none,
  _synthEnv_hit,
  _synthEnv_pluck,
  _synthEnv_strum,
  _synthEnv_slow,
  _synthEnv_reverse
};
enum {
  _GM_instruments,
  _MT32_instruments
};

// this is a simple kind of "variant" type.
// it holds the bit-wise representation of
// a numerical value, which you can then express
// as its associated boolean/int/float value at will
const size_t bytes_per_setting = 8;
union Setting_Value {
  bool b;
  int i;
  double d;
  std::array<uint8_t, bytes_per_setting> w;
};
using hexBoard_Setting_Array = std::array<Setting_Value, _settingSize>;

void load_factory_defaults_to(hexBoard_Setting_Array& refS, int version = 0) {
  // clear settings
  for (auto& each_setting : refS) {
    for (int each_byte = 0; each_byte < bytes_per_setting; ++each_byte) {
      each_setting.w[each_byte] = 0x00;  // reset values to zero
    }
  }
  refS[_defaults].b = true;   // if you are loading factory defaults, set to true
  refS[_changed].b  = true;   // flag as true so that it will save at first oppt'y
  refS[_debug].b    = true;   // send debug messages through Serial monitor
  refS[_anchorX].i  = 0;      // hex at {0,0} is the root note
  refS[_anchorY].i  = 0;      // hex at {0,0} is the root note
  refS[_anchorN].i  = 69;     // default to A4
  refS[_anchorC].d  = 0.0;    // default to A4=440Hz, no microtones
  refS[_txposeS].i  = 0;      // default no transposing
  refS[_txposeC].d  = 100.0;  // shift by 100 cents each transpose
  refS[_axisA].i    = 3;      // axis A is left
  refS[_axisB].i    = 1;      // axis B is up-right
  refS[_equaveJI].b = false;  // false = use cents value
  refS[_equaveC].d  = 1200.0; // 1200 cents = octave
  refS[_equaveN].i  = 2;      // 2/1 = octave
  refS[_equaveD].i  = 1;      // 2/1 = octave
  refS[_tuneSys].i  = 0;      // Equal divisions
  refS[_eqDivs].i   = 12;     // 12 steps
  refS[_eqStepA].i  = -2;     // one whole tone
  refS[_eqStepB].i  = 7;      // perfect fifth
  refS[_lgSteps].i  = 5;      // five whole tones
  refS[_smSteps].i  = 2;      // two semitones
  refS[_lgStepA].i  = -1;     // one whole tone 
  refS[_smStepA].i  = 0;      // no semitones
  refS[_lgStepB].i  = 3;      // three whole tones
  refS[_smStepB].i  = 1;      // and one semitone
  refS[_lgToSmND].b = true;   // 
  refS[_lgToSmR].d  = 2.0;    // whole tone is 2x the semitone
  refS[_lgToSmN].i  = 2;      // whole tone is 2x the semitone
  refS[_lgToSmD].i  = 1;      // whole tone is 2x the semitone
  refS[_modeLgSm].i = 1;      // mode 1 = major, mode 0 = lydian
  refS[_JInumA].i   = 8;      // 9/8 = JI major second
  refS[_JIdenA].i   = 9;      // 9/8 = JI major second
  refS[_JInumB].i   = 5;      // 5/4 = JI major third
  refS[_JIdenB].i   = 4;      // 5/4 = JI major third
  refS[_scaleLck].b = false;  // default to no scale lock
  refS[_animFPS].i  = 32;     // default to 32 frames per 1.048576 seconds
  refS[_palette].i  = _palette_rainbow;
  refS[_animType].i = _animType_none;
  refS[_globlBrt].i = (version >= 12 ? _globlBrt_dim : _globlBrt_mid);
  refS[_hueLoop].d  = 30.0;  // seconds for 360 degrees 
  refS[_tglWheel].b = false; // bool; 0 = mod, 1 = pb
  refS[_whlMode].b  = false; // standard vs. fine tune mode
  refS[_mdSticky].b = false;
  refS[_pbSticky].b = false;
  refS[_vlSticky].b = false;
  refS[_mdSpeed].i  = 8;
  refS[_pbSpeed].i  = 8; // scale by x128
  refS[_vlSpeed].i  = 8;		
  refS[_rotInv].b   = (version >= 12 ? true : false);
  refS[_rotDblCk].i = 500; // milliseconds
  refS[_rotLongP].i = 750; // milliseconds
  refS[_SStime].i   = 10; // seconds
  refS[_MIDImode].i = _MIDImode_standard;
  refS[_MIDIusb].b  = true;
  refS[_MIDIjack].b = (version >= 12);
  refS[_MPEzoneC].i = 2;
  refS[_MPEzoneL].i = 9;
  refS[_MPEzoneR].i = 11;
  refS[_MPEpb].i    = 48; // 2, 12, 24, 48, or 96
  refS[_MIDIorMT].i = _GM_instruments;
  refS[_MIDIpc].i   = 1; // program chg 1 - 128
  refS[_MT32pc].i   = 1;
  refS[_synthTyp].i = _synthTyp_poly;
  refS[_synthWav].i = _synthWav_hybrid;
  refS[_synthEnv].i = _synthEnv_none;
  refS[_synthVol].i = 96;
  refS[_synthBuz].b = true;
  refS[_synthJac].b = (version >= 12);
}