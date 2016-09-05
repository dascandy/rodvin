#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>
#include <stddef.h>

struct DiskBlock {
  DiskBlock(size_t size)
  : size(size)
  , data(new uint8_t[size])
  { 
  }
  ~DiskBlock() {
    delete data;
  }
  size_t size;
  uint8_t *data;
};

class Storage {
public:
  virtual DiskBlock* readBlock(size_t sector, size_t count) = 0;
};

#endif


