/*
 * Copyright (C) 2024 MegaKG.
 *
 * Licensed under the GPLV3 License.
 *
 */

#pragma once
#include <LittleFS.h>
#include <SingleFileDrive.h>

void initFilesystem(){
  LittleFS.begin();

}

File openImage(){
  singleFileDrive.end();
  File f = LittleFS.open("sample.ppm", "w");
  return f;
}

File openSample(){
  singleFileDrive.end();
  File f = LittleFS.open("sample.csv", "w");
  return f;
}

File openReadConfig(){
  File f = LittleFS.open("config", "r");
  return f;
}

File openWriteConfig(){
  File f = LittleFS.open("config", "w");
  return f;
}

void closeFile(File f){
  f.close();
}

void makeSampleVisible(){
  singleFileDrive.begin("sample.csv", "sample.csv");
}

void makeImageVisible(){
  singleFileDrive.begin("sample.ppm", "sample.ppm");
}
