// elf_loader.c
#include "elf_loader.h"
#include "paging.h"
#include "memory_manager.h"
#include "klibc.h"
#include "graphics.h"

#include "bitmap_allocator.h"
#include "config.h"

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

uint64_t elf_load(uint64_t page_table_phys, uint8_t *elf_start, uint8_t *elf_end) {
    Elf64_Ehdr *ehdr = (Elf64_Ehdr*)elf_start;

    // Kiểm tra magic number
    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        kprintf("ELF Loader: Invalid ELF magic\n");
        return NULL;
    }

    uint64_t entry_point = ehdr->e_entry;

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
                return NULL;
            }

            // Ánh xạ trang vào không gian địa chỉ của tiến trình
            if (!map_memory(page_table_phys, vaddr, phys_addr, memsz, PAGING_PAGE_PRESENT | PAGING_PAGE_RW | PAGING_PAGE_USER)) {
                kprintf("ELF Loader: Failed to map memory for segment\n");
                free_memory_bytes(phys_addr, memsz);
                return NULL;
            }

            // Sao chép dữ liệu từ ELF vào bộ nhớ
            memcpy(PHYS_TO_VIRT(phys_addr), elf_start + offset, filesz);

            // Nếu p_memsz > p_filesz, cần zero phần còn lại
            if (memsz > filesz) {
                memset(PHYS_TO_VIRT(phys_addr + filesz), 0, memsz - filesz);
            }

            // Cập nhật entry point nếu cần
            if (i == 0) { // Giả sử segment đầu tiên chứa entry point
                // proc->context.rip = ehdr->e_entry; // Được thiết lập trong process_create
            }
        }
    }

    return entry_point;
}
