start:
  xor ax, ax
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 07C00h

  mov [disk.drive_no], dl

  push word 63
  push word 07E0h
  push word 0h
  mov edx, [bootblock.offset + 4]
  mov eax, [bootblock.offset]
  inc eax
  adc edx, 0
  push edx
  push eax
  call disk_readsector

  jmp setup


