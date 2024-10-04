// elf_loader.c
#include "elf_loader.h"
#include "paging.h"
#include "memory_manager.h"
#include "klibc.h"
#include "graphics.h"

// Cấu trúc ELF header cho 64-bit
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

// Cấu trúc Program header cho 64-bit
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

// Định nghĩa các loại segment
#define PT_LOAD 1

bool elf_load(uint64_t page_table_phys, uint8_t *elf_start, uint8_t *elf_end) {
    Elf64_Ehdr *ehdr = (Elf64_Ehdr*)elf_start;

    // Kiểm tra magic number
    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        kprintf("ELF Loader: Invalid ELF magic\n");
        return false;
    }

    // Kiểm tra phiên bản
    if (ehdr->e_ident[4] != 1) { // EV_CURRENT
        kprintf("ELF Loader: Unsupported ELF version\n");
        return false;
    }

    // Kiểm tra loại
    if (ehdr->e_type != 2) { // ET_EXEC
        kprintf("ELF Loader: Unsupported ELF type\n");
        return false;
    }

    // Lấy program headers
    Elf64_Phdr *phdr = (Elf64_Phdr*)(elf_start + ehdr->e_phoff);

    // Lặp qua các program headers
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            uint64_t vaddr = phdr[i].p_vaddr;
            uint64_t memsz = phdr[i].p_memsz;
            uint64_t filesz = phdr[i].p_filesz;
            uint64_t offset = phdr[i].p_offset;

            // Cấp phát trang bộ nhớ cho segment
            uint64_t phys_addr = allocate_memory_bytes(memsz);
            if (!phys_addr) {
                kprintf("ELF Loader: Failed to allocate memory for segment\n");
                return false;
            }

            // Ánh xạ trang vào không gian địa chỉ của tiến trình
            if (!map_memory(page_table_phys, vaddr, phys_addr, memsz, PAGING_PAGE_PRESENT | PAGING_PAGE_RW | PAGING_PAGE_USER)) {
                kprintf("ELF Loader: Failed to map memory for segment\n");
                free_memory_bytes(phys_addr, memsz);
                return false;
            }

            // Sao chép dữ liệu từ ELF vào bộ nhớ
            memcpy((void*)vaddr, elf_start + offset, filesz);

            // Nếu p_memsz > p_filesz, cần zero phần còn lại
            if (memsz > filesz) {
                memset((void*)(vaddr + filesz), 0, memsz - filesz);
            }
        }
    }

    return true;
}
