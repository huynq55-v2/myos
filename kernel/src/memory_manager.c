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

    // Mark all blocks as allocated
    for (uint64_t i = 0; i < phys_allocator.total_blocks; i++) {
        phys_allocator.bitmap[i] = 0xFF;
    }

    // Iterate over all memmap entries
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        // Chỉ xử lý vùng bộ nhớ USABLE
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t start = entry->base;
            uint64_t length = entry->length;

            // Calculate the number of blocks
            uint64_t block_count = length / BLOCK_SIZE;
            // Calculate the start block index
            uint64_t start_block = start / BLOCK_SIZE;

            // Mark all blocks as free
            for (uint64_t b = 0; b < block_count; b++) {
                bitmap_free(&phys_allocator, start_block + b);
            }
        }
    }
}

/**
 * Allocates a single physical block.
 *
 * The function allocates a single physical block from the allocator and
 * returns its physical address. If the allocation fails, the function returns
 * 0.
 *
 * @return The physical address of the allocated block or 0 on failure.
 */
uint64_t allocate_physical_block() {
    uint64_t block_index = bitmap_alloc(&phys_allocator);
    if (block_index == (uint64_t)-1) {
        return 0; // Thất bại trong việc cấp phát
    }
    return block_index * BLOCK_SIZE; // Trả về địa chỉ vật lý
}

/**
 * Frees a single physical block.
 *
 * The function takes a physical address as parameter and frees the corresponding
 * block in the allocator. If the address is invalid (i.e. 0 or not aligned to
 * BLOCK_SIZE), the function does nothing.
 *
 * @param phys_address The physical address of the block to free.
 */
void free_physical_block(uint64_t phys_address) {
    if (phys_address == 0 || phys_address % BLOCK_SIZE != 0) {
        return; // Thông số không hợp lệ
    }

    // Calculate the block index
    uint64_t block_index = phys_address / BLOCK_SIZE;
    bitmap_free(&phys_allocator, block_index);
}

/*************  ✨ Codeium Command ⭐  *************/
/**
 * Checks if a physical block is allocated.
 *
 * The function takes a physical address as parameter and returns 1 if the
 * corresponding block is allocated, 0 if it is free. If the address is invalid
 * (i.e. 0 or not aligned to BLOCK_SIZE), the function returns 0.
 *
 * @param phys_address The physical address of the block to check.
 * @return 1 if the block is allocated, 0 if it is free.
 */
/******  fa3e3764-cfb0-452b-a948-e34c073ce7e5  *******/int is_block_allocated(uint64_t phys_address) {
    if (phys_address == 0 || phys_address % BLOCK_SIZE != 0) {
        return 0; // Thông số không hợp lệ
    }

    uint64_t block_index = phys_address / BLOCK_SIZE;
    return bitmap_is_allocated(&phys_allocator, block_index);
}

/**
 * Allocates multiple contiguous physical blocks.
 *
 * The function allocates `count` contiguous physical blocks from the allocator
 * and returns the physical address of the first block. If the allocation fails,
 * the function returns 0.
 *
 * @param count The number of blocks to allocate.
 * @return The physical address of the first allocated block or 0 on failure.
 */
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

/**
 * Frees a range of contiguous physical blocks.
 *
 * The function takes a physical address and a count of blocks as parameters.
 * It frees the range of blocks starting from the given physical address.
 * If the address is invalid (i.e. 0 or not aligned to BLOCK_SIZE) or the
 * count is invalid (i.e. 0), the function does nothing.
 *
 * @param phys_address The physical address of the first block to free.
 * @param count The number of blocks to free.
 */
void free_physical_blocks(uint64_t phys_address, uint64_t count) {
    if (phys_address == 0 || count == 0 || phys_address % BLOCK_SIZE != 0) {
        return; // Thông số không hợp lệ
    }

    uint64_t start_block = phys_address / BLOCK_SIZE;
    bitmap_free_contiguous(&phys_allocator, start_block, count);
}

/**
 * Checks if a range of contiguous physical blocks is allocated.
 *
 * The function takes a physical address and a count of blocks as parameters.
 * It returns 1 if all blocks in the range are allocated, 0 if any block is free.
 * If the address is invalid (i.e. 0 or not aligned to BLOCK_SIZE) or the
 * count is invalid (i.e. 0), the function returns 0.
 *
 * @param phys_address The physical address of the first block to check.
 * @param count The number of blocks to check.
 * @return 1 if all blocks are allocated, 0 if any block is free.
 */
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

/**
 * Allocates memory of the given size in bytes.
 *
 * The function takes a size in bytes as parameter and allocates the corresponding
 * number of physical blocks from the allocator. It returns the physical address of
 * the allocated memory or 0 on failure.
 *
 * If the size is invalid (i.e. 0), the function returns 0.
 *
 * @param size The size of the memory to allocate in bytes.
 * @return The physical address of the allocated memory or 0 on failure.
 */
uint64_t allocate_memory_bytes(uint64_t size) {
    if (size == 0) {
        return 0; // Kích thước không hợp lệ
    }

    // Calculate the number of pages needed
    uint64_t pages_needed = size / BLOCK_SIZE;
    if (size % BLOCK_SIZE != 0) {
        pages_needed++;
    }

    // Cấp phát các trang liên tiếp
    uint64_t phys_address = allocate_physical_blocks(pages_needed);
    if (phys_address == 0) {
        return 0; // Thất bại trong việc cấp phát
    }

    // Trả về địa chỉ vật lý của vùng nhớ đã cấp phát
    return phys_address;
}

/**
 * Frees memory of the given size in bytes.
 *
 * The function takes a physical address and a size in bytes as parameters.
 * It calculates the number of physical blocks to free and calls
 * free_physical_blocks() to free the blocks.
 *
 * If the parameters are invalid (i.e. 0 or not aligned to BLOCK_SIZE), the
 * function does nothing.
 *
 * @param phys_address The physical address of the memory to free.
 * @param size The size of the memory to free in bytes.
 */
void free_memory_bytes(uint64_t phys_address, uint64_t size) {
    if (phys_address == 0 || size == 0 || phys_address % BLOCK_SIZE != 0) {
        return; // Thông số không hợp lệ
    }

    // Calculate the number of pages to free
    uint64_t pages_to_free = size / BLOCK_SIZE;
    if (size % BLOCK_SIZE != 0) {
        pages_to_free++;
    }

    // Giải phóng các trang
    free_physical_blocks(phys_address, pages_to_free);
}
