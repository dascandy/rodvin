#ifndef ELF_H
#define ELF_H

typedef unsigned short int Elf32_Half;
typedef unsigned int Elf32_Word;
typedef unsigned long Elf32_DWord;
typedef void (*Elf32_Entry_Point)(void *);
typedef char *Elf32_Addr;
typedef unsigned long Elf32_Off;

struct elf_header {
  unsigned char e_ident[16];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  Elf32_Entry_Point e_entry;
  Elf32_Off e_phoff;
  Elf32_Off e_shoff;
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
};

#ifdef __x86_64__
struct elf_program_header {
  Elf32_Word p_type;
  Elf32_Word p_flags;
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_DWord p_filesz;
  Elf32_DWord p_memsz;
  Elf32_DWord p_align;
};
#else
struct elf_program_header {
  Elf32_Word p_type;
  Elf32_Off p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_DWord p_filesz;
  Elf32_DWord p_memsz;
  Elf32_Word p_flags;
  Elf32_DWord p_align;
};
#endif
 
void elf_load(struct elf_header *head);
int elf_check(struct elf_header *head);

#endif

