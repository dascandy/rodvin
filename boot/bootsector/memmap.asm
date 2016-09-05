

load_memmap:
  mov di, 0A0h
  mov es, di

  xor edi, edi
  mov ecx, 384 ; 64 entries, plus one additional field
  xor eax, eax
  rep stosd

  xor di, di
  xor ebx, ebx
  mov edx, 0534D4150h

.next_entry:
  mov dword [es:di + 20], 1        ; default value for last param
  mov eax, 0E820h
  mov ecx, 24
  int 015h
  jc .out
  cmp ebx, 0
  je .out

  add di, 24
  jmp .next_entry

.out:
  ret
