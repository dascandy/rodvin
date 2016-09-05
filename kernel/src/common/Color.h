#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>
#include "math.h"

class Color
{
public:
  float r, g, b;
  Color(float r = 0.0f, float g = 0.0f, float b = 0.0f)
  : r(r)
  , g(g)
  , b(b)
  {
  }
  uint32_t toInt() {
    uint8_t rv = round(r * 255);
    uint8_t gv = round(g * 255);
    uint8_t bv = round(b * 255);
    return (rv << 16) | (gv << 8) | (bv);
  }
};

#endif

