#include "interrupts.h"
#include <vector>
#include <stdio.h>

std::vector<InterruptHandler*> &handlers() {
  static std::vector<InterruptHandler*> inst;
  return inst;
}

void register_interrupt(InterruptHandler* handler) {
  handlers().push_back(handler);
}

void onInterrupt() {
  printf("oninterrupt entry\n");
  for (auto handler : handlers()) {
    handler->onInterrupt();
  }
  printf("oninterrupt exit\n");
}


