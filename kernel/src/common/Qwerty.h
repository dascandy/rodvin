#ifndef QWERTY_H
#define QWERTY_H

#include "Layout.h"

class Qwerty : public Layout {
public:
  uint32_t lookup(ScanCode sc, bool shifted, bool alt) override;
};

#endif


