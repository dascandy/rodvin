#include "platform.h"
#include "Shell.h"
#include "interrupts.h"

int main(void* platformPtr) {
  platform_setup(platformPtr);
#ifdef __x86_64__
  platform_enable_interrupts();
#endif
  Shell sh;
  while(1) {
    sh.run();
#ifdef __x86_64__
    platform_wait_for_interrupt();
#else
// TODO: stop faking interrupts
    onInterrupt();
#endif
  }
}
 

