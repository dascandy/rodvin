#include "device.h"
#include "Storage.h"
#include "mbr.h"
#include "fat.h"
#include <stdio.h>

bool storageIsFat(Storage* s);
bool storageIsMbr(Storage* s);

void register_storage(Storage* storage) {
  printf("got storage\n");
  if (storageIsFat(storage)) {
    printf("is fat\n");
    new FatFilesystem(storage);
  } else if (storageIsMbr(storage)) {
    printf("is mbr\n");
    new MBRPartitioning(*storage);
  }
}


