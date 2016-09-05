[BITS 16]

[SECTION .text]

[ORG 07C00h]

        jmp     short   start
        nop
bsOemName               DB      "BootProg"      ; 0x03

bpbBytesPerSector       DW      0               ; 0x0B
bpbSectorsPerCluster    DB      0               ; 0x0D
bpbReservedSectors      DW      0               ; 0x0E
bpbNumberOfFATs         DB      0               ; 0x10
bpbRootEntries          DW      0               ; 0x11
bpbTotalSectors         DW      0               ; 0x13
bpbMedia                DB      0               ; 0x15
bpbSectorsPerFAT        DW      0               ; 0x16
bpbSectorsPerTrack      DW      0               ; 0x18
bpbHeadsPerCylinder     DW      0               ; 0x1A
bpbHiddenSectors        DD      0               ; 0x1C
bpbTotalSectorsBig      DD      0               ; 0x20
bsSectorsPerFAT32               DD      0               ; 0x24
bsExtendedFlags                 DW      0               ; 0x28
bsFSVersion                     DW      0               ; 0x2A
bsRootDirectoryClusterNo        DD      0               ; 0x2C
bsFSInfoSectorNo                DW      0               ; 0x30
bsBackupBootSectorNo            DW      0               ; 0x32
bsreserved             times 12 DB      0               ; 0x34
bsDriveNumber                   DB      0               ; 0x40
bsreserved1                     DB      0               ; 0x41
bsExtendedBootSignature         DB      0               ; 0x42
bsVolumeSerialNumber            DD      0               ; 0x43
bsVolumeLabel                   DB      "RODVIN BOOT"   ; 0x47
bsFileSystemName                DB      "FAT32   "      ; 0x52

start:
        cld

	mov	ax, 09F00h
	mov	ss, ax
	mov	sp, 2048
	xor     ax, ax
	mov	es, ax
	mov	ds, ax

        push    es
        push    bs_entry
        retf

bs_entry:
        mov     [bsDriveNumber], dl     ; store boot drive number

        mov     bp, ProgramName         ; ds:si -> program name
	push	word 07E0h
	pop	es
	xor	bx, bx
	call	ReadFile
        jmp     setup


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Reads a file                             ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Input:  DS:BP    = filename              ;;
;;         ES:BX -> buffer address          ;;
;; Output: CF = 1 if error                  ;;
;;         EAX = filesize                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ReadFile:

RootDirReadContinue:
        and     byte [bsRootDirectoryClusterNo+3], 0Fh ; mask cluster value				may be removed
        mov     esi, [bsRootDirectoryClusterNo] ; esi=cluster # of root dir

	push	es
	push	bx

        push    word 9000h
        pop     es
        xor     bx, bx
	push	bp
        call    ReadCluster             ; read one cluster of root dir
	pop	bp

        push    esi                     ; save esi=next cluster # of root dir
        pushf                           ; save carry="not last cluster" flag

        push    word 9000h
        pop     es
        xor     di, di                  ; es:di -> root entries array
        mov     si, bp

        mov     al, [bpbSectorsPerCluster]
        cbw
        mul     word [bpbBytesPerSector]; ax = bytes per cluster
        shr     ax, 5
        mov     dx, ax                  ; dx = # of dir entries to search in

FindName:
        mov     cx, 11
FindNameCycle:
        cmp     byte [es:di], ch
        je      ErrFind                 ; end of root directory (NULL entry found)
FindNameNotEnd:
        pusha
        repe    cmpsb
        popa
        je      FindNameFound
        add     di, 32
        dec     dx
        jnz     FindNameCycle           ; next root entry
        popf                            ; restore carry="not last cluster" flag
        pop     esi                     ; restore esi=next cluster # of root dir
        jc      RootDirReadContinue     ; continue to the next root dir cluster
        jmp     short ErrFind                 ; end of root directory (dir end reached)
FindNameFound:
        popf
        pop     esi
        push    word [es:di+14h]
        push    word [es:di+1Ah]
        pop     esi                     ; si = cluster no.
        mov     edi, [es:di+1Ch]

	pop	bx
	pop	es
        push    edi
FileReadContinue:
        call    ReadCluster             ; read one cluster of root dir
        jc      FileReadContinue

        pop     eax
	ret

ErrRead:
ErrFind:
        mov     ax, 0E00h+'E'
        mov     bx, 7
        int     10h
        jmp     short $

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Reads a sector using BIOS Int 13h fn 42h ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Input:  EAX = LBA                        ;;
;;         CX    = sector count             ;;
;;         ES:BX -> buffer address          ;;
;; Output: CF = 1 if error                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ReadSectorLBA:
        pushad

ReadSectorLBANext:
        pusha

        push    byte 0
        push    byte 0
        push    eax
        push    es
        push    bx
        push    byte 1
        push    byte 16

        mov     ah, 42h
        mov     dl, [bsDriveNumber]
        mov     si, sp
        push    ss
        pop     ds
        int     13h
        push    cs
        pop     ds

        jc      short ErrRead

        add     sp, 16

        popa
        dec     cx
        jz      ReadSectorLBADone2      ; last sector

        add     bx, [bpbBytesPerSector] ; adjust offset for next sector
        add     eax, byte 1             ; adjust LBA for next sector
        jmp     short ReadSectorLBANext

ReadSectorLBADone2:
        popad
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Reads a FAT32 cluster        ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Inout:  ES:BX -> buffer      ;;
;;           ESI = cluster no   ;;
;; Output:   ESI = next cluster ;;
;;         ES:BX -> next addr   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

ReadCluster:
        mov     ax, [bpbBytesPerSector]
        shr     ax, 2                           ; ax=# of FAT32 entries per sector
        cwde
        mov     ebp, esi                        ; ebp=esi=cluster #
        xchg    eax, esi
        cdq
        div     esi                             ; eax=FAT sector #, edx=entry # in sector
        movzx   edi, word [bpbReservedSectors]
        add     edi, [bpbHiddenSectors]
        add     eax, edi

        push    dx                              ; save dx=entry # in sector on stack
        mov     cx, 1
        call    ReadSectorLBA                   ; read 1 FAT32 sector

        pop     si                              ; si=entry # in sector
        add     si, si
        add     si, si
        and     byte [es:si+3], 0Fh             ; mask cluster value
        mov     esi, [es:si]                    ; esi=next cluster #

        lea     eax, [ebp-2]
        movzx   ecx, byte [bpbSectorsPerCluster]
        mul     ecx
        mov     ebp, eax

        movzx   eax, byte [bpbNumberOfFATs]
        mul     dword [bsSectorsPerFAT32]

        add     eax, ebp
        add     eax, edi

        call    ReadSectorLBA

        mov     ax, [bpbBytesPerSector]
        shr     ax, 4                   ; ax = paragraphs per sector
        mul     cx                      ; ax = paragraphs read

        mov     cx, es
        add     cx, ax
        mov     es, cx                  ; es:bx updated

        cmp     esi, 0FFFFFF8h          ; carry=0 if last cluster, and carry=1 otherwise
        ret

                times (512-13-($-$$)) db 0

ProgramName     db      "BOOTLDR BIN"

                dw      0AA55h
