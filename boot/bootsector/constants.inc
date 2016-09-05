; !!! constants for use in control registers
; Use in CR0
PG  equ 80000000h
PE  equ 00000001h
MP  equ 00000002h
  or eax, PG | PE | MP

; Use in CR4
PSE equ 00000010h
PAE equ 00000020h
OSFXSR equ 00000200h
OSXMMEXCPT equ 00000400h

; Use in EFER
NXE   equ 00000800h
LME   equ 00000100h
SCE   equ 00000001h

; !!! constants for CPUID
; Check bits for CPUID features
LM equ 20000000h

; !!! MSR definitions
EFER equ 0C0000080h

