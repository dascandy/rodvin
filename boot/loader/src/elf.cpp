#include <stdint.h>
#include <stddef.h>
#include "paging.h"
#include "elf.h"
#include "boot.h"

struct kernel_boot_header *bootheader = (struct kernel_boot_header *)0;

void setup_die(const char *why) {
  char *vidmem = (char *)0xB8000;
  while(*why != 0) {
    *vidmem++ = *why++;
    *vidmem++ = 7;
  }
  while(1)
    asm("cli; hlt");
}

extern "C" void _start(void *kernel, size_t filesize) __attribute__((section(".start")));
extern "C" void _start(void *kernel, size_t filesize) {
  // TODO: use file size too
  (void)filesize;
  struct elf_header *head = (struct elf_header *)kernel;

  if (elf_check(head)) {
    elf_load(head);

    head->e_entry((void*)bootheader->e820_map);
    setup_die("Entrypoint returned! You can't do that!");
  } else {
    setup_die("Executable of invalid type");
  }
}

void elf_load(struct elf_header *header) {
  struct elf_program_header *phdrs = (struct elf_program_header *)((unsigned long)header + header->e_phoff);
  struct elf_program_header *entry = phdrs;

  for (int i=0; i<header->e_phnum; i++) {
    // map pages to the target location
    for (unsigned long j=0; j<(((entry->p_memsz + 4096 - 1) >> 12)); j++) {
      map_page_to(((unsigned long)entry->p_vaddr + (j << 12)));
    }
    // copy the content there
    char *src = (char *)(entry->p_offset + (unsigned long)header);
    char *dst = entry->p_vaddr;
    unsigned long bytecount = entry->p_filesz;
    while (bytecount > 0) {
      bytecount--;
      dst[bytecount] = src[bytecount];
    }
    dst += entry->p_filesz;
    bytecount = entry->p_memsz - entry->p_filesz;
    while (bytecount > 0) {
      bytecount--;
      dst[bytecount] = 0;
    }
    entry = (struct elf_program_header *)((unsigned long)entry + header->e_phentsize);
  }
}

int elf_check(struct elf_header *header) {
  if (header->e_type != 2) return 0;
  if (header->e_version != 1) return 0;
#if defined(__arm__)
  if (header->e_machine != 0x28) return 0;
#elif defined(__x86_64__)
  if (header->e_machine != 0x3E) return 0;
#else
  if (header->e_machine != 0x3) return 0;
#endif
  return 1;
}

