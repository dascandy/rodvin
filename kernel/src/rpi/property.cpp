#include "property.h"
#include "mailbox.h"
#include <string.h>

struct {
  uint32_t buffersize;
  uint32_t code;
  uint32_t tagid;
  uint32_t bufsize;
  uint32_t datasize;
  char buffer[44];
} prop __attribute__((aligned(16)));

bool got_prop_cb = false;
void prop_cb(uint32_t) {
  got_prop_cb = true;
}

void *property_read(PropertyId id, size_t length, void* inbuf, size_t size) {
  got_prop_cb = false;
  prop.buffersize = 64;
  prop.code = 0;
  prop.tagid = id;
  prop.bufsize = length;
  prop.datasize = size;
  if (size && inbuf)
    memcpy(prop.buffer, inbuf, size);
  mailbox_register(Property, &prop_cb);
  mailbox_send(Property, uint32_t(&prop) + 0x40000000);
  while (!got_prop_cb) {
    mailbox_poll();
  }
  return prop.buffer;
}


