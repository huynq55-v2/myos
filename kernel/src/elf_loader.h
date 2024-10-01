// elf_loader.h
#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include "process.h"

// Load an ELF binary into the given process's address space
bool load_elf(const uint8_t *elf_data, size_t size, process_t *proc);

#endif // ELF_LOADER_H
