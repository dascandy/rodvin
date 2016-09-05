#include <string.h>
#include "multiboot.h"
#include <stdio.h>
#include <stdlib.h>
#include "pci.h"
#include "bga.h"
#include "ide.h"

struct e820map {
    unsigned long long offset;
    unsigned long long size;
    unsigned int type;
    unsigned int pad;
};

extern "C" char __end;
void platform_setup(void* platform_struct) {
  e820map* map = (e820map*)platform_struct;

  printf("map at %p\n", map);
  uint64_t endptr = 0x2400000;
  for (size_t n = 0; n < 64; n++) {
    if (map[n].offset + map[n].size <= endptr)
      continue;

    if (map[n].offset < endptr) {
      map[n].size -= endptr - map[n].offset;
      map[n].offset = endptr;
    }

    if (map[n].type != 1) 
      continue;   

    printf("region at %p %X\n", map[n].offset, map[n].size);
    malloc_add_region(map[n].offset, map[n].size);
  }
  printf("x\n");

  pci_detect([](pcidevice dev){
    uint32_t vendor_device = pciread32(dev, 0);
    bool deviceFound = true;
    switch(vendor_device) {
      case 0x11111234:
      case 0x80EEBEEF:
        new BgaFramebuffer(dev);
        break;
      default:
        deviceFound = false;
        break;
    }
    if (deviceFound) return;
    uint32_t devClass = pciread32(dev, 8);
    if ((devClass & 0xFFFF0000) == 0x01010000)
      createIdeStorage(dev);
  });
}

void platform_wait_for_interrupt() {
  asm volatile ("hlt");
}


