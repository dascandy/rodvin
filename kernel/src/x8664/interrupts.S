.globl _Z11onInterruptv

.globl _Z17x8664_oninterruptv
_Z17x8664_oninterruptv:
  push %rsi
  push %rdx
  push %rdi
  push %rcx
  push %rbx
  push %rbp
  push %rax
  push %r9
  push %r8
  push %r15
  push %r14
  push %r13
  push %r12
  push %r11
  push %r10
  call _Z11onInterruptv
  call _Z20x8664_endofinterruptv
  pop %r10
  pop %r11
  pop %r12
  pop %r13
  pop %r14
  pop %r15
  pop %r8
  pop %r9
  pop %rax
  pop %rbp
  pop %rbx
  pop %rcx
  pop %rdi
  pop %rdx
  pop %rsi
  iretq


