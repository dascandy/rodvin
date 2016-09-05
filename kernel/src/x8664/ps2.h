#ifndef PS2_H
#define PS2_H

#include <stdint.h>
#include "Keyboard.h"
#include "interrupts.h"

class PS2 {
public:
  PS2();
  ~PS2();
  uint8_t ReadStatus();
  void SendCommand(uint8_t command);
  uint8_t ReadData();
  void WriteData(uint8_t value);
};

class PS2Keyboard : public KeyboardDevice, public InterruptHandler {
public:
  PS2Keyboard(PS2* bus);
private:
  int getTranslatedBuffer();
  void onInterrupt();
  PS2* bus;
  uint8_t buffer[3];
  uint8_t offset;
};

#endif


