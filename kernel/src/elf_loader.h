// elf_loader.h
#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <stdint.h>
#include <stdbool.h>

// Hàm tải ELF vào không gian địa chỉ của tiến trình
bool elf_load(uint64_t page_table_phys, uint8_t *elf_start, uint8_t *elf_end);

#endif // ELF_LOADER_H
