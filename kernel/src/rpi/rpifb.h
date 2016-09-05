#ifndef RPIFB_H
#define RPIFB_H
 
#include "Framebuffer.h"

class RpiFramebuffer : public Framebuffer {
public:
  RpiFramebuffer();
  void setResolution(size_t x, size_t y, size_t bufferCount) override;
  size_t getWidth() override;
  size_t getHeight() override;
  uint32_t *getBufferLine(size_t lineno) override;
private:
  struct framebuffer_data {
    uint32_t physwidth;
    uint32_t physheight;
    uint32_t virtwidth;
    uint32_t virtheight;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t disp_x;
    uint32_t disp_y;
    uint8_t *framebuffer;
    uint32_t fb_size;
  } fb_config __attribute__((aligned(16)));
  static int modeset_rv;
  static void mode_cb(uint32_t msg);
};

#endif


