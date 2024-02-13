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
extern int displayMode;

void saveConfig(){
  float emissivity = myCamera.getEmissivity();
  int colourMapSel = selectedColourMap;

  File configFile = openWriteConfig();
  configFile.printf("%f\n",emissivity);
  configFile.printf("%i\n",colourMapSel);
  configFile.printf("%i\n",backlightPercentage);
  configFile.printf("%i\n",displayMode);
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

    displayMode = configFile.parseInt();
    if (displayMode < 0){
      displayMode = 0;
    }

    closeFile(configFile);
  }
  
}