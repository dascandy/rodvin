#include "platform.h"
#include <string.h>
#include "mem.h"
#include <stdio.h>

struct pte {
        uint64_t p:1;
        uint64_t rw:1;
        uint64_t us:1;
        uint64_t pwt:1;
        uint64_t pcd:1;
        uint64_t a:1;
        uint64_t d:1;
        uint64_t pat:1;
        uint64_t g:1;
        uint64_t avl1:3;
        uint64_t addr:40;
        uint64_t avl2:11;
        uint64_t nx:1;
};

static inline void invlpg(void* x) {
  asm volatile ("invlpg (%0)"::"r"(x));
}

void platform_map(void* virt_addr, uint64_t physaddr, MappingUse use) {
  uint64_t t;
  struct pte *entries;

  entries = (struct pte *)0xFFFFFFFFFFFFF000ULL;
  t = ((uint64_t)virt_addr >> (12+9*3));
  t &= 0x1FF;

  if (entries[t].p == 0) {
    entries[t].addr = get_free_page() >> 12;
    entries[t].p = 1;
    entries[t].rw = 1;
    entries[t].us = 1;
    entries[t].pwt = 1;
    entries[t].pcd = 1;
    memset((void *)(0xFFFFFFFFFFE00000 + (t << 12)), 0, 4096);
  }

  entries = (struct pte *)0xFFFFFFFFFFE00000ULL;
  t = ((uint64_t)virt_addr >> (12+9*2));
  t &= 0x3FFFF;

  if (entries[t].p == 0) {
    entries[t].addr = get_free_page() >> 12;
    entries[t].p = 1;
    entries[t].rw = 1;
    entries[t].us = 1;
    entries[t].pwt = 1;
    entries[t].pcd = 1;
    memset((void *)(0xFFFFFFFFC0000000 + (t << 12)), 0, 4096);
  }

  entries = (struct pte *)0xFFFFFFFFC0000000ULL;
  t = ((uint64_t)virt_addr >> (12+9*1));
  t &= 0x7FFFFFF;

  if (entries[t].p == 0) {
    entries[t].addr = get_free_page() >> 12;
    entries[t].p = 1;
    entries[t].rw = 1;
    entries[t].us = 1;
    entries[t].pwt = 1;
    entries[t].pcd = 1;
    memset((void *)(0xFFFFFF8000000000 + (t << 12)), 0, 4096);
  }

  entries = (struct pte *)0xFFFFFF8000000000;
  t = ((uint64_t)virt_addr >> (12+9*0));
  t &= 0xFFFFFFFFF;

  entries[t].addr = physaddr >> 12;
  entries[t].p = 1;
  entries[t].rw = 1;
  entries[t].us = 0;
  entries[t].nx = 1;
  entries[t].g = 1;
  entries[t].pcd = 0;
  entries[t].pwt = 0;
  entries[t].pat = 0;
  switch(use) {
  case DeviceRegisters:
  case DeviceMemory:
  case ReadWriteMemory:
    break;
  case ReadOnlyMemory:
  case CopyOnWriteMemory:
    entries[t].rw = 0;
    break;
  case ExecutableMemory:
    entries[t].nx = 0;
    break;
  }
}

uint64_t platform_unmap(void* addr) {
  uint64_t t;
  struct pte *entries;

  entries = (struct pte *)0xFFFFFFFFFFFFF000ULL;
  t = ((uint64_t)addr >> (12+9*3));
  t &= 0x1FF;
  if (entries[t].p == 0) {
    return (uint64_t)0;
  }

  entries = (struct pte *)0xFFFFFFFFFFE00000;
  t = ((uint64_t)addr >> (12+9*2));
  t &= 0x3FFFF;

  if (entries[t].p == 0) {
    return (uint64_t)0;
  }

  entries = (struct pte *)0xFFFFFFFFC0000000;
  t = ((uint64_t)addr >> (12+9*1));
  t &= 0x7FFFFFF;

  if (entries[t].p == 0) {
    return (uint64_t)0;
  }

  entries = (struct pte *)0xFFFFFF8000000000;
  t = ((uint64_t)addr >> (12+9*0));
  t &= 0xFFFFFFFFF;

  if (entries[t].p == 0) {
    return (uint64_t)0;
  }
  uint64_t p = entries[t].addr << 12;
  entries[t].p = 0;
  invlpg(addr);
  return p;
}


