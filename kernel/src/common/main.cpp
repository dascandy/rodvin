#include "platform.h"
#include "Shell.h"
#include "interrupts.h"
#include "future.h"

// TODO: stop faking interrupts
void fake_interrupts() {
  onInterrupt();
  async(fake_interrupts);
}

int main(void* platformPtr) {
  platform_setup(platformPtr);
#ifdef __x86_64__
  platform_enable_interrupts();
#else
  fake_interrupts();
#endif
  Shell sh;
  while(1) {
    run_queued_tasks();
    platform_wait_for_interrupt();
  }
}
 

