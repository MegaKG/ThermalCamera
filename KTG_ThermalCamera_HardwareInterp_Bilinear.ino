//Ideally 240mhz clock speed

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
    delay(10000);
    myDisplay.setBacklightPercentage(0);
    while (batteryIsFlat()){
      delay(100);
    }
  }
}

void setup() {
  initVideo();
  myDisplay.setBacklightPercentage(255);
  initMenu();
  initBattery();

  delay(100);
  checkBattery();  
  Serial.begin(9600);

  loadConfig();
}

float emiss;
int counter = 0;
void loop() {
  videoFrame();
  
  counter ++;
  if (counter == 100){
    checkBattery();
    counter = 0;
  }

  //Check for button presses
  switch (getKeypress()){
    case MenuButton:
      mainMenu();
      break;
    case DownButton:
      emiss = myCamera.getEmissivity();
      emiss = emiss + 0.05;
      emiss = (emiss > 1) ? 1 : emiss;
      myCamera.setEmissivity(emiss);
      break;
    case UpButton:
      emiss = myCamera.getEmissivity();
      emiss = emiss - 0.05;
      emiss = (emiss < 0) ? 0 : emiss;
      myCamera.setEmissivity(emiss);
      break;
    default:
      ;
  };

}

