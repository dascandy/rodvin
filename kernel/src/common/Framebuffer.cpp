#include <algorithm>
#include "Color.h"
#include "device.h"
#include "Font.h"
#include "Framebuffer.h"
#include "Image.h"
#include <stdint.h>

static Framebuffer* rootfb = NULL;

void register_framebuffer(Framebuffer* fb) {
  if (!rootfb) {
    rootfb = fb;
    rootfb->setResolution(1280, 720, 1);
  }
}

void draw_image(const Image& image, size_t xoff, size_t yoff, size_t tx, size_t ty, size_t w, size_t h) {
  printf("img %d/%d\n", tx, ty);
  size_t drawY = std::min(rootfb->getHeight() - ty, h);
  size_t drawX = std::min(rootfb->getWidth() - tx, w);
  for (size_t j = 0; j < drawY; j++) {
    uint32_t *line = rootfb->getBufferLine(ty+j);
    for (size_t i = 0; i < drawX; i++) {
      line[i+tx] = image.sample(xoff+i, yoff+j);
    }
  }
}

void draw_square(size_t x, size_t y, size_t w, size_t h, Color c) {
  size_t drawY = std::min(rootfb->getHeight() - y, h);
  size_t drawX = std::min(rootfb->getWidth() - x, w);
  for (size_t j = 0; j < drawY; j++) {
    uint32_t *line = rootfb->getBufferLine(y);
    for (size_t i = 0; i < drawX; i++) {
      line[i+x] = c.toInt();
    }
  }
}

void scrollup(size_t lines) {
  for (size_t n = 0; n < (rootfb->getHeight() - lines); n++) {
    memcpy(rootfb->getBufferLine(n), rootfb->getBufferLine(n+lines), rootfb->getWidth() * 4);
  }
  for (size_t n = rootfb->getHeight() - lines; n < rootfb->getHeight(); n++) {
    memset(rootfb->getBufferLine(n), 0, rootfb->getWidth() * 4);
  }
}

void draw_text(const Font& font, const rodvin::string &str, size_t /*length*/, size_t x, size_t y, size_t /*w*/, size_t /*h*/) {
  // TODO: also use w/h
  // TODO: use some form of scaling
  size_t cx = x, cy = y;
  uint32_t lastChar = 0;
  printf("draw with font\n");
  for (uint32_t letter : str) {
    cx += font.getKerning(lastChar, letter);
    Font::glyph *g = font.getGlyph(letter);
    lastChar = letter;
    printf("%d = %p\n", letter, g);
    if (!g) continue;
    printf("%d %d/%d %d %d/%d\n", letter, g->xoff, g->yoff, g->xadv, g->w, g->h);
    draw_image(*font.image, g->x, g->y, cx + g->xoff, cy + g->yoff, g->w, g->h);

    cx += g->xadv;
  }
}


