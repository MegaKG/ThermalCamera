/*
 * Copyright (C) 2024 MegaKG.
 *
 * Licensed under the GPLV3 License.
 *
 */

#pragma once
#include "FileUtilities.h"
#include "VideoHandler.h"
#include "ThermalCamera.h"

extern ThermalCamera myCamera;
extern int selectedColourMap;

void saveConfig(){
  float emissivity = myCamera.getEmissivity();
  int colourMapSel = selectedColourMap;

  File configFile = openWriteConfig();
  configFile.printf("%f\n",emissivity);
  configFile.printf("%i\n",colourMapSel);
  closeFile(configFile);
}

void loadConfig(){
  File configFile = openReadConfig();
  if (configFile){
    myCamera.setEmissivity(configFile.parseFloat());
    selectedColourMap = configFile.parseInt();
    closeFile(configFile);
  }
  
}
