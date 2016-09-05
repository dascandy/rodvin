ENTRY("_start")

SECTIONS {
  .init 0xFFFFFF0000000000 : {
    *(.init)
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
