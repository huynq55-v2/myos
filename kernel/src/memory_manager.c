#include "memory_manager.h"
#include "bitmap_allocator.h"
#include <limine.h>

// Yêu cầu MEMMAP từ Limine
extern volatile struct limine_memmap_request memmap_request;

// Biến toàn cục cho allocator
bitmap_allocator_t phys_allocator;

// Vùng nhớ cho bitmap (tùy chỉnh kích thước phù hợp với tổng bộ nhớ)
#define BITMAP_MEMORY_SIZE  0x10000 // 64KB cho bitmap (quản lý đến ~2GB bộ nhớ)

uint8_t bitmap_memory[BITMAP_MEMORY_SIZE] __attribute__((aligned(4096)));


void memory_manager_init() {
    // Chờ Limine cung cấp phản hồi về MEMMAP
    while (!memmap_request.response);

    // Khởi tạo bitmap allocator
    bitmap_allocator_init(&phys_allocator, bitmap_memory, BITMAP_MEMORY_SIZE);

    // Đánh dấu tất cả các khối là đã được cấp phát
    for (uint64_t i = 0; i < phys_allocator.total_blocks / 8; i++) {
        phys_allocator.bitmap[i] = 0xFF;
    }

    // Lặp qua các entry của MEMMAP
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        // Chỉ xử lý vùng bộ nhớ USABLE
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t start = entry->base;
            uint64_t length = entry->length;

            // Chuyển đổi địa chỉ thành khối
            uint64_t start_block = start / BLOCK_SIZE;
            uint64_t block_count = length / BLOCK_SIZE;

            // Đánh dấu các khối là trống (có thể sử dụng)
            for (uint64_t b = 0; b < block_count; b++) {
                bitmap_free(&phys_allocator, start_block + b);
            }
        }
    }
}

// Hàm cấp phát một khối bộ nhớ vật lý
uint64_t allocate_physical_block() {
    uint64_t block_index = bitmap_alloc(&phys_allocator);
    if (block_index == (uint64_t)-1) {
        return 0; // Thất bại trong việc cấp phát
    }
    return block_index * BLOCK_SIZE; // Trả về địa chỉ vật lý
}

// Hàm giải phóng một khối bộ nhớ vật lý
void free_physical_block(uint64_t phys_address) {
    if (phys_address == 0 || phys_address % BLOCK_SIZE != 0) {
        return; // Thông số không hợp lệ
    }

    uint64_t block_index = phys_address / BLOCK_SIZE;
    bitmap_free(&phys_allocator, block_index);
}

// Hàm kiểm tra trạng thái của một khối
int is_block_allocated(uint64_t phys_address) {
    if (phys_address == 0 || phys_address % BLOCK_SIZE != 0) {
        return 0; // Thông số không hợp lệ
    }

    uint64_t block_index = phys_address / BLOCK_SIZE;
    return bitmap_is_allocated(&phys_allocator, block_index);
}

// Hàm cấp phát nhiều khối bộ nhớ vật lý liên tiếp
uint64_t allocate_physical_blocks(uint64_t count) {
    if (count == 0) {
        return 0; // Số lượng không hợp lệ
    }

    uint64_t start_block = bitmap_alloc_contiguous(&phys_allocator, count);
    if (start_block == (uint64_t)-1) {
        return 0; // Thất bại trong việc cấp phát
    }
    return start_block * BLOCK_SIZE; // Trả về địa chỉ vật lý của khối đầu tiên
}

// Hàm giải phóng nhiều khối bộ nhớ vật lý liên tiếp
void free_physical_blocks(uint64_t phys_address, uint64_t count) {
    if (phys_address == 0 || count == 0 || phys_address % BLOCK_SIZE != 0) {
        return; // Thông số không hợp lệ
    }

    uint64_t start_block = phys_address / BLOCK_SIZE;
    bitmap_free_contiguous(&phys_allocator, start_block, count);
}

// Hàm kiểm tra xem một dãy các khối đã được cấp phát hay chưa
int are_blocks_allocated(uint64_t phys_address, uint64_t count) {
    if (count == 0 || phys_address == 0 || phys_address % BLOCK_SIZE != 0) {
        return 0;
    }

    uint64_t start_block = phys_address / BLOCK_SIZE;
    if ((start_block + count) > phys_allocator.total_blocks) {
        return 0;
    }

    for (uint64_t i = start_block; i < start_block + count; i++) {
        if (!bitmap_is_allocated(&phys_allocator, i)) {
            return 0;
        }
    }
    return 1;
}

// **Hàm mới: Cấp phát bộ nhớ theo kích thước byte**
uint64_t allocate_memory_bytes(uint64_t size) {
    if (size == 0) {
        return 0; // Kích thước không hợp lệ
    }

    // Tính toán số trang cần cấp phát (làm tròn lên)
    uint64_t pages_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    // Cấp phát các trang liên tiếp
    uint64_t phys_address = allocate_physical_blocks(pages_needed);
    if (phys_address == 0) {
        return 0; // Thất bại trong việc cấp phát
    }

    // Trả về địa chỉ vật lý của vùng nhớ đã cấp phát
    return phys_address;
}

// **Hàm mới: Giải phóng bộ nhớ theo địa chỉ và kích thước byte**
void free_memory_bytes(uint64_t phys_address, uint64_t size) {
    if (phys_address == 0 || size == 0 || phys_address % BLOCK_SIZE != 0) {
        return; // Thông số không hợp lệ
    }

    // Tính toán số trang cần giải phóng (làm tròn lên)
    uint64_t pages_to_free = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    // Giải phóng các trang
    free_physical_blocks(phys_address, pages_to_free);
}
