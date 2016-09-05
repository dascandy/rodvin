#ifndef MMC_H
#define MMC_H

#include "Storage.h"

namespace RaspberryPi 
{
  class SdCard : public Storage 
  {
  public:
    SdCard();
    DiskBlock* readBlock(size_t sector, size_t count);
  };
}

#endif


