#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

#define PAGESHIFT 12

typedef unsigned long phys_addr_t;
typedef signed long virt_addr_t;

void paging_init(void);
phys_addr_t get_free_page(void);
void map_page_to(virt_addr_t todovirt);

#define AL_FL_GLOBAL   0x0001
#define AL_FL_WRITABLE 0x0002
#define AL_FL_USERACC  0x0004
#define AL_FL_NOEXEC   0x0008

#endif

