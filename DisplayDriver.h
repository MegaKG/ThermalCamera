/*
 * Copyright (C) 2024 MegaKG.
 *
 * Licensed under the GPLV3 License.
 *
 */

#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h>
#include <SPI.h>


#pragma once
class Display {
  private:
    Adafruit_ST7735* tft = NULL;
    GFXcanvas16* frameBuffer = NULL; 

  public:
    Display(){

    }

    Display(int CS, int RST, int RS, int SCK, int SDI){
      SPI.setCS(CS);
      SPI.setSCK(SCK);
      SPI.setTX(SDI);

      tft = new Adafruit_ST7735(CS, RS, RST);
      frameBuffer = new GFXcanvas16(128, 128);

      tft->initR(INITR_144GREENTAB); //For 1.44in
      tft->fillScreen(ST77XX_BLACK);
      
      tft->setCursor(0, 0);
      tft->setTextColor(ST77XX_WHITE);
      tft->setTextWrap(true);
      tft->print("Ready");
    }

    void printAt(int x, int y, char* text, uint16_t colour){
      frameBuffer->setCursor(x, y);
      frameBuffer->setTextColor(colour);
      frameBuffer->print(text);
    }

    static uint16_t convertPixel(uint8_t r, uint8_t g, uint8_t b){
      uint16_t outPixelColour = 0;
      //Add the r component
      outPixelColour |= (0xf800 & ((r >> 3) << 11));
      outPixelColour |= (0x07e0 & ((g >> 2) << 5));
      outPixelColour |= (0x001f & ((b >> 3) << 0));
      return outPixelColour;
    }

    void setPixel(uint16_t x, uint16_t y, uint16_t pixelColour){
      frameBuffer->drawPixel(x,y,pixelColour);
    }

    void clearFB(){
      frameBuffer->fillScreen(ST77XX_BLACK);
    }

    void refresh(){
      tft->drawRGBBitmap(0, 0, frameBuffer->getBuffer(), frameBuffer->width(), frameBuffer->height());
    }

    
};
