#ifndef I386_PORTIO_H
#define I386_PORTIO_H

#include <stdint.h>

inline uint8_t inb(uint16_t port) {
  uint8_t val;
  asm volatile ("inb %%dx, %%al"
    : "=a"(val)
    : "d" (port)
    );
  return val;
}

inline uint16_t inw(uint16_t port) {
  uint16_t val;
  asm volatile ("inw %%dx, %%ax"
    : "=a"(val)
    : "d" (port)
    );
  return val;
}

inline uint32_t ind(uint32_t port) {
  uint32_t val;
  asm volatile ("inl %%dx, %%eax"
    : "=a"(val)
    : "d" (port)
    );
  return val;
}

inline void outb(uint16_t port, uint8_t val) {
  asm volatile ("outb %%al, %%dx"
    :: "d" (port), "a" (val)
    );
}

inline void outw(uint16_t port, uint16_t val) {
  asm volatile ("outw %%ax, %%dx"
    :: "d" (port), "a" (val)
    );
}

inline void outd(uint16_t port, uint32_t val) {
  asm volatile ("outl %%eax, %%dx"
    :: "d" (port), "a" (val)
    );
}

inline uint64_t rdmsr(uint32_t msr) {
  uint32_t high, low;
  asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((uint64_t)high << 32) | low;
}

inline void wrmsr(uint32_t msr, uint64_t value) {
  uint32_t high = (value >> 32),
           low = value;
  asm volatile ("wrmsr" :: "c"(msr), "a"(low), "d"(high));
}

#endif


