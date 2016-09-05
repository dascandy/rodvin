#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>

enum Channel {
  Power = 0,
  FramebufferConfig = 1,
  Property = 8,
};
void mailbox_send(Channel target, uint32_t message);
void mailbox_register(Channel target, void(*cb)(uint32_t));
void mailbox_poll();

#endif


