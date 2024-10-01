// elf.h
#ifndef ELF_H
#define ELF_H

#include <stdint.h>

// Các định nghĩa liên quan đến ELF
#define ELFMAG      "\177ELF"
#define SELFMAG     4
#define EI_CLASS    4
#define EI_DATA     5
#define ELFCLASS64  2
#define ELFDATA2LSB 1

// ELF Header
typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) Elf64_Ehdr;

// Program Header
typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} __attribute__((packed)) Elf64_Phdr;

// Constants
#define PT_LOAD 1

#endif // ELF_H
