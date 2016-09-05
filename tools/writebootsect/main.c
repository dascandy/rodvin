#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int disk = open(argv[1], O_RDWR);
  int sector = open(argv[2], O_RDONLY);
  uint8_t bs[512];
  read(sector, bs, sizeof(bs));
  close(sector);
  uint8_t boot[512];
  read(disk, boot, sizeof(boot));
  lseek(disk, 0, SEEK_SET);
  for (size_t n = 0; n < 3; n++) {
    boot[n] = bs[n];
  }
  for (size_t n = 90; n < sizeof(boot); n++) {
    boot[n] = bs[n];
  }
  write(disk, boot, sizeof(boot));
  fsync(disk);
  close(disk);
}


