#include "device.h"
#include <vector>
#include "Keyboard.h"
#include "Buffer.h"
#include "Scancodes.h"
#include "Qwerty.h"
#include <string.h>
#include "future.h"
#include <deque>

template <typename T>
struct PendingBuffer {
  Buffer<T> buffer;
  std::deque<promise<T>> futures;
  future<T> get() {
    if (buffer.empty()) {
      futures.emplace_back();
      return futures.back().get_future();
    } else {
      return make_ready_future<T>(buffer.pop());
    }
  }
  void add(T value) {
    if (futures.empty()) {
      buffer.push(value);
    } else {
      promise<T> &p = futures.front();
      p.set_value(value);
      futures.pop_front();
    }
  }
};

static PendingBuffer<uint32_t>& inputs() {
  static PendingBuffer<uint32_t> in;
  return in;
}

future<uint32_t> getchar() {
  return inputs().get();
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
  printf("got %04X\n", sc);
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
    printf("got input %04X\n", c);
    inputs().add(c);
  }
}


