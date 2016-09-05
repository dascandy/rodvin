#ifndef GPIO_H
#define GPIO_H

#include <stddef.h>

enum PinMode {
  Input,
  Output
};

class GpioPin {
public:
  GpioPin(size_t id);
  void setOn(bool on);
  void setMode(PinMode mode);
private:
  size_t id;
};

#endif


