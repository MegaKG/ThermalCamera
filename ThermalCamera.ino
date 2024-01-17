/*
 * Copyright (C) 2024 MegaKG.
 *
 * Licensed under the GPLV3 License.
 *
 */

#include <math.h>
#include "VideoHandler.h"
#include "MenuHandler.h"
#include "BatteryUtils.h"
#include "ConfigSaveLoad.h"

extern Display myDisplay;
extern ThermalCamera myCamera;

void checkBattery(){
  if (batteryIsFlat()){
    lowBatteryImage();
    while (batteryIsFlat()){
      delay(100);
    }
  }
}

void setup() {
  initVideo();
  initMenu();
  initBattery();

  checkBattery();  
  Serial.begin(9600);

  loadConfig();
}

float emiss;
int counter = 0;
void loop() {
  videoFrame();
  
  counter ++;
  if (counter == 1000){
    checkBattery();
    counter = 0;
  }

  //Check for button presses
  switch (getKeypress()){
    case MenuButton:
      mainMenu();
      break;
    case UpButton:
      emiss = myCamera.getEmissivity();
      emiss = emiss + 0.05;
      emiss = (emiss > 1) ? 1 : emiss;
      myCamera.setEmissivity(emiss);
      break;
    case DownButton:
      emiss = myCamera.getEmissivity();
      emiss = emiss - 0.05;
      emiss = (emiss < 0) ? 0 : emiss;
      myCamera.setEmissivity(emiss);
      break;
    default:
      ;
  };

}

