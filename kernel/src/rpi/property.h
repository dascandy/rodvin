#ifndef PROPERTY_H
#define PROPERTY_H

#include <stddef.h>
#include <string.h>

enum PropertyId {
  PropertyCpuMem = 0x10005,
  PropertyGpuMem = 0x10006,
  PropertyMacAddr = 0x10003,
  PropertyPowerState = 0x28001,
  PropertyMmcBaseClock = 0x30002,
};

void *property_read(PropertyId id, size_t length, void *buffer = NULL, size_t size = 0);

template <typename T>
inline void property_read_s(PropertyId id, T &rv) {
  memcpy(&rv, property_read(id, sizeof(rv), NULL, 0), sizeof(rv));
}

template <typename T, typename T2>
inline void property_read_s(PropertyId id, T &rv, T2 data) {
  memcpy(&rv, property_read(id, sizeof(T), &data, sizeof(T2)), sizeof(rv));
}

#endif


