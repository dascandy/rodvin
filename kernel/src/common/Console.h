#ifndef CONSOLE_H
#define CONSOLE_H

#include "Font.h"
#include <stddef.h>
#include <stdint.h>

class Console {
public:
  Console(size_t width, size_t height);
  void printtext(const rodvin::string& text);
private:
  void setchar(size_t x, size_t y, uint32_t c);
  void scrollup();
  Font fixedwidth;
  size_t x, y;
  size_t w, h;
};

#endif


