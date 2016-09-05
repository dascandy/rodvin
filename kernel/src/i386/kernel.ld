ENTRY("_start")

SECTIONS {
  .init 0xC0000000 : {
    *(.init)
    *(.multiboot)
  }
  .text : {
    *(COMMON)
    *(.text*)
    *(.rodata*)
  }
  .data : {
    *(.data*)
  }
  __bss_start = .;
  .bss : {
    *(.bss*)
  }
  __bss_end = .;
  /DISCARD/ : {
    *(*)
  }
  . = ALIGN(4096);
  __end = .;
}
