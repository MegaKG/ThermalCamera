#pragma once
#define v_input_pin A3
#define diodeDrop 0.538
#define dividerRatio 0.33
#define maxVoltage 3.3

#define batteryMin 3.3
#define batteryMax 4.2

#define batteryUpdateInterval 100

#define batteryLevelCount 4
const uint8_t batteryLevels[batteryLevelCount] = {
  100,
  90,
  12,
  0
};

const float batteryVoltages[batteryLevelCount] = {
  4.20,
  4.00,
  3.6,
  3.3
};




void initBattery(){
  pinMode(A3,INPUT);
}

float readBatteryVoltage(){
  analogReadResolution(10);
  int raw = analogRead(v_input_pin);
  float vsysVoltage = (((float)raw/1024)*maxVoltage) / dividerRatio;
  return vsysVoltage + diodeDrop;
}

uint8_t determineBatteryPercentage(){
  float currentVoltage = readBatteryVoltage();

  //Scan the arrays
  int lowIndex = 0;
  int highIndex = 1;
  for (int index = 0; index < batteryLevelCount-1; index++){
    if (currentVoltage <= batteryVoltages[index]){
      lowIndex = index;
      highIndex = index+1;
    }
  }

  //Roughly interpolate
  float gradient = (batteryLevels[highIndex] - batteryLevels[lowIndex])/(batteryVoltages[highIndex] - batteryVoltages[lowIndex]);
  float scaled = (gradient * (currentVoltage - batteryVoltages[lowIndex])) + batteryLevels[lowIndex];
  if (scaled < 0){
    scaled = 0;
  }
  if (scaled > 100){
    scaled = 100;
  }
  return (uint8_t)scaled;
}

uint32_t checkCounter = 0;
uint8_t cachedBatteryValue = 0;
uint8_t readCachedBatteryPercentage(){
  if (checkCounter == 0){
    cachedBatteryValue = determineBatteryPercentage();
  }
  checkCounter += 1;
  if (checkCounter == batteryUpdateInterval){
    checkCounter = 0;
  }
  return cachedBatteryValue;
}

int batteryIsFlat(){
  if (readBatteryVoltage() <= batteryMin){
    return 1;
  }
  return 0;
}

int batteryIsCharging(){
  if (readBatteryVoltage() > batteryMax){
    return 1;
  }
  return 0;
}

