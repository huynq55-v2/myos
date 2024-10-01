// paging.h

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Tạo bảng trang cho tiến trình người dùng
void* create_user_page_table();

// Hủy bảng trang
void destroy_page_table(void *page_table);

// Ánh xạ địa chỉ ảo tới địa chỉ vật lý
bool map_memory(void *page_table, uintptr_t virtual_addr, size_t size, uint64_t flags);

#endif // PAGING_H
