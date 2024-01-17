/*
 * Copyright (C) 2024 MegaKG.
 *
 * Licensed under the GPLV3 License.
 *
 */

#pragma once
#define v_input_pin A3
#define diodeDrop 0.538
#define dividerRatio 0.33
#define maxVoltage 3.3

#define batteryMin 3.3
#define batteryMax 4.2

void initBattery(){
  pinMode(A3,INPUT);
}

float readBatteryVoltage(){
  analogReadResolution(10);
  int raw = analogRead(v_input_pin);
  float vsysVoltage = (((float)raw/1024)*maxVoltage) / dividerRatio;
  return vsysVoltage + diodeDrop;
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

