_origin:

%include "bootsector.asm"

kernel_name_64 db "KERNEL  ELF"

%include "constants.inc"

%include "memmap.asm"
%include "setup.asm"

times (12288 - $ + _origin) db 0

loader64:
incbin "../../out/x8664/boot/loader.bin"


