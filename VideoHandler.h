#pragma once
#include "ThermalCamera.h"
#include "DisplayDriver.h"
#include "MathFunctions.h"
#include "ColourMaps.h"
#include "FileUtilities.h"
#include "BatteryUtils.h"

#define I2C_SCL 1
#define I2C_SDA 0

#define SPI_CS 5
#define SPI_RS 6
#define SPI_RST 7
#define SPI_SDI 3
#define SPI_SCK 2
#define Backlight 27

#define height 24
#define width 32
#define imScale 4
#define imYOffset 10

ThermalCamera myCamera;
Display myDisplay;


uint16_t rawImageMap[24][32];
uint16_t frameBuffer[height*imScale][width*imScale];


uint16_t _colourMap(uint16_t pixValue, uint8_t selectedMap){
  int index;
  for (index = getColourMapLength(selectedMap)-1; index >= 0; index--){
    if (getColourPoint(selectedMap,index) <= pixValue){
      break;
    }
  }
  
  
  return Display::convertPixel(
    lerp(getColour(selectedMap,index,0),getColour(selectedMap,index+1,0),getColourPoint(selectedMap,index),getColourPoint(selectedMap,index+1),pixValue),
    lerp(getColour(selectedMap,index,1),getColour(selectedMap,index+1,1),getColourPoint(selectedMap,index),getColourPoint(selectedMap,index+1),pixValue),
    lerp(getColour(selectedMap,index,2),getColour(selectedMap,index+1,2),getColourPoint(selectedMap,index),getColourPoint(selectedMap,index+1),pixValue)
  );
}


void _findMinMax(float* min, float* max){
  //Find Minimum and Maxmimum
  *min = myCamera.getPixel(0);
  *max = myCamera.getPixel(0);
  

  for (uint16_t i = 0; i < (height*width); i++){
    if (myCamera.getPixel(i) < *min){
      *min = myCamera.getPixel(i);
    }
    if (myCamera.getPixel(i) > *max){
      *max = myCamera.getPixel(i);
    }
  }
}

void _scaleMap(float min, float max){
  if ((max - min) > 0){
    for (uint8_t y = 0; y < height; y++){
      for (uint8_t x = 0; x < width; x++){
        rawImageMap[y][x] = (uint16_t)round(65535*((myCamera.getPixel(x,y)-min)/(max-min)));
      }
    }
  }  
}

void _imageToDisplay(){
  for (uint16_t y = 0; y < height*imScale; y++){
    for (uint16_t x = 0; x < width*imScale; x++){
      myDisplay.setPixel(x, y+imYOffset, frameBuffer[y][x]);
    }
  }
}


int selectedColourMap = 0;
void setColourMap(int index){
  selectedColourMap = index;
}

void _bilinearProc(){
  const uint8_t scaleFactor = imScale;
  uint16_t pixValue;
  uint16_t x1,x2,y1,y2;

  
  //for (uint8_t y = (core * ((height-2)/coreCount))+1; y < (((core+1) * ((height-2)/coreCount)))+1; y++){
  for (uint16_t y = 1; y < height; y++){
    for (uint16_t x = 1; x < width; x++){
      for (uint8_t yy = 0; yy < scaleFactor; yy++){
        for (uint8_t xx = 0; xx < scaleFactor; xx++){
          x1 = x-1;
          x2 = x;
          y1 = y-1;
          y2 = y;


          pixValue = bilinear(0,scaleFactor,0,scaleFactor,
            rawImageMap[y1][x1],rawImageMap[y2][x1],
            rawImageMap[y1][x2],rawImageMap[y2][x2],
            xx,yy
          );
          
          //pixValue = (uint8_t)round(rawImageMap[y][x]);
          frameBuffer[(y*scaleFactor)+yy][(x*scaleFactor)+xx] = _colourMap(pixValue,selectedColourMap);
          
          
        }
      }
    }
  }
  
}

void _statusToDisplay(float min, float max, float emissivity){
  char buffer[20];
  snprintf(buffer,20,"Min: %.2f C ",min);
  myDisplay.printAt(4, 110, buffer, 0x001f);
  snprintf(buffer,20,"Max: %.2f C ",max);
  myDisplay.printAt(4, 120, buffer, 0xf800);

  snprintf(buffer,20,"e: %.2f ",emissivity);
  myDisplay.printAt(4, 2, buffer, 0xffff);

  uint8_t batteryPercentage = readCachedBatteryPercentage();
  snprintf(buffer,20,"Bat: %i %%",batteryPercentage);
  uint16_t colour = 0xffff;
  if (batteryPercentage >= 75){
    colour = 0x07e0;
  }
  else if ((batteryPercentage < 75) && (batteryPercentage > 25)) {
    colour = 0xffe0;
  }
  else {
    colour = 0xf800;
  }
  myDisplay.printAt(70, 2, buffer, colour); 
}

void initVideo(){
  //32x24 thermal camera
  //128x128 screen
  myCamera = ThermalCamera(I2C_SCL,I2C_SDA);
  myDisplay = Display(SPI_CS, SPI_RST, SPI_RS, SPI_SCK, SPI_SDI, Backlight);

  initColours();
  initFilesystem();
}



void videoFrame(){
  float minTemp, maxTemp;
  myCamera.takeReading();
  _findMinMax(&minTemp,&maxTemp);
  _scaleMap(minTemp,maxTemp);
  myDisplay.clearFB();
  _bilinearProc();
  _statusToDisplay(minTemp,maxTemp,myCamera.getEmissivity());
  _imageToDisplay();
  myDisplay.refresh();
}

void lowBatteryImage(){
  myDisplay.clearFB();
  myDisplay.printAt(10, 10, "Low Battery!", 0xffff);
  myDisplay.printAt(10, 20, "Attach Charger", 0xffff);
  myDisplay.refresh();
}

void takePhoto(){
  //Take the Sample
  videoFrame();

  noInterrupts();
  File myFile = openImage();

  //Write as netpbm
  myFile.printf("P6\n%i %i\n255\n",width*imScale,height*imScale);

  //Dump each pixel
  uint8_t r;
  uint8_t g;
  uint8_t b;
  for (uint8_t y = 0; y < height*imScale; y++){
    for (uint8_t x = 0; x < width*imScale; x++){
      r = ((frameBuffer[y][x] >> 11) & 0xff) << 3;
      g = ((frameBuffer[y][x] >> 5) & 0xff) << 2;
      b = ((frameBuffer[y][x] >> 0) & 0xff) << 3;
      myFile.printf("%c%c%c",r,g,b);
    }
  }

  closeFile(myFile);
  interrupts();

  makeImageVisible();
}

void takeSample(){
  //Take the Sample
  videoFrame();

  noInterrupts();
  File myFile = openSample();

  //Write as csv
  //Dump each sample
  for (uint8_t y = 0; y < height; y++){
    for (uint8_t x = 0; x < width; x++){
      if (x == width-1){
        myFile.printf("%.2f\n",myCamera.getPixel(x,y));
      }
      else {
        myFile.printf("%f,",myCamera.getPixel(x,y));
      }
      
    }
  }

  closeFile(myFile);
  interrupts();

  makeSampleVisible();
}
