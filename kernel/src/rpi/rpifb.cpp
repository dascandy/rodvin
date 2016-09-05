#include "rpifb.h"
#include "mailbox.h"
#include <stdint.h>
#include "algorithm"
#include <stdarg.h>
#include "stdio.h"
#include "ctype.h"
#include <string.h>
#include "debug.h"
#include "Color.h"
#include "device.h"

int RpiFramebuffer::modeset_rv;


void RpiFramebuffer::mode_cb(uint32_t msg)
{
  modeset_rv = msg;
}

RpiFramebuffer::RpiFramebuffer() 
{
  mailbox_register(FramebufferConfig, &mode_cb);
  register_framebuffer(this);
}

void RpiFramebuffer::setResolution(size_t x, size_t y, size_t bufferCount) {
  modeset_rv = -1;
  fb_config.physwidth = x;
  fb_config.physheight = y;
  fb_config.virtwidth = x;
  fb_config.virtheight = bufferCount * y;
  fb_config.bpp = 32;
  fb_config.disp_x = 0;
  fb_config.disp_y = 0;
  mailbox_send(FramebufferConfig, uint32_t(&fb_config) + 0x40000000);
  while (modeset_rv == -1) {
    mailbox_poll();
  }
  if (modeset_rv != 0) {
    // handle errors 
  }
}

size_t RpiFramebuffer::getWidth() {
  return fb_config.physwidth;
}

size_t RpiFramebuffer::getHeight() {
  return fb_config.physheight;
}

uint32_t *RpiFramebuffer::getBufferLine(size_t lineno) {
  return (uint32_t*)(fb_config.framebuffer + fb_config.pitch * lineno);
}

void platform_draw_text(const char *text, size_t length) {
  debug::instance().write(text, length);
}

extern "C" void LogPrint(const char* message, unsigned int messageLength) {
  platform_draw_text(message, messageLength);
}

/*
void platform_set_current_buffer(size_t buffer) {
  modeset_rv = -1;
  fb_config.disp_y = buffer * fb_config.physheight;
  mailbox_send(FramebufferConfig, uint32_t(&fb_config) + 0x40000000);
  while (modeset_rv == -1) {
    mailbox_poll();
  }
}
*/
