#include <stdlib.h>
#include "property.h"
#include <string.h>
#include "ui.h"
#include "mmc.h"
#include "device.h"
#include "rpifb.h"

extern char __end;

void setup_memory() {
  struct mem_region {
    uint64_t start;
    size_t length;
  } region;
  property_read_s(PropertyCpuMem, region);
  // Tell it not to use the kernel space
  if (region.start < (uint32_t)&__end) {
    region.length -= ((uint32_t)&__end - region.start);
    region.start = (uint32_t)&__end;
  }
  // Tell it to skip over the 64MB ramdisk at 128M
  if (region.start + region.length > 0x8000000) {
    if (region.start + region.length > 0xC000000) {
      malloc_add_region(0xC000000, region.start + region.length - 0xC000000);
    }
    region.length = 0x8000000 - region.start;
  }

  malloc_add_region(region.start, region.length);
}

void platform_setup(void*) {
  setup_memory();
  static RpiFramebuffer fb;
  static RaspberryPi::SdCard sdcard;
}

void platform_wait_for_interrupt() {
  asm volatile ("wfi");
}


