#ifndef BGA_H
#define BGA_H

#include "Framebuffer.h"
#include "pci.h"

class BgaFramebuffer : public Framebuffer {
public:
  BgaFramebuffer(pcidevice dev);
  void setResolution(size_t x, size_t y, size_t bufferCount) override;
  size_t getWidth() override;
  size_t getHeight() override;
  uint32_t *getBufferLine(size_t lineno) override;
private:
  size_t xres, yres, bufferId;
  uint32_t *buffer;
};

#endif


