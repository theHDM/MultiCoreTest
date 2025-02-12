#pragma once
#include "settings.h"
#include "LittleFS.h"       // code to use flash drive space as a file system -- not implemented yet, as of May 2024
#include "debug.h"

const char* settingFileName = "temp222.dat";
bool fileSystemExists;

void mount_file_system() {
  fileSystemExists = true;
  if (LittleFS.begin()) return;
  debug.add("LittleFS could not mount; re-formatting storage space.\n");
  if (LittleFS.format()) return;
  fileSystemExists = false;
  debug.add("That's odd, there was even a problem re-formatting.\n");
}

bool load_settings(hexBoard_Setting_Array& refS) {
  if (!fileSystemExists) return false;
  File f = LittleFS.open(settingFileName,"r+");
  if (!f) {
    debug.add("Settings file did not exist.\n");
    return false;
  }
  int p = 0;
  while (f.available()) {
    Byte_Array b;
    for (int i = 0; i < bytes_per_setting; ++i) {
      b[i] = f.read();
    }
    refS[p].w = b;
    ++p;
  }
  refS[_defaults].b = false; // so that we don't try to overwrite with hardwire
  refS[_changed].b = false;
  debug.add("Settings loaded from file.\n");
  f.close();
  return true;
}

void save_settings(hexBoard_Setting_Array& refS) {
  if (!refS[_changed].b) return;
  if (!fileSystemExists) return;
  File f = LittleFS.open(settingFileName,"w+");
  if (!f) {
    debug.add("An Error has occurred while saving settings\n");        
  }
  for (int i = 0; i < _settingSize; ++i) {
    for (int j = 0; j < bytes_per_setting; ++j) {
      f.write((refS[i].w)[j]);
    }
  }
  debug.add("settings saved\n");
  f.close();
  refS[_changed].b = false;
}