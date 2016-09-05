#include "bga.h"
#include "portio.h"
#include "device.h"
#include "pci.h"
#include "platform.h"
#include <stdio.h>

enum BgaReg {
  Id,
  XRes,
  YRes,
  Bpp,
  Enable,
  Bank,
  VirtWidth,
  VirtHeight,
  XOffset,
  YOffset
};

#define VIRTUAL_ADDRESS 0xD0000000

static size_t xres = 1920;
static size_t yres = 1080;

static inline void write_reg(BgaReg reg, uint16_t value) {
  outw(0x1CE, reg);
  outw(0x1CF, value);
}

static inline uint16_t read_reg(BgaReg reg) {
  outw(0x1CE, reg);
  return inw(0x1CF);
}

BgaFramebuffer::BgaFramebuffer(pcidevice dev) 
{
  register_framebuffer(this);
  uint32_t physaddr = pciread32(dev, 0x10) & 0xFFFFFFF0;
  for (size_t n = 0; n < 4096; n++) {
    platform_map((void*)(VIRTUAL_ADDRESS + n * 4096), physaddr + n * 4096, DeviceMemory);
  }
}

void BgaFramebuffer::setResolution(size_t x, size_t y, size_t bufferCount) {
  // TODO: any checking on result values
  xres = x;
  yres = y;
  bufferId = 0;
  write_reg(Enable, 0);
  write_reg(XRes, x);
  write_reg(YRes, y);
  write_reg(Bpp, 32);
  write_reg(VirtWidth, x);
  write_reg(VirtHeight, bufferCount * y);
  write_reg(XOffset, 0);
  write_reg(YOffset, 0);
  write_reg(Enable, 0x61);
}

size_t BgaFramebuffer::getWidth() {
  return xres;
}

size_t BgaFramebuffer::getHeight() {
  return yres;
}

uint32_t *BgaFramebuffer::getBufferLine(size_t lineno) {
  return (uint32_t*)VIRTUAL_ADDRESS + (bufferId * yres + xres) * lineno;
}
/*
void platform_set_current_buffer(size_t buffer) {
  write_reg(YOffset, buffer*yres);
}
*/
