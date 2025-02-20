#pragma once
#include "OLED.h" // OLED graphics object
#include <GEM_u8g2.h>           // library of code to create menu objects on the B&W display
#include "config/enable-advanced-mode.h"
GEM_u8g2 menu(u8g2);

struct GEMItemPublic : public GEMItem {
  GEMItemPublic(const GEMItem& g) : GEMItem(g) {}
  byte getLinkedType() { return linkedType; }
};
struct GEMPagePublic : public GEMPage {
  int GUI_context;
  GEMAppearance derived_appearance;
  void initialize_appearance(byte titleRows_, byte menuItemsPerScreen_, byte valueMargin_) {
    derived_appearance = {
      GEM_POINTER_ROW, 
      menuItemsPerScreen_, 
      (byte)(_LG_FONT_HEIGHT + 2), 
      (byte)(4 + _SM_FONT_HEIGHT + (titleRows_ * (_LG_FONT_HEIGHT + 2))),
      (byte)(_OLED_WIDTH - _RIGHT_MARGIN - (valueMargin_ * _LG_FONT_WIDTH))
    };
    _appearance = &derived_appearance;
  }
  GEMPagePublic(const char* title_, int GUI_context_, byte titleRows_, byte menuItemsPerScreen_, byte valueMargin_)
  : GEMPage(title_), GUI_context(GUI_context_) {
    initialize_appearance(titleRows_, menuItemsPerScreen_, valueMargin_);
  }
  GEMPagePublic(const char* title_, int GUI_context_, byte titleRows_, byte menuItemsPerScreen_, byte valueMargin_, GEMPage& parentMenuPage_)
  : GEMPage(title_, parentMenuPage_), GUI_context(GUI_context_) {
    initialize_appearance(titleRows_, menuItemsPerScreen_, valueMargin_);
  }
  GEMPagePublic(const char* title_, int GUI_context_, byte titleRows_, byte menuItemsPerScreen_, byte valueMargin_, void (*on_exit_)())
  : GEMPage(title_, on_exit_), GUI_context(GUI_context_) {
    initialize_appearance(titleRows_, menuItemsPerScreen_, valueMargin_);
  }
};

void doNothing() {}
GEMPagePublic pgNoMenu("",_show_HUD + _show_custom_msg, 0,0,0, doNothing);

GEMPagePublic pgHome("Hexboard v1.1", _show_HUD, 0, 7, 0);
GEMPagePublic pgShowMsg("",_show_custom_msg, 6, 1, 0, pgHome);

GEMPagePublic pgLayout("Layout editor", _show_HUD, 0, 7, 0, pgHome);
GEMPagePublic pgTuning("Tuning system parameters", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 6, pgLayout);
GEMPagePublic pgGenerate("Select layout parameters", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 6, pgLayout);
GEMPagePublic pgAnchor("Select root key and tuning", _show_pixel_ID, 1, GEM_ITEMS_COUNT_AUTO, 6, pgGenerate);

GEMPagePublic pgPlayback("MIDI and synth settings", _show_HUD, 0, GEM_ITEMS_COUNT_AUTO, 3, pgHome);
GEMPagePublic pgMIDI("MIDI settings",   _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 19, pgPlayback);
GEMPagePublic pgMIDIout("MIDI out ports", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 3, pgMIDI);
GEMPagePublic pgSynth("Synth settings", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 8, pgPlayback);

GEMPagePublic pgHardware("Hardware settings", _show_HUD, 0, GEM_ITEMS_COUNT_AUTO, 0, pgHome);
GEMPagePublic pgRotary("Rotary settings",   _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 5, pgHardware);
GEMPagePublic pgCommand("Command key settings",   _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 5, pgHardware);
GEMPagePublic pgOLED("OLED settings",   _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 5, pgHardware);

GEMPagePublic pgSoftware("Software settings", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 7, pgHome);

GEMPagePublic pgSavePreset("Select slot to save in", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 7, pgHome);

GEMPagePublic pgLoadPreset("Select slot to load from", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 7, pgHome);

GEMPagePublic pgReboot("Power off options", _show_HUD, 0, GEM_ITEMS_COUNT_AUTO, 7, pgHome);
GEMPagePublic pgEdit("Edit current layout...", _show_pixel_ID, 1, GEM_ITEMS_COUNT_AUTO, 7, pgLayout);

GEMSelect dropdown_notes(128, (SelectOptionInt[]){
 {"C -1", 0},{"C#-1", 1},{"D -1", 2},{"Eb-1", 3}, {"E -1", 4},{"F -1", 5},
 {"F#-1", 6},{"G -1", 7},{"G#-1", 8},{"A -1", 9}, {"Bb-1",10},{"B -1",11},
 {"C 0", 12},{"C#0", 13},{"D 0", 14},{"Eb0", 15}, {"E 0", 16},{"F 0", 17},
 {"F#0", 18},{"G 0", 19},{"G#0", 20},{"A 0", 21}, {"Bb0", 22},{"B 0", 23},
 {"C 1", 24},{"C#1", 25},{"D 1", 26},{"Eb1", 27}, {"E 1", 28},{"F 1", 29},
 {"F#1", 30},{"G 1", 31},{"G#1", 32},{"A 1", 33}, {"Bb1", 34},{"B 1", 35},
 {"C 2", 36},{"C#2", 37},{"D 2", 38},{"Eb2", 39}, {"E 2", 40},{"F 2", 41},
 {"F#2", 42},{"G 2", 43},{"G#2", 44},{"A 2", 45}, {"Bb2", 46},{"B 2", 47},
 {"C 3", 48},{"C#3", 49},{"D 3", 50},{"Eb3", 51}, {"E 3", 52},{"F 3", 53},
 {"F#3", 54},{"G 3", 55},{"G#3", 56},{"A 3", 57}, {"Bb3", 58},{"B 3", 59},
 {"C 4", 60},{"C#4", 61},{"D 4", 62},{"Eb4", 63}, {"E 4", 64},{"F 4", 65},
 {"F#4", 66},{"G 4", 67},{"G#4", 68},{"A 4", 69}, {"Bb4", 70},{"B 4", 71},
 {"C 5", 72},{"C#5", 73},{"D 5", 74},{"Eb5", 75}, {"E 5", 76},{"F 5", 77},
 {"F#5", 78},{"G 5", 79},{"G#5", 80},{"A 5", 81}, {"Bb5", 82},{"B 5", 83},
 {"C 6", 84},{"C#6", 85},{"D 6", 86},{"Eb6", 87}, {"E 6", 88},{"F 6", 89},
 {"F#6", 90},{"G 6", 91},{"G#6", 92},{"A 6", 93}, {"Bb6", 94},{"B 6", 95},
 {"C 7", 96},{"C#7", 97},{"D 7", 98},{"Eb7", 99}, {"E 7",100},{"F 7",101},
 {"F#7",102},{"G 7",103},{"G#7",104},{"A 7",105}, {"Bb7",106},{"B 7",107},
 {"C 8",108},{"C#8",109},{"D 8",110},{"Eb8",111}, {"E 8",112},{"F 8",113},
 {"F#8",114},{"G 8",115},{"G#8",116},{"A 8",117}, {"Bb8",118},{"B 8",119},
 {"C 9",120},{"C#9",121},{"D 9",122},{"Eb9",123}, {"E 9",124},{"F 9",125},
 {"F#9",126},{"G 9",127}});

// Roland MT-32 mode (1987)
GEMSelect dropdown_mt32(128, (SelectOptionInt[]){
  {"Acoustic Piano 1",    1},
  {"Acoustic Piano 2",    2},
  {"Acoustic Piano 3",    3},
  {"Electric Piano 1",    4},
  {"Electric Piano 2",    5},
  {"Electric Piano 3",    6},
  {"Electric Piano 4",    7},
  {"Honkytonk",           8},
  {"Electric Organ 1",    9},
  {"Electric Organ 2",   10},
  {"Electric Organ 3",   11},
  {"Electric Organ 4",   12},
  {"Pipe Organ 1",       13},
  {"Pipe Organ 2",       14},
  {"Pipe Organ 3",       15},
  {"Accordion",          16},
  {"Harpsichord 1",      17},
  {"Harpsichord 2",      18},
  {"Harpsichord 3",      19},
  {"Clavinet 1",         20},
  {"Clavinet 2",         21},
  {"Clavinet 3",         22},
  {"Celesta 1",          23},
  {"Celesta 2",          24},
  {"Synth Brass 1",      25},
  {"Synth Brass 2",      26},
  {"Synth Brass 3",      27},
  {"Synth Brass 4",      28},
  {"Synth Bass 1",       29},
  {"Synth Bass 2",       30},
  {"Synth Bass 3",       31},
  {"Synth Bass 4",       32},
  {"Fantasy",            33},
  {"Harmo Pan",          34},
  {"Chorale",            35},
  {"Glasses",            36},
  {"Soundtrack",         37},
  {"Atmosphere",         38},
  {"Warm Bell",          39},
  {"Funny Vox",          40},
  {"Echo Bell",          41},
  {"Ice Rain",           42},
  {"Oboe 2001",          43},
  {"Echo Pan",           44},
  {"Doctor Solo",        45},
  {"School Daze",        46},
  {"Bellsinger",         47},
  {"Square Wave",        48},
  {"String Section 1",   49},
  {"String Section 2",   50},
  {"String Section 3",   51},
  {"Pizzicato",          52},
  {"Violin 1",           53},
  {"Violin 2",           54},
  {"Cello 1",            55},
  {"Cello 2",            56},
  {"Contrabass",         57},
  {"Harp 1",             58},
  {"Harp 2",             59},
  {"Guitar 1",           60},
  {"Guitar 2",           61},
  {"Elec Gtr 1",         62},
  {"Elec Gtr 2",         63},
  {"Sitar",              64},
  {"Acou Bass 1",        65},
  {"Acou Bass 2",        66},
  {"Elec Bass 1",        67},
  {"Elec Bass 2",        68},
  {"Slap Bass 1",        69},
  {"Slap Bass 2",        70},
  {"Fretless 1",         71},
  {"Fretless 2",         72},
  {"Flute 1",            73},
  {"Flute 2",            74},
  {"Piccolo 1",          75},
  {"Piccolo 2",          76},
  {"Recorder",           77},
  {"Pan Pipes",          78},
  {"Sax 1",              79},
  {"Sax 2",              80},
  {"Sax 3",              81},
  {"Sax 4",              82},
  {"Clarinet 1",         83},
  {"Clarinet 2",         84},
  {"Oboe",               85},
  {"English Horn",       86},
  {"Bassoon",            87},
  {"Harmonica",          88},
  {"Trumpet 1",          89},
  {"Trumpet 2",          90},
  {"Trombone 1",         91},
  {"Trombone 2",         92},
  {"French Horn 1",      93},
  {"French Horn 2",      94},
  {"Tuba",               95},
  {"Brass Section 1",    96},
  {"Brass Section 2",    97},
  {"Vibe 1",             98},
  {"Vibe 2",             99},
  {"Synth Mallet",      100},
  {"Windbell",          101},
  {"Glock",             102},
  {"Tube Bell",         103},
  {"Xylophone",         104},
  {"Marimba",           105},
  {"Koto",              106},
  {"Sho",               107},
  {"Shakuhachi",        108},
  {"Whistle 1",         109},
  {"Whistle 2",         110},
  {"Bottleblow",        111},
  {"Breathpipe",        112},
  {"Timpani",           113},
  {"Melodic Tom",       114},
  {"Deep Snare",        115},
  {"Elec Perc 1",       116},
  {"Elec Perc 2",       117},
  {"Taiko",             118},
  {"Taiko Rim",         119},
  {"Cymbal",            120},
  {"Castanets",         121},
  {"Triangle",          122},
  {"Orchestra Hit",     123},
  {"Telephone",         124},
  {"Bird Tweet",        125},
  {"One Note Jam",      126},
  {"Water Bell",        127},
  {"Jungle Tune",       128}
});

// General MIDI 1 Patch List
GEMSelect dropdown_gm(128, (SelectOptionInt[]){
  {"Acoustic GrandPiano",  1},
  {"BrightAcousticPiano",  2},
  {"Electric GrandPianp",  3},
  {"Honky-tonk Piano",     4},
  {"Electric Piano 1",     5},
  {"Electric Piano 2",     6},
  {"Harpsichord",          7},
  {"Clavi",                8},
  {"Celesta",              9},
  {"Glockenspiel",        10},
  {"Music Box",           11},
  {"Vibraphone",          12},
  {"Marimba",             13},
  {"Xylophone",           14},
  {"Tubular Bells",       15},
  {"Dulcimer",            16},
  {"Drawbar Organ",       17},
  {"Percussive Organ",    18},
  {"Rock Organ",          19},
  {"Church Organ",        20},
  {"Reed Organ",          21},
  {"Accordion",           22},
  {"Harmonica",           23},
  {"Tango Accordion",     24},
  {"AcousticGuitarNylon", 25},
  {"AcousticGuitarSteel", 26},
  {"ElectricGuitar Jazz", 27},
  {"ElectricGuitarClean", 28},
  {"ElectricGuitarMuted", 29},
  {"Overdrive Guitar",    30},
  {"Distortion Guitar",   31},
  {"Guitar harmonics",    32},
  {"Acoustic Bass",       33},
  {"ElectricBass Finger", 34},
  {"El Bass (pick)",      35},
  {"Fretless Bass",       36},
  {"Slap Bass 1",         37},
  {"Slap Bass 2",         38},
  {"Synth Bass 1",        39},
  {"Synth Bass 2",        40},
  {"Violin",              41},
  {"Viola",               42},
  {"Cello",               43},
  {"Contrabass",          44},
  {"Tremolo Strings",     45},
  {"Pizzicato Strings",   46},
  {"Orchestral Harp",     47},
  {"Timpani",             48},
  {"String Ensemble 1",   49},
  {"String Ensemble 2",   50},
  {"Synth Strings 1",     51},
  {"Synth Strings 2",     52},
  {"Choir Aahs",          53},
  {"Voice Oohs",          54},
  {"Synth Voice",         55},
  {"Orchestra Hit",       56},
  {"Trumpet",             57},
  {"Trombone",            58},
  {"Tuba",                59},
  {"Muted Trumpet",       60},
  {"French Horn",         61},
  {"Brass Section",       62},
  {"Synth Brass 1",       63},
  {"Synth Brass 2",       64},
  {"Soprano Sax",         65},
  {"Alto Sax",            66},
  {"Tenor Sax",           67},
  {"Baritone Sax",        68},
  {"Oboe",                69},
  {"English Horn",        70},
  {"Bassoon",             71},
  {"Clarinet",            72},
  {"Piccolo",             73},
  {"Flute",               74},
  {"Recorder",            75},
  {"Pan Flute",           76},
  {"Blown Bottle",        77},
  {"Shakuhachi",          78},
  {"Whistle",             79},
  {"Ocarina",             80},
  {"Lead 1 (square)",     81},
  {"Lead 2 (sawtooth)",   82},
  {"Lead 3 (calliope)",   83},
  {"Lead 4 (chiff)",      84},
  {"Lead 5 (charang)",    85},
  {"Lead 6 (voice)",      86},
  {"Lead 7 (fifths)",     87},
  {"Lead 8 (bass+lead)",  88},
  {"Pad 1 (new age)",     89},
  {"Pad 2 (warm)",        90},
  {"Pad 3 (poly synth)",  91},
  {"Pad 4 (choir)",       92},
  {"Pad 5 (bowed)",       93},
  {"Pad 6 (metallic)",    94},
  {"Pad 7 (halo)",        95},
  {"Pad 8 (sweep)",       96},
  {"FX 1 (rain)",         97},
  {"FX 2 (soundtrack)",   98},
  {"FX 3 (crystal)",      99},
  {"FX 4 (atmosphere)",  100},
  {"FX 5 (brightness)",  101},
  {"FX 6 (goblins)",     102},
  {"FX 7 (echoes)",      103},
  {"FX 8 (sci-fi)",      104},
  {"Sitar",              105},
  {"Banjo",              106},
  {"Shamisen",           107},
  {"Koto",               108},
  {"Kalimba",            109},
  {"Bag pipe",           110},
  {"Fiddle",             111},
  {"Shanai",             112},
  {"Tinkle Bell",        113},
  {"Agogo",              114},
  {"Steel Drums",        115},
  {"Woodblock",          116},
  {"Taiko Drum",         117},
  {"Melodic Tom",        118},
  {"Synth Drum",         119},
  {"Reverse Cymbal",     120},
  {"Guitar Fret Noise",  121},
  {"Breath Noise",       122},
  {"Seashore",           123},
  {"Bird Tweet",         124},
  {"Telephone Ring",     125},
  {"Helicopter",         126},
  {"Applause",           127},
  {"Gunshot",            128}
});

GEMSelect dropdown_dir(6, (SelectOptionInt[]){
  {"Right",0},{"Up-Rt",1},{"Up-Lft",2},{"Left",3},{"Dn-Lft",4},{"Dn-Rt",5}});
GEMSelect dropdown_speed(7, (SelectOptionInt[]){
  {"TooSlow",1},
  {"Turtle", 2},
  {"Slow",   4}, 
  {"Medium", 8},
  {"Fast",   16},
  {"Cheetah",32},
  {"Instant",127}
});
GEMSelect dropdown_anim(9, (SelectOptionInt[]){
  {"None",   _animType_none},
  {"Star",   _animType_star},
  {"Splash", _animType_splash},
  {"Orbit",  _animType_orbit},
  {"Octave", _animType_octave},
  {"By Note",_animType_by_note},
  {"Beams",  _animType_beams},
  {"Rv.Spls",_animType_splash_reverse},
  {"Rv.Star",_animType_star_reverse}
});
GEMSelect dropdown_bright(7, (SelectOptionInt[]){
  {"Off",    _globlBrt_off},
  {"Dimmer", _globlBrt_dimmer},
  {"Dim",    _globlBrt_dim},
  {"Low",    _globlBrt_low},
  {"Mid",    _globlBrt_mid},
  {"High",   _globlBrt_high},
  {"THE SUN",_globlBrt_max}
});
GEMSelect dropdown_synth_mode(4, (SelectOptionInt[]){
  {"  Off",   _synthTyp_off},
  {"  Mono",  _synthTyp_mono},
  {"Arpeggio",_synthTyp_arpeggio},
  {"  Poly",  _synthTyp_poly}
});
GEMSelect dropdown_wave(7, (SelectOptionInt[]){
  {" Hybrid", _synthWav_hybrid},
  {" Square", _synthWav_square},
  {"  Saw",   _synthWav_saw},
  {"Triangle",_synthWav_triangle},
  {" Sine",   _synthWav_sine},
  {"Strings", _synthWav_strings},
  {"Clarinet",_synthWav_clarinet}
});
GEMSelect dropdown_adsr(6,(SelectOptionInt[]){
  {"  None",   _synthEnv_none},
  {"  Hit",    _synthEnv_hit},
  {" Pluck",   _synthEnv_pluck},
  {" Strum",   _synthEnv_strum},
  {"  Slow",   _synthEnv_slow},
  {"Reverse",  _synthEnv_reverse}
});
GEMSelect dropdown_palette(3, (SelectOptionInt[]){
  {"Rainbow",_palette_rainbow},
  {"Tiered", _palette_tiered},
  {"Alt",    _palette_alternate}
});
GEMSelect dropdown_fps(4, (SelectOptionInt[]){
  {"24",24},{"30",30},{"60",60},{"70",70}});
GEMSelect dropdown_MIDImode(4, (SelectOptionInt[]){
  {"MIDI Mode: Normal", _MIDImode_standard},
  {"MIDI Mode: MPE", _MIDImode_MPE},
  {"MIDI Mode: MTS", _MIDImode_tuning_table},
  {"MIDI Mode: 2.0", _MIDImode_2_point_oh}
});
GEMSelect dropdown_instruments(2, (SelectOptionInt[]){
  {"General MIDI PC:", _GM_instruments},
  {"Roland MT-32 PC:", _MT32_instruments}
});
GEMSelect dropdown_MPE(3, (SelectOptionInt[]){
  {"1-zone master ch 1 ", 0},
  {"1-zone master ch 16", 1},
  {"2-zone (dual banks)", 2},
});  
GEMSelect dropdown_MPE_left(14, (SelectOptionInt[]){
  {" 2 voices, ch 1-3  ", 3},
  {" 3 voices, ch 1-4  ", 4},
  {" 4 voices, ch 1-5  ", 5},
  {" 5 voices, ch 1-6  ", 6},
  {" 6 voices, ch 1-7  ", 7},
  {" 7 voices, ch 1-8  ", 8},
  {" 8 voices, ch 1-9  ", 9},
  {" 9 voices, ch 1-10 ",10},
  {"10 voices, ch 1-11 ",11},
  {"11 voices, ch 1-12 ",12},
  {"12 voices, ch 1-13 ",13},
  {"13 voices, ch 1-14 ",14},
  {"14 voices, ch 1-15 ",15},
  {"15 voices, ch 1-16 ",16}
});
GEMSelect dropdown_MPE_right(14, (SelectOptionInt[]){
  {" 2 voices, ch 14-16",14},
  {" 3 voices, ch 13-16",13},
  {" 4 voices, ch 12-16",12},
  {" 5 voices, ch 11-16",11},
  {" 6 voices, ch 10-16",10},
  {" 7 voices, ch 9-16 ", 9},
  {" 8 voices, ch 8-16 ", 8},
  {" 9 voices, ch 7-16 ", 7},
  {"10 voices, ch 6-16 ", 6},
  {"11 voices, ch 5-16 ", 5},
  {"12 voices, ch 4-16 ", 4},
  {"13 voices, ch 3-16 ", 3},
  {"14 voices, ch 2-16 ", 2},
  {"15 voices, ch 1-16 ", 1}
});
GEMSelect dropdown_MPE_PB(5, (SelectOptionInt[]){
  {"Pitch bend +/-  2",2 },
  {"Pitch bend +/- 12",12},
  {"Pitch bend +/- 24",24},
  {"Pitch bend +/- 48",48},
  {"Pitch bend +/- 96",96}
});

GEMSpinner spin_0_127   ((GEMSpinnerBoundariesInt){1,   0, 127});
GEMSpinner spin_n127_127((GEMSpinnerBoundariesInt){1,-127, 127});
GEMSpinner spin_0_255   ((GEMSpinnerBoundariesInt){1,   0, 255});
GEMSpinner spin_1_255   ((GEMSpinnerBoundariesInt){1,   1, 255});
GEMSpinner spin_100_1k  ((GEMSpinnerBoundariesInt){10,100,1000});
GEMSpinner spin_500_2k  ((GEMSpinnerBoundariesInt){10,500,2000});

#include "settings.h"
GEMItem *menuItem[_settingSize];

GEMItem blank("",doNothing);        // always visible
GEMItem spacer_1("",doNothing); // can set visible or hidden independently
GEMItem spacer_2("",doNothing); // can set visible or hidden independently

void showHide_tuning() {
  menuItem[_equaveC]->hide(settings[_equaveJI].b);
  spacer_1.hide(settings[_equaveJI].b);
  menuItem[_equaveN]->hide(!settings[_equaveJI].b); 
  menuItem[_equaveD]->hide(!settings[_equaveJI].b); 
  menuItem[_lgSteps]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_smSteps]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_lgToSmR]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  // others tbd
  menuItem[_modeLgSm]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_eqDivs]->hide(settings[_tuneSys].i != _tuneSys_equal);
}
void showHide_MPE() {
  menuItem[_MPEzoneC]->hide(settings[_MIDImode].i != _MIDImode_MPE);
  menuItem[_MPEzoneL]->hide(settings[_MIDImode].i != _MIDImode_MPE);
  menuItem[_MPEzoneR]->hide(settings[_MIDImode].i != _MIDImode_MPE);
  menuItem[_MPEpb]->hide(settings[_MIDImode].i != _MIDImode_MPE);
}
void showHide_PC() {
  menuItem[_MIDIpc]->hide(settings[_MIDIorMT].i != _GM_instruments);
  menuItem[_MT32pc]->hide(settings[_MIDIorMT].i != _MT32_instruments);
}
void showHide_generate() {
  menuItem[_lgStepA]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_smStepA]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_lgStepB]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_smStepB]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);

  menuItem[_eqStepA]->hide((settings[_tuneSys].i != _tuneSys_equal)
                        && (settings[_tuneSys].i != _tuneSys_normal));
  menuItem[_eqStepB]->hide((settings[_tuneSys].i != _tuneSys_equal)
                        && (settings[_tuneSys].i != _tuneSys_normal));
  
  menuItem[_JInumA]->hide(settings[_tuneSys].i != _tuneSys_just);
  menuItem[_JIdenA]->hide(settings[_tuneSys].i != _tuneSys_just);
  menuItem[_JInumB]->hide(settings[_tuneSys].i != _tuneSys_just);
  menuItem[_JIdenB]->hide(settings[_tuneSys].i != _tuneSys_just);
}

// the menu items created based on settings
// will pass thru this callback function
// first. here we'll ONLY put in menu-related
// callback items (i.e. showing/hiding entries).
// then in the main program we'll
// flesh out other code-related activity.
// positive integer callbacks mean it came from the
// associated setting menu item.
// negative integer callbacks are to trigger
// special routines.

enum {
  _run_routine_to_generate_layout = -1,

};

extern void menu_handler(int settingNumber);

void onChg(GEMCallbackData callbackData) {
  switch (callbackData.valInt) {
    case _equaveJI:
      showHide_tuning();
      break;
    case _MIDImode:
      showHide_MPE();
      break;
    case _MIDIorMT:
      showHide_PC();
      break;
    default: 
      break;
  }
  menu_handler(callbackData.valInt);
}

void onSelect_generate(GEMCallbackData callbackData) {
  settings[_tuneSys].i = callbackData.valInt;
  showHide_tuning();
  showHide_generate();
  switch (callbackData.valInt) {
    case _tuneSys_normal:
      menu.setMenuPageCurrent(pgGenerate);
      break;
    default:
      showHide_tuning();
      menu.setMenuPageCurrent(pgTuning);
      break;
  }
  menu.drawMenu();
}

void create_menu_items_for_user_settings() {
  #define _CREATE_MANUAL(A, T, L)    menuItem[A] = new GEMItem(L, settings[A].T,    onChg, A)
  #define _CREATE_SELECT(A, T, L, S) menuItem[A] = new GEMItem(L, settings[A].T, S, onChg, A)
  // defaults.b not selectable
  // changed.b  not selectable
  _CREATE_MANUAL(_debug,    b, "Debug?");
  // anchorX.i  chosen via keypress
  // anchorY.i  chosen via keypress
  _CREATE_SELECT(_anchorN,  i, "Anchor note:", dropdown_notes);
  _CREATE_MANUAL(_anchorC,  d, "+/- cents:");
  _CREATE_SELECT(_txposeS,  i, "Transpose:",   spin_n127_127);
  _CREATE_MANUAL(_txposeC,  d, "cents: ");
  _CREATE_SELECT(_axisA,    i, "Axis A:",      dropdown_dir);
  _CREATE_SELECT(_axisB,    i, "Axis B:",      dropdown_dir);
  _CREATE_MANUAL(_equaveJI, b, "Equave in JI?");
  _CREATE_MANUAL(_equaveC,  d, "Cents:");
  _CREATE_SELECT(_equaveN,  i, "Numerator:",   spin_1_255);
  _CREATE_SELECT(_equaveD,  i, "Denominator:", spin_1_255);
  // tuneSys.i is complicated
  _CREATE_SELECT(_eqDivs,   i, "Equal div's:", spin_1_255);
  _CREATE_SELECT(_eqStepA,  i, "Axis A steps", spin_n127_127);
  _CREATE_SELECT(_eqStepB,  i, "Axis B steps", spin_n127_127);
  _CREATE_SELECT(_lgSteps,  i, "Large steps:", spin_1_255);
  _CREATE_SELECT(_smSteps,  i, "Small steps:", spin_1_255);
  _CREATE_SELECT(_lgStepA,  i, "Axis A, Lg:",  spin_n127_127);
  _CREATE_SELECT(_smStepA,  i, "Axis A, Sm:",  spin_n127_127);
  _CREATE_SELECT(_lgStepB,  i, "Axis B, Lg:",  spin_n127_127);  
  _CREATE_SELECT(_smStepB,  i, "Axis B, Sm:",  spin_n127_127);
  _CREATE_MANUAL(_lgToSmR,  d, "Ratio Lg/Sm:");
  // lgToSmN.i from a special dropdown
  // lgToSmD.i from a special dropdown
  _CREATE_SELECT(_modeLgSm, i, "Mode number:", spin_0_127);
  _CREATE_SELECT(_JInumA,   i, "Numerator:",   spin_1_255);
  _CREATE_SELECT(_JIdenA,   i, "Denominator:", spin_1_255);
  _CREATE_SELECT(_JInumB,   i, "Numerator:",   spin_1_255);
  _CREATE_SELECT(_JIdenB,   i, "Denominator:", spin_1_255);
  _CREATE_MANUAL(_scaleLck, b, "Scale lock");
  _CREATE_SELECT(_animFPS,  i, "LED refresh Hz", dropdown_fps);
  _CREATE_SELECT(_palette,  i, "Palette", dropdown_palette);
  _CREATE_SELECT(_animType, i, "Animation:",   dropdown_anim);
  _CREATE_SELECT(_globlBrt, i, "Brightness:",  dropdown_bright);
  _CREATE_MANUAL(_hueLoop,  d, "Seconds:");
  _CREATE_MANUAL(_tglWheel, b, "Wheel toggle");
  _CREATE_MANUAL(_whlMode,  b, "Fine tune?");
  _CREATE_MANUAL(_mdSticky, b, "Mod sticky?");
  _CREATE_MANUAL(_pbSticky, b, "PB sticky?");
  _CREATE_MANUAL(_vlSticky, b, "Vel sticky?");
  _CREATE_SELECT(_mdSpeed,  i, "Mod speed:", dropdown_speed);
  _CREATE_SELECT(_pbSpeed,  i, "PB speed:",  dropdown_speed);
  _CREATE_SELECT(_vlSpeed,  i, "Vel speed:", dropdown_speed);
  _CREATE_MANUAL(_rotInv,   b, "Knob invert");
  _CREATE_SELECT(_rotDblCk, i, "DblClk in ms", spin_100_1k);
  _CREATE_SELECT(_rotLongP, i, "LongPr in ms", spin_500_2k);
  _CREATE_SELECT(_SStime,   i, "ScrSvr in sec", spin_1_255);
  _CREATE_SELECT(_MIDImode, i, "", dropdown_MIDImode);
  _CREATE_MANUAL(_MIDIusb,  b, "USB port");
  _CREATE_MANUAL(_MIDIjack, b, "Serial port");
  _CREATE_SELECT(_MPEzoneC, i, "", dropdown_MPE);
  _CREATE_SELECT(_MPEzoneL, i, "", dropdown_MPE_left);
  _CREATE_SELECT(_MPEzoneR, i, "", dropdown_MPE_right);
  _CREATE_SELECT(_MPEpb,    i, "", dropdown_MPE_PB);
  _CREATE_SELECT(_MIDIorMT, i, "", dropdown_instruments);
  _CREATE_SELECT(_MIDIpc,   i, "", dropdown_gm);
  _CREATE_SELECT(_MT32pc,   i, "", dropdown_mt32);
  _CREATE_SELECT(_synthTyp, i, "Playback", dropdown_synth_mode);
  _CREATE_SELECT(_synthWav, i, "Waveform",   dropdown_wave);
  _CREATE_SELECT(_synthEnv, i, "Envelope",   dropdown_adsr);
  // _synthVol.i set via control
  _CREATE_MANUAL(_synthBuz, b, "Use piezo?");
  _CREATE_MANUAL(_synthJac, b, "Use jack?");
}

void populate_menu_structure() {
	pgHome // main menu contains links to pages
		.addMenuItem(*new GEMItem("Layout",   pgLayout))
		.addMenuItem(*new GEMItem("Playback", pgPlayback))
		.addMenuItem(*new GEMItem("Hardware", pgHardware))
		.addMenuItem(*new GEMItem("Software", pgSoftware))
		.addMenuItem(*new GEMItem("Save preset...", pgSavePreset))
		.addMenuItem(*new GEMItem("Load preset...", pgLoadPreset))
		.addMenuItem(*new GEMItem("Power off", pgReboot))
    ;
	pgLayout // layout menu
    .setParentMenuPage(pgHome)
    .addMenuItem(*new GEMItem("Edit current layout", pgEdit))
    .addMenuItem(*new GEMItem("Generate layout...",  doNothing))
    .addMenuItem(*new GEMItem("...normal 12-tone",   onSelect_generate, _tuneSys_normal))
    .addMenuItem(*new GEMItem("...by equal steps",   onSelect_generate, _tuneSys_equal))
    .addMenuItem(*new GEMItem("...by lg/sm steps",   onSelect_generate, _tuneSys_lg_sm))
    .addMenuItem(*new GEMItem("...as a JI lattice",  onSelect_generate, _tuneSys_just))
    ;
    pgTuning // first page of layout application
      .addMenuItem(*menuItem[_equaveJI]) 
      .addMenuItem(*menuItem[_equaveN] ) .addMenuItem(*menuItem[_equaveC] ) 
      .addMenuItem(*menuItem[_equaveD] ) .addMenuItem(spacer_1) 
      .addMenuItem(*menuItem[_lgSteps] ) .addMenuItem(*menuItem[_eqDivs]  )
      .addMenuItem(*menuItem[_smSteps] )
      .addMenuItem(*menuItem[_lgToSmR] ) 
      .addMenuItem(*menuItem[_modeLgSm])
      .addMenuItem(*new GEMItem(">> Continue", pgGenerate))
      ;
    pgGenerate // second page of layout application
      .addMenuItem(*menuItem[_axisA]   )
      .addMenuItem(*menuItem[_axisB]   )
      .addMenuItem(*menuItem[_lgStepA] ) .addMenuItem(*menuItem[_JInumA] ) .addMenuItem(*menuItem[_eqStepA] ) 
      .addMenuItem(*menuItem[_smStepA] ) .addMenuItem(*menuItem[_JIdenA] ) .addMenuItem(*menuItem[_eqStepB] )
      .addMenuItem(*menuItem[_lgStepB] ) .addMenuItem(*menuItem[_JInumB] ) 
      .addMenuItem(*menuItem[_smStepB] ) .addMenuItem(*menuItem[_JIdenB] )
      .addMenuItem(*new GEMItem(">> Continue", pgAnchor))
      ;
    pgAnchor // last page of layout application
      .addMenuItem(*menuItem[_anchorN])
      .addMenuItem(*menuItem[_anchorC])
      .addMenuItem(*menuItem[_txposeS])
      .addMenuItem(*new GEMItem(">> Generate!!", onChg, _run_routine_to_generate_layout))
      ;
  pgPlayback
    .addMenuItem(*new GEMItem("MIDI...", pgMIDI))
    .addMenuItem(*new GEMItem("Synth...", pgSynth))
    .addMenuItem(*menuItem[_scaleLck])
    ;
    pgMIDI
      .addMenuItem(*new GEMItem("MIDI out ports...", pgMIDIout))
      .addMenuItem(*menuItem[_MIDIorMT])
      .addMenuItem(*menuItem[_MIDIpc])   .addMenuItem(*menuItem[_MT32pc])
      .addMenuItem(*menuItem[_MIDImode])
      .addMenuItem(*menuItem[_MPEzoneC])
      .addMenuItem(*menuItem[_MPEzoneL])
      .addMenuItem(*menuItem[_MPEzoneR])
      .addMenuItem(*menuItem[_MPEpb])
      ;
      pgMIDIout
        .addMenuItem(*menuItem[_MIDIusb])
        .addMenuItem(*menuItem[_MIDIjack])
        ;
    pgSynth
      .addMenuItem(*menuItem[_synthTyp])
      .addMenuItem(*menuItem[_synthWav])
      .addMenuItem(*menuItem[_synthEnv])
      .addMenuItem(*menuItem[_synthBuz])
      .addMenuItem(*menuItem[_synthJac])
      ;
  pgHardware
    .addMenuItem(*new GEMItem("Rotary...", pgRotary))
    .addMenuItem(*new GEMItem("Command keys...", pgCommand))
    .addMenuItem(*new GEMItem("Display...", pgOLED))
    ;
    pgRotary
      .addMenuItem(*menuItem[_rotInv])
      .addMenuItem(*menuItem[_rotDblCk])
      .addMenuItem(*menuItem[_rotLongP])
      ;
    pgCommand
      .addMenuItem(*menuItem[_tglWheel])
      .addMenuItem(*menuItem[_whlMode])
      .addMenuItem(*menuItem[_mdSticky])
      .addMenuItem(*menuItem[_pbSticky])
      .addMenuItem(*menuItem[_vlSticky])
      .addMenuItem(*menuItem[_mdSpeed])
      .addMenuItem(*menuItem[_pbSpeed])
      .addMenuItem(*menuItem[_vlSpeed])
      ;
    pgOLED
      .addMenuItem(*menuItem[_SStime])
      ;


  pgSoftware
    .setParentMenuPage(pgHome)
		.addMenuItem(*menuItem[_debug])
    ;
}

extern void draw_GUI(int context);

void query_GUI() {
  draw_GUI(static_cast<GEMPagePublic*>(menu.getCurrentMenuPage())->GUI_context);
}

void menu_setup() {
  menu.setSplashDelay(0);
  create_menu_items_for_user_settings();
	populate_menu_structure();
  menu.init();
  menu.invertKeysDuringEdit(true);
	menu.setDrawMenuCallback(query_GUI);
  menu.setMenuPageCurrent(pgNoMenu);
}
// give your preset a name then press ok or cancel
// Save your current preset first? if yes, go to save preset then after execute then load.
// Quick reboot
// Flash firmware
// Factory reset -> are you sure?