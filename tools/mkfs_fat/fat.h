#ifndef FAT_H
#define FAT_H

struct fat_hdr {
  char jmp[3];
  char oem[8];
  uint16_t bytespersector;
  uint8_t sectorspercluster;
  uint16_t reservedsectors;
  uint8_t fats;
  uint16_t direntries;
  uint16_t sectorsinvolume;
  uint8_t mediatype;
  uint16_t sectorsperfat;
  uint16_t sectorspertrack;
  uint16_t heads;
  uint32_t hiddensectors;
  uint32_t sectorsinvolume32;
} __attribute__((packed));

struct fat16_hdr : public fat_hdr {
  uint8_t driveno;
  uint8_t winntflag;
  uint8_t signature;
  uint32_t serialno;
  char volumeid[11];
  char sysid[8];
} __attribute__((packed));

struct fat32_hdr : public fat_hdr {
  uint32_t sectorsperfat32;
  uint16_t flags;
  uint16_t fatversion;
  uint32_t rootclusterno;
  uint16_t fsinfo_sector;
  uint16_t backupboot_sector;
  uint8_t reserved[12];
  uint8_t driveno;
  uint8_t winntflag;
  uint8_t signature;
  uint32_t serialno;
  char volumeid[11];
  char sysid[8];
} __attribute__((packed));

struct fat_dirent {
  char name[11];
  uint8_t attrs;
  uint8_t winntflag;
  uint8_t creation_tenths;
  uint16_t ctime;
  uint16_t cdate;
  uint16_t adate;
  uint16_t clusterhigh;
  uint16_t mtime;
  uint16_t mdate;
  uint16_t clusterlow;
  uint32_t filesize;
};

struct lfn_ent {
  uint8_t order;
  uint16_t d1[5];
  uint8_t attr;
  uint8_t entrytype;
  uint8_t csum;
  uint16_t d2[6];
  uint16_t res;
  uint16_t d3[2];
} __attribute__((packed));

#endif


