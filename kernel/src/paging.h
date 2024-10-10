// paging.h

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Định nghĩa cờ của trang bộ nhớ
#define PAGING_PAGE_PRESENT        0x001  // Trang có mặt trong bộ nhớ
#define PAGING_PAGE_RW             0x002  // Quyền ghi được phép (Read/Write)
#define PAGING_PAGE_USER           0x004  // Quyền truy cập của người dùng (User mode)
#define PAGING_PAGE_WRITE_THROUGH  0x008  // Kích hoạt chế độ ghi qua (Write-Through)
#define PAGING_PAGE_CACHE_DISABLE  0x010  // Vô hiệu hóa cache
#define PAGING_PAGE_ACCESSED       0x020  // Đã được truy cập (Accessed)
#define PAGING_PAGE_DIRTY          0x040  // Trang đã bị thay đổi (Dirty)
#define PAGING_PAGE_HUGE           0x080  // Sử dụng trang lớn (Huge page)
#define PAGING_PAGE_GLOBAL         0x100  // Trang toàn cục (Global page)

// create user page table
void* create_user_page_table();

// Ánh xạ địa chỉ ảo tới địa chỉ vật lý
bool map_memory(uintptr_t pml4_phys, uint64_t virt_addr, uint64_t phys_addr, uint64_t size, uint64_t flags);

// Hàm chuyển đổi page table
void switch_page_table(void *page_table);

#endif // PAGING_H
