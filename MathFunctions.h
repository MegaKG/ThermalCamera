#pragma once
#include "pico/stdlib.h"
#include "hardware/interp.h"

//Perform linear interpolation on the hardware interpolator
uint16_t lerp(uint16_t y1, uint16_t y2, uint16_t x1, uint16_t x2, uint16_t pos){
  if ((y2-y1) == 0){
    return y1;
  }

  interp_config cfg = interp_default_config();
  interp_config_set_blend(&cfg, true);
  interp_set_config(interp0, 0, &cfg);

  cfg = interp_default_config();
  interp_set_config(interp0, 1, &cfg);

  //Set the y min and max
  interp0->base[0] = y1;
  interp0->base[1] = y2;

  //Set the fraction
  interp0->accum[1] = map(pos,x1,x2,0,255);

  return (uint16_t) interp0->peek[1];
}

uint16_t bilinear(int x1,int x2,int y1,int y2,uint16_t z11,uint16_t z12,uint16_t z21,uint16_t z22,uint16_t x, uint16_t y){
  uint16_t z_low_y, z_high_y;
  z_low_y = lerp(z12,z22,x1,x2,x);
  z_high_y = lerp(z11,z21,x1,x2,x);
  return lerp(z_high_y,z_low_y,y1,y2,y);
}