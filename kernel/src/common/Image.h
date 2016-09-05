#ifndef IMAGE_H
#define IMAGE_H

#include <stddef.h>
#include <stdint.h>
#include <path>

enum ColorFormat {
  FMT_Lum = 1,
  FMT_LumA,
  FMT_RGB,
  FMT_RGBA,
};

class Image {
public:
  static Image* ConstructFromFile(const rodvin::path& filename);
  static Image* ConstructFromMemory(uint8_t* buffer, size_t bufferlength);
  uint32_t sample(int x, int y) const {
    switch(fmt) {
    case FMT_Lum:  return 0x10101 * imagedata[y*w+x];
    case FMT_LumA: return 0x10101 * imagedata[(y*w+x) * 2];
    case FMT_RGB:  return (imagedata[(y*w+x) * 3 + 0] << 16) |
                         (imagedata[(y*w+x) * 3 + 1] << 8) |
                         (imagedata[(y*w+x) * 3 + 2] << 0);
    case FMT_RGBA: 
    default:       return *(uint32_t*)(imagedata + (y*w+x) * 4);
    }
  }
  size_t width() { return w; }
  size_t height() { return h; }
private:
  Image(uint8_t *imagedata, size_t x, size_t y, ColorFormat fmt);
  uint8_t *imagedata;
  size_t w, h;
  ColorFormat fmt;
};

#endif


