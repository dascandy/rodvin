#ifndef POWER_H
#define POWER_H

enum PowerChannel {
  UsbPower = 3,
};

void set_power(PowerChannel pc, bool on);

#endif


