#ifndef RAMDISK_H
#define RAMDISK_H

#include "Storage.h"
#include <stddef.h>

class Ramdisk : public Storage {
public:
  Ramdisk(void *ptr, size_t size);
  DiskBlock* readBlock(size_t sector, size_t count) override;
private:
  void *ptr;
  size_t size;
};

#endif


