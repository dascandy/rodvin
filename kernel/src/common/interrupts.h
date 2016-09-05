#ifndef INTERRUPTS_H
#define INTERRUPTS_H

class InterruptHandler {
public:
  virtual void onInterrupt() = 0;
};

void register_interrupt(InterruptHandler* handler);
void onInterrupt();

#endif


