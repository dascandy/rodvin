_origin:

%include "bootsector.asm"

kernel_name_32 db "KERNEL32ELF"
kernel_name_64 db "KERNEL64ELF"

%include "constants.inc"

%include "memmap.asm"
%include "setup.asm"

times (5120 - $ + _origin) db 0

loader32:
incbin "../../out/i386/boot/loader.bin"

times (12288 - $ + loader32) db 0

loader64:
incbin "../../out/x8664/boot/loader.bin"


