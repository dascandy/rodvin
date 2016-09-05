#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stddef.h>

enum MappingUse {
  DeviceRegisters,
  DeviceMemory,
  ReadWriteMemory,
  ReadOnlyMemory,
  CopyOnWriteMemory,
  ExecutableMemory,
};

void platform_setup(void* platform_ptr);
void platform_draw_text(const char *text, size_t length);
void platform_map(void* virt_addr, uint64_t physaddr, MappingUse use);
uint64_t platform_unmap(void* addr);

void platform_enable_interrupts();
void platform_wait_for_interrupt();
void platform_disable_interrupts();

#endif


