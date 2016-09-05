.section .init
.globl main

.globl _start
_start:
  movq $0x9FB00, %rsp
  cli

  call main
halt:
  jmp halt;
