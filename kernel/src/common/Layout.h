#ifndef LAYOUT_H
#define LAYOUT_H

#include "Keyboard.h"
#include "Layout.h"

class Layout {
public:
  virtual ~Layout() {}
  virtual uint32_t lookup(ScanCode sc, bool shifted, bool alt) = 0;
};

#endif


