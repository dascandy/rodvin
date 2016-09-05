#include "debug.h"
#include "string.h"
#include <stdint.h>
#include <stddef.h>

struct uart_regs {
  uint32_t dr;
  uint32_t pad[5];
  uint32_t fr;
  uint32_t pad2[2];
  uint32_t ibrd;
  uint32_t fbrd;
  uint32_t lcrh;
  uint32_t cr;
  uint32_t pad3;
  uint32_t imsc;
  uint32_t pad4[2];
  uint32_t icr;
};

static volatile uart_regs *regs = (uart_regs *)0x20201000;

debug::debug()
{
  regs->cr = 0x0;
  // TODO: ask for GPIO pins
  regs->icr = 0x7FF;
  regs->ibrd = 1;
  regs->fbrd = 40;
  regs->lcrh = 0x70;
  regs->imsc = 0x7F2;
  regs->cr = 0x301;
}

void debug::write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    while (regs->fr & 0x20) {}
    regs->dr = data[i];
  }
}

bool debug::readline(char *buffer, size_t limit) {
  size_t i = 0;
  do {
    while (regs->fr & 0x10) {}
    buffer[i] = regs->dr;
  } while(buffer[i++] != '\n' &&
          i < limit);
  bool fullLine = (buffer[i-1] == '\n');
  buffer[i-1] = 0;
  return fullLine;
}


