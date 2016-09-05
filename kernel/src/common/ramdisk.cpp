#include "ramdisk.h"
#include <string.h>

Ramdisk::Ramdisk(void *ptr, size_t size)
: ptr(ptr)
, size(size)
{}

DiskBlock* Ramdisk::readBlock(size_t sector, size_t count) {
  // TODO: assert (sector+count <= size);
  DiskBlock *block = new DiskBlock(512 * count);
  memcpy(block->data, (char*)ptr + sector * 512, 512 * count);
  return block;
}

