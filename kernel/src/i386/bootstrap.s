.section .init
.globl main

.globl _start
_start:
  pop %ebx
  pop %ebx
  movl $0x9FB00, %esp
  cli
  push %ebx
  call main
halt:
  jmp halt;

.set FLAGS,    0x3
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

