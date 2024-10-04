// paging.h

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// create user page table
void* create_user_page_table();

// Ánh xạ địa chỉ ảo tới địa chỉ vật lý
bool map_memory(uintptr_t pml4_phys, uint64_t virt_addr, uint64_t phys_addr, uint64_t size, uint64_t flags);

// Hàm chuyển đổi page table
void switch_page_table(void *page_table);

#endif // PAGING_H
