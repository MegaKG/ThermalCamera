#include <Wire.h>
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#pragma once

const byte MLX90640_address = 0x33;
#define TA_SHIFT 8


class ThermalCamera {
  private:
    paramsMLX90640 mlx90640;
    float frameBuffer[24*32];
    float output[24*32];

    float emissivity = 0.95;

    void loadParam(){
      uint16_t eepromData[832];
      MLX90640_DumpEE(MLX90640_address, eepromData);
      MLX90640_ExtractParameters(eepromData, &mlx90640);
    }

  public:
    void setEmissivity(float e){
      emissivity = e;
    }

    float getEmissivity(){
      return emissivity;
    }

    ThermalCamera(){

    }

    ThermalCamera(int sclpin, int sdapin){
      Wire.setSCL(1);
      Wire.setSDA(0);
      Wire.begin();
      Wire.setClock(400000); // Set 400khz for config

      MLX90640_SetRefreshRate(MLX90640_address, 0x05);

      loadParam();

      Wire.setClock(1000000L); // Set 1MHZ for reading
    }

    void takeReading(){
      for (uint8_t x = 0 ; x < 2 ; x++){
        uint16_t rawFrame[834];
        MLX90640_GetFrameData(MLX90640_address, rawFrame);

        float vdd = MLX90640_GetVdd(rawFrame, &mlx90640);
        float Ta = MLX90640_GetTa(rawFrame, &mlx90640);

        float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature

        MLX90640_CalculateTo(rawFrame, &mlx90640, emissivity, tr, frameBuffer);
      }


      //Apply a Box Blur
      float sum;
      uint8_t app;
      for (int8_t y = 0; y < 24; y++){
        for (int8_t x = 0; x < 32; x++){
          sum = 0;
          app = 0;
          for (int8_t dy = -1; dy < 2; dy++){
            for (int8_t dx = -1; dx < 2; dx++){
              if ((((y + dy) >= 0) && ((x + dx) >= 0)) && (((y + dy) < 24) && ((x + dx) < 32))){
                sum += frameBuffer[((y+dy)*32) + x + dx];
                app += 1;
              }
            }
          }
          if (app > 0){
            output[(y*32)+x] = sum/app;
          }
          else {
            output[(y*32)+x] = 0;
          }
          
        }
      }

    }

    float getPixel(uint8_t x, uint8_t y){
      uint16_t index = ((23-y) * 32) + x;
      return output[index];
    }

    float getPixel(uint16_t index){
      return output[index];
    }
};