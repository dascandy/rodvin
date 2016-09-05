#include "power.h"
#include "mailbox.h"

static uint32_t powerEnabled = 0;

static bool powerUpdate = false;

void power_cb(uint32_t) {
  powerUpdate = true;
}

void set_power(PowerChannel pc, bool on) {
  uint32_t shiftedBit = 1 << (int(pc) + 4);
  powerEnabled = (powerEnabled & ~shiftedBit) | (on * shiftedBit);
  powerUpdate = false;
  mailbox_register(Power, &power_cb);
  mailbox_send(Power, powerEnabled);
  while (!powerUpdate) {}
}


