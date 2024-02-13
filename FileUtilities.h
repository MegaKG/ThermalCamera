#pragma once
#include <LittleFS.h>
#include "ExposeFS.h"

#define maxPhotos 16
#define maxSamples 8

void initFilesystem(){
  LittleFS.begin();

  initExposedFS();

}

void clearFilesystem(){
  LittleFS.format();
}

bool canSaveImage(){
  char fileName[20];
  memset(fileName,0,20);

  for (int index = 0; index < maxPhotos; index++){
    memset(fileName,0,20);
    snprintf(fileName,20,"Photo%i.ppm",index);

    if (!LittleFS.exists(fileName)){
     return 1;
    }
  }

  return 0; 
}

bool canSaveSample(){
  char fileName[20];
  memset(fileName,0,20);

  for (int index = 0; index < maxSamples; index++){
    memset(fileName,0,20);
    snprintf(fileName,20,"Sample%i.csv",index);

    if (!LittleFS.exists(fileName)){
     return 1;
    }
  }

  return 0; 
}

File openImage(){
  char fileName[20];
  memset(fileName,0,20);

  for (int index = 0; index < maxPhotos; index++){
    memset(fileName,0,20);
    snprintf(fileName,20,"Photo%i.ppm",index);

    if (!LittleFS.exists(fileName)){
      break;
    }
  }

  File f = LittleFS.open(fileName, "w");
  return f;
}

File openSample(){
  char fileName[20];
  memset(fileName,0,20);

  int availableIndex = 0;
  for (int index = 0; index < maxSamples; index++){
    memset(fileName,0,20);
    snprintf(fileName,20,"Sample%i.csv",index);

    if (!LittleFS.exists(fileName)){
      break;
    }
  }

  File f = LittleFS.open(fileName, "w");
  return f;
}

File openReadConfig(){
  File f = LittleFS.open("config.txt", "r");
  return f;
}

File openWriteConfig(){
  File f = LittleFS.open("config.txt", "w");
  return f;
}

void closeFile(File f){
  f.flush();
  f.close();
}

void updateFS(){
  rebuildFatTable();
}
