#include "apic.h"
#include "portio.h"
#include "platform.h"

namespace Apic 
{
  enum APIC_MODE {
    REGULAR = 0,
    XAPIC = 1,
    X2APIC = 2,
  } apic_mode;

  static void set_apic_mode(APIC_MODE mode) {
    static const uint32_t IA32_APIC_BASE = 0x1B;
    switch(mode) {
      case REGULAR:
        break;
      case XAPIC: 
        wrmsr(IA32_APIC_BASE, rdmsr(IA32_APIC_BASE) | 0x800);
        break;
      case X2APIC: 
        wrmsr(IA32_APIC_BASE, rdmsr(IA32_APIC_BASE) | 0xC00);
        break;
    }
    apic_mode = mode;
  }

  static bool isX2() {
    return false;
//    return (cpuid(1) & 0x200000) == 0x200000;
  }

  static bool isX() {
    return false;
  }

  static volatile uint32_t *regs = (uint32_t *)0xFFFFFF00F0000000;
  static volatile struct ioapic {
    uint32_t address;
    uint32_t pad[3];
    uint32_t data;
  } *ioapic = (struct ioapic*)0xFFFFFF00E0000000;

  void ioapic_set_interrupt(uint8_t intsrc, uint8_t intno, uint8_t destcpu) {
    ioapic->address = intsrc * 2 + 0x10;
    ioapic->data |= 0x10000;
    ioapic->address = intsrc * 2 + 0x11;
    ioapic->data = (uint32_t)destcpu << 24;
    ioapic->address = intsrc * 2 + 0x10;
    ioapic->data = (ioapic->data & 0xF000) | intno;
  }

  void init() {
    if (isX2()) {
      set_apic_mode(X2APIC);
    } else {
      platform_map((void*)regs, 0xFEE00000, DeviceRegisters);
      if (isX()) {
        set_apic_mode(XAPIC);
      } else {
        set_apic_mode(REGULAR);
      }
    }

    write(Spurious, read(Spurious) | 0x100);
    platform_map((void*)ioapic, 0xFEC00000, DeviceRegisters);

//    for (size_t n = 0; n < 24; n++) {
    int n = 1; {
      ioapic_set_interrupt(n, 0x20 + n, 0);
    }
  }

  void write(uint8_t reg, uint32_t value)
  {
    if (apic_mode == X2APIC)
      wrmsr(0x800 + reg, value);
    else
      regs[reg*4] = value;
  }

  uint32_t read(uint8_t reg)
  {
    if (apic_mode == X2APIC)
      return rdmsr(0x800 + reg);
    else
      return regs[reg*4];
  }

  void write_ICR(uint64_t value)
  {
    if (apic_mode != X2APIC)
      write(ICR+1, value >> 32);
    write(ICR, value);
  }

  uint64_t read_ICR()
  {
    if (apic_mode == X2APIC)
      return read(ICR);
    return ((uint64_t)read(ICR+1) << 32) | (read(ICR));
  }
}

void x8664_endofinterrupt() {
  Apic::write(Apic::EOI, 0);
}


