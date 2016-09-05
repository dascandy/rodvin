OUTPUT_FORMAT("binary")
ENTRY("_start")

SECTIONS {
  .start 0xC000 : {
    *(.start*)
  }
  .all : {
    KEEP (*(.text*))
    KEEP (*(.rodata*))
    KEEP (*(.data))
    KEEP (*(COMMON))
    KEEP (*(.bss))
  }
  /DISCARD/ : {
    *(.eh_frame)
    *(.dtors)
    *(.rel*)
    *(.comment)
  }
}
