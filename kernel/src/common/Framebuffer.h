#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stddef.h>
#include "Color.h"
#include <string>

class Font;
class Image;

void draw_image(const Image& image, size_t xoff, size_t yoff, size_t tx, size_t ty, size_t w, size_t h);
void draw_text(const Font& font, const rodvin::string&, size_t length, size_t x, size_t y, size_t w, size_t h);
void draw_square(size_t x, size_t y, size_t w, size_t h, Color c);
void scrollup(size_t pixels);

class Framebuffer {
public:
  virtual void setResolution(size_t x, size_t y, size_t bufferCount) = 0;
  virtual size_t getWidth() = 0;
  virtual size_t getHeight() = 0;
  virtual uint32_t *getBufferLine(size_t lineno) = 0;
};

#endif


