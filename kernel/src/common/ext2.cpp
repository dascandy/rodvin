#include <stdint.h>


struct ext2_superblock {
  uint32_t inodecount, blockcount, reservedcount, freeblocks, freeinodes;
  uint32_t superblockno;
  uint32_t blocksize, fragmentsize;
  uint32_t blockspergroup, fragmentspergroup, inodespergroup;
  uint32_t lastmounttime, lastwritetime;
  uint16_t mountcount, mountsbeforefsck;
  uint16_t e2fssig, fsstate, fserror, minorversion;
  uint32_t lastfscktime, fsckinterval;
  uint32_t osid;
  uint32_t majorversion;
  uint16_t rootuid, rootgid;
};


