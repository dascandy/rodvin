#ifndef STARFS_H
#define STARFS_H

#include <stdint.h>

#define STARFS_SIGNATURE 0x2A5346726174532AULL
//#define STARFS_SIGNATURE 0x2A5374617246532AULL // Big-endian variant
#define INVALID_INODE 0xFFFFFFFFFFFFFFFFULL
#define INVALID_BLOCK 0xFFFFFFFFFFFFFFFFULL
#define INO_EXTENT_INDIRECTION 0x00000003
#define INO_SYSTEM 0x00000004
#define INO_COPY_ON_WRITE 0x00000008
#define INO_MODIFIED 0x00000010
#define INO_READONLY 0x00000020
#define INO_DIRECTORY 0x00000040

struct fsheader {
  uint64_t signature;
  uint64_t blockcount;
};

struct extent {
  uint64_t startblock;
  uint64_t length;
};

struct inode {
  uint32_t flags;
  uint32_t linkcount;
  uint64_t filesize;
  struct extent ext;
};

struct dirformat {
  uint32_t formatid;
  uint32_t entrycount;
};

struct direntry {
  uint32_t inodeno;
  uint16_t filenamelength;
  char filename[1];
} __attribute__((__packed__));

#endif


