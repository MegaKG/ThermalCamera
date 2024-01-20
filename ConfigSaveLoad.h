#pragma once
#include "FileUtilities.h"
#include "VideoHandler.h"
#include "ThermalCamera.h"
#include "MenuHandler.h"
#include "DisplayDriver.h"

extern Display myDisplay;
extern uint8_t backlightPercentage;
extern ThermalCamera myCamera;
extern int selectedColourMap;

void saveConfig(){
  float emissivity = myCamera.getEmissivity();
  int colourMapSel = selectedColourMap;

  File configFile = openWriteConfig();
  configFile.printf("%f\n",emissivity);
  configFile.printf("%i\n",colourMapSel);
  configFile.printf("%i\n",backlightPercentage);
  closeFile(configFile);
}

void loadConfig(){
  File configFile = openReadConfig();
  if (configFile){
    myCamera.setEmissivity(configFile.parseFloat());
    selectedColourMap = configFile.parseInt();
    backlightPercentage = configFile.parseInt();
    if (backlightPercentage == 0){
      backlightPercentage = 100;
    }
    myDisplay.setBacklightPercentage(backlightPercentage);
    closeFile(configFile);
  }
  
}