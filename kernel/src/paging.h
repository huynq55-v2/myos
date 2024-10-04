// paging.h

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Định nghĩa các chỉ số phân trang
#define PML4_INDEX(x) (((x) >> 39) & 0x1FF)
#define PDPT_INDEX(x) (((x) >> 30) & 0x1FF)
#define PD_INDEX(x)   (((x) >> 21) & 0x1FF)
#define PT_INDEX(x)   (((x) >> 12) & 0x1FF)

#define PAGE_READ  0x1
#define PAGE_WRITE 0x2
#define PAGE_EXEC  0x4

// Tạo bảng trang cho tiến trình người dùng
void* create_user_page_table();

// Hủy bảng trang
void destroy_page_table(void *page_table);

// Ánh xạ địa chỉ ảo tới địa chỉ vật lý
bool map_memory(uint64_t *pml4, uint64_t virt_addr, uint64_t phys_addr, uint64_t size, uint64_t flags);

uintptr_t get_physical_address(uint64_t entry);

#endif // PAGING_H
