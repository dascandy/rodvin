#ifndef XHCI_H
#define XHCI_H

#include "pci.h"
#include "UsbHost.h"

class Xhci : public UsbHost {
public:
  Xhci(pcidevice dev);
private:
  void start();
  pcidevice dev;
};

#endif


