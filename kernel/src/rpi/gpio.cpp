#include "gpio.h"
#include <stdint.h>

static volatile uint32_t *gpio = (uint32_t*)0x20200000;

GpioPin::GpioPin(size_t id)
: id(id)
{}

void GpioPin::setOn(bool on)
{
  gpio[on ? 10 : 7] = (1 << id);
}

void GpioPin::setMode(PinMode mode)
{
  size_t reg = id / 10, offs = 3 * (id % 10);
  uint32_t val = gpio[reg];
  val &= (~(7 << offs));
  val |= mode << offs;
  gpio[reg] = val;
}


