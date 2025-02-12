#pragma once
#include "OLED.h" // OLED graphics object
#include <GEM_u8g2.h>           // library of code to create menu objects on the B&W display
#include "config/enable-advanced-mode.h"
GEM_u8g2 menu(u8g2);

struct GEMItemPublic : public GEMItem {
  GEMItemPublic(const GEMItem &g) : GEMItem(g) {}
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
GEMPagePublic pgPlayback("MIDI and synth settings", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 19, pgHome);
GEMPagePublic pgHardware("Hardware settings", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 7, pgHome);
GEMPagePublic pgSoftware("Software settings", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 7, pgHome);
GEMPagePublic pgSavePreset("Select slot to save in", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 7, pgHome);
GEMPagePublic pgLoadPreset("Select slot to load from", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 7, pgHome);
GEMPagePublic pgReboot("Power off options", _show_HUD, 0, GEM_ITEMS_COUNT_AUTO, 7, pgHome);
GEMPagePublic pgEdit("Edit current layout...", _show_pixel_ID, 1, GEM_ITEMS_COUNT_AUTO, 7, pgLayout);
GEMPagePublic pgTuning("Tuning system parameters", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 6, pgLayout);
GEMPagePublic pgGenerate("Select layout parameters", _hide_GUI, 0, GEM_ITEMS_COUNT_AUTO, 6, pgLayout);
GEMPagePublic pgAnchor("Select root key and tuning", _show_pixel_ID, 1, GEM_ITEMS_COUNT_AUTO, 6, pgGenerate);

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
GEMSelect dropdown_MIDImode(4, (SelectOptionInt[]){
  {"Normal", _MIDImode_standard},
  {"MPE", _MIDImode_MPE},
  {"MTS", _MIDImode_tuning_table},
  {"MIDI2.0", _MIDImode_2_point_oh}
});
GEMSelect dropdown_MIDIout(4, (SelectOptionInt[]){
  {"None",_MIDIout_none},
  {"USB",_MIDIout_USB},
  {"Jack",_MIDIout_serial},
  {"Both",_MIDIout_both}
});
GEMSelect dropdown_synth_mode(4, (SelectOptionInt[]){
  {"Off",_synthTyp_off},
  {"Mono",_synthTyp_mono},
  {"Arp'gio",_synthTyp_arpeggio},
  {"Poly",_synthTyp_poly}
});
GEMSelect dropdown_wave(7, (SelectOptionInt[]){
  {"Hybrid",_synthWav_hybrid},
  {"Square",_synthWav_square},
  {"Saw",_synthWav_saw},
  {"Triangl",_synthWav_triangle},
  {"Sine",_synthWav_sine},
  {"Strings",_synthWav_strings},
  {"Clarint",_synthWav_clarinet}
});
GEMSelect dropdown_fps(4, (SelectOptionInt[]){
  {"24",24},{"30",30},{"60",60},{"70",70}});
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

GEMSpinner spin_0_127   ((GEMSpinnerBoundariesInt){1,   0, 127});
GEMSpinner spin_n127_127((GEMSpinnerBoundariesInt){1,-127, 127});
GEMSpinner spin_0_255   ((GEMSpinnerBoundariesInt){1,   0, 255});
GEMSpinner spin_1_255   ((GEMSpinnerBoundariesInt){1,   1, 255});

#include "settings.h"
GEMItem *menuItem[_settingSize];
GEMItem blank("",doNothing);        // always visible
GEMItem spacer_1("",doNothing); // can set visible or hidden independently
GEMItem spacer_2("",doNothing); // can set visible or hidden independently

// this is the callback handler for any menu items that are
// linked to a user settings variable

void showHide_tuning() {
  menuItem[_equaveC]->hide(settings[_equaveJI].b);
  spacer_1.hide(settings[_equaveJI].b);
  menuItem[_equaveN]->hide(!settings[_equaveJI].b); 
  menuItem[_equaveD]->hide(!settings[_equaveJI].b); 
  menuItem[_lgSteps]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_smSteps]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_lgToSm]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_modeLgSm]->hide(settings[_tuneSys].i != _tuneSys_lg_sm);
  menuItem[_eqDivs]->hide(settings[_tuneSys].i != _tuneSys_equal);
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

void onComplete_generate() {
  // make the layout
  // success/failure screen
  menu.setMenuPageCurrent(pgShowMsg);
}

extern void change_audio_out_pins();

void onChg(GEMCallbackData callbackData) {
  switch (callbackData.valInt) {
    case _debug: 
      break;
    case _equaveJI:
      showHide_tuning();
      break;
    default: 
      break;
  }
}


#define _CREATE_MANUAL(A, T, L)    menuItem[A] = new GEMItem(L, settings[A].T,    onChg, A)
#define _CREATE_SELECT(A, T, L, S) menuItem[A] = new GEMItem(L, settings[A].T, S, onChg, A)

void create_menu_items_for_user_settings() {
  _CREATE_MANUAL(_debug,    b, "Debug?");
  _CREATE_SELECT(_anchorN,  i, "Anchor note:", dropdown_notes);
  _CREATE_MANUAL(_anchorC,  d, "+/- cents:");
  _CREATE_SELECT(_txposeS,  i, "Transpose:",   spin_n127_127);
  _CREATE_MANUAL(_txposeC,  d, "cents: ");
  _CREATE_MANUAL(_equaveJI, b, "Equave in JI?");
  _CREATE_MANUAL(_equaveC,  d, "Cents:");
  _CREATE_SELECT(_equaveN,  i, "Numerator:",   spin_1_255);
  _CREATE_SELECT(_equaveD,  i, "Denominator:", spin_1_255);
  _CREATE_SELECT(_axisA,    i, "Axis A:",      dropdown_dir);
  _CREATE_SELECT(_axisB,    i, "Axis B:",      dropdown_dir);
  _CREATE_SELECT(_eqDivs,   i, "Equal div's:", spin_1_255);
  _CREATE_SELECT(_eqStepA,  i, "Axis A steps", spin_n127_127);
  _CREATE_SELECT(_eqStepB,  i, "Axis B steps", spin_n127_127);
  _CREATE_SELECT(_lgSteps,  i, "Large steps:", spin_1_255);
  _CREATE_SELECT(_smSteps,  i, "Small steps:", spin_1_255);
  _CREATE_SELECT(_lgStepA,  i, "Axis A, Lg:",  spin_n127_127);
  _CREATE_SELECT(_smStepA,  i, "Axis A, Sm:",  spin_n127_127);
  _CREATE_SELECT(_lgStepB,  i, "Axis B, Lg:",  spin_n127_127);  
  _CREATE_SELECT(_smStepB,  i, "Axis B, Sm:",  spin_n127_127);
  _CREATE_MANUAL(_lgToSm,   d, "Ratio Lg/Sm:");
  _CREATE_SELECT(_modeLgSm, i, "Mode number:", spin_0_127);
  _CREATE_SELECT(_JInumA,   i, "Numerator:",   spin_1_255);
  _CREATE_SELECT(_JIdenA,   i, "Denominator:", spin_1_255);
  _CREATE_SELECT(_JInumB,   i, "Numerator:",   spin_1_255);
  _CREATE_SELECT(_JIdenB,   i, "Denominator:", spin_1_255);
  _CREATE_SELECT(_animType, i, "Animation:",   dropdown_anim);
  _CREATE_SELECT(_globlBrt, i, "Brightness:",  dropdown_bright);
  _CREATE_MANUAL(_tglWheel, b, "Wheel toggle");
  _CREATE_MANUAL(_whlMode,  b, "Fine tune?");
  _CREATE_MANUAL(_mdSticky, b, "Mod sticky?");
  _CREATE_MANUAL(_pbSticky, b, "PB sticky?");
  _CREATE_MANUAL(_vlSticky, b, "Vel sticky?");


  _CREATE_SELECT(_MPEzoneC, i, "", dropdown_MPE);
  _CREATE_SELECT(_MPEzoneL, i, "", dropdown_MPE_left);
  _CREATE_SELECT(_MPEzoneR, i, "", dropdown_MPE_right);


  _CREATE_MANUAL(_scaleLck, b, "Scale lock");
  _CREATE_SELECT(_animFPS,  i, "LED refresh Hz", dropdown_fps);
  _CREATE_MANUAL(_rotInv,   b, "Knob invert");

  _CREATE_MANUAL(_synthBuz, b, "Piezo");
  _CREATE_MANUAL(_synthJac, b, "Audio jack");

/*
  refS[_palette].i  = _palette_rainbow;
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
  refS[_SStime].i   = 10; // seconds
  refS[_MIDImode].i = _MIDImode_standard;
  refS[_MPEpb].i    = 48; // 2, 12, 24, 48, or 96
  refS[_MIDIout].i  = (version = 12 ? _MIDIout_both : _MIDIout_USB);
  refS[_MIDIpc].i   = 0; // program chg 1 - 128
  refS[_synthTyp].i = _synthTyp_poly;
  refS[_synthWav].i = _synthWav_hybrid;
  refS[_synthOut].i = (version >= 12 ? _synthOut_both : _synthOut_piezo);
*/

}

void populate_menu_structure() {
	pgHome // main menu contains links to pages
		.addMenuItem(*new GEMItem("Layout", pgLayout))
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
  pgTuning //
    .setParentMenuPage(pgLayout)
    .addMenuItem(*menuItem[_equaveJI]) 
    .addMenuItem(*menuItem[_equaveN] ) .addMenuItem(*menuItem[_equaveC] ) 
    .addMenuItem(*menuItem[_equaveD] ) .addMenuItem(spacer_1) 
    .addMenuItem(*menuItem[_lgSteps] ) .addMenuItem(*menuItem[_eqDivs]  )
    .addMenuItem(*menuItem[_smSteps] )
    .addMenuItem(*menuItem[_lgToSm]  ) 
    .addMenuItem(*menuItem[_modeLgSm])
    .addMenuItem(*new GEMItem(">> Continue", pgGenerate))
    ;
  pgGenerate
    .setParentMenuPage(pgLayout)
    .addMenuItem(*menuItem[_axisA]   )
    .addMenuItem(*menuItem[_axisB]   )
    .addMenuItem(*menuItem[_lgStepA] ) .addMenuItem(*menuItem[_JInumA] ) .addMenuItem(*menuItem[_eqStepA] ) 
    .addMenuItem(*menuItem[_smStepA] ) .addMenuItem(*menuItem[_JIdenA] ) .addMenuItem(*menuItem[_eqStepB] )
    .addMenuItem(*menuItem[_lgStepB] ) .addMenuItem(*menuItem[_JInumB] ) 
    .addMenuItem(*menuItem[_smStepB] ) .addMenuItem(*menuItem[_JIdenB] )
    .addMenuItem(*new GEMItem(">> Continue", pgAnchor))
    ;
  pgAnchor
    .addMenuItem(*menuItem[_anchorN])
    .addMenuItem(*menuItem[_anchorC])
    .addMenuItem(*menuItem[_txposeS])
    ;

  pgPlayback
		.addMenuItem(*menuItem[_scaleLck])
    .addMenuItem(*menuItem[_MPEzoneC])
    .addMenuItem(*menuItem[_MPEzoneL])
    .addMenuItem(*menuItem[_MPEzoneR])
    .addMenuItem(*menuItem[_synthBuz])
    .addMenuItem(*menuItem[_synthJac])
    ;
  pgHardware
		.addMenuItem(*menuItem[_rotInv])
    ;
  pgSoftware
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