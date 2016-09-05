#include "pci.h"
#include "portio.h"
#include <stdio.h>

uint8_t pciread8(pcidevice dev, uint8_t regno) {
  uint32_t value = pciread32(dev, regno);
  return (value >> (8*(regno&3))) & 0xFF;
}

uint16_t pciread16(pcidevice dev, uint8_t regno) {
  uint32_t value = pciread32(dev, regno);
  return (value >> (8*(regno&2))) & 0xFF;
}

uint32_t pciread32(pcidevice dev, uint8_t regno) {
  outd(0xCF8, 0x80000000 |            // enable
     (dev << 8) |    // function
     (regno & 0xFC));          // register
  uint32_t value = ind(0xCFC);
  outd(0xCF8, 0x00000000);            // disable
  return value;
}

void pciwrite8(pcidevice dev, uint8_t regno, uint8_t value) {
  uint32_t cvalue = pciread32(dev, regno);
  cvalue &= ~(0xFF << (8*(regno & 3)));
  cvalue |= value << (8*(regno & 3));
  pciwrite32(dev, regno, cvalue);
}

void pciwrite16(pcidevice dev, uint8_t regno, uint16_t value) {
  uint32_t cvalue = pciread32(dev, regno);
  cvalue &= ~(0xFFFF << (8*(regno & 2)));
  cvalue |= value << (8*(regno & 2));
  pciwrite32(dev, regno, cvalue);
}

void pciwrite32(pcidevice dev, uint8_t regno, uint32_t value) {
  outd(0xCF8, 0x80000000 |
       (dev << 8) |
       (regno & 0xFC));
  outd(0xCFC, value);
  outd(0xCF8, 0x00000000);            // disable
}

void pci_detect(void (*onDevice)(pcidevice), uint8_t bus) {
  printf("pci bus detect start %d\n", bus);
  for ( unsigned int slot = 0; slot < 32; slot++ )
  {
    unsigned int num_functions = 1;
    for ( unsigned int function = 0; function < num_functions; function++ )
    {
      pcidevice dev = (bus << 8) | (slot << 3) | function;

      uint32_t vendor_device = pciread32(dev, 0);
      if (vendor_device != 0 && vendor_device != 0xFFFFFFFF)
        onDevice(dev);

      uint8_t header = pciread8(dev, 0x0E);
      if ( header & 0x80 )
        num_functions = 8;
      if ( (header & 0x7F) == 0x01 )
      {
        uint8_t subbusid = pciread8(dev, 0x19);
        pci_detect(onDevice, subbusid);
      }
    }
  }
  printf("pci bus detect end %d\n", bus);
}



