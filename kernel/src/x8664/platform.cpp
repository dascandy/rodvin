#include <string.h>
#include "multiboot.h"
#include <stdio.h>
#include <stdlib.h>
#include "portio.h"
#include "pci.h"
#include "bga.h"
#include "ide.h"
#include "ps2.h"
#include "xhci.h"
#include "interrupts.h"
#include "platform.h"
#include "apic.h"

struct gdt {
  uint16_t limit;
  uint16_t base1;
  uint8_t base2;
  uint8_t access;
  uint8_t limit_flags;
  uint8_t base3;
} gdt_[] = {
  { 0, 0, 0, 0, 0, 0 },
  { 0xFFFF, 0x0000, 0x00, 0x98, 0x2F, 0x00 },
  { 0xFFFF, 0x0000, 0x00, 0xF8, 0x2F, 0x00 },
  { 0xFFFF, 0x0000, 0x00, 0x92, 0x2F, 0x00 },
  { 0xFFFF, 0x0000, 0x00, 0x89, 0x1F, 0x00 },
  { 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00 },
};

struct load {
  uint16_t size;
  uint64_t offset;
} __attribute__((packed));

static load _loadgdt = {
  sizeof(gdt_), 
  (uint64_t)gdt_
};

struct tss {
  uint32_t reserve1;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  void* ist[8];
  uint64_t reserve3;
  uint16_t reserve4;
  char iobitmap[1];
} __attribute__ ((packed));
static tss _tss;

void tss_set_ist(uint8_t ist, void* stackptr) {
  _tss.ist[ist] = stackptr;
}

static struct idt_entry {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t ist;
  uint8_t type_attr;
  uint16_t offset_middle;
  uint32_t offset_high;
  uint32_t zero;
} idt[0x100];

extern void x8664_oninterrupt();
static char stack_for_interrupts[4096] __attribute__((aligned(4096)));

static load _loadidt = {
  sizeof(idt), 
  (uint64_t)idt
};

static void disable_pic() {
  outb(0x20, 0x11);
  outb(0xa0, 0x11);
  outb(0x21, 0x20);
  outb(0xa1, 0x28);
  outb(0x21, 4);
  outb(0xa1, 2);
  outb(0x21, 1);
  outb(0xa1, 1);
  outb(0x21, 0x00);
  outb(0xa1, 0xFF);
}

static void platform_setup_interrupts() {
  disable_pic();
  
  memset(&_tss, 0, sizeof(_tss));
  printf("stack for ist = %p\n", stack_for_interrupts);
  printf("tss = %p\n", &_tss);
  tss_set_ist(1, stack_for_interrupts + sizeof(stack_for_interrupts));

  uint64_t ptr = (uint64_t)&x8664_oninterrupt;
  for (size_t n = 32; n < 256; n++) {
    idt[n].ist = 1;
    idt[n].zero = 0;
    idt[n].selector = 0x8;
    idt[n].type_attr = 0x8E;
    idt[n].offset_low = ptr & 0xFFFF;
    idt[n].offset_middle = (ptr >> 16) & 0xFFFF;
    idt[n].offset_high = (ptr >> 32) & 0xFFFFFFFF;
  }

  asm volatile ("lidt (%%rax)" :: "a"(&_loadidt));


//  60800067 00008bb9 fffff800 00000000
//  00662000 00890004 ffffff00 00000000

//0xffffff000003a340 <bogus+      32>:  0x00  0x20  0x66  0x00  0x04  0x00  0x89  0x00
//0xffffff000003a348 <bogus+      40>:  0x00  0xff  0xff  0xff  0x00  0x00  0x00  0x00

  uint64_t p = (uint64_t)&_tss;
  uint32_t size = sizeof(_tss);
  gdt_[4].access = 0x89;
  gdt_[4].limit_flags = ((size & 0xF0000) >> 16);
  gdt_[4].limit = (size & 0xFFFF);
  gdt_[4].base1 = p & 0xFFFF;
  gdt_[4].base2 = (p >> 16) & 0xFF;
  gdt_[4].base3 = (p >> 24) & 0xFF;
  gdt_[5].limit = (p >> 32) & 0xFFFF;
  gdt_[5].base1 = (p >> 48) & 0xFFFF;
  gdt_[5].base2 = 0;
  gdt_[5].limit_flags = 0;
  gdt_[5].access = 0;
  gdt_[5].base3 = 0;

  asm volatile ("lgdt (%%rax)" :: "a"(&_loadgdt));
  asm volatile ("ltr %%ax\n" :: "a"(0x20));
  printf("apic init now\n");
  Apic::init();
  printf("apic inited now\n");
}

void platform_eoi() {
  outb(0xA0, 0x20);
  outb(0x20, 0x20);
}

void platform_enable_interrupts() {
  platform_eoi();
  asm volatile ("sti");
}

void platform_disable_interrupts() {
  asm volatile ("cli");
}

void platform_wait_for_interrupt() {
  asm volatile ("hlt");
}

struct e820map {
    unsigned long long offset;
    unsigned long long size;
    unsigned int type;
    unsigned int pad;
};

extern "C" char __end;
void platform_setup(void* platform_struct) {
  e820map* map = (e820map*)platform_struct;

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

    malloc_add_region(map[n].offset, map[n].size);
  }

  platform_setup_interrupts();

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
    else if ((devClass & 0xFFFFFF00) == 0x0c033000)
      new Xhci(dev);
  });
  static PS2 ps2;
}


