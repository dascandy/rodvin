#define _LARGEFILE64_SOURCE

#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "fat.h"

struct fatparams {
  size_t reservedSectors;
  size_t sectorsPerCluster;
  size_t clusterCount;
  size_t fatsize;
};

fatparams determineFatParams(size_t sectorCount) {
  // start from 4k cluster size and go up until it fits
  fatparams p;
  p.
}

static void writeFatHeader(int file, size_t spc, size_t clustercount) {
  size_t reserved_sectors = 6;
  size_t sectorsperfat = (clustercount + 127) / 128;
  size_t sectorsinvolume = clustercount * spc + sectorsperfat + reserved_sectors;
  fat32_hdr hdr;
  memcpy(hdr.oem, "_RODVIN_", 8);
  hdr.bytespersector = 512;
  hdr.sectorspercluster = spc;
  hdr.reservedsectors = reserved_sectors;
  hdr.fats = 2;
  hdr.direntries = 0;
  hdr.sectorsinvolume = sectorsinvolume < 65536 ? sectorsinvolume : 0;
  hdr.mediatype = 0xF8;
  hdr.sectorsperfat = sectorsperfat < 65536 ? sectorsperfat : 0;
  hdr.sectorspertrack = 63;
  hdr.heads = 16;
  hdr.hiddensectors = 0;
  hdr.sectorsinvolume32 = sectorsinvolume >= 65536 ? sectorsinvolume : 0;
  hdr.sectorsperfat32 = sectorsperfat >= 65536 ? sectorsperfat : 0;
  hdr.flags = 0;
  hdr.fatversion = 0x0101;
  hdr.rootclusterno = 2;
  hdr.fsinfo_sector = -1;
  hdr.backupboot_sector = 6;
  hdr.driveno = 0x80;
  hdr.winntflag = 0;
  hdr.signature = 0x28;
  hdr.serialno = 0xDEADBEEF;
  memcpy(hdr.volumeid, "RODVIN FMTD", 11);
  memcpy(hdr.sysid, "_RODVIN_", 8);
}

int main(int argc, char **argv) {
  uint64_t disksize;
  char *buffer = (char*)malloc(32768);
  memset(buffer, 0, 32768);
  int fd = -1;
  if (argc == 2) {
    fd = open(argv[1], O_RDWR);
    if (fd) {
      disksize = lseek64(fd, 0, SEEK_END);
    }
  } else if (argc == 3) {
    disksize = strtoull(argv[2], NULL, 10);
    fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0755);
    lseek64(fd, disksize-1, SEEK_SET);
    write(fd, buffer, 1);
  }
  
  if (fd == -1) {
    printf("Usage: %s targetfile [size_in_bytes]\n", argv[0]);
    exit(-1);
  }

  

  close(fd);
}


