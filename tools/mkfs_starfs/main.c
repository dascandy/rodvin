#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>
#include "starfs.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char **argv) {
  unsigned long long disksize;
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

  unsigned long long inodecount = disksize / 16384;
  lseek64(fd, 0, SEEK_SET);
  write(fd, buffer, 32768);
  lseek64(fd, 0x8000, SEEK_SET);
  struct fsheader hdr;
  hdr.signature = STARFS_SIGNATURE;
  hdr.blockcount = disksize / 4096;
  write(fd, &hdr, sizeof(hdr));

  struct inode inodetbl[128];
  memset(inodetbl, 0, 4096);
  // Boot block
  inodetbl[0].flags = INO_SYSTEM | INO_READONLY;
  inodetbl[0].linkcount = 1;
  inodetbl[0].filesize = 32768;
  inodetbl[0].ext.startblock = 0;
  inodetbl[0].ext.length = 8;
  // Partition header
  inodetbl[1].flags = INO_SYSTEM | INO_READONLY;
  inodetbl[1].linkcount = 1;
  inodetbl[1].filesize = 4096;
  inodetbl[1].ext.startblock = 8;
  inodetbl[1].ext.length = 1;
  // Inode table
  inodetbl[2].flags = INO_SYSTEM | INO_READONLY;
  inodetbl[2].linkcount = 1;
  inodetbl[2].filesize = inodecount * sizeof(struct inode);
  inodetbl[2].ext.startblock = 9;
  inodetbl[2].ext.length = (inodecount + 127) / 128;
  // Root directory
  inodetbl[3].flags = INO_DIRECTORY;
  inodetbl[3].linkcount = 1;
  inodetbl[3].filesize = 8 + 17 + 14 + 14 + 21 + 12 + 17;
  inodetbl[3].ext.startblock = inodetbl[2].ext.startblock + inodetbl[2].ext.length;
  inodetbl[3].ext.length = 1;
  // Free block table
  inodetbl[4].flags = INO_SYSTEM | INO_READONLY;
  inodetbl[4].linkcount = 1;
  inodetbl[4].ext.startblock = inodetbl[3].ext.startblock + inodetbl[3].ext.length;
  inodetbl[4].filesize = disksize - (inodetbl[4].ext.startblock * 4096);
  inodetbl[4].ext.length = disksize / 4096 - inodetbl[4].ext.startblock;
  // Bad block table
  inodetbl[5].flags = INO_SYSTEM | INO_READONLY;
  inodetbl[5].linkcount = 1;
  inodetbl[5].filesize = 0;
  inodetbl[5].ext.startblock = INVALID_BLOCK;
  inodetbl[5].ext.length = 0;

  lseek64(fd, 32768 + 4096, SEEK_SET);
  write(fd, inodetbl, 4096);

  memset(buffer, 0, 4096);
  struct dirformat *fmt = (struct dirformat *)buffer;
  fmt->formatid = 1;
  fmt->entrycount = 6;

  struct direntry *ent = (struct direntry *)(buffer + 8);
  ent->inodeno = 0;
  ent->filenamelength = 11;
  memcpy(ent->filename, "$bootblock", ent->filenamelength);

  ent = (struct direntry *)(buffer + 8 + 17);
  ent->inodeno = 1;
  ent->filenamelength = 8;
  memcpy(ent->filename, "$header", 8);

  ent = (struct direntry *)(buffer + 8 + 17 + 14);
  ent->inodeno = 2;
  ent->filenamelength = 8;
  memcpy(ent->filename, "$inodes", 8);

  ent = (struct direntry *)(buffer + 8 + 17 + 14 + 14);
  ent->inodeno = 3;
  ent->filenamelength = 15;
  memcpy(ent->filename, "$rootdirectory", 15);

  ent = (struct direntry *)(buffer + 8 + 17 + 14 + 14 + 21);
  ent->inodeno = 4;
  ent->filenamelength = 6;
  memcpy(ent->filename, "$free", 6);

  ent = (struct direntry *)(buffer + 8 + 17 + 14 + 14 + 21 + 12);
  ent->inodeno = 5;
  ent->filenamelength = 11;
  memcpy(ent->filename, "$badblocks", 11);

  lseek64(fd, 4096 * inodetbl[3].ext.startblock, SEEK_SET);
  write(fd, buffer, 4096);

  free(buffer);
  close(fd);
}


