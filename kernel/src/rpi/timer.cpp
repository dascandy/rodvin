#include "timer.h"
#include <stdint.h>
#include <stddef.h>

static volatile uint64_t *counter = (uint64_t*)0x20003004;

uint64_t get_time_us() {
  return *counter;
}

void wait_us(size_t amount) {
  uint64_t origCtr = get_time_us();
  uint64_t targetCtr = origCtr + amount;
  while (targetCtr > get_time_us()) {}
};


