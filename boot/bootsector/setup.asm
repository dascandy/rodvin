BITS 16

setup:

  ; clear 3 pages worth
  xor eax, eax
  mov es, ax
  mov cx, 1000h
  mov di, cx
  rep stosd
  ; Identity-map the first 16M using 2M pages
  mov [es:3000h], dword 0000087h
  mov [es:3008h], dword 0200087h
  mov [es:3010h], dword 0400087h
  mov [es:3018h], dword 0600087h
  mov [es:3020h], dword 0800087h
  mov [es:3028h], dword 0A00087h
  mov [es:3030h], dword 0C00087h
  mov [es:3038h], dword 0E00087h

  mov eax, 80000000h
  cpuid
  cmp eax, 80000000h
  jle nolm ;pm_start
  mov eax, 80000001h
  cpuid
  test edx, LM
  jnz lm_start

nolm:
  jmp nolm

;pm_start:
;  push 1000h
;  pop es
;  xor bx, bx
;  mov bp, kernel_name_32
;  call ReadFile
;  mov esi, eax
;
;  call load_memmap
;
;  cli
;  ; build paging tables
;  xor di, di
;  mov es, di
;  mov di, 1000h
;  mov [es:2FF8h], dword 02001h
;  mov [es:2FE0h], dword 03001h
;  mov eax, 02FE0h
;
;  lgdt [gdt32_load]
;  mov cr3, eax
;  mov eax, cr4
;  or eax, PAE | PSE
;  mov cr4, eax
;  mov eax, PG | PE
;  mov cr0, eax
;  jmp 08h:pm_entry ; jump required to reload CS & to actually actuate the switch
;
;; it is a gdt... no matter why
;gdt32_load:
;  dw 1Fh
;gdt32:
;  dq gdt32
;  dd 0FFFFh,000CF9800h
;  dd 0FFFFh,000CFF800h
;  dd 0FFFFh,000CF9200h
;
;BITS 32
;
;pm_entry:
;  mov ax, 18h
;  mov ss, ax
;  mov es, ax
;  mov ds, ax
;  mov esp, 07b00h
;  mov ebx, 010000h
;  push ebx
;  call loader32
;pm_entry_ret:
;  jmp pm_entry_ret
;
BITS 16
lm_start:
  push 1000h
  pop es
  xor bx, bx
  mov bp, kernel_name_64
  call ReadFile
  mov esi, eax

  call load_memmap

  cli
  ; build paging tables
  xor di, di
  mov es, di
  mov di, 1000h
  ; Identity-map the first 16M using 2M pages
  mov [es:1ff8h], dword 01007h
  mov [es:1000h], dword 02007h
  mov [es:2000h], dword 03007h
  mov eax, 01000h

  ; Trigger switch to LM directly
  mov cr3, eax
  lgdt [gdt64_load]
  mov eax, cr4
  or eax, PAE | OSFXSR | OSXMMEXCPT
  mov cr4, eax
  mov ecx, EFER
  rdmsr
  or eax, LME | SCE | NXE
  wrmsr
  mov eax, PG | PE | MP
  mov cr0, eax
  jmp 08h:lm_entry ; jump required to reload CS & to actually actuate the switch

BITS 64

gdt64_load:
  dw 1Fh
gdt64:
  dq gdt64
  dd 0FFFFh,0002F9800h
  dd 0FFFFh,0002FF800h
  dd 0FFFFh,0002F9200h

lm_entry:
  mov ax, 18h
  mov es, ax
  mov ds, ax
  mov ss, ax
  mov rsp, 07c00h
  mov edi, 010000h
  jmp loader64


BITS 16
