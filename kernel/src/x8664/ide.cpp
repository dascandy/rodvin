#include "Storage.h"
#include "pci.h"
#include "device.h"
#include "portio.h"
#include <stdio.h>

#define REG_DATA        0
#define REG_ERROR       1
#define REG_COUNT       2
#define REG_LBA1        3
#define REG_LBA2        4
#define REG_LBA3        5
#define REG_DRIVE       6
#define REG_CMD         7

class IdeStorage final : public Storage {
public:
  IdeStorage(uint32_t cmd, uint32_t ctrl, bool slave) 
  : failed(false)
  , cmd(cmd)
  , ctrl(ctrl)
  , slave(slave)
  {
    SendCommand(0, 0, 0xEC, false);
    while (true) {
      uint8_t status = GetInterruptStatus();
      if (status == 0 || status == 0xFF) {
        failed = true;
        return;
      }
      if ((status & 0x8) == 0x8) 
        break;
    }
    uint8_t buffer[512];
    ReadPio(buffer);
    bool lbasupport = ((buffer[99] & 0x2) == 0x2);
    is48bit = ((buffer[167] & 0x4) == 0x4);
    if (lbasupport) {
      register_storage(this);
    }
  }

  DiskBlock* readBlock(size_t sector, size_t count) override;
  void SendCommand(uint64_t addr, uint16_t count, uint8_t command, bool isLongCommand) {
    if (isLongCommand) {
      outb(cmd + REG_ERROR, 0x0);
      outb(cmd + REG_ERROR, 0x0);
      outb(cmd + REG_DRIVE, 0xE0 | (slave << 4));
      outb(cmd + REG_COUNT, (count >> 8) & 0xFF);
      outb(cmd + REG_COUNT, (count >> 0) & 0xFF);
      outb(cmd + REG_LBA3,  (addr >> 40) & 0xFF);
      outb(cmd + REG_LBA2,  (addr >> 32) & 0xFF);
      outb(cmd + REG_LBA1,  (addr >> 24) & 0xFF);
      outb(cmd + REG_LBA3,  (addr >> 16) & 0xFF);
      outb(cmd + REG_LBA2,  (addr >>  8) & 0xFF);
      outb(cmd + REG_LBA1,  (addr >>  0) & 0xFF);
      outb(cmd + REG_CMD,   command);
    } else {
      outb(cmd + REG_ERROR, 0x0);
      outb(cmd + REG_DRIVE, 0xE0 | (slave << 4) | (addr >> 24));
      outb(cmd + REG_COUNT, count & 0xFF);
      outb(cmd + REG_LBA3,  (addr >> 16) & 0xFF);
      outb(cmd + REG_LBA2,  (addr >>  8) & 0xFF);
      outb(cmd + REG_LBA1,  (addr >>  0) & 0xFF);
      outb(cmd + REG_CMD,   command);
    }
  }
  uint8_t GetInterruptStatus() {
    return inb(cmd + 0x7);
  }
  uint8_t WaitForInterrupt() {
    while ((GetInterruptStatus() & 0x08) == 0x0) {}
    return GetInterruptStatus();
  }
  void WritePio(const uint8_t *buffer) {
    for (int i=0; i<512; i+=2) {
      uint16_t value = buffer[i] + (buffer[i+1] << 8);
      outw(cmd, value);
    }
  }
  void ReadPio(uint8_t *buffer) {
    for (size_t i = 0; i < 512; i+=2) {
      uint16_t value = inw(cmd);
      buffer[i] = value & 0xFF;
      buffer[i+1] = (value >> 8) & 0xFF;
    }
  }
  bool failed;
private:
  uint32_t cmd, ctrl;
  bool slave;
  bool is48bit;
};

DiskBlock* IdeStorage::readBlock(size_t sector, size_t count) {
  DiskBlock* db = new DiskBlock(count * 512);
  if (is48bit) {
    SendCommand(sector, count, 0x24, true);
  } else {
    SendCommand(sector, count, 0x20, false);
  }
  for (size_t n = 0; n < count; n++) {
    WaitForInterrupt();
    ReadPio(db->data + 512 * n);
  }
  return db;
}

//#define ATA_CMD_WRITE_PIO         0x30
//#define ATA_CMD_WRITE_PIO_EXT     0x34

void createIdeStorage(pcidevice dev) {
  uint32_t bar0 = pciread32(dev, 0x10) & 0xFFFF8;
  uint32_t bar1 = pciread32(dev, 0x14) & 0xFFFF8;
  uint32_t bar2 = pciread32(dev, 0x18) & 0xFFFF8;
  uint32_t bar3 = pciread32(dev, 0x1C) & 0xFFFF8;
  if (bar0 <= 1) bar0 = 0x1F0;
  if (bar1 <= 1) bar1 = 0x3F6;
  if (bar2 <= 1) bar2 = 0x170;
  if (bar3 <= 1) bar3 = 0x376;

  IdeStorage *device = new IdeStorage(bar0, bar1, false);
  if (device->failed) delete device;
  device = new IdeStorage(bar0, bar1, true);
  if (device->failed) delete device;
  device = new IdeStorage(bar2, bar3, false);
  if (device->failed) delete device;
  device = new IdeStorage(bar2, bar3, true);
  if (device->failed) delete device;
}


