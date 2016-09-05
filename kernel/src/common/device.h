#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include "future.h"
class Storage;
class Framebuffer;
class KeyboardDevice;

void register_storage(Storage* storage);
void register_framebuffer(Framebuffer* fb);
void register_keyboard(KeyboardDevice* keyb);
future<uint32_t> getchar();

#endif


