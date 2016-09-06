#include "device.h"
#include <vector>
#include "Keyboard.h"
#include "Buffer.h"
#include "Scancodes.h"
#include "Qwerty.h"
#include <string.h>
#include <deque>

static std::deque<uint8_t>& inputs() {
  std::deque<uint8_t> inputs;
  return inputs;
}

uint32_t getchar() {
  uint32_t front = inputs().front();
  inputs().pop_front();
  return front;
}

static std::vector<Keyboard*> keyboards;

void register_keyboard(KeyboardDevice* keyb) {
  // TODO: use this list for something?
  keyboards.push_back(new Keyboard(keyb, new Qwerty())); 
}

Keyboard::Keyboard(KeyboardDevice* device, Layout* layout) 
: device(device)
, layout(layout)
, keys(new bool[ScanCodes::Maximum])
{
  device->setListener(this);
  memset(keys, false, ScanCodes::Maximum);
}

Keyboard::~Keyboard() {
  delete device;
  delete layout;
  delete [] keys;
}

static uint32_t toupper(uint32_t ch) {
  if (ch >= 'a' && ch <= 'z') return ch - 32;
  return ch;
}

void Keyboard::onInput(ScanCode sc) {
  bool isRelease = (sc & ScanCodes::KeyReleased) != 0;
  keys[sc & 0x7FFF] = !isRelease;
  if (isRelease) {
    return;
  }

  bool shifted = keys[ScanCodes::ShiftLeft] || keys[ScanCodes::ShiftRight];
  bool upcase = shifted;
  bool altgr = keys[ScanCodes::AltRight];
  uint32_t c = layout->lookup(sc, shifted, altgr);
  if (upcase) c = toupper(c);
  if (c)
  {
    inputs().push_back(c);
  }
}


