#pragma once
#define numberOfColourMaps 9
#define totalColourLength 8+2+4+2+11+5+3+5+5
uint8_t mapLengths[numberOfColourMaps] = {8, 2, 4, 2, 11, 5, 3, 5, 5};

const char* ColourMapLabels[numberOfColourMaps] = {
  "Default",
  "Black & White",
  "50 50 BW",
  "Blue & Red",
  "Fire",
  "BROYW",
  "Halo",
  "Heated Metal",
  "Extremes"
};

//Default Colourmap
//RGB
uint8_t _colours[totalColourLength][3] = {
  //Default
  {0,0,0},
  {0,0,255},
  {0,255,0},
  {255,127,0},
  {255,255,0},
  {255,0,0},
  {255,0,255},
  {255,255,255},

  //Black White
  {0,0,0},
  {255,255,255},

  //50 50
  {0,0,0},
  {0,0,0},
  {255,255,255},
  {255,255,255},

  //Blue Red
  {0,0,255},
  {255,0,0},

  //Fire
  {3,7,30},
  {55,6,23},
  {106,4,15},
  {157,2,8},
  {208,0,0},
  {220,47,2},
  {232,93,4},
  {244,140,6},
  {250,163,7},
  {255,186,8},
  {255,255,255},

  //BROYW
  {0,0,0},
  {255,0,0},
  {255,127,0},
  {255,255,0},
  {255,255,255},

  //Halo
  {0,0,0},
  {0,255,0},
  {0,0,0},

  //Heated Metal
  {0,0,0},
  {74,0,128},
  {255,0,0},
  {255,255,0},
  {255,255,255},

  //Extremes
  {0,0,255},
  {0,0,255},
  {0,0,0},
  {255,0,0},
  {255,0,0}
};
//Of 65535, padded with 65535 as endpoint
uint16_t _colourPoints[totalColourLength] = {
  //Default
  0,
  10000,
  20000,
  30000,
  40000,
  50000,
  60000,
  65535,

  //Black White
  0,
  65535,

  //50 50
  0,
  32768,
  32769,
  65535,

  //Blue Red
  0,
  65535,

  //Fire
  0,
  6666,
  13333,
  20000,
  26666,
  33333,
  40000,
  46666,
  53333,
  60000,
  65535,

  //BROYW
  0,
  16383,
  32767,
  49151,
  65535,

  //Halo
  0,
  32768,
  65535,

  //Heated Metal
  0,
  26214,
  39321,
  52428,
  65535,

  //Extremes
  0,
  6553,
  32767,
  58982,
  65535


};

int* offsets;
void initColours(){
  offsets = (int*)malloc(sizeof(int)*numberOfColourMaps);

  int counter = 0;
  for (int index = 0; index < numberOfColourMaps; index++){
    offsets[index] = counter;
    counter += mapLengths[index];
  }
}

uint16_t getColourPoint(uint8_t mapNumber, uint8_t index){
  return _colourPoints[offsets[mapNumber]+index];
}

uint16_t getColour(uint8_t mapNumber, uint8_t index, uint8_t col){
  return _colours[offsets[mapNumber]+index][col];
}

uint8_t getColourMapLength(uint8_t mapNumber){
  return mapLengths[mapNumber];
}
