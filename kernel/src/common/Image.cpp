#include "Image.h"
#include "stb_image.h"
#include "vfs.h"
#include <stdio.h>

Image* Image::ConstructFromFile(const rodvin::path& filename) {
  DirEntry* de = Vfs::lookup(filename);
  if (!de) return NULL;
  File* f = de->open();
  size_t length = f->filesize();
  uint8_t *buffer = new uint8_t[length];
  f->read(buffer, length);
  delete f;
  Image* im = ConstructFromMemory(buffer, length);
  delete buffer;
  return im;
}

Image* Image::ConstructFromMemory(uint8_t* buffer, size_t bufferlength) {
  int x, y;
  int comp;
  uint8_t *data = stbi_load_from_memory(buffer, bufferlength, &x, &y, &comp, 0);
  printf("%s\n", stbi_failure_reason());
  return new Image(data, x, y, (ColorFormat)comp);
}

Image::Image(uint8_t *imagedata, size_t w, size_t h, ColorFormat fmt)
: imagedata(imagedata)
, w(w)
, h(h)
, fmt(fmt)
{
  printf("image from %p %d %d %d\n", imagedata, w, h, fmt);
}


