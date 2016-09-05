#include "mbr.h"
#include <stdio.h>
#include <string.h>
#include "device.h"

bool storageIsMbr(Storage* s) {
  DiskBlock *b = s->readBlock(0, 1);

  bool isMbr = true;
  if (b->data[0x1FE] != 0x55 || b->data[0x1FF] != 0xAA) isMbr = false;

  delete b;
  return isMbr;
}

class MBRPartition : public Storage {
public:
  MBRPartition(Storage& parent, uint32_t startSector, uint32_t sectorCount)
  : parent(parent)
  , startSector(startSector)
  , sectorCount(sectorCount)
  {}
  DiskBlock* readBlock(size_t sector, size_t count) {
    return parent.readBlock(sector + startSector, count);
  }
private:
  Storage& parent;
  uint32_t startSector, sectorCount;
};

struct mbr_partition {
  uint8_t bootpart;
  uint8_t pad1[3];
  uint8_t type;
  uint8_t pad2[3];
  uint32_t startsector;
  uint32_t sectorcount;
};

MBRPartitioning::MBRPartitioning(Storage& parent) {
  DiskBlock* db = parent.readBlock(0, 1);
  mbr_partition p[4];
  parts = new MBRPartition*[4];
  memcpy(p, db->data + 0x1BE, sizeof(p));
  for (size_t n = 0; n < 4; n++) {
    if (p[n].type != 0) {
      printf("found partition type %02X start %08X count %08X\n", p[n].type, p[n].startsector, p[n].sectorcount);
      parts[n] = new MBRPartition(parent, p[n].startsector, p[n].sectorcount);
      register_storage(parts[n]);
    } else {
      parts[n] = NULL;
    }
  }
  delete db;
}


