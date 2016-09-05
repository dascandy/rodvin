#ifndef APIC_H
#define APIC_H

#include <stdint.h>

namespace Apic {
  enum {
    LocalAPIC_ID = 2,
    LocalAPIC_Version = 3,
    TPR = 8,
    PPR = 0x0A,
    EOI = 0x0B,
    LDR = 0x0D,
    Spurious = 0x0F,
    ISRBASE = 0x10,
    TMRBASE = 0x18,
    IRRBASE = 0x20,
    ESR = 0x28,
    ICR = 0x30,
    // 0x31 == ICR high bits, only in MMIO mode
    LVT_Timer = 0x32,
    LVT_Thermal = 0x33,
    LVT_Perfmon = 0x34,
    LVT_LINT0 = 0x35,
    LVT_LINT1 = 0x36,
    LVT_Error = 0x37,
    TimerInitialCount = 0x38,
    TimerCurrentCount = 0x39,
    TimerDivideConfig = 0x3E,
    SelfIPI = 0x3F,
  };
  void init();
  void write(uint8_t reg, uint32_t value);
  uint32_t read(uint8_t reg);
  void write_ICR(uint64_t value);
  uint64_t read_ICR();
}

#endif


