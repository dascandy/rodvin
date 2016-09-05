#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

typedef uint16_t ScanCode;

class Layout;

class KeyboardDevice;

class Keyboard {
public:
  Keyboard(KeyboardDevice* device, Layout* layout);
  ~Keyboard();
  uint32_t getInput();
  void onInput(ScanCode sc);
private:
  KeyboardDevice* device;
  Layout* layout;
  bool* keys;
};

class KeyboardDevice {
public:
  void setListener(Keyboard* k) {
    keyboard = k;
  }
protected:
  Keyboard* keyboard;
};

#endif


