#include <stddef.h>
#include <stdint.h>
#include "paging.h"

typedef unsigned long phys_addr_t;
typedef signed long virt_addr_t;

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
  uint64_t addr:51;
  uint64_t nx:1;
};

#define PAGESIZE 4096
#define PAGESHIFT 12

phys_addr_t gfp_cur = 0x100000;

phys_addr_t get_free_page(void) {
  phys_addr_t retVal = gfp_cur;
  gfp_cur += PAGESIZE;

  return retVal;
}

void clearpage(virt_addr_t page) {
  long long *pg = (long long *)page;
  for (size_t i=0; i < (1 << PAGESHIFT) / (sizeof(long long)); i++) {
    pg[i] = 0;
  }
}

void map_page_to(virt_addr_t todovirt) {
  virt_addr_t t;
  struct pte *entries;

#ifdef __x86_64__
  entries = (struct pte *)0xFFFFFFFFFFFFF000ULL;
  t = ((virt_addr_t)todovirt >> (12+9*3));
  t &= 0x1FF;
  
  if (entries[t].p == 0) {
    entries[t].addr = get_free_page() >> PAGESHIFT;
    entries[t].p = 1;
    entries[t].rw = 1;
    entries[t].us = 1;
    entries[t].pwt = 1;
    entries[t].pcd = 1;
    clearpage(0xFFFFFFFFFFE00000 + (t << 12));
  }
#endif

  t = ((virt_addr_t)todovirt >> (12+9*2));
#ifdef __x86_64__
  entries = (struct pte *)0xFFFFFFFFFFE00000;
  t &= 0x3FFFF;
#else
  entries = (struct pte *)0xFFFFFFE0;
  t &= 0x3;
#endif
  
  if (entries[t].p == 0) {
    entries[t].addr = get_free_page() >> PAGESHIFT;
    entries[t].p = 1;
    entries[t].rw = 1;
    entries[t].us = 1;
    entries[t].pwt = 1;
    entries[t].pcd = 1;
    clearpage(0xFFFFFFFFC0000000 + (t << 12));
  }

  t = ((virt_addr_t)todovirt >> (12+9*1));
#ifdef __x86_64__
  entries = (struct pte *)0xFFFFFFFFC0000000;
  t &= 0x7FFFFFF;
#else
  entries = (struct pte *)0xFFFFC000;
  t &= 0x7FF;
#endif
  
  if (entries[t].p == 0) {
    entries[t].addr = get_free_page() >> PAGESHIFT;
    entries[t].p = 1;
    entries[t].rw = 1;
    entries[t].us = 1;
    entries[t].pwt = 1;
    entries[t].pcd = 1;
    clearpage(0xFFFFFF8000000000 + (t << 12));
  }

  t = ((virt_addr_t)todovirt >> (12+9*0));
#ifdef __x86_64__
  entries = (struct pte *)0xFFFFFF8000000000;
  t &= 0xFFFFFFFFF;
#else
  entries = (struct pte *)0xFF800000;
  t &= 0xFFFFF;
#endif

  if (entries[t].p == 0) {
    entries[t].addr = get_free_page() >> PAGESHIFT;
    entries[t].p = 1;
    entries[t].rw = 1;
    entries[t].us = 0;
    entries[t].g = 1;
    entries[t].nx = 0;
    entries[t].pwt = 1;
    entries[t].pcd = 1;
    clearpage(todovirt);
  }
}


