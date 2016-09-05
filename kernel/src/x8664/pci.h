#ifndef PCI_H
#define PCI_H

#include <stdint.h>

typedef uint32_t pcidevice;

void pci_detect(void (*onDevice)(pcidevice), uint8_t bus = 0);
uint8_t pciread8(pcidevice dev, uint8_t regno);
uint16_t pciread16(pcidevice dev, uint8_t regno);
uint32_t pciread32(pcidevice dev, uint8_t regno);
void pciwrite8(pcidevice dev, uint8_t regno, uint8_t value);
void pciwrite16(pcidevice dev, uint8_t regno, uint16_t value);
void pciwrite32(pcidevice dev, uint8_t regno, uint32_t value);

#endif


