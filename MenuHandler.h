/*
 * Copyright (C) 2024 MegaKG.
 *
 * Licensed under the GPLV3 License.
 *
 */

#include "DisplayDriver.h"
#include "BatteryUtils.h"
#include "ColourMaps.h"
#include "VideoHandler.h"
#include "ConfigSaveLoad.h"
extern Display myDisplay;

#define MenuButton 16
#define BackButton 19
#define UpButton 17
#define DownButton 18

int buttonSelection = 0;

void menuInterrupt(){
  buttonSelection = MenuButton;
}
void backInterrupt(){
  buttonSelection = BackButton;
}
void upInterrupt(){
  buttonSelection = UpButton;
}
void downInterrupt(){
  buttonSelection = DownButton;
}

void initMenu(){
  pinMode(MenuButton,INPUT_PULLUP);
  pinMode(BackButton,INPUT_PULLUP);
  pinMode(UpButton,INPUT_PULLUP);
  pinMode(DownButton,INPUT_PULLUP);

  attachInterrupt(MenuButton, menuInterrupt, FALLING);
  attachInterrupt(BackButton, backInterrupt, FALLING);
  attachInterrupt(UpButton,   upInterrupt,   FALLING);
  attachInterrupt(DownButton, downInterrupt, FALLING);
}

int getKeypress(){
  int out = buttonSelection;
  if (out){
    delay(100);
  }
  buttonSelection = 0;
  return out;
}

int awaitKeypress(){
  int out;
  while (1){
    out = getKeypress();
    if (out){
      break;
    }
    else {
      delay(10);
    }
  }

  return out;
}



void batteryMenu(){
  char buffer[20];
  myDisplay.clearFB();
  myDisplay.printAt(1, 1, "Battery Status:", 0xffff);

  float voltage = readBatteryVoltage();
  if (batteryIsCharging()){
    myDisplay.printAt(11, 11, "Battery Charging", 0x07e0);
  }
  else {
    myDisplay.printAt(11, 11, "Battery Discharging", 0xf800);
    snprintf(buffer,20,"Battery:  %.2f V",readBatteryVoltage());
    myDisplay.printAt(10, 21, buffer, 0xf800);
  }
  myDisplay.refresh();

  while (awaitKeypress() != BackButton){
    ;
  }
}

void colourMenu(){
  int selected = 0;
  int maxMenu = numberOfColourMaps;

  int button;
  uint16_t colour = 0xffff;
  while (1){
    myDisplay.clearFB();
    myDisplay.printAt(1, 1, "Select Colour Map:", 0xffff);

    for (int index = 0; index < numberOfColourMaps; index++){
      if (index == selectedColourMap){
        colour = 0x07e0;       
      } 
      else {
        colour = 0xffff;
      }
      myDisplay.printAt(11, 11+(10*index), (char*)ColourMapLabels[index], colour);
    }

    //Place the marker
    myDisplay.printAt(1,11+(10*(selected%maxMenu)), ">", 0xffff);
    myDisplay.refresh();

    button = awaitKeypress();
    switch (button){
      case BackButton:
        return;
      
      case UpButton:
        selected += 1;
        break;
      
      case DownButton:
        selected -= 1;
        break;

      case MenuButton:
        setColourMap(selected%maxMenu);
        break;
    };
  }
}

void photoMenu(){
  takePhoto();
  myDisplay.clearFB();
  myDisplay.printAt(1, 1, "Photo Saved", 0xffff);
  myDisplay.refresh();

  while (awaitKeypress() != BackButton){
    ;
  }
}

void sampleMenu(){
  takeSample();
  myDisplay.clearFB();
  myDisplay.printAt(1, 1, "Sample Saved", 0xffff);
  myDisplay.refresh();

  while (awaitKeypress() != BackButton){
    ;
  }
}

void configSaveMenu(){
  saveConfig();
  myDisplay.clearFB();
  myDisplay.printAt(1, 1, "Config Saved", 0xffff);
  myDisplay.refresh();

  while (awaitKeypress() != BackButton){
    ;
  }
}

void mainMenu(){
  int selected = 0;
  int maxMenu = 6;

  int button;
  while (1){
    myDisplay.clearFB();
    myDisplay.printAt(1, 1, "Main Menu:", 0xffff);
    myDisplay.printAt(11, 11, "Battery Status", 0xffff);
    myDisplay.printAt(11, 21, "Colour Map", 0xffff);
    myDisplay.printAt(11, 31, "Take Photo", 0xffff);
    myDisplay.printAt(11, 41, "Take Sample", 0xffff);
    myDisplay.printAt(11, 51, "Save Config", 0xffff);
    myDisplay.printAt(11, 61, "Firmware Update", 0xffff);

    //Place the marker
    myDisplay.printAt(1,11+(10*(selected%maxMenu)), ">", 0xffff);

    myDisplay.refresh();

    button = awaitKeypress();
    switch (button){
      case BackButton:
        return;
      
      case UpButton:
        selected += 1;
        break;
      
      case DownButton:
        selected -= 1;
        break;

      case MenuButton:
        switch (selected % maxMenu){
          case 0:
            batteryMenu();
            break;
          case 1:
            colourMenu();
            break;
          case 2:
            photoMenu();
            break;
          case 3:
            sampleMenu();
            break;
          case 4:
            configSaveMenu();
            break;
          case 5:
            rp2040.rebootToBootloader();
            break;
        };
        break;
    };
  }
}
