.section .init
.globl main

.globl _start
_start:
  mov sp, #0x8000
 
  ldr r4, =__bss_start
  ldr r9, =__bss_end

  # enable FPU in coprocessor enable register
  ldr r0, =(0xF << 20)
  mcr p15, 0, r0, c1, c0, 2
  # enable FPU in FP exception register
  MOV r3, #0x40000000
#  VMSR FPEXC, r3 - assembler bug
  .long 0xeee83a10

  mov r5, #0
  mov r6, #0
  mov r7, #0
  mov r8, #0
  b       2f
 
1:
  stmia r4!, {r5-r8}
 
2:
  cmp r4, r9
  blo 1b
 
  bl main
 
halt:
  wfe
  b halt

