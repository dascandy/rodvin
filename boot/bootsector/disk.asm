
; int disk_readsector(uint64_t sector, uint16_t offs, seg_t segment, uint16_t count)
disk_readsector:
  push bp
  mov bp, sp
  push bx
  push cx
  push dx
  push si
  push es

.retry:
  mov dl, byte [disk.drive_no]
  mov si, disk_buffer
  mov eax, dword [bp+4]
  mov dword [disk_buffer.blockno], eax
  mov eax, dword [bp+8]
  mov dword [disk_buffer.blockno + 4], eax
  mov eax, dword [bp+12]
  mov dword [disk_buffer.buffer], eax
  mov ax, word [bp+16]
  cmp ax, 080h
  jl .no_sub
  mov ax, 7fh
.no_sub:
  mov word [disk_buffer.count], ax
  mov ah, 42h
  int 13h
  mov ax, [disk_buffer.count]
  sub [bp+16], ax
  jc .err
  cmp word [bp+16], 0
  jne .retry
.done:
  pop es
  pop si
  pop dx
  pop cx
  pop bx
  pop bp
  ret
.err:
  stc
  jmp .done

disk.drive_no:
  db 0

disk_buffer
.size    db 10h, 00
.count    dw 3Fh
.buffer    dd 0
.blockno  dd 1, 0


