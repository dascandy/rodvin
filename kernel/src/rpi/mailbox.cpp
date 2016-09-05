#include "mailbox.h"
#include <stdint.h>

struct mailbox_regs {
  uint32_t read;
  uint32_t pad[3];
  uint32_t poll;
  uint32_t sender;
  uint32_t status;
  uint32_t config;
  uint32_t write;
};

static volatile mailbox_regs *regs = (mailbox_regs *)0x2000B880;
static void mailbox_ignore(uint32_t) {}
void (*cbs[16])(uint32_t) = {
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
  &mailbox_ignore,
};

void mailbox_send(Channel target, uint32_t message) {
  while (regs->status & 0x80000000U) {}
  regs->write = message | target;
}

void mailbox_register(Channel target, void(*cb)(uint32_t)) {
  cbs[target] = cb ? cb : mailbox_ignore;
}

void mailbox_poll() {
  while ((regs->status & 0x40000000) == 0) {
    uint32_t val = regs->read;
    Channel target = (Channel)(val & 0xF);
    val &= 0xFFFFFFF0U;
    cbs[target](val);
  }
}


