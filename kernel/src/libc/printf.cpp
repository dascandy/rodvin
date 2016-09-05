#include <stdint.h>
#include <algorithm>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "../common/platform.h"

void hexdump(const uint8_t *buf, size_t length) {
  for (size_t line = 0; line < length; line += 16) {
    for (size_t ch = 0; ch < 16; ch++) {
      if (line + ch < length) {
        printf("%02X ", buf[line + ch]);
      } else {
        printf("   ");
      }
      if (ch == 7) 
        printf(" ");
    }
    printf(" |");
    for (size_t ch = 0; ch < 16; ch++) {
      if (line + ch < length) {
        printf("%c", isprint(buf[line+ch]) ? buf[line+ch] : '.');
      } else {
        printf(" ");
      }
    }
    printf("|\n");
  }
}

void printf(const char *fmt, ...) {
  va_list l;
  va_start(l, fmt);
  size_t start = 0, len = 0;
  while (fmt[start+len]) {
    if (fmt[start+len] == '%') {
      char prefix = '0';
      size_t minLen = 1;
      platform_draw_text(fmt+start, len);
      start += len + 1;
      len = 0;
      if (fmt[start] == '0') {
        prefix = '0';
        start++;
      } else if (fmt[start] == ' ') {
        prefix = ' ';
        start++;
      }
      if (fmt[start] >= '0' && fmt[start] <= '9') {
        minLen = fmt[start] - '0';
        start++;
      }
      switch(fmt[start]) {
      case 'u':
        {
          size_t radix = 10;
          uint32_t val = va_arg(l, uint32_t);
          char buf[40];
          buf[39] = 0;
          size_t offs = 39;
          while (val > 0) {
            buf[offs] = '0' + (val % radix);
            if (buf[offs] > '9') buf[offs] += 'A' - '9' - 1;
            val /= radix;
            offs--;
          }
          while (offs > 39 - minLen) {
            buf[offs--] = prefix;
          }
          platform_draw_text(buf+offs+1, 39-offs);
        }
        break;
      case 'd':
        {
          size_t radix = 10;
          int32_t ival = va_arg(l, int32_t);
          bool negative = (ival < 0);
          uint32_t val = (negative ? -ival : ival);
          char buf[40];
          buf[39] = 0;
          size_t offs = 39;
          while (val > 0) {
            buf[offs] = '0' + (val % radix);
            if (buf[offs] > '9') buf[offs] += 'A' - '9' - 1;
            val /= radix;
            offs--;
          }
          while (offs > 39 - minLen) {
            buf[offs--] = prefix;
          }
          if (negative)
            buf[offs--] = '-';
          platform_draw_text(buf+offs+1, 39-offs);
        }
        break;
      case 'X':
      case 'p':
        {
          size_t radix = 16;
          uint64_t val;
          if (sizeof(void*) == 8)
            val = va_arg(l, uint64_t);
          else
            val = va_arg(l, uint32_t);
          char buf[40];
          buf[39] = 0;
          size_t offs = 39;
          while (val > 0) {
            buf[offs] = '0' + (val % radix);
            if (buf[offs] > '9') buf[offs] += 'A' - '9' - 1;
            val /= radix;
            offs--;
          }
          while (offs > 39 - minLen) {
            buf[offs--] = prefix;
          }
          platform_draw_text(buf+offs+1, 39-offs);
        }
        break;
/*
  // TODO: reimplement double support by actually printing it
      case 'f':
        {
          double val = va_arg(l, double);
          platform_draw_text("some float", 10);
        }
        break;
*/
      case 'c':
        {
          char c = va_arg(l, int);
          platform_draw_text(&c, 1);
        }
        break;
      case 's':
        {
          char* p = va_arg(l, char*);
          platform_draw_text(p, strlen(p));
        }
        break;
      }
      start++;
    } else {
      len++;
    }
  }
  platform_draw_text(fmt+start, len);
  va_end(l);
}


