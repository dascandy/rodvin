#include "Console.h"
#include "Framebuffer.h"

Console::Console(size_t width, size_t height) 
: fixedwidth("/couriernew.fnt")
, x(0)
, y(0)
, w(width / fixedwidth.getWidthFor(" "))
, h(height / fixedwidth.height)
{
}

void Console::setchar(size_t x, size_t y, uint32_t c) {
  uint32_t tx[2] = { c, 0 };
  draw_text(fixedwidth, rodvin::string(tx), 1, x * fixedwidth.getWidthFor(" "), y * fixedwidth.height, 10, 20);
}

void Console::scrollup() {
  ::scrollup(fixedwidth.height);
}

void Console::printtext(const rodvin::string& text) {
  for (uint32_t c : text) {
    if (c == 0x08) {
      setchar(x, y, 0);
      if (x > 0) x--;
    } else if (c == '\n') {
      x = 0;
      y++;
      if (y == h) {
        y--;
        scrollup();
      }
    } else {
      setchar(x, y, c);
      x++;
      if (x == w) {
        x = 0;
        y++;
        if (y == h) {
          y--;
          scrollup();
        }
      }
    }
  }
}


