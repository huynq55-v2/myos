#ifndef SLAB_ALLOCATOR_H
#define SLAB_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

// Kích thước trang (4KB)
#define PAGE_SIZE 4096

// Cấu trúc cho một khối bộ nhớ tự do (free memory block)
typedef struct free_block {
    size_t size;
    struct free_block* next;
    int is_free;
} free_block_t;

// Các hàm khởi tạo và quản lý bộ nhớ
void init_memory_allocator();
void* slab_malloc(size_t size);
void slab_free(void* ptr);
void* slab_realloc(void* ptr, size_t size);
size_t slab_total_free_memory(); // Hàm tính tổng bộ nhớ còn lại

size_t slab_allocated_size(void* ptr);

// Hàm kiểm tra bộ phân bổ bộ nhớ
void test_allocator();

#endif // SLAB_ALLOCATOR_H
