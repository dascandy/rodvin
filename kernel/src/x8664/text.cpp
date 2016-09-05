#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

uint16_t *textbuf = (uint16_t*)0xB8000;
size_t x, y;

void platform_draw_text(char const* text, size_t size) {
  for (size_t i = 0; i < size; i++) {
    switch(text[i]) {
      case '\n': 
        x = 0; 
        y++; 
        break;
      default:
        textbuf[y*80+x] = 0x700 | text[i];
        x++;
        if (x == 80) {
          x = 0;
          y++;
        }
        break;
    }
    if (y > 25) y = 24;
    asm ("outb %%al, %%dx"
             :: "d" (0xE9), "a" (text[i])
             );
  }
}


