// elf_loader.c
#include "elf.h"
#include "buddy.h"
#include "klibc.h"
#include "process.h"
#include "graphics.h"

// Function to load an ELF binary into memory
bool load_elf(const uint8_t *elf_data, size_t size, process_t *proc) {
    if (size < sizeof(Elf64_Ehdr)) {
        kprintf("ELF: File too small\n");
        return false;
    }

    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_data;

    // Validate ELF magic number
    if (ehdr->e_ident[0] != 0x7F || 
        ehdr->e_ident[1] != 'E' || 
        ehdr->e_ident[2] != 'L' || 
        ehdr->e_ident[3] != 'F') {
        kprintf("ELF: Invalid magic number\n");
        return false;
    }

    // Only support 64-bit ELF
    if (ehdr->e_ident[4] != 2) {
        kprintf("ELF: Unsupported ELF class\n");
        return false;
    }

    // Only support little endian
    if (ehdr->e_ident[5] != 1) {
        kprintf("ELF: Unsupported endianness\n");
        return false;
    }

    // Locate program headers
    Elf64_Phdr *phdr = (Elf64_Phdr *)(elf_data + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)
            continue;

        // Allocate memory for the segment
        void *segment = buddy_alloc(phdr[i].p_memsz);
        if (!segment) {
            kprintf("ELF: Failed to allocate memory for segment\n");
            return false;
        }

        // Copy segment data from ELF file
        memcpy(segment, elf_data + phdr[i].p_offset, phdr[i].p_filesz);

        // Zero out the remaining memory if p_memsz > p_filesz
        if (phdr[i].p_memsz > phdr[i].p_filesz) {
            memset(segment + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
        }

        // Update process's memory map
        proc->entry_point = ehdr->e_entry;
        // Additional mappings can be handled here (e.g., virtual memory setup)
    }

    return true;
}
